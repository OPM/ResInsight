/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'bootstrap_enkf.c' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <ert/util/int_vector.h>
#include <ert/util/util.h>
#include <ert/util/rng.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>

#include <ert/analysis/std_enkf.h>
#include <ert/analysis/cv_enkf.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/analysis_module.h>
#include <ert/analysis/enkf_linalg.h>


#define BOOTSTRAP_ENKF_TYPE_ID 741223

#define INVALID_SUBSPACE_DIMENSION  -1
#define INVALID_TRUNCATION          -1
#define DEFAULT_TRUNCATION          0.95
#define DEFAULT_NCOMP               INVALID_SUBSPACE_DIMENSION

#define  DEFAULT_DO_CV               false
#define  DEFAULT_NFOLDS              10
#define  NFOLDS_KEY                  "BOOTSTRAP_NFOLDS"


typedef struct {
  UTIL_TYPE_ID_DECLARATION;
  std_enkf_data_type   * std_enkf_data;
  cv_enkf_data_type    * cv_enkf_data;
  rng_type             * rng;
  long                   option_flags;
  bool                   doCV;
} bootstrap_enkf_data_type;


static UTIL_SAFE_CAST_FUNCTION( bootstrap_enkf_data , BOOTSTRAP_ENKF_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION_CONST( bootstrap_enkf_data , BOOTSTRAP_ENKF_TYPE_ID )


void bootstrap_enkf_set_doCV( bootstrap_enkf_data_type * data , bool doCV) {
  data->doCV = doCV;
}



void bootstrap_enkf_set_truncation( bootstrap_enkf_data_type * boot_data , double truncation ) {
  std_enkf_set_truncation( boot_data->std_enkf_data , truncation );
  cv_enkf_set_truncation( boot_data->cv_enkf_data , truncation );
}


void bootstrap_enkf_set_subspace_dimension( bootstrap_enkf_data_type * boot_data , int ncomp) {
  std_enkf_set_subspace_dimension( boot_data->std_enkf_data , ncomp );
  cv_enkf_set_subspace_dimension( boot_data->cv_enkf_data , ncomp );
}


void * bootstrap_enkf_data_alloc( rng_type * rng ) {
  bootstrap_enkf_data_type * boot_data = util_malloc( sizeof * boot_data );
  UTIL_TYPE_ID_INIT( boot_data , BOOTSTRAP_ENKF_TYPE_ID );

  boot_data->std_enkf_data = std_enkf_data_alloc( NULL );
  boot_data->cv_enkf_data = cv_enkf_data_alloc( rng );

  boot_data->rng = rng;
  bootstrap_enkf_set_truncation( boot_data , DEFAULT_TRUNCATION );
  bootstrap_enkf_set_subspace_dimension( boot_data , DEFAULT_NCOMP );
  bootstrap_enkf_set_doCV( boot_data , DEFAULT_DO_CV);
  boot_data->option_flags = ANALYSIS_NEED_ED + ANALYSIS_UPDATE_A + ANALYSIS_SCALE_DATA;
  return boot_data;
}





void bootstrap_enkf_data_free( void * arg ) {
  bootstrap_enkf_data_type * boot_data = bootstrap_enkf_data_safe_cast( arg );
  {
    std_enkf_data_free( boot_data->std_enkf_data );
    cv_enkf_data_free( boot_data->cv_enkf_data );
  }
  free( boot_data );
}


static int ** alloc_iens_resample( rng_type * rng , int ens_size ) {
  int ** iens_resample;
  int iens;

  iens_resample = util_calloc( ens_size , sizeof * iens_resample );
  for (iens = 0; iens < ens_size; iens++)
    iens_resample[iens] = util_calloc( ens_size , sizeof( ** iens_resample ) );

  {
    int i,j;
    for (i=0; i < ens_size; i++)
      for (j=0; j < ens_size; j++)
        iens_resample[i][j] = rng_get_int( rng , ens_size );
  }
  return iens_resample;
}


static void free_iens_resample( int ** iens_resample, int ens_size ) {
  for (int i=0; i < ens_size; i++)
    free( iens_resample[i] );
  free( iens_resample );
}



void bootstrap_enkf_updateA(void * module_data ,
                            matrix_type * A ,
                            matrix_type * S ,
                            matrix_type * R ,
                            matrix_type * dObs ,
                            matrix_type * E ,
                            matrix_type * D ,
                            const module_info_type* module_info) {

  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( module_data );
  {
    const int num_cpu_threads = 4;
    int ens_size              = matrix_get_columns( A );
    matrix_type * X           = matrix_alloc( ens_size , ens_size );
    matrix_type * A0          = matrix_alloc_copy( A );
    matrix_type * S_resampled = matrix_alloc_copy( S );
    matrix_type * A_resampled = matrix_alloc( matrix_get_rows(A0) , matrix_get_columns( A0 ));
    int ** iens_resample      = alloc_iens_resample( bootstrap_data->rng , ens_size );
    {
      int ensemble_members_loop;
      for ( ensemble_members_loop = 0; ensemble_members_loop < ens_size; ensemble_members_loop++) {
        int unique_bootstrap_components;
        int ensemble_counter;
        /* Resample A and meas_data. Here we are careful to resample the working copy.*/
        {
          {
            int_vector_type * bootstrap_components = int_vector_alloc( ens_size , 0);
            for (ensemble_counter  = 0; ensemble_counter < ens_size; ensemble_counter++) {
              int random_column = iens_resample[ ensemble_members_loop][ensemble_counter];
              int_vector_iset( bootstrap_components , ensemble_counter , random_column );
              matrix_copy_column( A_resampled , A0 , ensemble_counter , random_column );
              matrix_copy_column( S_resampled , S  , ensemble_counter , random_column );
            }
            int_vector_select_unique( bootstrap_components );
            unique_bootstrap_components = int_vector_size( bootstrap_components );
            int_vector_free( bootstrap_components );
          }

          if (bootstrap_data->doCV) {
            const bool_vector_type * ens_mask = NULL;
            cv_enkf_init_update( bootstrap_data->cv_enkf_data , ens_mask , S_resampled , R , dObs , E , D);
            cv_enkf_initX( bootstrap_data->cv_enkf_data , X , A_resampled , S_resampled , R , dObs , E , D);
          } else
            std_enkf_initX(bootstrap_data->std_enkf_data , X , NULL , S_resampled,R, dObs, E,D );


          matrix_inplace_matmul_mt1( A_resampled , X , num_cpu_threads );
          matrix_inplace_add( A_resampled , A0 );
          matrix_copy_column( A , A_resampled, ensemble_members_loop, ensemble_members_loop);

        }
      }
    }


    free_iens_resample( iens_resample , ens_size);
    matrix_free( X );
    matrix_free( S_resampled );
    matrix_free( A_resampled );
    matrix_free( A0 );
  }
}






long bootstrap_enkf_get_options( void * arg , long flag) {
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( arg );
  {
    return bootstrap_data->option_flags;
  }
}


bool bootstrap_enkf_set_double( void * arg , const char * var_name , double value) {
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( arg );
  {
    if (std_enkf_set_double( bootstrap_data->std_enkf_data , var_name , value ))
      return true;
    else {
      return false;
    }
  }
}


bool bootstrap_enkf_set_int( void * arg , const char * var_name , int value) {
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( arg );
  {
    if (std_enkf_set_int( bootstrap_data->std_enkf_data , var_name , value ))
      return true;
    else {
      return false;
    }
  }
}


bool bootstrap_enkf_set_bool( void * arg , const char * var_name , bool value) {
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , "CV" ) == 0)
      bootstrap_data->doCV = value;
    else
      name_recognized = false;

    return name_recognized;
  }
}


bool bootstrap_enkf_has_var( const void * arg, const char * var_name) {
    const bootstrap_enkf_data_type * module_data = bootstrap_enkf_data_safe_cast_const( arg );
    {
      return std_enkf_has_var(module_data->std_enkf_data, var_name);
    }
}

double bootstrap_enkf_get_double( const void * arg, const char * var_name) {
    const bootstrap_enkf_data_type * module_data = bootstrap_enkf_data_safe_cast_const( arg );
    {
      return std_enkf_get_double( module_data->std_enkf_data , var_name);
    }
}

int bootstrap_enkf_get_int( const void * arg, const char * var_name) {
    const bootstrap_enkf_data_type * module_data = bootstrap_enkf_data_safe_cast_const( arg );
    {
      return std_enkf_get_int( module_data->std_enkf_data , var_name);
    }
}




#ifdef INTERNAL_LINK
#define LINK_NAME BOOTSTRAP_ENKF
#else
#define LINK_NAME EXTERNAL_MODULE_SYMBOL
#endif


analysis_table_type LINK_NAME = {
  .name            = "BOOTSTRAP_ENKF",
  .alloc           = bootstrap_enkf_data_alloc,
  .freef           = bootstrap_enkf_data_free,
  .set_int         = bootstrap_enkf_set_int ,
  .set_double      = bootstrap_enkf_set_double ,
  .set_bool        = bootstrap_enkf_set_bool ,
  .set_string      = NULL ,
  .get_options     = bootstrap_enkf_get_options ,
  .initX           = NULL,
  .updateA         = bootstrap_enkf_updateA,
  .init_update     = NULL,
  .complete_update = NULL,
  .has_var         = bootstrap_enkf_has_var,
  .get_int         = bootstrap_enkf_get_int,
  .get_double      = bootstrap_enkf_get_double,
  .get_bool        = NULL,
  .get_ptr         = NULL,
};

/*
   copyright (C) 2011  Statoil ASA, Norway.

   The file 'fwd_step_enkf.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <math.h>
#include <stdio.h>

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/rng.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/stepwise.h>
#include <ert/util/stringlist.h>
#include <ert/util/double_vector.h>

#include <ert/analysis/fwd_step_enkf.h>
#include <ert/analysis/fwd_step_log.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/analysis_module.h>
#include <ert/analysis/module_data_block.h>
#include <ert/analysis/module_data_block_vector.h>
#include <ert/analysis/module_obs_block.h>
#include <ert/analysis/module_obs_block_vector.h>


#define FWD_STEP_ENKF_TYPE_ID 765524

#define DEFAULT_NFOLDS              5
#define DEFAULT_R2_LIMIT            0.99
#define NFOLDS_KEY                  "CV_NFOLDS"
#define R2_LIMIT_KEY                "FWD_STEP_R2_LIMIT"
#define DEFAULT_VERBOSE             false
#define VERBOSE_KEY                 "VERBOSE"
#define  LOG_FILE_KEY               "LOG_FILE"
#define  CLEAR_LOG_KEY              "CLEAR_LOG"

struct fwd_step_enkf_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  stepwise_type            * stepwise_data;
  rng_type                 * rng;
  int                        nfolds;
  long                       option_flags;
  double                     r2_limit;
  bool                       verbose;
  fwd_step_log_type        * fwd_step_log;
};


static UTIL_SAFE_CAST_FUNCTION_CONST( fwd_step_enkf_data , FWD_STEP_ENKF_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( fwd_step_enkf_data , FWD_STEP_ENKF_TYPE_ID )


void fwd_step_enkf_set_nfolds( fwd_step_enkf_data_type * data , int nfolds ) {
  data->nfolds = nfolds;
}

void fwd_step_enkf_set_r2_limit( fwd_step_enkf_data_type * data , double limit ) {
  data->r2_limit = limit;
}

void fwd_step_enkf_set_verbose( fwd_step_enkf_data_type * data , bool verbose ) {
  data->verbose = verbose;
}

void * fwd_step_enkf_data_alloc( rng_type * rng ) {
  fwd_step_enkf_data_type * data = util_malloc( sizeof * data );
  UTIL_TYPE_ID_INIT( data , FWD_STEP_ENKF_TYPE_ID );

  data->stepwise_data = NULL;
  data->rng          = rng;
  data->nfolds       = DEFAULT_NFOLDS;
  data->r2_limit     = DEFAULT_R2_LIMIT;
  data->option_flags = ANALYSIS_NEED_ED + ANALYSIS_UPDATE_A + ANALYSIS_SCALE_DATA;
  data->verbose      = DEFAULT_VERBOSE;
  data->fwd_step_log = fwd_step_log_alloc();
  return data;
}

void fwd_step_enkf_data_free( void * arg ) {
  fwd_step_enkf_data_type * fwd_step_data = fwd_step_enkf_data_safe_cast( arg );
  {

    if (fwd_step_data != NULL) {
      if (fwd_step_data->stepwise_data != NULL) {
        stepwise_free( fwd_step_data->stepwise_data );
      }
    }
  }
  fwd_step_log_free( fwd_step_data->fwd_step_log );
  free( fwd_step_data );
}


//**********************************************
// Log-file related stuff
//**********************************************


static void fwd_step_enkf_write_log_header( fwd_step_enkf_data_type * fwd_step_data, const char * ministep_name, const int nx, const int nd, const int ens_size) {
  const char * format = "%-25s%-25s%-25s%-25s\n";
  const char * column1 = "Parameter(ActiveIndex)";
  const char * column2 = "GlobalIndex";
  const char * column3 = "NumAttached";
  const char * column4 = "AttachedObs(ActiveIndex)[Percentage sensitivity]";
  int nfolds      = fwd_step_data->nfolds;
  double r2_limit = fwd_step_data->r2_limit;

  if (fwd_step_log_is_open( fwd_step_data->fwd_step_log )) {
    fwd_step_log_line(fwd_step_data->fwd_step_log, "===============================================================================================================================\n");
    fwd_step_log_line(fwd_step_data->fwd_step_log, "Ministep                    : %s\n",ministep_name);
    fwd_step_log_line(fwd_step_data->fwd_step_log, "Total number of parameters  : %d\n",nx);
    fwd_step_log_line(fwd_step_data->fwd_step_log, "Total number of observations: %d\n",nd);
    fwd_step_log_line(fwd_step_data->fwd_step_log, "Number of ensembles         : %d\n",ens_size);
    fwd_step_log_line(fwd_step_data->fwd_step_log, "CV folds                    : %d\n",nfolds);
    fwd_step_log_line(fwd_step_data->fwd_step_log, "Relative R2 tolerance       : %f\n",r2_limit);
    fwd_step_log_line(fwd_step_data->fwd_step_log, "===============================================================================================================================\n");
    fwd_step_log_line(fwd_step_data->fwd_step_log, format, column1, column2, column3, column4);
    fwd_step_log_line(fwd_step_data->fwd_step_log, "===============================================================================================================================\n");
  }

  printf("===============================================================================================================================\n");
  printf("Ministep                    : %s\n",ministep_name);
  printf("Total number of parameters  : %d\n",nx);
  printf("Total number of observations: %d\n",nd);
  printf("Number of ensembles         : %d\n",ens_size);
  printf("CV folds                    : %d\n",nfolds);
  printf("Relative R2 tolerance       : %f\n",r2_limit);
  printf("===============================================================================================================================\n");
  printf(format, column1, column2, column3, column4);
  printf("===============================================================================================================================\n");
}

static void fwd_step_enkf_write_iter_info( fwd_step_enkf_data_type * data , stepwise_type * stepwise, const char* key, const int data_active_index, const int global_index, const module_info_type * module_info ) {

  const char * format = "%-25s%-25d%-25d";
  int n_active = stepwise_get_n_active( stepwise);
  bool_vector_type * active_set = stepwise_get_active_set(stepwise);
  bool has_log = fwd_step_log_is_open( data->fwd_step_log );
  module_obs_block_vector_type * module_obs_block_vector  = module_info_get_obs_block_vector(module_info);
  char * loc_key = util_alloc_string_copy(key);
  char * data_active_index_str = util_alloc_sprintf( "(%d)" , data_active_index );
  char * cat = util_strcat_realloc(loc_key , data_active_index_str );
  if (has_log)
    fwd_step_log_line( data->fwd_step_log , format, cat, global_index, n_active);

  printf(format, cat, global_index,n_active);

  const double sum_beta = stepwise_get_sum_beta(stepwise);
  int obs_active_index = 0;
  stringlist_type * obs_list = stringlist_alloc_new( );
  double_vector_type * r_list = double_vector_alloc(0, 0);

  const char * format1 = "%s(%d)[%.1f]   ";
  for (int ivar = 0; ivar < bool_vector_size( active_set); ivar++) {
    if (!bool_vector_iget( active_set , ivar))
      continue;

    const module_obs_block_type  * module_obs_block = module_obs_block_vector_search_module_obs_block(module_obs_block_vector, ivar);
    const int* active_indices = module_obs_block_get_active_indices(module_obs_block);
    bool all_active = active_indices == NULL; /* Inactive are not present in D */
    int row_start = module_obs_block_get_row_start(module_obs_block);
    int row_end   = module_obs_block_get_row_end(module_obs_block);
    const char* obs_key = module_obs_block_get_key(module_obs_block);
    const double var_beta = stepwise_iget_beta(stepwise, ivar);
    const double var_beta_percent = 100.0 * fabs(var_beta) / sum_beta;

    int local_index = 0;
    for (int i = row_start; i < row_end; i++) {
      if (i == ivar){
        if (all_active)
          obs_active_index = local_index;
        else
          obs_active_index = active_indices[local_index];
        break;
      }
      local_index ++;
    }

    char * obs_list_entry = util_alloc_sprintf(format1 , obs_key, obs_active_index,var_beta_percent);
    stringlist_append_copy(obs_list, obs_list_entry);
    double_vector_append(r_list, var_beta_percent);
    free( obs_list_entry );
  }

  {
    /* Sorting with respect to sensitivity */
    perm_vector_type * sort_perm =  double_vector_alloc_rsort_perm(r_list);
    for (int i = 0; i < stringlist_get_size( obs_list); i++) {
      const char * obs_list_entry = stringlist_iget(obs_list, perm_vector_iget(sort_perm, i));
      if (has_log)
        fwd_step_log_line( data->fwd_step_log , "%s", obs_list_entry);

      printf("%s", obs_list_entry);
    }
    perm_vector_free(sort_perm);
  }



  if (has_log)
    fwd_step_log_line( data->fwd_step_log , "\n");

  printf("\n");

  stringlist_free(obs_list);
  util_safe_free(data_active_index_str);
  util_safe_free(cat);
}

/*Main function: */
void fwd_step_enkf_updateA(void * module_data ,
                           matrix_type * A ,
                           matrix_type * S ,
                           matrix_type * R ,
                           matrix_type * dObs ,
                           matrix_type * E ,
                           matrix_type * D ,
                           const module_info_type* module_info) {



  fwd_step_enkf_data_type * fwd_step_data = fwd_step_enkf_data_safe_cast( module_data );
  fwd_step_log_open(fwd_step_data->fwd_step_log);
  module_data_block_vector_type * data_block_vector = module_info_get_data_block_vector(module_info);
  printf("Running Forward Stepwise regression:\n");
  {

    int ens_size    = matrix_get_columns( S );
    int nx          = matrix_get_rows( A );
    int nd          = matrix_get_rows( S );
    int nfolds      = fwd_step_data->nfolds;
    double r2_limit = fwd_step_data->r2_limit;
    bool verbose    = fwd_step_data->verbose;
    int num_kw     =  module_data_block_vector_get_size(data_block_vector);


    if ( ens_size <= nfolds)
      util_abort("%s: The number of ensembles must be larger than the CV fold - aborting\n", __func__);


    {

      stepwise_type * stepwise_data = stepwise_alloc1(ens_size, nd , fwd_step_data->rng);

      matrix_type * workS = matrix_alloc( ens_size , nd  );
      matrix_type * workE = matrix_alloc( ens_size , nd  );

      /*workS = S' */
      matrix_subtract_row_mean( S );           /* Shift away the mean */
      workS   = matrix_alloc_transpose( S );
      workE   = matrix_alloc_transpose( E );

      stepwise_set_X0( stepwise_data , workS );
      stepwise_set_E0( stepwise_data , workE );

      matrix_type * di = matrix_alloc( 1 , nd );

      if (verbose){
        char * ministep_name = module_info_get_ministep_name(module_info);
        fwd_step_enkf_write_log_header(fwd_step_data, ministep_name, nx, nd, ens_size);
      }

      for (int kw = 0; kw < num_kw; kw++) {

        module_data_block_type * data_block = module_data_block_vector_iget_module_data_block(data_block_vector, kw);
        const char * key = module_data_block_get_key(data_block);
        int row_start = module_data_block_get_row_start(data_block);
        int row_end   = module_data_block_get_row_end(data_block);
        const int* active_indices = module_data_block_get_active_indices(data_block);
        int local_index = 0;
        int active_index = 0;
        bool all_active = active_indices == NULL; /* Inactive are not present in A */

        for (int i = row_start; i < row_end; i++) {

          /*Update values of y */
          /*Start of the actual update */
          matrix_type * y = matrix_alloc( ens_size , 1 );

          for (int j = 0; j < ens_size; j++) {
            matrix_iset(y , j , 0 , matrix_iget( A, i , j ) );
          }

          stepwise_set_Y0( stepwise_data , y );

          stepwise_estimate(stepwise_data , r2_limit , nfolds );

          /*manipulate A directly*/
          for (int j = 0; j < ens_size; j++) {
            for (int k = 0; k < nd; k++) {
              matrix_iset(di , 0 , k , matrix_iget( D , k , j ) );
            }
            double aij = matrix_iget( A , i , j );
            double xHat = stepwise_eval(stepwise_data , di );
            matrix_iset(A , i , j , aij + xHat);
          }

          if (verbose){
            if (all_active)
             active_index = local_index;
            else
             active_index = active_indices[local_index];

            fwd_step_enkf_write_iter_info(fwd_step_data, stepwise_data, key, active_index, i, module_info);

          }

          local_index ++;
        }
      }

      if (verbose)
       printf("===============================================================================================================================\n");

      printf("Done with stepwise regression enkf\n");

      stepwise_free( stepwise_data );
      matrix_free( di );

    }



  }

  fwd_step_log_close( fwd_step_data->fwd_step_log );
}






bool fwd_step_enkf_set_double( void * arg , const char * var_name , double value) {
  fwd_step_enkf_data_type * module_data = fwd_step_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , R2_LIMIT_KEY ) == 0)
      fwd_step_enkf_set_r2_limit( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}


bool fwd_step_enkf_set_int( void * arg , const char * var_name , int value) {
  fwd_step_enkf_data_type * module_data = fwd_step_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    /*Set number of CV folds */
    if (strcmp( var_name , NFOLDS_KEY) == 0)
      fwd_step_enkf_set_nfolds( module_data , value);
    else
      name_recognized = false;

    return name_recognized;
   }
}

bool fwd_step_enkf_set_bool( void * arg , const char * var_name , bool value) {
  fwd_step_enkf_data_type * module_data = fwd_step_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    /*Set verbose */
    if (strcmp( var_name , VERBOSE_KEY) == 0)
      fwd_step_enkf_set_verbose( module_data , value);
    else if (strcmp( var_name , CLEAR_LOG_KEY) == 0)
      fwd_step_log_set_clear_log( module_data->fwd_step_log , value );
    else
      name_recognized = false;

    return name_recognized;
   }
}

bool fwd_step_enkf_set_string( void * arg , const char * var_name , const char * value) {
  fwd_step_enkf_data_type * module_data = fwd_step_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , LOG_FILE_KEY) == 0)
      fwd_step_log_set_log_file( module_data->fwd_step_log , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}

long fwd_step_enkf_get_options( void * arg , long flag) {
  fwd_step_enkf_data_type * fwd_step_data = fwd_step_enkf_data_safe_cast( arg );
  {
    return fwd_step_data->option_flags;
  }
}

bool fwd_step_enkf_has_var( const void * arg, const char * var_name) {
  {
    if (strcmp(var_name , NFOLDS_KEY) == 0)
      return true;
    else if (strcmp(var_name , R2_LIMIT_KEY ) == 0)
      return true;
    else if (strcmp(var_name , VERBOSE_KEY ) == 0)
      return true;
    else if (strcmp(var_name , LOG_FILE_KEY) == 0)
      return true;
    else if (strcmp(var_name , CLEAR_LOG_KEY) == 0)
      return true;
    else
      return false;
  }
}

double fwd_step_enkf_get_double( const void * arg, const char * var_name) {
  const fwd_step_enkf_data_type * module_data = fwd_step_enkf_data_safe_cast_const( arg );
  {
    if (strcmp(var_name , R2_LIMIT_KEY ) == 0)
      return module_data->r2_limit;
    else
      return -1;
  }
}

int fwd_step_enkf_get_int( const void * arg, const char * var_name) {
  const fwd_step_enkf_data_type * module_data = fwd_step_enkf_data_safe_cast_const( arg );
  {
    if (strcmp(var_name , NFOLDS_KEY) == 0)
      return module_data->nfolds;
    else
      return -1;
  }
}

bool fwd_step_enkf_get_bool( const void * arg, const char * var_name) {
  const fwd_step_enkf_data_type * module_data = fwd_step_enkf_data_safe_cast_const( arg );
  {
    if (strcmp(var_name , VERBOSE_KEY) == 0)
      return module_data->verbose;
     else if (strcmp(var_name , CLEAR_LOG_KEY) == 0)
      return fwd_step_log_get_clear_log( module_data->fwd_step_log );
    else
      return false;
  }
}

void * fwd_step_enkf_get_ptr( const void * arg , const char * var_name ) {
  const fwd_step_enkf_data_type * module_data = fwd_step_enkf_data_safe_cast_const( arg );
  {
    if (strcmp(var_name , LOG_FILE_KEY) == 0)
      return (void *) fwd_step_log_get_log_file( module_data->fwd_step_log );
    else
      return NULL;
  }
}





#ifdef INTERNAL_LINK
#define LINK_NAME FWD_STEP_ENKF
#else
#define LINK_NAME EXTERNAL_MODULE_SYMBOL
#endif


analysis_table_type LINK_NAME = {
  .name            = "FWD_STEP_ENKF",
  .alloc           = fwd_step_enkf_data_alloc,
  .freef           = fwd_step_enkf_data_free,
  .set_int         = fwd_step_enkf_set_int ,
  .set_double      = fwd_step_enkf_set_double ,
  .set_bool        = fwd_step_enkf_set_bool ,
  .set_string      = fwd_step_enkf_set_string ,
  .get_options     = fwd_step_enkf_get_options ,
  .initX           = NULL ,
  .updateA         = fwd_step_enkf_updateA,
  .init_update     = NULL ,
  .complete_update = NULL ,
  .has_var         = fwd_step_enkf_has_var,
  .get_int         = fwd_step_enkf_get_int ,
  .get_double      = fwd_step_enkf_get_double ,
  .get_bool        = fwd_step_enkf_get_bool ,
  .get_ptr         = fwd_step_enkf_get_ptr
};



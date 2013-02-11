/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'obs_data.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

/**
See the file README.obs for ducumentation of the varios datatypes
involved with observations/measurement/+++.


The file contains two different variables holding the number of
observations, nrobs_total and nrobs_active. The first holds the total
number of observations at this timestep, and the second holds the
number of active measurements at this timestep; the inactive
measurements have been deactivated the obs_data_deactivate_outliers()
function.

The flow is as follows:

 1. All the observations have been collected in an obs_data instance,
    and all the corresponding measurements of the state have been
    collected in a meas_data instance - we are ready for analysis.

 2. The functions meas_data_alloc_stats() is called to calculate
    the ensemble mean and std of all the measurements.

 3. The function obs_data_deactivate_outliers() is called to compare
    the ensemble mean and std with the observations, in the case of
    outliers the number obs_active flag of the obs_data instance is
    set to false.

 4. The remaining functions (and matrices) now refer to the number of
    active observations, however the "raw" observations found in the
    obs_data instance are in a vector with nrobs_total observations;
    i.e. we must handle two indices and two total lengths. A bit
    messy.


Variables of size nrobs_total:
------------------------------
 o obs->value / obs->std / obs->obs_active
 o meanS , innov, stdS


variables of size nrobs_active:
-------------------------------
Matrices: S, D, E and various internal variables.
*/


#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>

#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/matrix.h>
#include <ert/util/rng.h>

#include <ert/enkf/obs_data.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/enkf_util.h>

#define OBS_BLOCK_TYPE_ID 995833

struct obs_block_struct {
  UTIL_TYPE_ID_DECLARATION;
  char               * obs_key;
  int                  size;
  double             * value;
  double             * std;

  int                * active_mode;   
  int                  active_size;
  matrix_type        * error_covar;
  bool                 error_covar_owner;   /* If true the error_covar matrix is free'd when construction of the R matrix is complete. */
};



struct obs_data_struct {
  vector_type   * data;            /* vector with obs_block instances. */
  int           __active_size;   
}; 



static UTIL_SAFE_CAST_FUNCTION(obs_block , OBS_BLOCK_TYPE_ID )

static obs_block_type * obs_block_alloc( const char * obs_key , int obs_size , matrix_type * error_covar , bool error_covar_owner) {
  obs_block_type * obs_block = util_malloc( sizeof * obs_block );

  UTIL_TYPE_ID_INIT( obs_block , OBS_BLOCK_TYPE_ID );
  obs_block->size        = obs_size;
  obs_block->obs_key     = util_alloc_string_copy( obs_key );
  obs_block->value       = util_calloc( obs_size , sizeof * obs_block->value       );
  obs_block->std         = util_calloc( obs_size , sizeof * obs_block->std         );
  obs_block->active_mode = util_calloc( obs_size , sizeof * obs_block->active_mode );
  obs_block->error_covar = error_covar;
  obs_block->error_covar_owner = error_covar_owner;
  {
    for (int iobs = 0; iobs < obs_size; iobs++)
      obs_block->active_mode[iobs] = LOCAL_INACTIVE;
  }
  obs_block->active_size = 0;
  return obs_block;
}



static void obs_block_free( obs_block_type * obs_block ) {
  free( obs_block->obs_key );
  free( obs_block->value );
  free( obs_block->std );
  free( obs_block->active_mode );
  free( obs_block );
}


static void obs_block_free__( void * arg ) {
  obs_block_type * obs_block = obs_block_safe_cast( arg );
  obs_block_free( obs_block );
}


void obs_block_deactivate( obs_block_type * obs_block , int iobs , const char * msg) {
  if (obs_block->active_mode[ iobs ] == ACTIVE) {
    printf("Deactivating: %s(%d) : %s \n",obs_block->obs_key , iobs , msg);
    obs_block->active_mode[ iobs ] = DEACTIVATED;
    obs_block->active_size--;
  }
}


const char * obs_block_get_key( const obs_block_type * obs_block) { return obs_block->obs_key; }

void obs_block_iset( obs_block_type * obs_block , int iobs , double value , double std) {
  obs_block->value[ iobs ] = value;
  obs_block->std[ iobs ]   = std;
  if (obs_block->active_mode[ iobs ] != ACTIVE) {
    obs_block->active_mode[iobs]  = ACTIVE;
    obs_block->active_size++;
  }
}

void obs_block_iset_missing( obs_block_type * obs_block , int iobs ) {
  if (obs_block->active_mode[ iobs ] == ACTIVE) 
    obs_block->active_size--;
  obs_block->active_mode[iobs] = MISSING;
}


double obs_block_iget_std( const obs_block_type * obs_block , int iobs) {
  return obs_block->std[ iobs ];
}


double obs_block_iget_value( const obs_block_type * obs_block , int iobs) {
  return obs_block->value[ iobs ];
}


active_type obs_block_iget_active_mode( const obs_block_type * obs_block , int iobs) {
  return obs_block->active_mode[ iobs ];
}



int obs_block_get_size( const obs_block_type * obs_block ) {
  return obs_block->size;
}


/*Function that sets each element of the scaling factor equal to 1 divided by the prior standard deviation (from the
  obs_data input file.
*/
static void obs_block_init_scaling( const obs_block_type * obs_block , double * scale_factor , int * __obs_offset) {
  int obs_offset = *__obs_offset;
  int iobs;
  for (iobs =0; iobs < obs_block->size; iobs++) {
    if (obs_block->active_mode[iobs] == ACTIVE) {
      scale_factor[ obs_offset ] = 1.0 / obs_block->std[ iobs ];
      obs_offset++;
    }
  }
  *__obs_offset = obs_offset;
}


/*
static void obs_block_init_innov( const obs_block_type * obs_block , const meas_block_type * meas_block , matrix_type * innov , int * __obs_offset) {
  int obs_offset = *__obs_offset;
  int iobs;
  for (iobs =0; iobs < obs_block->size; iobs++) {
    if (obs_block->active_mode[iobs] == ACTIVE) {
      matrix_iset( innov , obs_offset , 0 , obs_block->value[ iobs ] - meas_block_iget_ens_mean( meas_block , iobs ));
      obs_offset++;
    }
  }
  *__obs_offset = obs_offset;
}
*/

static void obs_block_initdObs( const obs_block_type * obs_block , matrix_type * dObs , int * __obs_offset) {
  int obs_offset = *__obs_offset;
  int iobs;
  for (iobs =0; iobs < obs_block->size; iobs++) {
    if (obs_block->active_mode[iobs] == ACTIVE) {
      matrix_iset( dObs , obs_offset , 0 , obs_block->value[ iobs ]);
      obs_offset++;
    }
  }
  *__obs_offset = obs_offset;
}






static void obs_block_initR( const obs_block_type * obs_block , matrix_type * R, int * __obs_offset) {
  int obs_offset = *__obs_offset;
  if (obs_block->error_covar == NULL) {
    int iobs;
    int iactive = 0;
    for (iobs =0; iobs < obs_block->size; iobs++) {
      if (obs_block->active_mode[iobs] == ACTIVE) {
        double var = obs_block->std[iobs] * obs_block->std[iobs];
        matrix_iset_safe(R , obs_offset + iactive, obs_offset + iactive, var);
        iactive++;
      } 
    }
  } else {
    int row_active = 0;   /* We have a covar matrix */
    for (int row = 0; row < obs_block->size; row++) {
      if (obs_block->active_mode[row] == ACTIVE) {
        int col_active = 0;
        for (int col = 0; col < obs_block->size; col++) {
          if (obs_block->active_mode[col] == ACTIVE) {
            matrix_iset_safe(R , obs_offset + row_active , obs_offset + col_active , matrix_iget( obs_block->error_covar , row , col ));
            col_active++;
          }
        }
        row_active++;
      }
    }
  }
  
  *__obs_offset = obs_offset + obs_block->active_size;
  if ((obs_block->error_covar_owner) && (obs_block->error_covar != NULL))
    matrix_free( obs_block->error_covar );
}



static void obs_block_initE( const obs_block_type * obs_block , matrix_type * E, const double * pert_var , int * __obs_offset) {
  int ens_size   = matrix_get_columns( E );
  int obs_offset = *__obs_offset;
  int iobs;
  for (iobs =0; iobs < obs_block->size; iobs++) {
    if (obs_block->active_mode[iobs] == ACTIVE) {
      double factor = obs_block->std[iobs] * sqrt( ens_size / pert_var[ obs_offset ]);
      for (int iens = 0; iens < ens_size; iens++) 
        matrix_imul(E , obs_offset , iens , factor );

      obs_offset++;
    }
  }
  
  *__obs_offset = obs_offset;
}


static void obs_block_initE_non_centred( const obs_block_type * obs_block , matrix_type * E, int * __obs_offset) {
  int ens_size   = matrix_get_columns( E );
  int obs_offset = *__obs_offset;
  int iobs;
  for (iobs =0; iobs < obs_block->size; iobs++) {
    if (obs_block->active_mode[iobs] == ACTIVE) {
      double factor = obs_block->std[iobs];
      for (int iens = 0; iens < ens_size; iens++) 
        matrix_imul(E , obs_offset , iens , factor );

      obs_offset++;
    }
  }
  
  *__obs_offset = obs_offset;
}



static void obs_block_initD( const obs_block_type * obs_block , matrix_type * D, int * __obs_offset) {
  int ens_size   = matrix_get_columns( D );
  int obs_offset = *__obs_offset;
  int iobs;
  for (iobs =0; iobs < obs_block->size; iobs++) {
    if (obs_block->active_mode[iobs] == ACTIVE) {
      for (int iens = 0; iens < ens_size; iens++) 
        matrix_iadd(D , obs_offset , iens , obs_block->value[ iobs ]);
      
      obs_offset++;
    }
  }
  
  *__obs_offset = obs_offset;
}



obs_data_type * obs_data_alloc() {
  obs_data_type * obs_data = util_malloc(sizeof * obs_data );
  obs_data->data = vector_alloc_new();
  obs_data_reset(obs_data);
  return obs_data;
}



void obs_data_reset(obs_data_type * obs_data) { 
  vector_clear( obs_data->data );
  obs_data->__active_size = -1;
}


obs_block_type * obs_data_add_block( obs_data_type * obs_data , const char * obs_key , int obs_size , matrix_type * error_covar, bool error_covar_owner) {
  obs_block_type * new_block = obs_block_alloc( obs_key , obs_size , error_covar , error_covar_owner);
  vector_append_owned_ref( obs_data->data , new_block , obs_block_free__ );
  return new_block;
}


obs_block_type * obs_data_iget_block( obs_data_type * obs_data , int index ) {
  return vector_iget( obs_data->data , index);
}


const obs_block_type * obs_data_iget_block_const( const obs_data_type * obs_data , int index ) {
  return vector_iget_const( obs_data->data , index );
}


void obs_data_free(obs_data_type * obs_data) {
  vector_free( obs_data->data );
  free(obs_data);
}



matrix_type * obs_data_allocE(const obs_data_type * obs_data , rng_type * rng , int ens_size, int active_size ) {
  double *pert_mean , *pert_var;
  matrix_type * E;
  int iens, iobs_active;
  
  E         = matrix_alloc( active_size , ens_size);

  pert_mean = util_calloc(active_size , sizeof * pert_mean );
  pert_var  = util_calloc(active_size , sizeof * pert_var  );
  {
    double * tmp = util_calloc( active_size * ens_size , sizeof * tmp );
    int i,j;
    int k = 0;
    
    enkf_util_rand_stdnormal_vector(active_size * ens_size , tmp , rng);
    for (j=0; j < ens_size; j++) {
      for (i=0; i < active_size; i++) {
        matrix_iset( E , i , j , tmp[k]);
        k++;
      }
    }
    free(tmp);
  }
  
  for (iobs_active = 0; iobs_active < active_size; iobs_active++) {
    pert_mean[iobs_active] = 0;
    pert_var[iobs_active]  = 0;
  }
  
  for (iens = 0; iens < ens_size; iens++) 
    for (iobs_active = 0; iobs_active < active_size; iobs_active++) 
      pert_mean[iobs_active] += matrix_iget(E , iobs_active , iens);
  

  for (iobs_active = 0; iobs_active < active_size; iobs_active++) 
    pert_mean[iobs_active] /= ens_size;

  for  (iens = 0; iens < ens_size; iens++) {
    for (iobs_active = 0; iobs_active < active_size; iobs_active++) {
      double tmp;
      matrix_iadd(E , iobs_active , iens , -pert_mean[iobs_active]);
      tmp = matrix_iget(E , iobs_active , iens);
      pert_var[iobs_active] += tmp * tmp;
    }
  }

  /*
    The actual observed data are not accessed before this last block. 
  */
  {
    int obs_offset = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block = vector_iget_const( obs_data->data , block_nr);
      obs_block_initE( obs_block , E , pert_var , &obs_offset);
    }
  }

  free(pert_mean);
  free(pert_var);

  matrix_set_name( E , "E");
  matrix_assert_finite( E );
  return E;
}


/* Function that returns a matrix of independent, normal distributed random vector having mean zero,
 and variance (covariance) specified in the input (obs_data) file. NOTICE THE DIFFERENCE WITH allocE, WHERE THE
 RETURNED MATRIX IS CENTRED
*/
   

matrix_type * obs_data_allocE_non_centred(const obs_data_type * obs_data , rng_type * rng , int ens_size, int active_size ) {
  matrix_type * E;
  
  E         = matrix_alloc( active_size , ens_size);

  {
    double * tmp = util_calloc( active_size * ens_size , sizeof * tmp );
    int i,j;
    int k = 0;
    
    enkf_util_rand_stdnormal_vector(active_size * ens_size , tmp , rng); 
    for (j=0; j < ens_size; j++) {
      for (i=0; i < active_size; i++) {
        matrix_iset( E , i , j , tmp[k]);
        k++;
      }
    }
    free(tmp);
  }
  

  /*
    The actual observed data are not accessed before this last block. 
  */
  {
    int obs_offset = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block = vector_iget_const( obs_data->data , block_nr);
      obs_block_initE_non_centred( obs_block , E , &obs_offset);
    }
  }


  matrix_set_name( E , "E");
  matrix_assert_finite( E );
  return E;
}

matrix_type * obs_data_allocD(const obs_data_type * obs_data , const matrix_type * E  , const matrix_type * S) {
  matrix_type * D = matrix_alloc_copy( E );
  matrix_inplace_sub( D , S );

  {
    int obs_offset = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block = vector_iget_const( obs_data->data , block_nr);
      obs_block_initD( obs_block , D , &obs_offset);
    }
  }
  
  matrix_set_name( D , "D");
  matrix_assert_finite( D );
  return D;
}






matrix_type * obs_data_allocR(const obs_data_type * obs_data , int active_size) {
  matrix_type * R = matrix_alloc( active_size , active_size );
  {
    int obs_offset = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block = vector_iget_const( obs_data->data , block_nr);
      obs_block_initR( obs_block , R , &obs_offset);
    }
  }
  
  matrix_set_name( R , "R");
  matrix_assert_finite( R );
  return R;
}

/*
matrix_type * obs_data_alloc_innov(const obs_data_type * obs_data , const meas_data_type * meas_data , int active_size) {
  matrix_type * innov = matrix_alloc( active_size , 1 );
  {
    int obs_offset = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block   = vector_iget_const( obs_data->data , block_nr );
      const meas_block_type * meas_block = meas_data_iget_block_const( meas_data , block_nr );
      
      obs_block_init_innov( obs_block , meas_block , innov , &obs_offset);
    }
  }
  return innov;
}
*/

matrix_type * obs_data_allocdObs(const obs_data_type * obs_data , int active_size) {
  matrix_type * dObs = matrix_alloc( active_size , 1 );
  {
    int obs_offset = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block   = vector_iget_const( obs_data->data , block_nr );
      
      obs_block_initdObs( obs_block ,  dObs , &obs_offset);
    }
  }
  return dObs;
}



void obs_data_scale(const obs_data_type * obs_data , matrix_type *S , matrix_type *E , matrix_type *D , matrix_type *R , matrix_type * dObs) {
  const int nrobs_active = matrix_get_rows( S );
  const int ens_size     = matrix_get_columns( S );
  double * scale_factor  = util_calloc(nrobs_active , sizeof * scale_factor );
  int iens, iobs_active;
  
  {
    int obs_offset = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block   = vector_iget_const( obs_data->data , block_nr );
      
      /* Init. the scaling factor ( 1/std(dObs) ) */
      obs_block_init_scaling( obs_block , scale_factor  , &obs_offset);
    }
  }


  for  (iens = 0; iens < ens_size; iens++) {
    for (iobs_active = 0; iobs_active < nrobs_active; iobs_active++) {

      /* Scale the forecasted data so that they (in theory) have the same variance 
         (if the prior distribution for the observation errors is correct) */
      matrix_imul(S , iobs_active , iens , scale_factor[iobs_active]);

      if (D != NULL)
        /* Scale the combined data matrix: D = DObs + E - S, where DObs is the iobs_active times ens_size matrix where 
           each column contains a copy of the observed data
         */
        matrix_imul(D , iobs_active , iens , scale_factor[iobs_active]);

      if (E != NULL)
        /* Same with E (used for low rank representation of the error covariance matrix*/
        matrix_imul(E , iobs_active , iens , scale_factor[iobs_active]);
    }
  }
  
  if (dObs != NULL)
    for (iobs_active = 0; iobs_active < nrobs_active; iobs_active++) 
      matrix_imul( dObs , iobs_active , 0 , scale_factor[iobs_active]);
  
  if (R != NULL) {
    /* Scale the error covariance matrix*/
    for (int i=0; i < nrobs_active; i++)
      for (int j=0; j < nrobs_active; j++)
        matrix_imul(R , i , j , scale_factor[i] * scale_factor[j]);
  }
  free(scale_factor);
}


void obs_data_scale_kernel(const obs_data_type * obs_data , matrix_type *S , matrix_type *E , matrix_type *D , double *dObs) {
  const int nrobs_active = matrix_get_rows( S );
  const int ens_size     = matrix_get_columns( S );
  double * scale_factor  = util_calloc(nrobs_active , sizeof * scale_factor );
  int iens, iobs_active;
  
  {
    int obs_offset = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block   = vector_iget_const( obs_data->data , block_nr );
      
      /* Init. the scaling factor ( 1/std(dObs) ) */
      obs_block_init_scaling( obs_block , scale_factor  , &obs_offset);
    }
  }


  for  (iens = 0; iens < ens_size; iens++) {
    for (iobs_active = 0; iobs_active < nrobs_active; iobs_active++) {

      /* Scale the forecasted data so that they (in theory) have the same variance 
         (if the prior distribution for the observation errors is correct) */
      matrix_imul(S , iobs_active , iens , scale_factor[iobs_active]);

      if (D != NULL)
        /* Scale the combined data matrix: D = DObs + E - S, where DObs is the iobs_active times ens_size matrix where 
           each column contains a copy of the observed data
         */
        matrix_imul(D , iobs_active , iens , scale_factor[iobs_active]);
      
      if (E != NULL)
        /* Same with E (used for low rank representation of the error covariance matrix*/
        matrix_imul(E , iobs_active , iens , scale_factor[iobs_active]);
    }
  }
  
  /* Scale the vector of observed data*/
  if (dObs != NULL) {
    for (iobs_active = 0; iobs_active < nrobs_active; iobs_active++) 
      dObs[iobs_active] *= scale_factor[iobs_active];
  }
    

  free(scale_factor);
}




int obs_data_get_active_size(  obs_data_type * obs_data ) {
  
  if (obs_data->__active_size < 0) {
    int active_size = 0;
    for (int block_nr = 0; block_nr < vector_get_size( obs_data->data ); block_nr++) {
      const obs_block_type * obs_block   = vector_iget_const( obs_data->data , block_nr );
      active_size += obs_block->active_size; 
    }
    obs_data->__active_size = active_size;
  }
  
  return obs_data->__active_size;
}


int obs_data_get_num_blocks( const obs_data_type * obs_data ) {
  return vector_get_size( obs_data->data );
}

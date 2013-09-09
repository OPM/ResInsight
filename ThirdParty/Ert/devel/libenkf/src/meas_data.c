/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'meas_data.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
*/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/matrix.h>
#include <ert/util/set.h>
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>

#include <ert/enkf/meas_data.h>

#define MEAS_BLOCK_TYPE_ID 661936407
#define MEAS_DATA_TYPE_ID  561000861


struct meas_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                 ens_size;
  vector_type       * data; 
  pthread_mutex_t     data_mutex;
  set_type          * lookup_keys;  /* Mangled obs_key and report_step */ 
};


struct meas_block_struct {
  UTIL_TYPE_ID_DECLARATION;
  int          ens_size;
  int          obs_size;
  int          ens_stride;
  int          obs_stride;
  int          data_size;
  int          report_step;  /* Not really necessary ?? */
  char       * obs_key;
  double     * data;
  bool       * active;
};


static UTIL_SAFE_CAST_FUNCTION( meas_block , MEAS_BLOCK_TYPE_ID )


/**
   Observe that meas_block instance must be allocated with a correct
   value for obs_size; it can not grow during use, and it does also
   not count the number of elements added.
   
   Observe that the input argument @obs_size should be the total size
   of the observation; if parts of the observation have been excluded
   due to local analysis it should still be included in the @obs_size
   value.
*/

static meas_block_type * meas_block_alloc( const char * obs_key , int report_step , int ens_size , int obs_size) {
  meas_block_type * meas_block = util_malloc( sizeof * meas_block );
  UTIL_TYPE_ID_INIT( meas_block , MEAS_BLOCK_TYPE_ID );
  meas_block->ens_size    = ens_size;
  meas_block->obs_size    = obs_size;
  meas_block->obs_key     = util_alloc_string_copy( obs_key );
  meas_block->data        = util_calloc( (ens_size + 2)     * obs_size , sizeof * meas_block->data   );
  meas_block->active      = util_calloc(                      obs_size , sizeof * meas_block->active );
  meas_block->ens_stride  = 1;
  meas_block->obs_stride  = ens_size + 2; 
  meas_block->data_size   = (ens_size + 2) * obs_size;
  meas_block->report_step = report_step;
  {
    int i;
    for (i=0; i  <obs_size; i++)
      meas_block->active[i] = false;
  }
  return meas_block;
}

static void meas_block_fprintf( const meas_block_type * meas_block , FILE * stream) {
  int iens;
  int iobs;
  for (iobs = 0; iobs < meas_block->obs_size; iobs++) {
    for (iens = 0; iens < meas_block->ens_size; iens++) {
      int index = iens * meas_block->ens_stride + iobs * meas_block->obs_stride;
      fprintf(stream , " %10.2f ", meas_block->data[ index ]);
    }
    fprintf(stream , "\n");
  }
}


static void meas_block_free( meas_block_type * meas_block ) {
  free( meas_block->obs_key );
  free( meas_block->data );
  free( meas_block->active );
  free( meas_block );
}


static void meas_block_free__( void * arg ) {
  meas_block_type * meas_block = meas_block_safe_cast( arg );
  meas_block_free( meas_block );
}



static void meas_block_initS( const meas_block_type * meas_block , matrix_type * S, int * __obs_offset) {
  int obs_offset = *__obs_offset;
  for (int iobs =0; iobs < meas_block->obs_size; iobs++) {
    if (meas_block->active[iobs]) {
      for (int iens =0; iens < meas_block->ens_size; iens++) {
        int obs_index = iens * meas_block->ens_stride + iobs* meas_block->obs_stride;
        
        matrix_iset( S , obs_offset, iens , meas_block->data[ obs_index ]);
      }
      obs_offset++;
    }
  }
  *__obs_offset = obs_offset;
}


static void meas_data_assign_block( meas_block_type * target_block , const meas_block_type * src_block , int target_iens , int src_iens ) {
  int iobs;
  for (iobs =0; iobs < target_block->obs_size; iobs++) {  
    int target_index = target_iens * target_block->ens_stride + iobs * target_block->obs_stride;
    int src_index    = src_iens * src_block->ens_stride + iobs * src_block->obs_stride;
    target_block->data[ target_index ] = src_block->data[ src_index ];
  }
}


void meas_block_calculate_ens_stats( meas_block_type * meas_block ) {
  bool include_inactive = true;
  int iobs , iens;
  for (iobs =0; iobs < meas_block->obs_size; iobs++) {  
    if (meas_block->active[iobs] || include_inactive) { 
      double M1 = 0;
      double M2 = 0;
      for (iens =0; iens < meas_block->ens_size; iens++) {
        int index = iens * meas_block->ens_stride + iobs * meas_block->obs_stride;
        M1 += meas_block->data[ index ];
        M2 += meas_block->data[ index ] * meas_block->data[ index ];
      }
      {
        int mean_index = (meas_block->ens_size + 0) * meas_block->ens_stride + iobs * meas_block->obs_stride;
        int std_index  = (meas_block->ens_size + 1) * meas_block->ens_stride + iobs * meas_block->obs_stride;
        double mean    = M1 / meas_block->ens_size; 
        double var     = M2 / meas_block->ens_size - mean * mean;
        meas_block->data[ mean_index ] = mean;
        meas_block->data[ std_index ]  = sqrt( util_double_max( 0.0 , var));
      }
    }
  }
}



void meas_block_iset( meas_block_type * meas_block , int iens , int iobs , double value) {
  int index = iens * meas_block->ens_stride + iobs * meas_block->obs_stride;
  meas_block->data[ index ] = value;
  if (!meas_block->active[ iobs ]) 
    meas_block->active[ iobs ] = true;

}


double meas_block_iget_ens_std( const meas_block_type * meas_block , int iobs) {
  int std_index  = (meas_block->ens_size + 1) * meas_block->ens_stride + iobs * meas_block->obs_stride;
  return meas_block->data[ std_index ];
}


double meas_block_iget_ens_mean( const meas_block_type * meas_block , int iobs) {
  int mean_index  = meas_block->ens_size * meas_block->ens_stride + iobs * meas_block->obs_stride;
  return meas_block->data[ mean_index ];
}


bool meas_block_iget_active( const meas_block_type * meas_block , int iobs) {
  return meas_block->active[ iobs ];
}


void meas_block_deactivate( meas_block_type * meas_block , int iobs ) {
  if (meas_block->active[ iobs ]) 
    meas_block->active[ iobs ] = false;
}


int meas_block_get_total_size( const meas_block_type * meas_block ) {
  return meas_block->obs_size;
}



/*****************************************************************/

UTIL_IS_INSTANCE_FUNCTION( meas_data , MEAS_DATA_TYPE_ID )

meas_data_type * meas_data_alloc( const int_vector_type * ens_active_list ) {
  int ens_size = int_vector_size( ens_active_list );
  if (ens_size <= 0) 
    util_abort("%s: ens_size must be > 0 - aborting \n",__func__);
  {
    meas_data_type * meas = util_malloc(sizeof * meas );
    UTIL_TYPE_ID_INIT( meas , MEAS_DATA_TYPE_ID );
    
    meas->ens_size     = ens_size;
    meas->data         = vector_alloc_new();
    meas->lookup_keys  = set_alloc_empty();
    pthread_mutex_init( &meas->data_mutex , NULL );
    
    return meas;
  }
}



void meas_data_free(meas_data_type * matrix) {
  vector_free( matrix->data );
  set_free( matrix->lookup_keys );
  free( matrix );
}



void meas_data_reset(meas_data_type * matrix) {
  set_clear( matrix->lookup_keys );
  vector_clear( matrix->data );  /* Will dump and discard all the meas_block instances. */
}


/**
   The code actually adding new blocks to the vector must be run in single-thread mode. 
*/

meas_block_type * meas_data_add_block( meas_data_type * matrix , const char * obs_key , int report_step , int obs_size) {
  char * lookup_key = util_alloc_sprintf( "%s-%d" , obs_key , report_step );  /* The obs_key is not alone unique over different report steps. */
  pthread_mutex_lock( &matrix->data_mutex );
  {
    if (!set_has_key( matrix->lookup_keys , lookup_key )) {
      meas_block_type  * new_block = meas_block_alloc(obs_key , report_step , matrix->ens_size , obs_size);
      vector_append_owned_ref( matrix->data , new_block , meas_block_free__ );
      set_add_key( matrix->lookup_keys , lookup_key );
    }
  }
  pthread_mutex_unlock( &matrix->data_mutex );
  free( lookup_key );
  return vector_get_last( matrix->data );
}



meas_block_type * meas_data_iget_block( meas_data_type * matrix , int block_nr) {
  return vector_iget( matrix->data , block_nr);
}


const meas_block_type * meas_data_iget_block_const( const meas_data_type * matrix , int block_nr) {
  return vector_iget_const( matrix->data , block_nr);
}



matrix_type * meas_data_allocS(const meas_data_type * matrix, int active_size) {
  int obs_offset = 0;
  matrix_type * S  = matrix_alloc( active_size , matrix->ens_size );

  for (int block_nr = 0; block_nr < vector_get_size( matrix->data ); block_nr++) {
    const meas_block_type * meas_block = vector_iget_const( matrix->data , block_nr);
    meas_block_initS( meas_block , S , &obs_offset);
  }

  matrix_set_name( S , "S");
  matrix_assert_finite( S );

  return S;
}



int meas_data_get_nrobs( const meas_data_type * meas_data ) {
  return -1;
}


int meas_data_get_ens_size( const meas_data_type * meas_data ) {
  return meas_data->ens_size;
}


void meas_data_assign_vector(meas_data_type * target_matrix, const meas_data_type * src_matrix , int target_index , int src_index) {
  if (target_matrix->ens_size != src_matrix->ens_size)
    util_abort("%s: size mismatch \n",__func__);
    
  for (int block_nr = 0; block_nr < vector_get_size( target_matrix->data ); block_nr++) {
    meas_block_type * target_block    = meas_data_iget_block( target_matrix , block_nr );
    const meas_block_type * src_block = meas_data_iget_block_const( src_matrix , block_nr );
    
    meas_data_assign_block( target_block , src_block , target_index , src_index );
  }
}




void meas_data_fprintf( const meas_data_type * matrix , FILE * stream ) {
  fprintf(stream , "-----------------------------------------------------------------\n");
  for (int block_nr = 0; block_nr < vector_get_size( matrix->data ); block_nr++) {
    const meas_block_type * block = meas_data_iget_block_const( matrix , block_nr );
    meas_block_fprintf( block , stream );
    fprintf(stream , "\n");
  }
  fprintf(stream , "-----------------------------------------------------------------\n");
}

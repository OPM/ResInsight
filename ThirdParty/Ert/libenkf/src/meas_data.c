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
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/type_vector_functions.h>

#include <ert/enkf/meas_data.h>

#define MEAS_BLOCK_TYPE_ID 661936407
#define MEAS_DATA_TYPE_ID  561000861


struct meas_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                 active_ens_size;
  vector_type       * data;
  pthread_mutex_t     data_mutex;
  hash_type         * blocks;
  bool_vector_type  * ens_mask;
};


struct meas_block_struct {
  UTIL_TYPE_ID_DECLARATION;
  int          active_ens_size;
  int          obs_size;
  int          ens_stride;
  int          obs_stride;
  int          data_size;
  char       * obs_key;
  double     * data;
  bool       * active;
  bool         stat_calculated;
  const bool_vector_type * ens_mask;
  int_vector_type  * index_map;
};


UTIL_SAFE_CAST_FUNCTION( meas_block , MEAS_BLOCK_TYPE_ID )


/**
   Observe that meas_block instance must be allocated with a correct
   value for obs_size; it can not grow during use, and it does also
   not count the number of elements added.

   Observe that the input argument @obs_size should be the total size
   of the observation; if parts of the observation have been excluded
   due to local analysis it should still be included in the @obs_size
   value.
*/

meas_block_type * meas_block_alloc( const char * obs_key , const bool_vector_type * ens_mask , int obs_size) {
  meas_block_type * meas_block = util_malloc( sizeof * meas_block );
  UTIL_TYPE_ID_INIT( meas_block , MEAS_BLOCK_TYPE_ID );
  meas_block->active_ens_size    = bool_vector_count_equal( ens_mask , true );
  meas_block->ens_mask    = ens_mask;
  meas_block->obs_size    = obs_size;
  meas_block->obs_key     = util_alloc_string_copy( obs_key );
  meas_block->data        = util_calloc( (meas_block->active_ens_size + 2)     * obs_size , sizeof * meas_block->data   );
  meas_block->active      = util_calloc(                                  obs_size , sizeof * meas_block->active );
  meas_block->ens_stride  = 1;
  meas_block->obs_stride  = meas_block->active_ens_size + 2;
  meas_block->data_size   = (meas_block->active_ens_size + 2) * obs_size;
  meas_block->index_map   = bool_vector_alloc_active_index_list( meas_block->ens_mask , -1);
  {
    int i;
    for (i=0; i < obs_size; i++)
      meas_block->active[i] = false;
  }
  meas_block->stat_calculated = false;
  return meas_block;
}

static void meas_block_fprintf( const meas_block_type * meas_block , FILE * stream) {
  int iens;
  int iobs;
  for (iobs = 0; iobs < meas_block->obs_size; iobs++) {
    for (iens = 0; iens < meas_block->active_ens_size; iens++) {
      int index = iens * meas_block->ens_stride + iobs * meas_block->obs_stride;
      fprintf(stream , " %10.2f ", meas_block->data[ index ]);
    }
    fprintf(stream , "\n");
  }
}


void meas_block_free( meas_block_type * meas_block ) {
  free( meas_block->obs_key );
  free( meas_block->data );
  free( meas_block->active );
  int_vector_free( meas_block->index_map );
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
      for (int iens =0; iens < meas_block->active_ens_size; iens++) {
        int obs_index = iens * meas_block->ens_stride + iobs* meas_block->obs_stride;

        matrix_iset( S , obs_offset, iens , meas_block->data[ obs_index ]);
      }
      obs_offset++;
    }
  }
  *__obs_offset = obs_offset;
}

bool meas_block_iens_active( const meas_block_type * meas_block , int iens) {
  return bool_vector_iget( meas_block->ens_mask , iens);
}


/*
static void meas_data_assign_block( meas_block_type * target_block , const meas_block_type * src_block , int target_iens , int src_iens ) {
  int iobs;
  for (iobs =0; iobs < target_block->obs_size; iobs++) {
    int target_index = target_iens * target_block->ens_stride + iobs * target_block->obs_stride;
    int src_index    = src_iens * src_block->ens_stride + iobs * src_block->obs_stride;
    target_block->data[ target_index ] = src_block->data[ src_index ];
  }
  target_block->stat_calculated = false;
}
*/

static void meas_block_calculate_ens_stats( meas_block_type * meas_block ) {
  bool include_inactive = true;
  int iobs , iens;
  for (iobs =0; iobs < meas_block->obs_size; iobs++) {
    if (meas_block->active[iobs] || include_inactive) {
      double M1 = 0;
      double M2 = 0;
      for (iens =0; iens < meas_block->active_ens_size; iens++) {
        int index = iens * meas_block->ens_stride + iobs * meas_block->obs_stride;
        M1 += meas_block->data[ index ];
        M2 += meas_block->data[ index ] * meas_block->data[ index ];
      }
      {
        int mean_index = (meas_block->active_ens_size + 0) * meas_block->ens_stride + iobs * meas_block->obs_stride;
        int std_index  = (meas_block->active_ens_size + 1) * meas_block->ens_stride + iobs * meas_block->obs_stride;
        double mean    = M1 / meas_block->active_ens_size;
        double var     = M2 / meas_block->active_ens_size - mean * mean;
        meas_block->data[ mean_index ] = mean;
        meas_block->data[ std_index ]  = sqrt( util_double_max( 0.0 , var));
      }
    }
  }
  meas_block->stat_calculated = true;
}


static void meas_block_assert_ens_stat( meas_block_type * meas_block ) {
  if (!meas_block->stat_calculated)
    meas_block_calculate_ens_stats( meas_block );
}


static void meas_block_assert_iens_active( const meas_block_type * meas_block , int iens) {
  if (!bool_vector_iget( meas_block->ens_mask , iens ))
    util_abort("%s: fatal error - trying to access inactive ensemble member:%d \n",__func__ , iens);
}


void meas_block_iset( meas_block_type * meas_block , int iens , int iobs , double value) {
  meas_block_assert_iens_active( meas_block , iens );
  {
    int active_iens = int_vector_iget( meas_block->index_map , iens );
    int index = active_iens * meas_block->ens_stride + iobs * meas_block->obs_stride;
    meas_block->data[ index ] = value;
    if (!meas_block->active[ iobs ])
      meas_block->active[ iobs ] = true;

    meas_block->stat_calculated = false;
  }
}


double meas_block_iget( const meas_block_type * meas_block , int iens , int iobs) {
  meas_block_assert_iens_active( meas_block , iens );
  {
    int active_iens = int_vector_iget( meas_block->index_map , iens );
    int index = active_iens * meas_block->ens_stride + iobs * meas_block->obs_stride;
    return meas_block->data[ index ];
  }
}


static int meas_block_get_active_obs_size( const meas_block_type * meas_block ) {
  int obs_size = 0;
  int i;

  for (i=0; i < meas_block->obs_size; i++)
    if (meas_block->active[i])
      obs_size++;

  return obs_size;
}


double meas_block_iget_ens_std( meas_block_type * meas_block , int iobs) {
  meas_block_assert_ens_stat( meas_block );
  {
    int std_index  = (meas_block->active_ens_size + 1) * meas_block->ens_stride + iobs * meas_block->obs_stride;
    return meas_block->data[ std_index ];
  }
}


double meas_block_iget_ens_mean( meas_block_type * meas_block , int iobs) {
  meas_block_assert_ens_stat( meas_block );
  {
    int mean_index  = meas_block->active_ens_size * meas_block->ens_stride + iobs * meas_block->obs_stride;
    return meas_block->data[ mean_index ];
  }
}


bool meas_block_iget_active( const meas_block_type * meas_block , int iobs) {
  return meas_block->active[ iobs ];
}


void meas_block_deactivate( meas_block_type * meas_block , int iobs ) {
  if (meas_block->active[ iobs ])
    meas_block->active[ iobs ] = false;
  meas_block->stat_calculated = false;
}


int meas_block_get_total_obs_size( const meas_block_type * meas_block ) {
  return meas_block->obs_size;
}


int meas_block_get_active_ens_size( const meas_block_type * meas_block ) {
  return meas_block->active_ens_size;
}


int meas_block_get_total_ens_size( const meas_block_type * meas_block ) {
  return bool_vector_size( meas_block->ens_mask );
}






/*****************************************************************/

UTIL_IS_INSTANCE_FUNCTION( meas_data , MEAS_DATA_TYPE_ID )

meas_data_type * meas_data_alloc( const bool_vector_type * ens_mask ) {
  meas_data_type * meas = util_malloc(sizeof * meas );
  UTIL_TYPE_ID_INIT( meas , MEAS_DATA_TYPE_ID );

  meas->data         = vector_alloc_new();
  meas->blocks       = hash_alloc();
  meas->ens_mask     = bool_vector_alloc_copy( ens_mask );
  meas->active_ens_size = bool_vector_count_equal( ens_mask , true );
  pthread_mutex_init( &meas->data_mutex , NULL );

  return meas;
}



void meas_data_free(meas_data_type * matrix) {
  vector_free( matrix->data );
  hash_free( matrix->blocks );
  bool_vector_free( matrix->ens_mask );
  free( matrix );
}



void meas_data_reset(meas_data_type * matrix) {
  hash_clear( matrix->blocks );
  vector_clear( matrix->data );  /* Will dump and discard all the meas_block instances. */
}


/*
   The obs_key is not alone unique over different report steps.
*/
static char * meas_data_alloc_key( const char * obs_key , int report_step) {
  return util_alloc_sprintf( "%s-%d" , obs_key , report_step );
}

/**
   The code actually adding new blocks to the vector must be run in single-thread mode.
*/

meas_block_type * meas_data_add_block( meas_data_type * matrix , const char * obs_key , int report_step , int obs_size) {
  char * lookup_key = meas_data_alloc_key( obs_key , report_step );
  pthread_mutex_lock( &matrix->data_mutex );
  {
    if (!hash_has_key( matrix->blocks , lookup_key )) {
      meas_block_type  * new_block = meas_block_alloc(obs_key , matrix->ens_mask , obs_size);
      vector_append_owned_ref( matrix->data , new_block , meas_block_free__ );
      hash_insert_ref( matrix->blocks , lookup_key , new_block );
    }
  }
  pthread_mutex_unlock( &matrix->data_mutex );
  free( lookup_key );
  return vector_get_last( matrix->data );
}


/*
  Observe that the key should compare with the keys created by meas_data_alloc_key().
*/
bool meas_data_has_block( const meas_data_type * matrix , const char * lookup_key) {
  return hash_has_key( matrix->blocks , lookup_key);
}

meas_block_type * meas_data_get_block( const meas_data_type * matrix , const char * lookup_key) {
  return hash_get( matrix->blocks , lookup_key );
}


meas_block_type * meas_data_iget_block( const meas_data_type * matrix , int block_nr) {
  return vector_iget( matrix->data , block_nr);
}


const meas_block_type * meas_data_iget_block_const( const meas_data_type * matrix , int block_nr) {
  return vector_iget_const( matrix->data , block_nr);
}


int meas_data_get_active_obs_size( const meas_data_type * matrix ) {
  int obs_size = 0;

  for (int block_nr = 0; block_nr < vector_get_size( matrix->data ); block_nr++) {
    const meas_block_type * meas_block = vector_iget_const( matrix->data , block_nr);
    obs_size += meas_block_get_active_obs_size( meas_block );
  }

  return obs_size;
}



/*
  Observe that this can return NULL is there is no data/observations.
*/

matrix_type * meas_data_allocS(const meas_data_type * matrix) {
  int obs_offset  = 0;
  matrix_type * S = matrix_alloc( meas_data_get_active_obs_size( matrix ) , matrix->active_ens_size);
  if (S) {
    for (int block_nr = 0; block_nr < vector_get_size( matrix->data ); block_nr++) {
      const meas_block_type * meas_block = vector_iget_const( matrix->data , block_nr);
      meas_block_initS( meas_block , S , &obs_offset);
    }

    matrix_set_name( S , "S");
    matrix_assert_finite( S );
  }
  return S;
}



int meas_data_get_nrobs( const meas_data_type * meas_data ) {
  return -1;
}


int meas_data_get_active_ens_size( const meas_data_type * meas_data ) {
  return meas_data->active_ens_size;
}


int meas_data_get_total_ens_size( const meas_data_type * meas_data ) {
  return bool_vector_size( meas_data->ens_mask );
}


int meas_data_get_num_blocks( const meas_data_type * meas_data ) {
  return vector_get_size( meas_data->data );
}



/*
void meas_data_assign_vector(meas_data_type * target_matrix, const meas_data_type * src_matrix , int target_index , int src_index) {
  if (target_matrix->active_ens_size != src_matrix->active_ens_size)
    util_abort("%s: size mismatch \n",__func__);

  for (int block_nr = 0; block_nr < vector_get_size( target_matrix->data ); block_nr++) {
    meas_block_type * target_block    = meas_data_iget_block( target_matrix , block_nr );
    const meas_block_type * src_block = meas_data_iget_block_const( src_matrix , block_nr );

    meas_data_assign_block( target_block , src_block , target_index , src_index );
  }
}
*/



void meas_data_fprintf( const meas_data_type * matrix , FILE * stream ) {
  fprintf(stream , "-----------------------------------------------------------------\n");
  for (int block_nr = 0; block_nr < vector_get_size( matrix->data ); block_nr++) {
    const meas_block_type * block = meas_data_iget_block_const( matrix , block_nr );
    meas_block_fprintf( block , stream );
    fprintf(stream , "\n");
  }
  fprintf(stream , "-----------------------------------------------------------------\n");
}

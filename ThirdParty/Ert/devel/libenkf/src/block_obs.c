/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'block_obs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
   See the overview documentation of the observation system in enkf_obs.c
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/smspec_node.h>

#include <ert/enkf/enkf_util.h>
#include <ert/enkf/field.h>
#include <ert/enkf/summary.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/container.h>
#include <ert/enkf/container_config.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/active_list.h>
#include <ert/enkf/block_obs.h> 
#include <ert/enkf/enkf_defaults.h>

#define BLOCK_OBS_TYPE_ID 661098
#define POINT_OBS_TYPE_ID 778196


typedef struct  {
  UTIL_TYPE_ID_DECLARATION;
  block_obs_source_type   source_type;
  int                     i;
  int                     j;
  int                     k;
  int                     active_index;
  double                  value;
  double                  std;
  char                  * sum_key;     
} point_obs_type; 



struct block_obs_struct {
  UTIL_TYPE_ID_DECLARATION;
  char   * obs_key;                       /** A user provided label for the observation.      */
  int      size;                          /** The number of field cells observed.             */
  point_obs_type         ** point_list;
  const ecl_grid_type     * grid;
  const void              * data_config;
  block_obs_source_type     source_type;  
};


static UTIL_SAFE_CAST_FUNCTION_CONST(block_obs , BLOCK_OBS_TYPE_ID);
static UTIL_SAFE_CAST_FUNCTION(block_obs , BLOCK_OBS_TYPE_ID);
UTIL_IS_INSTANCE_FUNCTION(block_obs , BLOCK_OBS_TYPE_ID);


/*****************************************************************/





static point_obs_type * point_obs_alloc( block_obs_source_type   source_type , int i , int j , int k , int active_index , const char * sum_key , double value , double std) {
  point_obs_type * point_obs = util_malloc( sizeof * point_obs );
  UTIL_TYPE_ID_INIT( point_obs , POINT_OBS_TYPE_ID );

  point_obs->source_type  = source_type;
  point_obs->i            = i;
  point_obs->j            = j;
  point_obs->k            = k;       
  point_obs->active_index = active_index;
  point_obs->value        = value;
  point_obs->std          = std;
  point_obs->sum_key      = util_alloc_string_copy( sum_key );
  
  
  return point_obs;
}




static void point_obs_free( point_obs_type * point_obs ) {
  util_safe_free( point_obs->sum_key );
  free( point_obs );
}


static double point_obs_measure( const point_obs_type * point_obs , const void * state , int iobs , node_id_type node_id) {
  if (point_obs->source_type == SOURCE_FIELD) {
    const field_type * field = field_safe_cast_const( state );
    return field_iget_double(field , point_obs->active_index);
  } else if (point_obs->source_type == SOURCE_SUMMARY) {
    const container_type * container = container_safe_cast_const( state );
    const summary_type * summary = summary_safe_cast_const( container_iget_node( container , iobs ));
    return summary_get( summary , node_id.report_step , node_id.state );    
  } else {
    util_abort("%s: unknown source type: %d \n",__func__, point_obs->source_type );
    return -1;
  }
}


/*****************************************************************/



static void block_obs_validate_ijk( const ecl_grid_type * grid , int size, const int * i , const int * j , const int * k) {
  int l;
  for (l = 0; l < size; l++) {
    if (ecl_grid_ijk_valid(grid , i[l] , j[l] , k[l])) {
      int active_index = ecl_grid_get_active_index3( grid , i[l] , j[l] , k[l]);
      if (active_index < 0) 
        util_abort("%s: sorry: cell:(%d,%d,%d) is not active - can not observe it. \n",__func__ , i[l]+1 , j[l]+1 , k[l]+1);
      
    } else
      util_abort("%s: sorry: cell (%d,%d,%d) is outside valid range:  \n",__func__ , i[l]+1 , j[l]+1 , k[l]+1);
  }
}

static void block_obs_resize( block_obs_type * block_obs , int new_size) {
  int i;
  block_obs->point_list = util_realloc( block_obs->point_list , new_size * sizeof * block_obs->point_list );

  for (i=block_obs->size; i < new_size; i++)
    block_obs->point_list[i] = NULL;

  block_obs->size = new_size;
}

/**
   The input vectors i,j,k should contain offset zero values.
*/
block_obs_type * block_obs_alloc(const char   * obs_key,
                                 block_obs_source_type source_type , 
                                 const stringlist_type * summary_keys , 
                                 const void * data_config , 
                                 const ecl_grid_type * grid ,
                                 int            size,
                                 const int    * i,
                                 const int    * j,
                                 const int    * k,
                                 const double * obs_value,
                                 const double * obs_std)
{
  block_obs_validate_ijk( grid , size , i,j,k);
  
  {
    block_obs_type * block_obs = util_malloc(sizeof * block_obs);

    UTIL_TYPE_ID_INIT( block_obs , BLOCK_OBS_TYPE_ID );
    block_obs->obs_key         = util_alloc_string_copy(obs_key);
    block_obs->data_config     = data_config;
    block_obs->source_type     = source_type; 
    block_obs->size            = 0;
    block_obs->point_list      = NULL;
    block_obs->grid            = grid;
    block_obs_resize( block_obs , size );
    
    {
      for (int l=0; l < size; l++) {
        int active_index = ecl_grid_get_active_index3( block_obs->grid , i[l],j[l],k[l]);
        const char * sum_key   = NULL;
        if (source_type == SOURCE_SUMMARY) 
          sum_key = stringlist_iget( summary_keys , l );
        
        block_obs->point_list[l] = point_obs_alloc(source_type , i[l] , j[l] , k[l] , active_index , sum_key , obs_value[l] , obs_std[l]);
      }
    }
    return block_obs;
  }
}



void block_obs_free( block_obs_type * block_obs) {
  for (int i=0; i < block_obs->size; i++) {
    if (block_obs->point_list[i] != NULL)
      point_obs_free( block_obs->point_list[i]);
  }
  
  util_safe_free(block_obs->point_list );
  free(block_obs->obs_key);
  free(block_obs);
}







void block_obs_get_observations(const block_obs_type * block_obs,  obs_data_type * obs_data,  int report_step , const active_list_type * __active_list) {
  int i;
  int active_size              = active_list_get_active_size( __active_list , block_obs->size );
  active_mode_type active_mode = active_list_get_mode( __active_list );
  obs_block_type * obs_block   = obs_data_add_block( obs_data , block_obs->obs_key , block_obs->size , NULL , false );
  
  if (active_mode == ALL_ACTIVE) {
    for (i=0; i < block_obs->size; i++) {
      const point_obs_type * point_obs = block_obs->point_list[i];
      obs_block_iset(obs_block , i , point_obs->value , point_obs->std );
    }
  } else if (active_mode == PARTLY_ACTIVE) {
    const int   * active_list    = active_list_get_active( __active_list ); 
    for (i =0 ; i < active_size; i++) {
      int iobs = active_list[i];
      const point_obs_type * point_obs = block_obs->point_list[iobs];
      obs_block_iset(obs_block , iobs , point_obs->value , point_obs->std );
    }
  }
}


static void block_obs_assert_data( const block_obs_type * block_obs , const void * state ) {
  if (block_obs->source_type == SOURCE_FIELD) {
    if (!field_is_instance( state ))
      util_abort("%s: state data is not of type FIELD - aborting \n",__func__);
  } else if (block_obs->source_type == SOURCE_SUMMARY) {
    if (!container_is_instance( state ))
      util_abort("%s: state data is not of type CONTAINER - aborting \n",__func__);
  }
}


void block_obs_measure(const block_obs_type * block_obs, const void * state , node_id_type node_id , meas_data_type * meas_data , const active_list_type * __active_list) {
  block_obs_assert_data( block_obs , state );
  {
    int active_size = active_list_get_active_size( __active_list , block_obs->size );
    meas_block_type * meas_block = meas_data_add_block( meas_data , block_obs->obs_key , node_id.report_step , block_obs->size );
    int iobs;
    
    active_mode_type active_mode = active_list_get_mode( __active_list );
    if (active_mode == ALL_ACTIVE) {
      for (iobs=0; iobs < block_obs->size; iobs++) {
        const point_obs_type * point_obs = block_obs->point_list[iobs];
        double value = point_obs_measure( point_obs , state , iobs , node_id);
        meas_block_iset( meas_block , node_id.iens , iobs , value );
      }
    } else if (active_mode == PARTLY_ACTIVE) {
      const int   * active_list    = active_list_get_active( __active_list ); 
      for (int i =0 ; i < active_size; i++) {
        iobs = active_list[i];
        {
          const point_obs_type * point_obs = block_obs->point_list[iobs];
          double value = point_obs_measure( point_obs , state , iobs , node_id);
          meas_block_iset( meas_block , node_id.iens , point_obs->active_index , value );
        }
      }
    }
  }
}



double block_obs_chi2(const block_obs_type * block_obs,  const field_type * field_state, node_id_type node_id) {
  double sum_chi2 = 0;
  for (int i=0; i < block_obs->size; i++) {
    const point_obs_type * point_obs = block_obs->point_list[i];
    double sim_value = point_obs_measure( point_obs , field_state , i, node_id );
    double x = (sim_value - point_obs->value) / point_obs->std;
    sum_chi2 += x*x;
  }
  return sum_chi2;
}




/**
   The index is into the the number of active cells which are observed by this observation.
*/
void block_obs_iget(const block_obs_type * block_obs, int index , double *value , double * std) {
  const point_obs_type * point_obs = block_obs->point_list[index];
  *value = point_obs->value;
  *std   = point_obs->std;
}


void block_obs_user_get(const block_obs_type * block_obs , const char * index_key , double *value , double * std, bool * valid) {
  int      i,j,k;

  *valid = false;
  if (field_config_parse_user_key__( index_key , &i , &j , &k)) {
    int active_index = ecl_grid_get_active_index3(block_obs->grid , i,j,k);
    int l = 0;
    /* iterating through all the cells the observation is observing. */

    while (!(*valid) && l < block_obs->size) {
      const point_obs_type * point_obs = block_obs->point_list[l];
      if (point_obs->active_index == active_index) {
        *value = point_obs->value;
        *std   = point_obs->std;
        *valid = true;
      }
      l++;
    }
  }
}




int block_obs_iget_i(const block_obs_type * block_obs, int index) {
  const point_obs_type * point_obs = block_obs->point_list[index];
  return point_obs->i;
}

int block_obs_iget_j(const block_obs_type * block_obs, int index) {
  const point_obs_type * point_obs = block_obs->point_list[index];
  return point_obs->j;
}

int block_obs_iget_k(const block_obs_type * block_obs, int index) {
  const point_obs_type * point_obs = block_obs->point_list[index];
  return point_obs->k;
}


/*
  Returns by reference i,j,k for observation point nr block_nr.
*/

void block_obs_iget_ijk(const block_obs_type * block_obs , int block_nr , int * i , int * j , int * k) {
  const point_obs_type * point_obs = block_obs->point_list[block_nr];
  *i = point_obs->i;
  *j = point_obs->j;
  *k = point_obs->k;
}


int block_obs_get_size(const block_obs_type * block_obs) {
  return block_obs->size;
}

void block_obs_scale_std(block_obs_type * block_obs, double scale_factor) {
  for (int i = 0; i < block_obs->size; i++) {
    if (block_obs->point_list[i] != NULL) {
      point_obs_type * point_observation = block_obs->point_list[i];
      point_observation->std = point_observation->std * scale_factor;
    }
  }
}

void block_obs_scale_std__(void * block_obs, double scale_factor) {
  block_obs_type * observation = block_obs_safe_cast(block_obs);
  block_obs_scale_std(observation, scale_factor); 
}


/*****************************************************************/

VOID_FREE(block_obs)
VOID_GET_OBS(block_obs)
VOID_MEASURE_UNSAFE(block_obs , data)  // The cast of data field is not checked - that is done in block_obs_measure().
VOID_USER_GET_OBS(block_obs)
VOID_CHI2(block_obs , field)

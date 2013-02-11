/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_obs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
   See the overview documentation of the observation system in
   enkf_obs.c
*/
#include <stdlib.h>

#include <ert/util/util.h>

#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/gen_obs.h>
#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/gen_data.h>
#include <ert/enkf/gen_obs.h>
#include <ert/enkf/gen_common.h>
#include <ert/enkf/active_list.h>

/**
   This file implemenets a structure for general observations. A
   general observation is just a vector of numbers - where EnKF has no
   understanding whatsover of the type of these data. The actual data
   is supposed to be found in a file.
   
   Currently it can only observe gen_data instances - but that should
   be generalized.
*/



#define GEN_OBS_TYPE_ID 77619

struct gen_obs_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                          obs_size;         /* This is the total size of the observation vector. */ 
  int                        * data_index_list;  /* The indexes which are observed in the corresponding gen_data instance - of length obs_size. */
  bool                         observe_all_data; /* Flag which indiactes whether all data in the gen_data instance should be observed - in that case we must do a size comparizon-check at use time. */

  double                     * obs_data;         /* The observed data. */
  double                     * obs_std;          /* The observed standard deviation. */ 

  char                       * obs_key;          /* The key this observation is held by - in the enkf_obs structur (only for debug messages). */  
  char                       * obs_file;         /* The file holding the observation. */ 
  gen_data_file_format_type    obs_format;       /* The format, i.e. ASCII, binary_double or binary_float, of the observation file. */
  matrix_type                * error_covar;
  gen_data_config_type       * data_config;
};

/******************************************************************/


static UTIL_SAFE_CAST_FUNCTION_CONST(gen_obs , GEN_OBS_TYPE_ID)
static UTIL_SAFE_CAST_FUNCTION(gen_obs , GEN_OBS_TYPE_ID)

void gen_obs_free(gen_obs_type * gen_obs) {
  util_safe_free(gen_obs->obs_data);
  util_safe_free(gen_obs->obs_std);
  util_safe_free(gen_obs->obs_file);
  util_safe_free(gen_obs->data_index_list);
  util_safe_free(gen_obs->obs_key);
  if (gen_obs->error_covar != NULL)
    matrix_free( gen_obs->error_covar );
  
  free(gen_obs);
}



/**
   This function loads the actual observations from disk, and
   initializes the obs_data and obs_std pointers with the
   observations. It also sets the obs_size field of the gen_obs
   instance.

   The file with observations should be a long vector of 2N elements,
   where the first N elements are data values, and the last N values
   are the corresponding standard deviations.
   
   The file is loaded with the gen_common_fload_alloc() function, and
   can be in formatted ASCII or binary_float / binary_double. Observe
   that there is *NO* header information in this file.
*/


static void gen_obs_load_observation(gen_obs_type * gen_obs, double scalar_value , double scalar_error) {
  ecl_type_enum load_type;
  void * buffer;
  
  gen_obs->obs_size = 0;
  if (gen_obs->obs_file != NULL)
    buffer = gen_common_fload_alloc(gen_obs->obs_file , gen_obs->obs_format , ECL_DOUBLE_TYPE , &load_type , &gen_obs->obs_size);
  else {
    double * double_buffer = util_calloc(2 , sizeof * double_buffer );
    buffer = double_buffer;
    double_buffer[0] = scalar_value;
    double_buffer[1] = scalar_error;
    load_type         = ECL_DOUBLE_TYPE;
    gen_obs->obs_size = 2;
  }
  
  /** Ensure that the data is of type double. */
  if (load_type == ECL_FLOAT_TYPE) {
    double * double_data = util_calloc(gen_obs->obs_size , sizeof * double_data );
    util_float_to_double(double_data , (const float *) buffer , gen_obs->obs_size);
    free(buffer);
    buffer = double_data;
  }
  
  gen_obs->obs_size /= 2; /* Originally contains BOTH data and std. */
  gen_obs->obs_data = util_realloc(gen_obs->obs_data , gen_obs->obs_size * sizeof * gen_obs->obs_data );
  gen_obs->obs_std  = util_realloc(gen_obs->obs_std  , gen_obs->obs_size * sizeof * gen_obs->obs_std  );
  {
    int iobs;
    double * double_buffer = (double * ) buffer;
    for (iobs = 0; iobs < gen_obs->obs_size; iobs++) {
      gen_obs->obs_data[iobs] =  double_buffer[2*iobs];
      gen_obs->obs_std[iobs]  =  double_buffer[2*iobs + 1];
    }
     
  } 
  free(buffer);
}





/**
   data_index_file is the name of a file with indices which should be
   observed, data_inde_string is the same, in the form of a
   "1,2,3,4-10, 17,19,22-100" string. Only one of these items can be
   != NULL. If both are NULL it is assumed that all the indices of the
   gen_data instance should be observed.

   @error_covar_file is the name of file which contains a matrix of
   error-covariance. The file data will be read with the function
   matrix_fscanf_data(), i.e. it should consist of formatted
   numbers. Since the matrix is symmetric it does not matter whether
   it is represented in row-major or column-major order; newlines for
   pretty reading can be inserted but are not necessary.

   The error_covar_file should contain NO header information.
*/


gen_obs_type * gen_obs_alloc(const gen_data_config_type * data_config , const char * obs_key , const char * obs_file , double scalar_value , double scalar_error , const char * data_index_file , const char * data_index_string , const char * error_covar_file) {
  gen_obs_type * obs = util_malloc(sizeof * obs);
  
  UTIL_TYPE_ID_INIT( obs , GEN_OBS_TYPE_ID );
  obs->obs_data         = NULL;
  obs->obs_std          = NULL;
  obs->obs_file         = util_alloc_string_copy( obs_file );
  obs->obs_format       = ASCII;  /* Hardcoded for now. */
  obs->obs_key          = util_alloc_string_copy( obs_key );   
  obs->data_config      = data_config;

  gen_obs_load_observation(obs , scalar_value , scalar_error); /* The observation data is loaded - and internalized at boot time - even though it might not be needed for a long time. */
  if ((data_index_file == NULL) && (data_index_string == NULL)) {
    /* 
       We observe all the elements in the remote (gen_data) instance,
       and the data_index_list just becomes a identity mapping. 
       
       At use time we must verify that the size of the observation
       corresponds to the size of the gen_data_instance; that this
       check is needed is indicated by the boolean flag
       observe_all_data.
    */
    obs->data_index_list = util_calloc( obs->obs_size , sizeof * obs->data_index_list );
    for (int i =0; i < obs->obs_size; i++)
      obs->data_index_list[i] = i;
    obs->observe_all_data = true;
  } else {
    obs->observe_all_data = false;
    if (data_index_file != NULL) 
      /* Parsing an a file with integers. */
      obs->data_index_list = gen_common_fscanf_alloc( data_index_file , ECL_INT_TYPE , &obs->obs_size);
    else   
      /* Parsing a string of the type "1,3,5,9-100,200,202,300-1000" */
      obs->data_index_list = util_sscanf_alloc_active_list(data_index_string , &obs->obs_size);
  }
  
  if (error_covar_file != NULL) {
    FILE * stream = util_fopen( error_covar_file , "r");
    
    obs->error_covar = matrix_alloc( obs->obs_size , obs->obs_size );
    matrix_fscanf_data( obs->error_covar , false , stream );

    fclose( stream );
  } else
    obs->error_covar = NULL;

  return obs;
}



static void gen_obs_assert_data_size(const gen_obs_type * gen_obs, const gen_data_type * gen_data) {
  if (gen_obs->observe_all_data) {
    int data_size = gen_data_get_size( gen_data );
    if (gen_obs->obs_size != data_size) 
      util_abort("%s: size mismatch: Observation: %s:%d      Data: %s:%d \n" , __func__ , gen_obs->obs_key , gen_obs->obs_size , gen_data_get_key( gen_data ) , data_size);
    
  } 
  /*
    Else the user has explicitly entered indices to observe in the
    gen_data instances, and we just have to trust them (however the
    gen_data_iget() does a range check. 
  */
}


double gen_obs_chi2(const gen_obs_type * gen_obs , const gen_data_type * gen_data, node_id_type node_id) {
  gen_obs_assert_data_size(gen_obs , gen_data);
  {
    double sum_chi2 = 0;
    for (int iobs = 0; iobs < gen_obs->obs_size; iobs++) {
      double x  = (gen_data_iget_double( gen_data , gen_obs->data_index_list[iobs]) - gen_obs->obs_data[iobs]) / gen_obs->obs_std[iobs];
      sum_chi2 += x*x;
    }
    return sum_chi2;
  }
}



void gen_obs_measure(const gen_obs_type * gen_obs , const gen_data_type * gen_data , node_id_type node_id , meas_data_type * meas_data, const active_list_type * __active_list) {
  gen_obs_assert_data_size(gen_obs , gen_data);
  {
    int active_size                               = active_list_get_active_size( __active_list , gen_obs->obs_size );
    meas_block_type * meas_block                  = meas_data_add_block( meas_data , gen_obs->obs_key , node_id.report_step , active_size );
    active_mode_type active_mode                  = active_list_get_mode( __active_list );
    const bool_vector_type * forward_model_active = gen_data_config_get_active_mask( gen_obs->data_config );
    
    int iobs;
    if (active_mode == ALL_ACTIVE) {
      for (iobs = 0; iobs < gen_obs->obs_size; iobs++) {
        int data_index = gen_obs->data_index_list[iobs] ;

        if (forward_model_active != NULL) {
          if (!bool_vector_iget( forward_model_active , data_index ))
            continue;  /* Forward model has deactivated this index - just continue. */
        }

        meas_block_iset( meas_block , node_id.iens , iobs , gen_data_iget_double( gen_data , data_index ));
      }
    } else if ( active_mode == PARTLY_ACTIVE) {
      const int   * active_list    = active_list_get_active( __active_list ); 
      int index;
      
      for (index = 0; index < active_size; index++) {
        iobs = active_list[ index ];
        int data_index = gen_obs->data_index_list[iobs] ;
        if (forward_model_active != NULL) {
          if (!bool_vector_iget( forward_model_active , data_index ))
            continue;  /* Forward model has deactivated this index - just continue. */
        }
        meas_block_iset( meas_block , node_id.iens , iobs , gen_data_iget_double( gen_data , gen_obs->data_index_list[iobs] ));
      }
    }
  }
}



void gen_obs_get_observations(gen_obs_type * gen_obs , obs_data_type * obs_data, int report_step , const active_list_type * __active_list) {
  gen_data_config_load_active( gen_obs->data_config , report_step , true);
  {
    int iobs;
    active_mode_type active_mode                  = active_list_get_mode( __active_list );
    obs_block_type * obs_block                    = obs_data_add_block( obs_data , gen_obs->obs_key , gen_obs->obs_size , NULL , false);
    const bool_vector_type * forward_model_active = gen_data_config_get_active_mask( gen_obs->data_config );
    
    if (active_mode == ALL_ACTIVE) {
      for (iobs = 0; iobs < gen_obs->obs_size; iobs++) 
        obs_block_iset( obs_block , iobs , gen_obs->obs_data[iobs] , gen_obs->obs_std[iobs]);
      
      /* Setting some of the elements as missing, i.e. deactivated by the forward model. */
      if (forward_model_active != NULL) { 
        for (iobs = 0; iobs < gen_obs->obs_size; iobs++) {
          int data_index = gen_obs->data_index_list[ iobs ];
          if (!bool_vector_iget( forward_model_active , data_index ))
            obs_block_iset_missing( obs_block , iobs );
        }
      }
    } else if (active_mode == PARTLY_ACTIVE) {
      const int   * active_list    = active_list_get_active( __active_list ); 
      int active_size              = active_list_get_active_size( __active_list , gen_obs->obs_size);
      int index;
      
      for (index = 0; index < active_size; index++) {
        iobs = active_list[index];
        obs_block_iset( obs_block , iobs , gen_obs->obs_data[iobs] , gen_obs->obs_std[iobs] );
        {
          int data_index = gen_obs->data_index_list[ iobs ];
          if ((forward_model_active != NULL) && (!bool_vector_iget( forward_model_active , data_index ))) 
            obs_block_iset_missing( obs_block , iobs );
        }
      }
    }
  }
}








/**
   In general the gen_obs observation vector can be smaller than the
   gen_data field it is observing, i.e. we can have a situation like
   this:

           Data               Obs
           ----               ---

          [ 6.0 ] ----\
          [ 2.0 ]      \---> [ 6.3 ]    
          [ 3.0 ] ---------> [ 2.8 ]   
          [ 2.0 ]      /---> [ 4.3 ]
          [ 4.5 ] ----/

   The situation here is as follows:

   1. We have a gen data vector with five elements.

   2. We have an observation vector of three elements, which observes
      three of the elements in the gen_data vector, in this particular
      case the data_index_list of the observation equals: [0 , 2 , 4].

   Now when we want to look at the match of observation quality of the
   last element in the observation vector it would be natural to use
   the user_get key: "obs_key:2" - however this is an observation of
   data element number 4, i.e. as seen from data context (when adding
   observations to an ensemble plot) the natural indexing would be:
   "data_key:4".


   The function gen_obs_user_get_with_data_index() will do the
   translation from data based indexing to observation based indexing, i.e.
   
      gen_obs_user_get_with_data_index("obs_key:4") 

   will do an inverse lookup of the '4' and further call

      gen_obs_user_get("obs_key:2")

*/
          

void gen_obs_user_get(const gen_obs_type * gen_obs , const char * index_key , double * value , double * std , bool * valid) { 
  int index; 
  *valid = false;

  if (util_sscanf_int( index_key , &index)) {
    if ((index >= 0) && (index < gen_obs->obs_size)) {
      *valid = true;
      *value = gen_obs->obs_data[ index ];
      *std   = gen_obs->obs_std[ index ];
    }
  }
}


void gen_obs_user_get_with_data_index(const gen_obs_type * gen_obs , const char * index_key , double * value , double * std , bool * valid) { 
  if (gen_obs->observe_all_data)
    /* The observation and data vectors are equally long - no reverse lookup necessary. */
    gen_obs_user_get(gen_obs , index_key , value , std , valid);
  else {
    *valid = false;
    int data_index;
    if (util_sscanf_int( index_key , &data_index )) {
      int obs_index      =  0; 
      do {
        if (gen_obs->data_index_list[ obs_index ] == data_index) 
          /* Found it - will use the 'obs_index' value. */
          break;
        
        obs_index++;
      } while (obs_index < gen_obs->obs_size);
      if (obs_index < gen_obs->obs_size) { /* The reverse lookup succeeded. */
        *valid = true;
        *value = gen_obs->obs_data[ obs_index ];
        *std   = gen_obs->obs_std[ obs_index ];
      }
    }
  }
}



  
/*****************************************************************/
UTIL_IS_INSTANCE_FUNCTION(gen_obs , GEN_OBS_TYPE_ID)
VOID_FREE(gen_obs)
VOID_GET_OBS(gen_obs)
VOID_MEASURE(gen_obs , gen_data)
VOID_USER_GET_OBS(gen_obs)
VOID_CHI2(gen_obs , gen_data)

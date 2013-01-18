/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'summary.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <math.h>
#include <string.h>

#include <util.h>
#include <log.h>
#include <double_vector.h>

#include <ecl_sum.h>
#include <ecl_smspec.h> 
#include <ecl_file.h>

#include <enkf_types.h>
#include <enkf_util.h>
#include <enkf_serialize.h>
#include <enkf_macros.h>
#include <enkf_util.h>
#include <summary.h>
#include <summary_config.h>

/*****************************************************************/

#define SUMMARY_UNDEF_FORECAST -9999
#define SUMMARY_UNDEF_ANALYZED -999

struct summary_struct {
  int                          __type_id;         /* Only used for run_time checking. */
  bool                         vector_storage; 
  summary_config_type        * config;            /* Can not be NULL - var_type is set on first load. */
  double                     * data;              /* Size is always one - but what the fuck ... */
  double_vector_type         * forecast_vector;
  double_vector_type         * analyzed_vector;
};


/*****************************************************************/

double_vector_type * SELECT_VECTOR(const summary_type * summary , state_enum state) {
  if (state == FORECAST)
    return summary->forecast_vector;
  else if (state == ANALYZED)
    return summary->analyzed_vector;
  else {
    util_abort("%s: invalid state value:%d \n",__func__ , state);
    return NULL;
  }
}


static double SUMMARY_GET_VALUE( const summary_type * summary , node_id_type node_id) {
  if (summary->vector_storage) {
    double_vector_type * storage_vector = SELECT_VECTOR( summary , node_id.state );
    return double_vector_iget( storage_vector , node_id.report_step );
  } else
    return summary->data[0];
}


static void SUMMARY_SET_VALUE( summary_type * summary , node_id_type node_id , double value) {
  if (summary->vector_storage) {
    double_vector_type * storage_vector = SELECT_VECTOR( summary , node_id.state );
    double_vector_iset( storage_vector , node_id.report_step , value);
  } else
    summary->data[0] = value;
}

/*****************************************************************/



void summary_clear(summary_type * summary) {
  const int data_size = summary_config_get_data_size( summary->config );
  for (int k=0; k < data_size; k++)
    summary->data[k] = 0;

  double_vector_reset( summary->forecast_vector );
  double_vector_reset( summary->analyzed_vector );
}



summary_type * summary_alloc(const summary_config_type * summary_config) {
  summary_type * summary   = util_malloc(sizeof *summary );
  summary->__type_id       = SUMMARY;
  summary->vector_storage  = summary_config_get_vector_storage( summary_config );
  summary->config          = (summary_config_type *) summary_config;
  summary->forecast_vector = double_vector_alloc(0 , SUMMARY_UNDEF_FORECAST);  
  summary->analyzed_vector = double_vector_alloc(0 , SUMMARY_UNDEF_ANALYZED);  
  {
    const int data_size = summary_config_get_data_size( summary_config );
    summary->data       = util_calloc( data_size , sizeof * summary->data );
  }
  return summary;
}




void summary_copy(const summary_type *src , summary_type * target) {
  if (src->config == target->config) {
    const int data_size = summary_config_get_data_size( src->config );
    for (int k=0; k < data_size; k++)
      target->data[k] = src->data[k];
    
    double_vector_memcpy( target->forecast_vector , src->forecast_vector );
    double_vector_memcpy( target->analyzed_vector , src->analyzed_vector );
  } else
    util_abort("%s: do not share config objects \n",__func__);
}




void summary_read_from_buffer(summary_type * summary , buffer_type * buffer, int report_step, state_enum state) {
  enkf_util_assert_buffer_type( buffer , SUMMARY );
  if (summary->vector_storage) {
    double_vector_type * storage_vector = SELECT_VECTOR( summary , state );
    double_vector_buffer_fread( storage_vector , buffer );
  } else {
    int  size = summary_config_get_data_size( summary->config );
    buffer_fread( buffer , summary->data , sizeof * summary->data , size);
  }
}


bool summary_write_to_buffer(const summary_type * summary , buffer_type * buffer, int report_step , state_enum state) {
  buffer_fwrite_int( buffer , SUMMARY );
  if (summary->vector_storage) {
    double_vector_type * storage_vector = SELECT_VECTOR( summary , state );
    double_vector_buffer_fwrite( storage_vector , buffer );
  } else {
    int  size = summary_config_get_data_size( summary->config );
    buffer_fwrite( buffer , summary->data , sizeof * summary->data , size);
  }
  return true;
}


bool summary_has_data( const summary_type * summary , int report_step , state_enum state) {
  if (summary->vector_storage) {
    double_vector_type * storage_vector = SELECT_VECTOR( summary , state );
    return (double_vector_size( storage_vector ) > report_step) ? true : false;
  } else 
    return true; 
}


void summary_free(summary_type *summary) {
  double_vector_free( summary->forecast_vector );
  double_vector_free( summary->analyzed_vector );
  free(summary->data);
  free(summary);
}






void summary_serialize(const summary_type * summary , node_id_type node_id , const active_list_type * active_list , matrix_type * A , int row_offset , int column) {
  double value = SUMMARY_GET_VALUE( summary , node_id );
  enkf_matrix_serialize( &value , 1 , ECL_DOUBLE_TYPE , active_list , A , row_offset , column);
}


void summary_deserialize(summary_type * summary , node_id_type node_id , const active_list_type * active_list , const matrix_type * A , int row_offset , int column) {
  double value;
  enkf_matrix_deserialize( &value , 1 , ECL_DOUBLE_TYPE , active_list , A , row_offset , column);
  SUMMARY_SET_VALUE( summary , node_id , value );
}


double summary_get(const summary_type * summary, int report_step , state_enum state) {
  node_id_type node_id = {.report_step = report_step , .state = state , .iens = -1};
  return SUMMARY_GET_VALUE( summary , node_id );
}


bool summary_user_get(const summary_type * summary , const char * index_key , int report_step , state_enum state, double * value) {
  if (summary->vector_storage) {
    double_vector_type * vector = SELECT_VECTOR( summary , state );
    
    if (double_vector_size( vector ) > report_step) {
      *value = double_vector_iget( vector , report_step);
      return true;
    } else {
      *value = -1;
      return false;
    }
    
  } else {
    *value = summary->data[0];
    return true;
  }
}



void summary_user_get_vector(const summary_type * summary , const char * index_key , state_enum state, double_vector_type * value) {
  if (summary->vector_storage) {
    double_vector_type * vector = SELECT_VECTOR( summary , state );
    double_vector_memcpy( value , vector );
  } else 
    util_abort("%s: internal error - should not call the %s function when not using vector storage \n",__func__ , __func__);
}



/**
   There are three typical reasons why the node data can not be loaded:

     1. The ecl_sum instance is equal to NULL.
     2. The ecl_sum instance does not have the report step we are asking for.
     3. The ecl_sum instance does not have the variable we are asking for.

   In the two first cases the function will return false, ultimately
   signaling that the simulation has failed. In the last case we check
   the required flag of the variable, and if this is set to false we
   return true. This is done because this is a typical situation for
   e.g. a well which has not yet opened.  
*/

bool summary_forward_load(summary_type * summary , const char * ecl_file_name , const ecl_sum_type * ecl_sum, const ecl_file_type * ecl_file , int report_step) {
  bool loadOK = false;
  double load_value;
  if (ecl_sum != NULL) {
    const char * var_key               = summary_config_get_var(summary->config);
    load_fail_type load_fail_action    = summary_config_get_load_fail_mode(summary->config );

    /* Check if the ecl_sum instance has this report step. */
    if (ecl_sum_has_report_step( ecl_sum , report_step )) {
      int last_report_index = ecl_sum_iget_report_end( ecl_sum , report_step );

      if (ecl_sum_has_general_var(ecl_sum , var_key)) {
        load_value = ecl_sum_get_general_var(ecl_sum , last_report_index  ,var_key );
        loadOK = true;
      } else {
        load_value = 0;
        /* 
           The summary object does not have this variable - probably
           meaning that it is a well/group which has not yet
           opened. When required == false we do not signal load
           failure in this situation.
           
           If the user has misspelled the name, we will go through
           the whole simulation without detecting that error.
        */
        if (load_fail_action == LOAD_FAIL_EXIT)
          loadOK = false;
        else {
          loadOK = true;
          if (load_fail_action == LOAD_FAIL_WARN)
            fprintf(stderr,"** WARNING ** Failed summary:%s does not have key:%s \n",ecl_sum_get_case( ecl_sum ) , var_key);
        }
      } 
    } else {
      load_value = 0;
      if (report_step == 0) 
        loadOK = true;  
        /* 
           We do not signal load failure if we do not have the S0000
           summary file - which does not contain any useful information
           anyway. 
           
           Hmmm - there is a "if (report_step > 0)" check in the
           enkf_state_internalize_x() function as well.
        */
      else {
        if (load_fail_action == LOAD_FAIL_EXIT)
          loadOK = false;
        else {
          loadOK = true;
          if (load_fail_action == LOAD_FAIL_WARN)
            fprintf(stderr,"** WARNING ** Failed summary:%s does not have report_step:%d \n",ecl_sum_get_case( ecl_sum ) , report_step);
        }
      }
    } 
  } 
  
  if (loadOK) {
    node_id_type node_id = {.report_step = report_step , .iens = -1 , .state = FORECAST };
    SUMMARY_SET_VALUE( summary , node_id , load_value );
  }
  
  return loadOK;
}



bool summary_forward_load_vector(summary_type * summary , const char * ecl_file_name , const ecl_sum_type * ecl_sum, const ecl_file_type * ecl_file , int report_step1, int report_step2) {
  bool loadOK = false;

  if (summary->vector_storage) {
    if (ecl_sum != NULL) {
      double_vector_type * storage_vector = SELECT_VECTOR( summary , FORECAST );
      const char * var_key                = summary_config_get_var(summary->config);
      const ecl_smspec_var_type var_type  = summary_config_get_var_type(summary->config , ecl_sum);
      load_fail_type load_fail_action     = summary_config_get_load_fail_mode(summary->config );
      bool normal_load = false;
      
      
      if (load_fail_action != LOAD_FAIL_EXIT) {
        /*
          The load will always ~succeed - but if we do not have the data;
          we will fill the vector with zeros.
        */
        
        if (!ecl_sum_has_general_var(ecl_sum , var_key)) {
          for (int report_step = report_step1; report_step <= report_step2; report_step++) 
            double_vector_iset( storage_vector , report_step , 0);
          loadOK = true;  
          
          if (load_fail_action == LOAD_FAIL_WARN)
            fprintf(stderr,"** WARNING ** Failed summary:%s does not have key:%s \n",ecl_sum_get_case( ecl_sum ) , var_key);
        } else
          normal_load = true;

      }
        
      
      if (normal_load) {
        int sum_index  = ecl_sum_get_general_var_params_index( ecl_sum , var_key );
        for (int report_step = report_step1; report_step <= report_step2; report_step++) {
          
          if (ecl_sum_has_report_step( ecl_sum , report_step )) {
            int last_report_index = ecl_sum_iget_report_end( ecl_sum , report_step );
            
            double_vector_iset( storage_vector , report_step , ecl_sum_iget(ecl_sum , last_report_index  , sum_index ));
          }
        }
        loadOK = true;
      }
    } 
  }
  
  
  return loadOK;
}






/*

Commented out in the preparation for vector storage.

void summary_set_inflation(summary_type * inflation , const summary_type * std , const summary_type * min_std) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    inflation->data[i] = util_double_max( 1.0 , min_std->data[i] / std->data[i]);
}


void summary_iadd( summary_type * summary , const summary_type * delta) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    summary->data[i] += delta->data[i];
}


void summary_iaddsqr( summary_type * summary , const summary_type * delta) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    summary->data[i] += delta->data[i] * delta->data[i];
}


void summary_imul( summary_type * summary , const summary_type * delta) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    summary->data[i] *= delta->data[i];
}

void summary_scale( summary_type * summary , double scale_factor) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    summary->data[i] *= scale_factor;
}

void summary_isqrt( summary_type * summary ) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    summary->data[i] = sqrt( summary->data[i] );
}
*/




/******************************************************************/
/* Anonumously generated functions used by the enkf_node object   */
/******************************************************************/
UTIL_SAFE_CAST_FUNCTION(summary , SUMMARY)
UTIL_SAFE_CAST_FUNCTION_CONST(summary , SUMMARY)
VOID_ALLOC(summary)
VOID_FREE(summary)
VOID_COPY     (summary)
VOID_FORWARD_LOAD(summary)
VOID_FORWARD_LOAD_VECTOR(summary)
VOID_USER_GET(summary)
VOID_USER_GET_VECTOR(summary)
VOID_WRITE_TO_BUFFER(summary)
VOID_READ_FROM_BUFFER(summary)
VOID_SERIALIZE(summary)
VOID_DESERIALIZE(summary)
VOID_CLEAR(summary)
VOID_HAS_DATA(summary)
/*
VOID_SET_INFLATION(summary)
VOID_IADD(summary)
VOID_SCALE(summary)
VOID_IMUL(summary)
VOID_IADDSQR(summary)
VOID_ISQRT(summary)
*/     

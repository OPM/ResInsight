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

#include <ert/util/util.h>
#include <ert/util/double_vector.h>

#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_smspec.h> 
#include <ert/ecl/ecl_file.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/summary.h>
#include <ert/enkf/summary_config.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/forward_load_context.h>

/*****************************************************************/

#define SUMMARY_UNDEF -9999

struct summary_struct {
  int                          __type_id;         /* Only used for run_time checking. */
  summary_config_type        * config;            /* Can not be NULL - var_type is set on first load. */
  double_vector_type         * data_vector;
};


/*****************************************************************/



static double SUMMARY_GET_VALUE( const summary_type * summary , int report_step) {
  return double_vector_iget( summary->data_vector , report_step );
}


static void SUMMARY_SET_VALUE( summary_type * summary , int report_step , double value) {
  double_vector_iset( summary->data_vector , report_step , value);
}

/*****************************************************************/



void summary_clear(summary_type * summary) {
  double_vector_reset( summary->data_vector );
}


summary_type * summary_alloc(const summary_config_type * summary_config) {
  summary_type * summary   = util_malloc(sizeof *summary );
  summary->__type_id       = SUMMARY;
  summary->config          = (summary_config_type *) summary_config;
  summary->data_vector     = double_vector_alloc(0 , SUMMARY_UNDEF);
  return summary;
}



bool summary_active_value( double value ) {

  if (value == SUMMARY_UNDEF)
    return false;

  return true;
}


void summary_copy(const summary_type *src , summary_type * target) {
  if (src->config == target->config)
    double_vector_memcpy( target->data_vector , src->data_vector );
  else
    util_abort("%s: do not share config objects \n",__func__);
}




void summary_read_from_buffer(summary_type * summary , buffer_type * buffer, enkf_fs_type * fs, int report_step) {
  enkf_util_assert_buffer_type( buffer , SUMMARY );
  double_vector_buffer_fread( summary->data_vector , buffer );
}


bool summary_write_to_buffer(const summary_type * summary , buffer_type * buffer, int report_step) {
  buffer_fwrite_int( buffer , SUMMARY );
  double_vector_buffer_fwrite( summary->data_vector , buffer );
  return true;
}


bool summary_has_data( const summary_type * summary , int report_step) {
  return (double_vector_size( summary->data_vector ) > report_step) ? true : false;
}


void summary_free(summary_type *summary) {
  double_vector_free( summary->data_vector );
  free(summary);
}






void summary_serialize(const summary_type * summary , node_id_type node_id , const active_list_type * active_list , matrix_type * A , int row_offset , int column) {
  double value = SUMMARY_GET_VALUE( summary , node_id.report_step );
  enkf_matrix_serialize( &value , 1 , ECL_DOUBLE_TYPE , active_list , A , row_offset , column);
}


void summary_deserialize(summary_type * summary , node_id_type node_id , const active_list_type * active_list , const matrix_type * A , int row_offset , int column) {
  double value;
  enkf_matrix_deserialize( &value , 1 , ECL_DOUBLE_TYPE , active_list , A , row_offset , column);
  SUMMARY_SET_VALUE( summary , node_id.report_step , value );
}

int summary_length(const summary_type * summary) {
  return double_vector_size(summary->data_vector);
}

double summary_get(const summary_type * summary, int report_step) {
  return SUMMARY_GET_VALUE( summary , report_step );
}


bool summary_user_get(const summary_type * summary , const char * index_key , int report_step , double * value) {
  if (double_vector_size( summary->data_vector ) > report_step) {
    *value = double_vector_iget( summary->data_vector , report_step);
    return true;
  } else {
    *value = -1;
    return false;
  }
}



void summary_user_get_vector(const summary_type * summary , const char * index_key , double_vector_type * value) {
  double_vector_memcpy( value , summary->data_vector);
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

bool summary_forward_load(summary_type * summary , const char * ecl_file_name , const forward_load_context_type * load_context) {
  bool loadOK = false;
  double load_value;
  int report_step                    = forward_load_context_get_load_step( load_context );
  const ecl_sum_type * ecl_sum = forward_load_context_get_ecl_sum( load_context );
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
  
  if (loadOK)
    SUMMARY_SET_VALUE( summary , report_step , load_value );
  
  return loadOK;
}



bool summary_forward_load_vector(summary_type * summary , 
				 const char * ecl_file_name , 
				 const forward_load_context_type * load_context , 
				 const int_vector_type * time_index) {
  bool loadOK = false;

  const ecl_sum_type * ecl_sum = forward_load_context_get_ecl_sum( load_context );
  if (ecl_sum != NULL) {
    const char * var_key                = summary_config_get_var(summary->config);
    load_fail_type load_fail_action     = summary_config_get_load_fail_mode(summary->config );
    bool normal_load = false;
      
      
    if (load_fail_action != LOAD_FAIL_EXIT) {
      /*
        The load will always ~succeed - but if we do not have the data;
        we will fill the vector with zeros.
      */
      
      if (!ecl_sum_has_general_var(ecl_sum , var_key)) {
        for (int step = 0; step < int_vector_size( time_index ); step++) {
          int summary_step = int_vector_iget( time_index , step );
          if (summary_step >= 0)
            double_vector_iset( summary->data_vector , summary_step , 0);
        }
        loadOK = true;  
        
        if (load_fail_action == LOAD_FAIL_WARN)
          fprintf(stderr,"** WARNING ** Failed summary:%s does not have key:%s \n",ecl_sum_get_case( ecl_sum ) , var_key);
      } else
        normal_load = true;
      
    }
    
      
    if (normal_load) {
      int key_index  = ecl_sum_get_general_var_params_index( ecl_sum , var_key );
      
      for (int store_index = 0; store_index < int_vector_size( time_index ); store_index++) {
        int summary_index = int_vector_iget( time_index , store_index );
        
        if (summary_index >= 0) {
          if (ecl_sum_has_report_step( ecl_sum , summary_index )) {
            int last_ministep_index = ecl_sum_iget_report_end( ecl_sum , summary_index );
            double_vector_iset( summary->data_vector , store_index , ecl_sum_iget(ecl_sum , last_ministep_index  , key_index ));
          }
        }
        
      }
      loadOK = true;
    }
  } 
  
  return loadOK;
}










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

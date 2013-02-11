/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_plot_member.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/util.h>
#include <ert/util/double_vector.h>
#include <ert/util/time_t_vector.h>

#include <ert/enkf/enkf_plot_member.h>
#include <ert/enkf/enkf_plot_arg.h>

#define ENKF_PLOT_MEMBER_ID 6111861

struct enkf_plot_member_struct {
  UTIL_TYPE_ID_DECLARATION;
  double_vector_type        * data;
  enkf_plot_arg_type        * arg;
  time_t                      start_time;  
  bool                        time_mode;
  bool                        shared_arg;
};



UTIL_SAFE_CAST_FUNCTION( enkf_plot_member , ENKF_PLOT_MEMBER_ID )
     
enkf_plot_member_type * enkf_plot_member_alloc( enkf_plot_arg_type * shared_arg , time_t start_time) {
  enkf_plot_member_type * plot_member = util_malloc( sizeof * plot_member);
  UTIL_TYPE_ID_INIT( plot_member , ENKF_PLOT_MEMBER_ID );

  plot_member->data          = double_vector_alloc( 0 , 0 );
  plot_member->shared_arg    = true;
  plot_member->start_time    = start_time;
  enkf_plot_member_reset( plot_member , shared_arg , false );
  
  return plot_member;
}


void enkf_plot_member_reset( enkf_plot_member_type * plot_member , enkf_plot_arg_type * shared_arg , bool time_mode) {
  if (!plot_member->shared_arg)
    enkf_plot_arg_free( plot_member->arg );
  
  plot_member->time_mode  = time_mode;
  if (shared_arg == NULL) {
    plot_member->arg = enkf_plot_arg_alloc( plot_member->time_mode , plot_member->start_time );
    plot_member->shared_arg = false;
  } else {
    plot_member->arg = shared_arg;
    plot_member->shared_arg = true;
  }
}




void enkf_plot_member_free( enkf_plot_member_type * plot_member ) {
  double_vector_free( plot_member->data );
  if (!plot_member->shared_arg)
    enkf_plot_arg_free( plot_member->arg );
  free( plot_member );
}



void enkf_plot_member_free__( void * arg ) {
  enkf_plot_member_type * plot_member = enkf_plot_member_safe_cast( arg );
  enkf_plot_member_free( plot_member );
}




void enkf_plot_member_load( enkf_plot_member_type * plot_member , 
                            enkf_node_type * enkf_node , 
                            enkf_fs_type * fs , 
                            const char * user_key , 
                            int iens , 
                            state_enum state , 
                            enkf_plot_arg_type * shared_arg , 
                            bool time_mode , 
                            int step1 , int step2) {
  enkf_plot_member_reset( plot_member , shared_arg , time_mode );

  if (enkf_node_vector_storage( enkf_node )) 
    enkf_node_user_get_vector(enkf_node , fs , user_key , iens , state , plot_member->data);
  else {
    if (shared_arg != NULL)
      util_abort("%s: implementation error - shared arg can ONLY be used with vector nodes\n",__func__);
    enkf_plot_arg_reset( plot_member->arg , time_mode , plot_member->start_time);
    int step;
    node_id_type node_id = {.iens        = iens,
                            .state       = state, 
                            .report_step = 0 };
    
    for (step = step1 ; step <= step2; step++) {
      double value;
      node_id.report_step = step;
      if (enkf_node_user_get(enkf_node , fs , user_key , node_id , &value)) {
        //??
        //??
      }
    }
  }
  
  /*if (enkf_node_vector_storage( enkf_node )) {
    enkf_node_user_get_vector(enkf_node , fs , user_key , iens , state , plot_member->data);
    time_t_vector_memcpy( plot_member->sim_time , member_config_get_sim_time_ref( plot_member->member_config , fs));
    if (step1 > 0) {
    time_t_vector_idel_block( plot_member->sim_time , 0 , step1 );
      double_vector_idel_block( plot_member->data , 0 , step1 );
    }
  } else {
    int step;
    node_id_type node_id = {.iens        = iens,
                            .state       = state, 
                            .report_step = 0 };

    double_vector_reset( plot_member->data );
    time_t_vector_reset( plot_member->sim_time );

    for (step = step1 ; step <= step2; step++) {
      double value;
      if (enkf_node_user_get(enkf_node , fs , user_key , node_id , &value)) {
        double_vector_append( plot_member->data , value);
        time_t_vector_append( plot_member->sim_time , member_config_iget_sim_time( plot_member->member_config , step , fs ));
      }
    }
  }
  */
}




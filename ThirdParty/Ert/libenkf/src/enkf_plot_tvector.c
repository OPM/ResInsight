/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'enkf_plot_tvector.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/bool_vector.h>

#include <ert/enkf/enkf_plot_tvector.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_node.h>
#include <ert/enkf/summary.h>

#define ENKF_PLOT_TVECTOR_ID 6111861

struct enkf_plot_tvector_struct {
  UTIL_TYPE_ID_DECLARATION;
  double_vector_type          * data;
  double_vector_type          * work;
  time_t_vector_type          * time;
  bool_vector_type            * mask;
  const enkf_config_node_type * config_node;
  int  iens;
  bool summary_mode;
};



UTIL_SAFE_CAST_FUNCTION( enkf_plot_tvector , ENKF_PLOT_TVECTOR_ID )
UTIL_IS_INSTANCE_FUNCTION( enkf_plot_tvector , ENKF_PLOT_TVECTOR_ID )



void enkf_plot_tvector_reset( enkf_plot_tvector_type * plot_tvector ) {
  double_vector_reset( plot_tvector->data );
  time_t_vector_reset( plot_tvector->time );
  bool_vector_reset( plot_tvector->mask );
}


enkf_plot_tvector_type * enkf_plot_tvector_alloc( const enkf_config_node_type * config_node , int iens) {
  enkf_plot_tvector_type * plot_tvector = util_malloc( sizeof * plot_tvector);
  UTIL_TYPE_ID_INIT( plot_tvector , ENKF_PLOT_TVECTOR_ID );

  plot_tvector->data = double_vector_alloc( 0 , 0 );
  plot_tvector->time = time_t_vector_alloc(-1 , 0);
  plot_tvector->mask = bool_vector_alloc( false , 0 );
  plot_tvector->work = double_vector_alloc(0,0);
  plot_tvector->iens = iens;

  plot_tvector->config_node = config_node;
  if (enkf_config_node_get_impl_type( config_node ) == SUMMARY)
    plot_tvector->summary_mode = true;
  else
    plot_tvector->summary_mode = false;

  return plot_tvector;
}


void enkf_plot_tvector_free( enkf_plot_tvector_type * plot_tvector ) {
  double_vector_free( plot_tvector->data );
  double_vector_free( plot_tvector->work );
  time_t_vector_free( plot_tvector->time );
  bool_vector_free( plot_tvector->mask );
}


bool enkf_plot_tvector_all_active( const enkf_plot_tvector_type * plot_tvector ) {
  bool all_active = true;
  for (int i=0; i < bool_vector_size( plot_tvector->mask ); i++)
    all_active = all_active && bool_vector_iget(plot_tvector->mask , i );

  return all_active;
}


int enkf_plot_tvector_size( const enkf_plot_tvector_type * plot_tvector ) {
  return bool_vector_size( plot_tvector->mask );
}


void enkf_plot_tvector_iset( enkf_plot_tvector_type * plot_tvector , int index , time_t time , double value) {
  time_t_vector_iset( plot_tvector->time , index , time );
  bool active_value = true;

  /* This is to handle holes in the summary vector storage. */
  if (plot_tvector->summary_mode && !summary_active_value( value ))
    active_value = false;

  if (active_value) {
    double_vector_iset( plot_tvector->data , index , value );
    bool_vector_iset( plot_tvector->mask , index , true );
  } else
    bool_vector_iset( plot_tvector->mask , index , false );

}



double enkf_plot_tvector_iget_value( const enkf_plot_tvector_type * plot_tvector , int index) {
  return double_vector_iget( plot_tvector->data , index);
}

time_t enkf_plot_tvector_iget_time( const enkf_plot_tvector_type * plot_tvector , int index) {
  return time_t_vector_iget( plot_tvector->time , index);
}

bool enkf_plot_tvector_iget_active( const enkf_plot_tvector_type * plot_tvector , int index) {
  return bool_vector_iget( plot_tvector->mask , index );
}






void enkf_plot_tvector_load( enkf_plot_tvector_type * plot_tvector ,
                             enkf_fs_type * fs ,
                             const char * index_key) {

  time_map_type * time_map = enkf_fs_get_time_map( fs );
  int step1 = 0;
  int step2 = time_map_get_last_step( time_map );
  enkf_node_type * work_node  = enkf_node_alloc( plot_tvector->config_node );

  if (enkf_node_vector_storage( work_node )) {
    bool has_data = enkf_node_user_get_vector(work_node , fs , index_key , plot_tvector->iens , plot_tvector->work);

    if(has_data) {
      for (int step = 0; step < double_vector_size( plot_tvector->work ); step++)
          enkf_plot_tvector_iset( plot_tvector ,
                                  step ,
                                  time_map_iget( time_map , step ) ,
                                  double_vector_iget( plot_tvector->work , step ));
    }
  } else {
    int step;
    node_id_type node_id = {.iens        = plot_tvector->iens,
                            .report_step = 0 };

    for (step = step1 ; step <= step2; step++) {
      double value;
      node_id.report_step = step;

      if (enkf_node_user_get(work_node , fs , index_key , node_id , &value)) {
        enkf_plot_tvector_iset( plot_tvector ,
                                step ,
                                time_map_iget( time_map , step ) ,
                                value );
      }
    }
  }
  enkf_node_free( work_node );
}


void * enkf_plot_tvector_load__( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  enkf_plot_tvector_type * tvector = arg_pack_iget_ptr( arg_pack , 0 );
  enkf_fs_type * fs = arg_pack_iget_ptr( arg_pack , 1 );
  const char * index_key = arg_pack_iget_ptr( arg_pack , 2 );

  enkf_plot_tvector_load( tvector , fs , index_key );
  return NULL;
}


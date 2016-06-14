/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'enkf_plot_data.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <time.h>
#include <stdbool.h>

#include <ert/util/double_vector.h>
#include <ert/util/vector.h>
#include <ert/util/thread_pool.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_plot_tvector.h>
#include <ert/enkf/enkf_plot_data.h>
#include <ert/enkf/state_map.h>


#define ENKF_PLOT_DATA_TYPE_ID 3331063

struct enkf_plot_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  const enkf_config_node_type * config_node;
  int                       size;
  enkf_plot_tvector_type ** ensemble;
  arg_pack_type          ** work_arg;
};



static void enkf_plot_data_resize( enkf_plot_data_type * plot_data , int new_size ) {
  if (new_size != plot_data->size) {
    int iens;

    if (new_size < plot_data->size) {
      for (iens = new_size; iens < plot_data->size; iens++) {
        enkf_plot_tvector_free( plot_data->ensemble[iens] );
        arg_pack_free( plot_data->work_arg[iens] );
      }
    }

    plot_data->ensemble = util_realloc( plot_data->ensemble , new_size * sizeof * plot_data->ensemble);
    plot_data->work_arg = util_realloc( plot_data->work_arg , new_size * sizeof * plot_data->work_arg);

    if (new_size > plot_data->size) {
      for (iens = plot_data->size; iens < new_size; iens++) {
        plot_data->ensemble[iens] = enkf_plot_tvector_alloc( plot_data->config_node , iens);
        plot_data->work_arg[iens] = arg_pack_alloc();
      }
    }
    plot_data->size = new_size;
  }
}


static void enkf_plot_data_reset( enkf_plot_data_type * plot_data ) {
  int iens;
  for (iens = 0; iens < plot_data->size; iens++) {
    enkf_plot_tvector_reset( plot_data->ensemble[iens] );
    arg_pack_clear( plot_data->work_arg[iens] );
  }
}


void enkf_plot_data_free( enkf_plot_data_type * plot_data ) {
  int iens;
  for (iens = 0; iens < plot_data->size; iens++) {
    enkf_plot_tvector_free( plot_data->ensemble[iens] );
    arg_pack_free( plot_data->work_arg[iens]);
  }
  free( plot_data->work_arg );
  free( plot_data->ensemble );
  free( plot_data );
}

UTIL_IS_INSTANCE_FUNCTION( enkf_plot_data , ENKF_PLOT_DATA_TYPE_ID )


enkf_plot_data_type * enkf_plot_data_alloc( const enkf_config_node_type * config_node ) {
  enkf_plot_data_type * plot_data = util_malloc( sizeof * plot_data);
  UTIL_TYPE_ID_INIT( plot_data , ENKF_PLOT_DATA_TYPE_ID );
  plot_data->config_node = config_node;
  plot_data->size        = 0;
  plot_data->ensemble    = NULL;
  plot_data->work_arg    = NULL;
  return plot_data;
}

enkf_plot_tvector_type * enkf_plot_data_iget( const enkf_plot_data_type * plot_data , int index) {
  return plot_data->ensemble[index];
}


int enkf_plot_data_get_size( const enkf_plot_data_type * plot_data ) {
  return plot_data->size;
}




void enkf_plot_data_load( enkf_plot_data_type * plot_data ,
                          enkf_fs_type * fs ,
                          const char * index_key ,
                          const bool_vector_type * input_mask) {
  state_map_type * state_map = enkf_fs_get_state_map( fs );
  int ens_size = state_map_get_size( state_map );
  bool_vector_type * mask;

  if (input_mask)
    mask = bool_vector_alloc_copy( input_mask );
  else
    mask = bool_vector_alloc( ens_size , false );
  state_map_select_matching( state_map , mask , STATE_HAS_DATA );

  enkf_plot_data_resize( plot_data , ens_size );
  enkf_plot_data_reset( plot_data );
  {
    const int num_cpu = 4;
    thread_pool_type * tp = thread_pool_alloc( num_cpu , true );
    for (int iens = 0; iens < ens_size ; iens++) {
      if (bool_vector_iget( mask , iens)) {
        enkf_plot_tvector_type * vector = enkf_plot_data_iget( plot_data , iens );
        arg_pack_type * work_arg = plot_data->work_arg[iens];

        arg_pack_append_ptr( work_arg , vector );
        arg_pack_append_ptr( work_arg , fs );
        arg_pack_append_const_ptr( work_arg , index_key );

        thread_pool_add_job( tp , enkf_plot_tvector_load__ , work_arg );
      }
    }
    thread_pool_join( tp );
    thread_pool_free( tp );
  }
  bool_vector_free( mask );
}



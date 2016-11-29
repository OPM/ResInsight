/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_plot_gen_kw.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <assert.h>
#include <stdbool.h>
#include <time.h>

#include <ert/util/bool_vector.h>
#include <ert/util/double_vector.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_plot_gen_kw.h>
#include <ert/enkf/enkf_plot_gen_kw_vector.h>
#include <ert/enkf/gen_kw_config.h>


#define ENKF_PLOT_GEN_KW_TYPE_ID 88362063

struct enkf_plot_gen_kw_struct {
  UTIL_TYPE_ID_DECLARATION;
  const enkf_config_node_type   * config_node;
  int                             size;        /* Number of ensembles. */
  enkf_plot_gen_kw_vector_type ** ensemble;    /* One vector for each ensemble. */
};


UTIL_IS_INSTANCE_FUNCTION( enkf_plot_gen_kw , ENKF_PLOT_GEN_KW_TYPE_ID )


enkf_plot_gen_kw_type * enkf_plot_gen_kw_alloc( const enkf_config_node_type * config_node ) {
  if (enkf_config_node_get_impl_type( config_node ) == GEN_KW) {
    enkf_plot_gen_kw_type * plot_gen_kw = util_malloc( sizeof * plot_gen_kw );
    UTIL_TYPE_ID_INIT( plot_gen_kw , ENKF_PLOT_GEN_KW_TYPE_ID );
    plot_gen_kw->config_node = config_node;
    plot_gen_kw->size = 0;
    plot_gen_kw->ensemble = NULL;
    return plot_gen_kw;
  }
  else {
    return NULL;
  }
}


void enkf_plot_gen_kw_free( enkf_plot_gen_kw_type * plot_gen_kw ) {
  int iens;
  for (iens = 0 ; iens < plot_gen_kw->size ; ++iens) {
    enkf_plot_gen_kw_vector_free( plot_gen_kw->ensemble[iens] );
  }
  free( plot_gen_kw );
}


int enkf_plot_gen_kw_get_size( const enkf_plot_gen_kw_type * plot_gen_kw ) {
  return plot_gen_kw->size;
}

enkf_plot_gen_kw_vector_type * enkf_plot_gen_kw_iget( const enkf_plot_gen_kw_type * plot_gen_kw , int iens)  {
  if ((iens < 0) || (iens >= plot_gen_kw->size))
    util_abort("%s: index:%d invalid. Valid interval: [0,%d>.\n",__func__ , iens , plot_gen_kw->size);

  return plot_gen_kw->ensemble[iens];
}


static void enkf_plot_gen_kw_resize( enkf_plot_gen_kw_type * plot_gen_kw , int new_size ) {
  if (new_size != plot_gen_kw->size) {
    int iens;

    if (new_size < plot_gen_kw->size) {
      for (iens = new_size; iens < plot_gen_kw->size; iens++) {
        enkf_plot_gen_kw_vector_free( plot_gen_kw->ensemble[iens] );
      }
    }

    plot_gen_kw->ensemble = util_realloc( plot_gen_kw->ensemble , new_size * sizeof * plot_gen_kw->ensemble);

    if (new_size > plot_gen_kw->size) {
      for (iens = plot_gen_kw->size; iens < new_size; iens++) {
        plot_gen_kw->ensemble[iens] = enkf_plot_gen_kw_vector_alloc( plot_gen_kw->config_node , iens );
      }
    }
    plot_gen_kw->size = new_size;
  }
}



void enkf_plot_gen_kw_load( enkf_plot_gen_kw_type  * plot_gen_kw,
                            enkf_fs_type           * fs,
                            bool                     transform_data , 
                            int                      report_step,
                            const bool_vector_type * input_mask ) {

  state_map_type * state_map = enkf_fs_get_state_map( fs );
  int ens_size = state_map_get_size( state_map );
  bool_vector_type * mask;

  if (input_mask)
    mask = bool_vector_alloc_copy( input_mask );
  else
    mask = bool_vector_alloc( ens_size , true );

  enkf_plot_gen_kw_resize( plot_gen_kw , ens_size );
  {
    int iens;
    for (iens = 0; iens < ens_size; ++iens) {
      if (bool_vector_iget( mask , iens)) {
        enkf_plot_gen_kw_vector_type * vector = enkf_plot_gen_kw_iget( plot_gen_kw , iens );
        enkf_plot_gen_kw_vector_load( vector , fs , transform_data , report_step );
      }
    }
  }
}



const char * enkf_plot_gen_kw_iget_key( const enkf_plot_gen_kw_type * plot_gen_kw, int index) {
  const gen_kw_config_type * gen_kw_config = enkf_config_node_get_ref( plot_gen_kw->config_node );
  return gen_kw_config_iget_name( gen_kw_config , index );
}

int enkf_plot_gen_kw_get_keyword_count( const enkf_plot_gen_kw_type * gen_kw ){
    const gen_kw_config_type * gen_kw_config = enkf_config_node_get_ref( gen_kw->config_node );
    return gen_kw_config_get_data_size(gen_kw_config);
}

bool enkf_plot_gen_kw_should_use_log_scale(const enkf_plot_gen_kw_type * gen_kw , int index) {
    const gen_kw_config_type * gen_kw_config = enkf_config_node_get_ref( gen_kw->config_node );
    return gen_kw_config_should_use_log_scale(gen_kw_config, index);
}

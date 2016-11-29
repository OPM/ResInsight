/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_plot_gen_kw_vector.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/stringlist.h>
#include <ert/util/thread_pool.h>
#include <ert/util/type_macros.h>
#include <ert/util/vector.h>

#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_node.h>
#include <ert/enkf/enkf_plot_gen_kw_vector.h>
#include <ert/enkf/gen_kw.h>


#define ENKF_PLOT_GEN_KW_VECTOR_TYPE_ID 88362064

struct enkf_plot_gen_kw_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                           iens;
  double_vector_type          * data;
  const enkf_config_node_type * config_node;
};


UTIL_IS_INSTANCE_FUNCTION( enkf_plot_gen_kw_vector , ENKF_PLOT_GEN_KW_VECTOR_TYPE_ID )


enkf_plot_gen_kw_vector_type * enkf_plot_gen_kw_vector_alloc( const enkf_config_node_type * config_node , int iens ) {
  enkf_plot_gen_kw_vector_type * vector = util_malloc( sizeof * vector );
  UTIL_TYPE_ID_INIT( vector , ENKF_PLOT_GEN_KW_VECTOR_TYPE_ID );
  vector->config_node = config_node;
  vector->data        = double_vector_alloc(0,0);
  vector->iens        = iens;
  return vector;
}


void enkf_plot_gen_kw_vector_free( enkf_plot_gen_kw_vector_type * vector ) {
  double_vector_free( vector->data );
  free( vector );
}


int enkf_plot_gen_kw_vector_get_size( const enkf_plot_gen_kw_vector_type * vector ) {
  return double_vector_size( vector->data );
}

double enkf_plot_gen_kw_vector_iget( const enkf_plot_gen_kw_vector_type * vector , int index )  {
  return double_vector_iget( vector->data , index );
}


void enkf_plot_gen_kw_vector_reset( enkf_plot_gen_kw_vector_type * vector ) {
  double_vector_reset( vector->data );
}


void enkf_plot_gen_kw_vector_load( enkf_plot_gen_kw_vector_type * vector , enkf_fs_type * fs , bool transform_data , int report_step ) {
  enkf_plot_gen_kw_vector_reset( vector );
  {
    node_id_type node_id = { .report_step = report_step ,
                             .iens        = vector->iens };

    enkf_node_type * data_node = enkf_node_alloc( vector->config_node );

    if (enkf_node_try_load( data_node , fs , node_id )) {
      gen_kw_type * gen_kw = enkf_node_value_ptr( data_node );
      int n_kw = gen_kw_data_size( gen_kw );
      int i_kw;

      for (i_kw = 0 ; i_kw < n_kw ; ++i_kw) {
        double_vector_append(vector->data , gen_kw_data_iget( gen_kw , i_kw , transform_data ) );
      }
    }

    enkf_node_free( data_node );
  }
}

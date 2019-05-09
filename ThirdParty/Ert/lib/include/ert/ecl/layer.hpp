/*
   Copyright (C) 2014  Equinor ASA, Norway.

   The file 'layer.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_LAYER_H
#define ERT_LAYER_H


#include <ert/util/int_vector.hpp>
#include <ert/util/type_macros.hpp>

#include <ert/ecl/ecl_grid.hpp>

#ifdef __cplusplus
extern "C" {
#endif

/*
   The elements in this enum are (ab)used as indexes into a int[] vector;
   i.e. the must span the values 0..3.
*/

  typedef enum {
    RIGHT_EDGE  = 0,
    LEFT_EDGE   = 1,
    TOP_EDGE    = 2,
    BOTTOM_EDGE = 3
  } edge_dir_enum;

  typedef struct {
    int i;
    int j;
  } int_point2d_type;



  typedef struct layer_struct       layer_type;

  bool         layer_iget_left_barrier( const layer_type * layer, int i , int j);
  bool         layer_iget_bottom_barrier( const layer_type * layer, int i , int j);
  int          layer_get_nx( const layer_type * layer );
  int          layer_get_ny( const layer_type * layer );
  void         layer_fprintf_cell( const layer_type * layer , int i , int j , FILE * stream);
  void         layer_fprintf( const layer_type * layer , FILE * stream);
  void         layer_fprintf_box( const layer_type * layer , FILE * stream , int i1 , int i2 , int j1 , int j2);
  layer_type * layer_alloc(int nx , int ny);
  void         layer_free( layer_type * layer );
  int          layer_replace_cell_values( layer_type * layer , int old_value , int new_value);
  bool         layer_iget_active( const layer_type * layer, int i , int j);
  int          layer_iget_cell_value( const layer_type * layer, int i , int j);
  void         layer_iset_cell_value( layer_type * layer , int i , int j , int value);
  int          layer_iget_edge_value( const layer_type * layer , int i , int j , edge_dir_enum dir);
  bool         layer_cell_on_edge( const layer_type * layer , int i , int j);
  int          layer_get_ny( const layer_type * layer);
  int          layer_get_nx( const layer_type * layer);
  int          layer_get_cell_sum( const layer_type * layer );
  bool         layer_trace_block_content( layer_type * layer , bool erase , int start_i , int start_j , int value , int_vector_type * i_list , int_vector_type * j_list);
  bool         layer_cell_contact( const layer_type * layer , int i1 , int j1 , int i2 , int j2);
  void         layer_add_interp_barrier( layer_type * layer , int c1 , int c2);
  void         layer_add_ijbarrier( layer_type * layer , int i1 , int j1 , int i2 , int j2 );
  void         layer_add_barrier( layer_type * layer , int c1 , int c2);
  void         layer_memcpy(layer_type * target_layer , const layer_type * src_layer);
  void         layer_update_active( layer_type * layer , const ecl_grid_type * grid , int k);
  void         layer_clear_cells( layer_type * layer);
  void         layer_update_connected_cells( layer_type * layer , int i , int j , int org_value , int new_value);
  void         layer_assign( layer_type * layer, int value);

  void         layer_cells_equal( const layer_type * layer , int value , int_vector_type * i_list , int_vector_type * j_list);
  int          layer_count_equal( const layer_type * layer , int value );

UTIL_IS_INSTANCE_HEADER( layer );
UTIL_SAFE_CAST_HEADER( layer );


#ifdef __cplusplus
}
#endif
#endif

/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'fault_block.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/type_macros.h>
#include <ert/util/int_vector.h>

#include <ert/geometry/geo_util.h>
#include <ert/geometry/geo_polygon.h>
#include <ert/geometry/geo_polygon_collection.h>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/fault_block.hpp>
#include <ert/ecl/fault_block_layer.hpp>
#include <ert/ecl/layer.hpp>

#define FAULT_BLOCK_ID 3297376


struct fault_block_struct {
  UTIL_TYPE_ID_DECLARATION;
  const ecl_grid_type    * grid;
  const fault_block_layer_type * parent_layer;
  int_vector_type        * i_list;
  int_vector_type        * j_list;
  int_vector_type        * global_index_list;
  int_vector_type        * region_list;
  int                      block_id;
  int                      k;
  double                   xc,yc;
  bool                     valid_center;
};


UTIL_IS_INSTANCE_FUNCTION( fault_block , FAULT_BLOCK_ID )
static UTIL_SAFE_CAST_FUNCTION( fault_block , FAULT_BLOCK_ID )


fault_block_type * fault_block_alloc( const fault_block_layer_type * parent_layer ,  int block_id ) {
  fault_block_type * block = (fault_block_type*)util_malloc( sizeof * block );
  UTIL_TYPE_ID_INIT( block , FAULT_BLOCK_ID );
  block->parent_layer = parent_layer;
  block->grid = fault_block_layer_get_grid( parent_layer );
  block->k = fault_block_layer_get_k( parent_layer );
  block->block_id = block_id;

  block->i_list = int_vector_alloc(0,0);
  block->j_list = int_vector_alloc(0,0);
  block->global_index_list = int_vector_alloc(0,0);
  block->region_list = int_vector_alloc(0,0);
  block->valid_center = false;
  return block;
}


int fault_block_get_size( const fault_block_type * block ) {
  return int_vector_size( block->i_list );
}


int fault_block_get_id( const fault_block_type * block ) {
  return block->block_id;
}


void fault_block_free( fault_block_type * block ) {
  int_vector_free( block->i_list );
  int_vector_free( block->j_list );
  int_vector_free( block->region_list );
  int_vector_free( block->global_index_list );
  free( block );
}


void fault_block_free__( void * arg) {
  fault_block_type * block = fault_block_safe_cast( arg );
  fault_block_free( block );
}


void fault_block_add_cell( fault_block_type * fault_block , int i , int j) {
  int_vector_append( fault_block->i_list , i);
  int_vector_append( fault_block->j_list , j);
  int_vector_append( fault_block->global_index_list , ecl_grid_get_global_index3( fault_block->grid , i , j , fault_block->k));
  fault_block->valid_center = false;
  layer_iset_cell_value( fault_block_layer_get_layer( fault_block->parent_layer ) , i , j , fault_block->block_id );
}


void fault_block_assign_to_region( fault_block_type * fault_block , int region_id ) {
  if (int_vector_size( fault_block->region_list ) == 0)
    int_vector_append( fault_block->region_list , region_id );
  else {
    if (int_vector_index_sorted( fault_block->region_list , region_id ) == -1)
      int_vector_append( fault_block->region_list , region_id );
  }
  int_vector_sort( fault_block->region_list );
}


const int_vector_type * fault_block_get_region_list( const fault_block_type * fault_block ) {
  return fault_block->region_list;
}


static void fault_block_assert_center( fault_block_type * fault_block ) {
  if (!fault_block->valid_center) {
    int index;
    double xc = 0;
    double yc = 0;

    for (index = 0; index < int_vector_size( fault_block->i_list ); index++) {
      int i = int_vector_iget( fault_block->i_list , index);
      int j = int_vector_iget( fault_block->j_list , index);
      int g = ecl_grid_get_global_index3( fault_block->grid , i , j , fault_block->k );
      double x , y , z;

      ecl_grid_get_xyz1( fault_block->grid , g , &x , &y , &z);
      xc += x;
      yc += y;
    }

    fault_block->xc = xc / int_vector_size( fault_block->i_list );
    fault_block->yc = yc / int_vector_size( fault_block->i_list );
  }
  fault_block->valid_center = true;
}


double fault_block_get_xc( fault_block_type * fault_block ) {
  fault_block_assert_center( fault_block );
  return fault_block->xc;
}


double fault_block_get_yc( fault_block_type * fault_block ) {
  fault_block_assert_center( fault_block );
  return fault_block->yc;
}




void  fault_block_export_cell(const fault_block_type * fault_block , int index , int * i , int * j , int * k , double * x, double * y, double * z) {
  *i = int_vector_iget( fault_block->i_list , index );
  *j = int_vector_iget( fault_block->j_list , index );
  *k = fault_block->k;

  ecl_grid_get_xyz3( fault_block->grid , *i , *j , *k , x , y , z);
}


int fault_block_iget_i(const fault_block_type * fault_block , int index) {
  return int_vector_iget( fault_block->i_list , index );
}


int fault_block_iget_j(const fault_block_type * fault_block , int index) {
  return int_vector_iget( fault_block->j_list , index );
}

const int_vector_type * fault_block_get_global_index_list( const fault_block_type * fault_block) {
  return fault_block->global_index_list;
}



bool fault_block_trace_edge( const fault_block_type * block , double_vector_type * x_list , double_vector_type * y_list, int_vector_type * cell_list) {
  if (fault_block_get_size( block ) > 0) {
    struct_vector_type * corner_list = struct_vector_alloc( sizeof(int_point2d_type) );
    int c;
    {
      int start_i = fault_block_iget_i( block , 0 );
      int start_j = fault_block_iget_j( block , 0 );

      layer_trace_block_edge(fault_block_layer_get_layer( block->parent_layer ) , start_i , start_j , block->block_id , corner_list , cell_list);
    }

    if (x_list && y_list) {
      double_vector_reset( x_list );
      double_vector_reset( y_list );
      for (c=0; c < struct_vector_get_size( corner_list ); c++) {
        double x,y,z;
        int_point2d_type ij;
        struct_vector_iget( corner_list , c , &ij );

        ecl_grid_get_corner_xyz( block->grid , ij.i , ij.j , block->k , &x , &y , &z);
        double_vector_append( x_list , x);
        double_vector_append( y_list , y);
      }
    }

    struct_vector_free( corner_list );
    return true;
  } else
    return false;
}

static bool fault_block_neighbour_xpolyline( const fault_block_type * block , int i1, int j1, int i2 , int j2, const geo_polygon_collection_type * polylines) {
  int g1 = ecl_grid_get_global_index3( block->grid , i1 , j1 , block->k );
  int g2 = ecl_grid_get_global_index3( block->grid , i2 , j2 , block->k );
  double x1,y1,z1;
  double x2,y2,z2;

  ecl_grid_get_xyz1( block->grid , g1 , &x1 , &y1 , &z1);
  ecl_grid_get_xyz1( block->grid , g2 , &x2 , &y2 , &z2);

  {
    int polyline_index = 0;
    bool intersection = false;
    while (true)  {
      if (polyline_index >= geo_polygon_collection_size(polylines))
        break;

      {
        const geo_polygon_type * polyline = geo_polygon_collection_iget_polygon( polylines , polyline_index );
        if (geo_polygon_segment_intersects( polyline , x1,y1,x2,y2 )) {
          intersection = true;
          break;
        }
      }

      polyline_index++;
    }

    return intersection;
  }
}


static bool fault_block_connected_neighbour( const fault_block_type * block , int i1 , int j1 , int i2 , int j2 , bool connected_only , const geo_polygon_collection_type * polylines ) {
  const layer_type * layer = fault_block_layer_get_layer( block->parent_layer );
  if ((i2 < 0) || (i2 >= layer_get_nx( layer )))
    return false;

  if ((j2 < 0) || (j2 >= layer_get_ny( layer )))
    return false;

  /*
    Inactive cells do "not exist" - can not be connected neighbour
    with an inactive cell.
  */
  if (!ecl_grid_cell_active3( block->grid , i2,j2,block->k))
    return false;
  
  {
    int cell_id = layer_iget_cell_value( layer , i1 , j1 );
    int neighbour_id = layer_iget_cell_value(layer , i2 , j2);
    if (cell_id == neighbour_id)
      return false;

    if (!connected_only)
      return true;

    return (layer_cell_contact( layer , i1 , j1 , i2 , j2)  && !fault_block_neighbour_xpolyline( block , i1,j1,i2,j2,polylines ));
  }
}



void fault_block_list_neighbours( const fault_block_type * block, bool connected_only , const geo_polygon_collection_type * polylines , int_vector_type * neighbour_list) {
  int_vector_reset( neighbour_list );
  {
    int c;
    layer_type * layer = fault_block_layer_get_layer( block->parent_layer );
    for (c = 0; c < int_vector_size( block->i_list ); c++) {
      int i = int_vector_iget( block->i_list , c);
      int j = int_vector_iget( block->j_list , c);

      if (fault_block_connected_neighbour( block , i , j , i - 1 , j , connected_only , polylines ))
        int_vector_append( neighbour_list , layer_iget_cell_value( layer , i - 1 , j));

      if (fault_block_connected_neighbour( block , i , j , i + 1 , j , connected_only , polylines ))
        int_vector_append( neighbour_list , layer_iget_cell_value( layer , i + 1 , j));

      if (fault_block_connected_neighbour( block , i , j , i  , j - 1, connected_only , polylines ))
        int_vector_append( neighbour_list , layer_iget_cell_value( layer , i , j - 1));

      if (fault_block_connected_neighbour( block , i , j , i , j + 1, connected_only , polylines ))
        int_vector_append( neighbour_list , layer_iget_cell_value( layer , i , j + 1));

    }
  }
  int_vector_select_unique( neighbour_list );
  int_vector_del_value( neighbour_list , 0 );
  int_vector_del_value( neighbour_list , block->block_id );
}



void  fault_block_copy_content(fault_block_type * target_block , const fault_block_type * src_block ) {
  int b;
  for (b = 0; b < int_vector_size( src_block->i_list ); b++)
    fault_block_add_cell( target_block , int_vector_iget( src_block->i_list , b) , int_vector_iget( src_block->j_list , b));

}



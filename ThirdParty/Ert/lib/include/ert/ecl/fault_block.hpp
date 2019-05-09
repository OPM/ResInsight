/*
   Copyright (C) 2014  Equinor ASA, Norway.

   The file 'fault_block.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_FAULT_BLOCK_H
#define ERT_FAULT_BLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/int_vector.hpp>
#include <ert/util/double_vector.hpp>
#include <ert/util/type_macros.hpp>

#include <ert/geometry/geo_polygon_collection.hpp>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_kw.hpp>




  typedef struct fault_block_struct  fault_block_type;

  void               fault_block_free__( void * arg);
  int                fault_block_get_size( const fault_block_type * block );
  double             fault_block_get_xc( fault_block_type * fault_block );
  double             fault_block_get_yc( fault_block_type * fault_block );
  int                fault_block_get_id( const fault_block_type * block );
  int                fault_block_get_size( const fault_block_type * fault_block );
  void               fault_block_export_cell(const fault_block_type * fault_block , int index , int * i , int * j , int * k , double * x, double * y, double * z);
  void               fault_block_assign_to_region( fault_block_type * fault_block , int region_id );
  const int_vector_type * fault_block_get_region_list( const fault_block_type * fault_block );
  int                fault_block_iget_j(const fault_block_type * fault_block , int index);
  int                fault_block_iget_i(const fault_block_type * fault_block , int index);
  void               fault_block_add_cell( fault_block_type * fault_block , int i , int j);
  bool               fault_block_trace_edge( const fault_block_type * block , double_vector_type * x_list , double_vector_type * y_list, int_vector_type * cell_list);
  const int_vector_type * fault_block_get_global_index_list( const fault_block_type * fault_block);
  void               fault_block_copy_content(fault_block_type * target_block , const fault_block_type * src_block );
  void               fault_block_list_neighbours( const fault_block_type * block , bool connected_only , const geo_polygon_collection_type * polylines , int_vector_type * neighbour_list);

  UTIL_IS_INSTANCE_HEADER(fault_block);

#ifdef __cplusplus
}
#endif
#endif

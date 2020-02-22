/*
   Copyright (C) 2014  Equinor ASA, Norway.

   The file 'fault_block_layer.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_FAULT_BLOCK_LAYER_H
#define ERT_FAULT_BLOCK_LAYER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.hpp>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/fault_block.hpp>
#include <ert/ecl/layer.hpp>

  UTIL_IS_INSTANCE_HEADER(fault_block_layer);

  typedef struct fault_block_layer_struct  fault_block_layer_type;

  fault_block_layer_type * fault_block_layer_alloc( const ecl_grid_type * grid , int k);
  void                     fault_block_layer_free( fault_block_layer_type * layer );
  void                     fault_block_layer_free__( void * arg );
  bool                     fault_block_layer_has_block( const fault_block_layer_type * layer , int block_id);
  void                     fault_block_layer_del_block( fault_block_layer_type * layer , int block_id);
  fault_block_type       * fault_block_layer_add_block( fault_block_layer_type * layer , int block_id);
  fault_block_type       * fault_block_layer_get_block( const fault_block_layer_type * layer , int block_id);
  fault_block_type       * fault_block_layer_iget_block( const fault_block_layer_type * layer , int storage_index);
  fault_block_type       * fault_block_layer_safe_get_block( fault_block_layer_type * layer , int block_id);
  int                      fault_block_layer_get_max_id( const fault_block_layer_type * layer );
  int                      fault_block_layer_get_next_id( const fault_block_layer_type * layer );
  int                      fault_block_layer_get_size( const fault_block_layer_type * layer);
  bool                     fault_block_layer_scan_kw( fault_block_layer_type * layer , const ecl_kw_type * fault_block_kw);
  bool                     fault_block_layer_load_kw( fault_block_layer_type * layer , const ecl_kw_type * fault_block_kw);
  int                      fault_block_layer_get_k( const fault_block_layer_type * layer );
  void                     fault_block_layer_scan_layer( fault_block_layer_type * fault_layer , layer_type * layer);
  void                     fault_block_layer_insert_block_content( fault_block_layer_type * layer , const fault_block_type * src_block);
  bool                     fault_block_layer_export( const fault_block_layer_type * layer , ecl_kw_type * faultblock_kw);
  const ecl_grid_type    * fault_block_layer_get_grid( const fault_block_layer_type * layer );
  layer_type             * fault_block_layer_get_layer( const fault_block_layer_type * layer );


#ifdef __cplusplus
}
#endif
#endif

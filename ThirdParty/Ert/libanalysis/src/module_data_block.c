/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'module_data_block.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <stdio.h>
#include <stdlib.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/analysis/module_data_block.h>

#define MODULE_DATA_BLOCK_TYPE_ID 73217801
static UTIL_SAFE_CAST_FUNCTION( module_data_block , MODULE_DATA_BLOCK_TYPE_ID);
UTIL_IS_INSTANCE_FUNCTION( module_data_block , MODULE_DATA_BLOCK_TYPE_ID)

module_data_block_type * module_data_block_alloc(  const char * key,  const int * index_list , const int row_start, const int n_active) {
  module_data_block_type * module_data_block = util_malloc( sizeof * module_data_block );
  UTIL_TYPE_ID_INIT( module_data_block , MODULE_DATA_BLOCK_TYPE_ID );
  module_data_block->key = util_alloc_string_copy( key );
  module_data_block->index_list  = index_list;
  module_data_block->A_row_start = row_start;
  module_data_block->n_active = n_active;
  return module_data_block;
}


const char * module_data_block_get_key(const module_data_block_type * module_data_block){
  return module_data_block->key;
}

const int module_data_block_get_row_start(const module_data_block_type * module_data_block){
  return module_data_block->A_row_start;
}

const int module_data_block_get_row_end(const module_data_block_type * module_data_block){
  return module_data_block->A_row_start + module_data_block->n_active;
}

const int  * module_data_block_get_active_indices(const module_data_block_type * module_data_block ){
  return module_data_block->index_list;
}

void module_data_block_free( module_data_block_type * module_data_block ) {
  util_safe_free(module_data_block->key);
  free( module_data_block );
}

void module_data_block_free__( void * arg ) {
  module_data_block_type * data_block = module_data_block_safe_cast( arg );
  module_data_block_free( data_block );
}

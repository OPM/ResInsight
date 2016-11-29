/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'module_obs_block.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/analysis/module_obs_block.h>

#define MODULE_OBS_BLOCK_TYPE_ID 73217901
static UTIL_SAFE_CAST_FUNCTION( module_obs_block , MODULE_OBS_BLOCK_TYPE_ID);
UTIL_IS_INSTANCE_FUNCTION( module_obs_block , MODULE_OBS_BLOCK_TYPE_ID)

module_obs_block_type * module_obs_block_alloc(  const char * key, const int * index_list, const int row_start, const int n_active) {
  module_obs_block_type * module_obs_block = util_malloc( sizeof * module_obs_block );
  UTIL_TYPE_ID_INIT( module_obs_block , MODULE_OBS_BLOCK_TYPE_ID );
  module_obs_block->key = util_alloc_string_copy( key );
  module_obs_block->index_list  = index_list;
  module_obs_block->D_row_start = row_start;
  module_obs_block->n_active = n_active;
  return module_obs_block;
}


const char * module_obs_block_get_key(const module_obs_block_type * module_obs_block){
  return module_obs_block->key;
}

const int module_obs_block_get_row_start(const module_obs_block_type * module_obs_block){
  return module_obs_block->D_row_start;
}

const int module_obs_block_get_row_end(const module_obs_block_type * module_obs_block){
  return module_obs_block->D_row_start + module_obs_block->n_active;
}

const int  * module_obs_block_get_active_indices(const module_obs_block_type * module_obs_block ){
  return module_obs_block->index_list;
}

void module_obs_block_free( module_obs_block_type * module_obs_block ) {
  util_safe_free(module_obs_block->key);
  free( module_obs_block );
}

void module_obs_block_free__( void * arg ) {
  module_obs_block_type * obs_block = module_obs_block_safe_cast( arg );
  module_obs_block_free( obs_block );
}

/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'module_obs_block_vector.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdlib.h>
#include <string.h>

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/analysis/module_obs_block.h>
#include <ert/analysis/module_obs_block_vector.h>

#define MODULE_OBS_BLOCK_VECTOR_TYPE_ID 732188012

struct module_obs_block_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type            * obs_block_vector;
};

UTIL_IS_INSTANCE_FUNCTION( module_obs_block_vector , MODULE_OBS_BLOCK_VECTOR_TYPE_ID)

module_obs_block_vector_type * module_obs_block_vector_alloc() {
  module_obs_block_vector_type * module_obs_block_vector = util_malloc( sizeof * module_obs_block_vector );
  UTIL_TYPE_ID_INIT( module_obs_block_vector , MODULE_OBS_BLOCK_VECTOR_TYPE_ID );
  module_obs_block_vector->obs_block_vector = vector_alloc_new();
  return module_obs_block_vector;
}


void module_obs_block_vector_free( module_obs_block_vector_type * module_obs_block_vector ) {
  vector_free( module_obs_block_vector->obs_block_vector );
  free( module_obs_block_vector );
}

void module_obs_block_vector_add_obs_block( module_obs_block_vector_type * module_obs_block_vector , module_obs_block_type * obs_block) {
  vector_append_owned_ref(module_obs_block_vector->obs_block_vector, obs_block , module_obs_block_free__);
}


module_obs_block_type * module_obs_block_vector_iget_module_obs_block(const module_obs_block_vector_type * module_obs_block_vector, int block_index){
 return vector_iget(module_obs_block_vector->obs_block_vector, block_index);
}

const module_obs_block_type * module_obs_block_vector_search_module_obs_block(const module_obs_block_vector_type * module_obs_block_vector, int global_index){
  /* This function maps from a global index to an observation information block. Will return NULL if block is not found */
  int block_nr = 0;
  while (true) {
    if (block_nr >= module_obs_block_vector_get_size( module_obs_block_vector ))
      break;

    module_obs_block_type * module_obs_block =  module_obs_block_vector_iget_module_obs_block (module_obs_block_vector, block_nr);
    int row_start =  module_obs_block_get_row_start(module_obs_block);
    int row_end =  module_obs_block_get_row_end(module_obs_block);
    if (global_index >= row_start && global_index < row_end)
      return module_obs_block;

    block_nr++;
  }
  return NULL;
}

int module_obs_block_vector_get_size(const module_obs_block_vector_type * module_obs_block_vector){
 return vector_get_size(module_obs_block_vector->obs_block_vector);
}



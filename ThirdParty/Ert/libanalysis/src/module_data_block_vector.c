/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'module_data_block_vector.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/analysis/module_data_block.h>
#include <ert/analysis/module_data_block_vector.h>

#define MODULE_DATA_BLOCK_VECTOR_TYPE_ID 732178012

struct module_data_block_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type            * data_block_vector;
};

UTIL_IS_INSTANCE_FUNCTION( module_data_block_vector , MODULE_DATA_BLOCK_VECTOR_TYPE_ID)

module_data_block_vector_type * module_data_block_vector_alloc( ) {
  module_data_block_vector_type * module_data_block_vector = util_malloc( sizeof * module_data_block_vector );
  UTIL_TYPE_ID_INIT( module_data_block_vector , MODULE_DATA_BLOCK_VECTOR_TYPE_ID );
  module_data_block_vector->data_block_vector = vector_alloc_new();
  return module_data_block_vector;
}


void module_data_block_vector_free( module_data_block_vector_type * module_data_block_vector ) {
  vector_free( module_data_block_vector->data_block_vector );
  free( module_data_block_vector );
}

void module_data_block_vector_add_data_block( module_data_block_vector_type * module_data_block_vector , const module_data_block_type * data_block) {
  vector_append_owned_ref(module_data_block_vector->data_block_vector, data_block , module_data_block_free__);
}


module_data_block_type * module_data_block_vector_iget_module_data_block(const module_data_block_vector_type * module_data_block_vector, int index){
 return vector_iget(module_data_block_vector->data_block_vector, index);
}

int module_data_block_vector_get_size(const module_data_block_vector_type * module_data_block_vector){
 return vector_get_size(module_data_block_vector->data_block_vector);
}



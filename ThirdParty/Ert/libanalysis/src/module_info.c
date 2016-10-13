/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'module_info.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/analysis/module_info.h>

#define MODULE_INFO_TYPE_ID 73780123

struct module_info_struct {
  UTIL_TYPE_ID_DECLARATION;
  char                          * ministep_name;
  module_data_block_vector_type * data_block_vector;
  module_obs_block_vector_type  * obs_block_vector;
};

UTIL_IS_INSTANCE_FUNCTION( module_info , MODULE_INFO_TYPE_ID)

module_info_type * module_info_alloc( const char* ministep_name ) {
  module_info_type * module_info = util_malloc( sizeof * module_info );
  UTIL_TYPE_ID_INIT( module_info , MODULE_INFO_TYPE_ID );
  module_info->ministep_name     = util_alloc_string_copy( ministep_name );
  module_info->data_block_vector = module_data_block_vector_alloc();
  module_info->obs_block_vector  = module_obs_block_vector_alloc();
  return module_info;
}


void module_info_free( module_info_type * module_info ) {
  util_safe_free(module_info->ministep_name);
  module_data_block_vector_free( module_info->data_block_vector );
  module_obs_block_vector_free( module_info->obs_block_vector );
  free( module_info );
}

module_data_block_vector_type *   module_info_get_data_block_vector(const module_info_type * module_info){
  return module_info->data_block_vector;
}

module_obs_block_vector_type *   module_info_get_obs_block_vector(const module_info_type * module_info){
  return module_info->obs_block_vector;
}

char * module_info_get_ministep_name(const module_info_type * module_info){
  return module_info->ministep_name;
}

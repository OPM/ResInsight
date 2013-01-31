/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'container_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/vector.h>
#include <ert/util/util.h>

#include <ert/enkf/container_config.h>
#include <ert/enkf/enkf_macros.h>

#define CONTAINER_CONFIG_TYPE_ID 51330852

struct container_config_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type * nodes;
};


container_config_type * container_config_alloc( const char * key ) {
  container_config_type * container = util_malloc( sizeof * container );
  UTIL_TYPE_ID_INIT( container , CONTAINER_CONFIG_TYPE_ID );
  container->nodes = vector_alloc_new();
  return container;
}



void container_config_free( container_config_type * container_config ) {
  vector_free( container_config->nodes );
  free( container_config );
}


void container_config_add_node( container_config_type * container_config , const enkf_config_node_type * config_node) {
  vector_append_ref( container_config->nodes , config_node );
}

const void * container_config_iget_node(const container_config_type * container_config , int index) {
  return vector_iget_const( container_config->nodes , index );
}

int container_config_get_size( const container_config_type * container_config ) {
  return vector_get_size( container_config->nodes );
}


int container_config_get_data_size( const container_config_type * container_config ) {
  util_exit("%s: not implemented \n",__func__);
  return 0;
}


/*****************************************************************/

UTIL_SAFE_CAST_FUNCTION(container_config , CONTAINER_CONFIG_TYPE_ID)
UTIL_SAFE_CAST_FUNCTION_CONST(container_config , CONTAINER_CONFIG_TYPE_ID)
VOID_GET_DATA_SIZE(container)
VOID_CONFIG_FREE(container)


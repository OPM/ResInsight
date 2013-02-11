/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'container_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __CONTAINER_CONFIG_H__
#define __CONTAINER_CONFIG_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_macros.h>
  
  typedef struct container_config_struct container_config_type;
  

  container_config_type * container_config_alloc( const char * key );
  void                    container_config_free( container_config_type * container );
  void                    container_config_add_node( container_config_type * container, const enkf_config_node_type * config_node);
  const char            * container_config_iget_key( const container_config_type * container_config , int index);
  const void            * container_config_iget_node(const container_config_type * container_config , int index);
  int                     container_config_get_size( const container_config_type * container_config );


  UTIL_SAFE_CAST_HEADER_CONST(container_config);
  GET_DATA_SIZE_HEADER(container);
  VOID_GET_DATA_SIZE_HEADER(container);
  VOID_CONFIG_FREE_HEADER(container);


#ifdef __cplusplus
}
#endif
#endif

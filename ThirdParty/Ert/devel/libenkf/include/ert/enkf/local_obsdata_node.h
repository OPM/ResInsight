/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'local_obsdata_node.h'
    
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
#ifndef __LOCAL_OBSDATA_NODE_H__
#define __LOCAL_OBSDATA_NODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>

#include <ert/enkf/obs_tstep_list.h>
#include <ert/enkf/active_list.h>


  typedef struct local_obsdata_node_struct local_obsdata_node_type;
  
  local_obsdata_node_type   * local_obsdata_node_alloc( const char * obs_key );
  const char                * local_obsdata_node_get_key( const local_obsdata_node_type * node );
  void                        local_obsdata_node_free( local_obsdata_node_type * node );
  void                        local_obsdata_node_free__( void * arg );
  active_list_type          * local_obsdata_node_get_active_list( const local_obsdata_node_type * node );
  const obs_tstep_list_type * local_obsdata_node_get_tstep_list( const local_obsdata_node_type * node);
  void                        local_obsdata_node_copy_active_list( local_obsdata_node_type * node , const active_list_type * active_list);
  void                        local_obsdata_node_add_tstep( local_obsdata_node_type * node, int tstep);
  void                        local_obsdata_node_add_range( local_obsdata_node_type * node, int step1, int step2);
  
UTIL_IS_INSTANCE_HEADER( local_obsdata_node );

#ifdef __cplusplus
}
#endif
#endif

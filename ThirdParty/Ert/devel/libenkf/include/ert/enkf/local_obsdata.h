/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'local_obsdata.h'
    
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
#ifndef __LOCAL_OBSDATA_H__
#define __LOCAL_OBSDATA_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/util/type_macros.h>

#include <ert/enkf/local_obsdata_node.h>

typedef struct local_obsdata_struct local_obsdata_type;

  bool                            local_obsdata_has_node( const local_obsdata_type * data , const char * key);
  local_obsdata_type            * local_obsdata_alloc( const char * name );
  void                            local_obsdata_free( local_obsdata_type * data );
  int                             local_obsdata_get_size( const local_obsdata_type * data );
  bool                            local_obsdata_add_node( local_obsdata_type * data , local_obsdata_node_type * node );
  const local_obsdata_node_type * local_obsdata_iget( const local_obsdata_type * data , int index);
  local_obsdata_type            * local_obsdata_alloc_wrapper( local_obsdata_node_type * node );
  const char * local_obsdata_get_name( const local_obsdata_type * data);

UTIL_IS_INSTANCE_HEADER( local_obsdata );

#ifdef __cplusplus
}
#endif
#endif

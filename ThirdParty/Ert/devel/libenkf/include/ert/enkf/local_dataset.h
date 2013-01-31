/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_dataset.h' is part of ERT - Ensemble based
   Reservoir Tool.
   
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

#ifndef __LOCAL_DATASET_H__
#define __LOCAL_DATASET_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct local_dataset_struct local_dataset_type;

local_dataset_type * local_dataset_alloc_copy( local_dataset_type * src_dataset , const char * copy_name );
local_dataset_type * local_dataset_alloc( const char * name );
void                 local_dataset_free( local_dataset_type * dataset );
void                 local_dataset_free__( void * arg );
void                 local_dataset_add_node(local_dataset_type * dataset, const char *node_key);
void                 local_dataset_del_node( local_dataset_type * dataset , const char * node_key);
void                 local_dataset_clear( local_dataset_type * dataset);
const char *         local_dataset_get_name( const local_dataset_type * dataset);
void                 local_dataset_fprintf( const local_dataset_type * dataset , FILE * stream);
active_list_type   * local_dataset_get_node_active_list(const local_dataset_type * dataset , const char * node_key );
stringlist_type    * local_dataset_alloc_keys( const local_dataset_type * dataset );
int                  local_dataset_get_size( const local_dataset_type * dataset );

#ifdef __cplusplus
}
#endif

#endif 

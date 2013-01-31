/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_obsset.h'
    
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
#ifndef __LOCAL_OBSSET_H__
#define __LOCAL_OBSSET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <ert/enkf/active_list.h>

typedef struct local_obsset_struct local_obsset_type;

local_obsset_type * local_obsset_alloc( const char * name );
local_obsset_type * local_obsset_alloc_copy( local_obsset_type * src_dataset , const char * copy_name );
void                local_obsset_free( local_obsset_type * obsset );
void                local_obsset_free__( void * arg );
void                local_obsset_add_obs(local_obsset_type * obsset, const char * obs_key);
void                local_obsset_del_obs( local_obsset_type * obsset , const char * obs_key);
const char        * local_obsset_get_name( const local_obsset_type * obsset );
void                local_obsset_fprintf(local_obsset_type * obsset , FILE * stream);
active_list_type  * local_obsset_get_obs_active_list( const local_obsset_type * obsset , const char * obs_key );
hash_iter_type    * local_obsset_alloc_obs_iter( const local_obsset_type * obsset );
void                local_obsset_clear( local_obsset_type * obsset );

#ifdef __cplusplus
}
#endif
#endif

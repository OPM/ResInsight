/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_ministep.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __LOCAL_MINISTEP_H__
#define __LOCAL_MINISTEP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/hash.h>
#include <ert/util/stringlist.h>

#include <ert/enkf/active_list.h>
#include <ert/enkf/local_dataset.h>
#include <ert/enkf/local_obsset.h>


typedef struct local_ministep_struct local_ministep_type;

local_ministep_type * local_ministep_alloc(const char * name , local_obsset_type * observations);
void                  local_ministep_free(local_ministep_type * ministep);
void                  local_ministep_free__(void * arg);
void                  local_ministep_add_obs(local_ministep_type * ministep, const char * obs_key);
active_list_type    * local_ministep_get_node_active_list(const local_ministep_type * ministep , const char * node_key );
hash_iter_type      * local_ministep_alloc_dataset_iter( const local_ministep_type * ministep );
local_ministep_type * local_ministep_alloc_copy( const local_ministep_type * src , const char * name);
void                  local_ministep_del_obs( local_ministep_type * ministep , const char * obs_key);
void                  local_ministep_del_node( local_ministep_type * ministep , const char * node_key);
const char          * local_ministep_get_name( const local_ministep_type * ministep );
void                  local_ministep_clear_nodes( local_ministep_type * ministep);
void                  local_ministep_clear_observations( local_ministep_type * ministep);
void                  local_ministep_fprintf( const local_ministep_type * ministep , FILE * stream );
void                  local_ministep_add_dataset( local_ministep_type * ministep , const local_dataset_type * dataset);
local_obsset_type   * local_ministep_get_obsset(const local_ministep_type * ministep);
local_dataset_type  * local_ministep_get_dataset( const local_ministep_type * ministep, const char * dataset_name);

UTIL_SAFE_CAST_HEADER(local_ministep);
UTIL_IS_INSTANCE_HEADER(local_ministep);

#ifdef __cplusplus
}
#endif
#endif

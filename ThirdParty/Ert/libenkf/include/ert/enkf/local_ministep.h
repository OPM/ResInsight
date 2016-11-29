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

#ifndef ERT_LOCAL_MINISTEP_H
#define ERT_LOCAL_MINISTEP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/hash.h>
#include <ert/util/stringlist.h>

#include <ert/analysis/analysis_module.h>

#include <ert/enkf/active_list.h>
#include <ert/enkf/local_dataset.h>
#include <ert/enkf/local_obsdata.h>
#include <ert/enkf/local_obsdata_node.h>

typedef struct local_ministep_struct local_ministep_type;

local_ministep_type * local_ministep_alloc(const char * name, analysis_module_type* analysis_module);
void                  local_ministep_free(local_ministep_type * ministep);
void                  local_ministep_free__(void * arg);
void                  local_ministep_add_obs(local_ministep_type * ministep, const char * obs_key);
active_list_type    * local_ministep_get_node_active_list(const local_ministep_type * ministep , const char * node_key );
hash_iter_type      * local_ministep_alloc_dataset_iter( const local_ministep_type * ministep );
stringlist_type     * local_ministep_alloc_data_keys( const local_ministep_type * ministep );
bool                  local_ministep_has_data_key(const local_ministep_type * ministep , const char * key);
local_ministep_type * local_ministep_alloc_copy( const local_ministep_type * src , const char * name);
void                  local_ministep_del_obs( local_ministep_type * ministep , const char * obs_key);
void                  local_ministep_del_node( local_ministep_type * ministep , const char * node_key);
const char          * local_ministep_get_name( const local_ministep_type * ministep );
void                  local_ministep_clear_nodes( local_ministep_type * ministep);
void                  local_ministep_clear_observations( local_ministep_type * ministep);
void                  local_ministep_fprintf( const local_ministep_type * ministep , FILE * stream );
void                  local_ministep_summary_fprintf( const local_ministep_type * ministep , FILE * stream);
void                  local_ministep_add_dataset( local_ministep_type * ministep , const local_dataset_type * dataset);
void                  local_ministep_add_obsdata( local_ministep_type * ministep , local_obsdata_type * obsdata);
void                  local_ministep_add_obsdata_node( local_ministep_type * ministep , local_obsdata_node_type * obsdatanode);
local_obsdata_type  * local_ministep_get_obsdata(const local_ministep_type * ministep);
local_dataset_type  * local_ministep_get_dataset( const local_ministep_type * ministep, const char * dataset_name);
bool                  local_ministep_has_dataset( const local_ministep_type * ministep, const char * dataset_name);
int                   local_ministep_get_num_dataset( const local_ministep_type * ministep );
bool                  local_ministep_has_analysis_module( const local_ministep_type * ministep );
analysis_module_type* local_ministep_get_analysis_module( const local_ministep_type * ministep );

UTIL_SAFE_CAST_HEADER(local_ministep);
UTIL_IS_INSTANCE_HEADER(local_ministep);

#ifdef __cplusplus
}
#endif
#endif

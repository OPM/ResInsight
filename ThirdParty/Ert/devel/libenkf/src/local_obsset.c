/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_obsset.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>

#include <ert/enkf/active_list.h>
#include <ert/enkf/local_obsset.h>
#include <ert/enkf/local_config.h>

#define LOCAL_OBSSET_TYPE_ID 991534

struct local_obsset_struct {
  UTIL_TYPE_ID_DECLARATION;
  char      * name;
  hash_type * observations;   /* Hash table of active_list instances. */
};


static UTIL_SAFE_CAST_FUNCTION( local_obsset , LOCAL_OBSSET_TYPE_ID )

local_obsset_type * local_obsset_alloc( const char * name ) {
  local_obsset_type * obsset = util_malloc( sizeof * obsset);
  UTIL_TYPE_ID_INIT( obsset , LOCAL_OBSSET_TYPE_ID );
  obsset->name = util_alloc_string_copy( name );
  obsset->observations = hash_alloc();
  return obsset;
}


local_obsset_type * local_obsset_alloc_copy( local_obsset_type * src_dataset , const char * copy_name ) {
  local_obsset_type * copy_dataset = local_obsset_alloc( copy_name );
  hash_iter_type * node_iter = hash_iter_alloc( src_dataset->observations );

  while (!hash_iter_is_complete( node_iter )) {
    const char * key = hash_iter_get_next_key( node_iter );
    active_list_type * active_list = active_list_alloc_copy( hash_get( src_dataset->observations , key ) );
    hash_insert_hash_owned_ref( copy_dataset->observations , key , active_list , active_list_free__);
  }

  hash_iter_free( node_iter );
  return copy_dataset;
}



void local_obsset_free( local_obsset_type * obsset ) {
  hash_free( obsset->observations );
  free( obsset->name );
  free( obsset );
}


void local_obsset_free__( void * arg ) {
  local_obsset_type * obsset = local_obsset_safe_cast( arg );
  local_obsset_free( obsset );
}


void local_obsset_add_obs(local_obsset_type * obsset, const char * obs_key) {
  if (hash_has_key( obsset->observations , obs_key ))
    util_abort("%s: tried to add existing observation key:%s \n",__func__ , obs_key);

  hash_insert_hash_owned_ref( obsset->observations , obs_key , active_list_alloc( ALL_ACTIVE ) , active_list_free__);
}


void local_obsset_del_obs( local_obsset_type * obsset , const char * obs_key) {
  hash_del( obsset->observations , obs_key );
}

void local_obsset_clear( local_obsset_type * obsset ) {
  hash_clear( obsset->observations );
}

const char * local_obsset_get_name( const local_obsset_type * obsset ) {
  return obsset->name;
}


void local_obsset_fprintf(local_obsset_type * obsset , FILE * stream) {
  hash_iter_type * obs_iter = hash_iter_alloc( obsset->observations );
  while (!hash_iter_is_complete( obs_iter )) {
    const char * obs_key           = hash_iter_get_next_key( obs_iter );
    active_list_type * active_list = hash_get( obsset->observations , obs_key );
    
    fprintf(stream , "%s %s %s\n", local_config_get_cmd_string( ADD_OBS ) , obsset->name , obs_key );
    active_list_fprintf( active_list , true , obs_key , stream );
  }
  hash_iter_free( obs_iter );
}


active_list_type * local_obsset_get_obs_active_list( const local_obsset_type * obsset , const char * obs_key ) {
  return hash_get( obsset->observations , obs_key );
}


hash_iter_type * local_obsset_alloc_obs_iter( const local_obsset_type * obsset ) {
  return hash_iter_alloc( obsset->observations );
}

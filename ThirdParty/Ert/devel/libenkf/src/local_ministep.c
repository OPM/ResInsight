/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_ministep.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/hash.h>
#include <ert/util/util.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/local_config.h>  
#include <ert/enkf/local_ministep.h>
#include <ert/enkf/local_dataset.h>
#include <ert/enkf/local_obsset.h>

/**
   This file implements a 'ministep' configuration for active /
   inactive observations and parameters for ONE enkf update. Observe
   that the updating at one report step can consist of several
   socalled ministeps, i.e. first the northern part of the field with
   the relevant observations, and then the southern part.

   The implementation, in local_ministep_type, is quite simple, it
   only contains the keys for the observations and nodes, with an
   accompanying pointer to an active_list instance which denotes the
   active indices. Observe that this implementation offers no access
   to the internals of the underlying enkf_node / obs_node objects.
*/


#define LOCAL_MINISTEP_TYPE_ID 661066




struct local_ministep_struct {
  UTIL_TYPE_ID_DECLARATION;
  char              * name;             /* A name used for this ministep - string is also used as key in a hash table holding this instance. */
  hash_type         * datasets;         /* A hash table of local_dataset_type instances - indexed by the name of the datasets. */
  local_obsset_type * observations;
};


/**
   Observe there is no link between the instances here and the real
   observations/nodes (apart from the key in the hash).
*/

UTIL_SAFE_CAST_FUNCTION(local_ministep , LOCAL_MINISTEP_TYPE_ID)
UTIL_IS_INSTANCE_FUNCTION(local_ministep , LOCAL_MINISTEP_TYPE_ID)

local_ministep_type * local_ministep_alloc(const char * name , local_obsset_type * observations) {
  local_ministep_type * ministep = util_malloc( sizeof * ministep );

  ministep->name         = util_alloc_string_copy( name );
  ministep->observations = observations;
  ministep->datasets     = hash_alloc();
  UTIL_TYPE_ID_INIT( ministep , LOCAL_MINISTEP_TYPE_ID);
  
  return ministep;
}


local_ministep_type * local_ministep_alloc_copy( const local_ministep_type * src , const char * name) {
  //local_ministep_type * new = local_ministep_alloc( name );
  //{
  //  hash_iter_type * obs_iter = hash_iter_alloc( src->observations );
  //  while (!hash_iter_is_complete( obs_iter )) {
  //    const char * obs_key = hash_iter_get_next_key( obs_iter );
  //    active_list_type * active_list_copy = active_list_alloc_copy( hash_get( src->observations , obs_key) );
  //    hash_insert_hash_owned_ref( new->observations , obs_key , active_list_copy , active_list_free__);
  //  }
  //}
  //
  //{
  //  hash_iter_type * nodeset_iter = hash_iter_alloc( src->datasets );
  //  while (!hash_iter_is_complete( nodeset_iter )) {
  //    const char * nodeset_key = hash_iter_get_next_key( nodeset_iter );
  //    local_nodeset_type * new_nodeset = local_nodeset_alloc_copy( hash_get( src->datasets , nodeset_key ));
  //    hash_insert_ref( new->datasets , nodeset_key , new_nodeset );
  //  }
  //}
  //
  //return new;
  return NULL;
}



void local_ministep_free(local_ministep_type * ministep) {
  free(ministep->name);
  hash_free( ministep->datasets );
  free( ministep );
}


void local_ministep_free__(void * arg) {
  local_ministep_type * ministep = local_ministep_safe_cast( arg );
  local_ministep_free( ministep );
}





/**
   When adding observations and update nodes here observe the following:

   1. The thing will fail hard if you try to add a node/obs which is
   already in the hash table.

   2. The newly added elements will be assigned an active_list
   instance with mode ALL_ACTIVE.
*/   



void local_ministep_add_dataset( local_ministep_type * ministep , const local_dataset_type * dataset) {
  hash_insert_ref( ministep->datasets , local_dataset_get_name( dataset ) , dataset );
}



local_dataset_type * local_ministep_get_dataset( const local_ministep_type * ministep, const char * dataset_name) {
  return hash_get( ministep->datasets, dataset_name );
}

local_obsset_type * local_ministep_get_obsset( const local_ministep_type * ministep ) {
  return ministep->observations;
}



const char * local_ministep_get_name( const local_ministep_type * ministep ) {
  return ministep->name;
}


/*****************************************************************/

hash_iter_type * local_ministep_alloc_dataset_iter( const local_ministep_type * ministep ) {
  return hash_iter_alloc( ministep->datasets );
}


void local_ministep_fprintf( const local_ministep_type * ministep , FILE * stream ) {
  fprintf(stream , "%s %s %s\n", local_config_get_cmd_string( CREATE_MINISTEP ), ministep->name , local_obsset_get_name( ministep->observations) );
  {
    hash_iter_type * dataset_iter = hash_iter_alloc( ministep->datasets );
    while (!hash_iter_is_complete( dataset_iter )) {
      const char * dataset_key          = hash_iter_get_next_key( dataset_iter );

      fprintf(stream , "%s %s %s\n", local_config_get_cmd_string( ATTACH_DATASET ) , ministep->name , dataset_key );
    }
    hash_iter_free( dataset_iter );
  }
}

/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'local_dataset.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/hash.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/local_ministep.h>
#include <ert/enkf/local_config.h>
#include <ert/enkf/active_list.h>
#include <ert/enkf/local_dataset.h>


#define LOCAL_DATASET_TYPE_ID  6615409

struct local_dataset_struct {
  UTIL_TYPE_ID_DECLARATION;
  char      * name;
  hash_type * nodes;            /* A hash table indexed by node keys - each element is an active_list instance. */
};



UTIL_SAFE_CAST_FUNCTION(local_dataset , LOCAL_DATASET_TYPE_ID)
UTIL_IS_INSTANCE_FUNCTION(local_dataset , LOCAL_DATASET_TYPE_ID)


local_dataset_type * local_dataset_alloc( const char * name ) {
  local_dataset_type * dataset = util_malloc( sizeof * dataset);

  UTIL_TYPE_ID_INIT( dataset , LOCAL_DATASET_TYPE_ID );
  dataset->nodes = hash_alloc();
  dataset->name  = util_alloc_string_copy( name );

  return dataset;
}

local_dataset_type * local_dataset_alloc_copy( local_dataset_type * src_dataset , const char * copy_name ) {
  local_dataset_type * copy_dataset = local_dataset_alloc( copy_name );
  hash_iter_type * node_iter = hash_iter_alloc( src_dataset->nodes );

  while (!hash_iter_is_complete( node_iter )) {
    const char * key = hash_iter_get_next_key( node_iter );
    active_list_type * active_list = active_list_alloc_copy( hash_get( src_dataset->nodes , key ) );
    hash_insert_hash_owned_ref( copy_dataset->nodes , key , active_list , active_list_free__);
  }

  hash_iter_free( node_iter );
  return copy_dataset;
}


void local_dataset_free( local_dataset_type * dataset ) {
  util_safe_free(dataset->name);
  hash_free( dataset->nodes );
  free( dataset );
}

void local_dataset_free__( void * arg ) {
  local_dataset_type * local_dataset = local_dataset_safe_cast( arg );
  local_dataset_free( local_dataset );
}

const char * local_dataset_get_name( const local_dataset_type * dataset) {
  return dataset->name;
}



void local_dataset_add_node(local_dataset_type * dataset, const char *node_key) {
  if (hash_has_key( dataset->nodes , node_key ))
    util_abort("%s: tried to add existing node key:%s \n",__func__ , node_key);

  hash_insert_hash_owned_ref( dataset->nodes , node_key , active_list_alloc( ALL_ACTIVE ) , active_list_free__);
}

bool local_dataset_has_key(const local_dataset_type * dataset, const char * key) {
  return hash_has_key( dataset->nodes , key );
}


void local_dataset_del_node( local_dataset_type * dataset , const char * node_key) {
  hash_del( dataset->nodes , node_key );
}


void local_dataset_clear( local_dataset_type * dataset) {
  hash_clear( dataset->nodes );
}


active_list_type * local_dataset_get_node_active_list(const local_dataset_type * dataset , const char * node_key ) {
  return hash_get( dataset->nodes , node_key );  /* Fails hard if you do not have the key ... */
}

stringlist_type * local_dataset_alloc_keys( const local_dataset_type * dataset ) {
  return hash_alloc_stringlist( dataset->nodes );
}

void local_dataset_summary_fprintf( const local_dataset_type * dataset , FILE * stream) {
{
  hash_iter_type * data_iter = hash_iter_alloc( dataset->nodes );
  while (!hash_iter_is_complete( data_iter )) {
    const char * data_key          = hash_iter_get_next_key( data_iter );
    fprintf(stream , "NAME OF DATA:%s,", data_key );

    active_list_type * active_list = hash_get( dataset->nodes , data_key );
    active_list_summary_fprintf( active_list , local_dataset_get_name(dataset) , data_key , stream);
  }
  hash_iter_free( data_iter );
 }
}


int local_dataset_get_size( const local_dataset_type * dataset ) {
  return hash_get_size( dataset->nodes );
}


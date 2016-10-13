/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'local_obsdata.c'

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

#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/util/vector.h>
#include <ert/util/hash.h>

#include <ert/enkf/local_config.h>
#include <ert/enkf/local_obsdata.h>


#define LOCAL_OBSDATA_TYPE_ID 86331309

struct local_obsdata_struct {
  UTIL_TYPE_ID_DECLARATION;
  hash_type   * nodes_map;
  vector_type * nodes_list;
  char * name;
};



UTIL_IS_INSTANCE_FUNCTION( local_obsdata  , LOCAL_OBSDATA_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( local_obsdata  , LOCAL_OBSDATA_TYPE_ID )

local_obsdata_type * local_obsdata_alloc( const char * name) {
  local_obsdata_type * data = util_malloc( sizeof * data );
  UTIL_TYPE_ID_INIT( data , LOCAL_OBSDATA_TYPE_ID );
  data->nodes_list = vector_alloc_new();
  data->nodes_map = hash_alloc();
  data->name = util_alloc_string_copy( name );
  return data;
}



local_obsdata_type * local_obsdata_alloc_wrapper( local_obsdata_node_type * node ) {
  local_obsdata_type * data = local_obsdata_alloc( local_obsdata_node_get_key( node ));
  local_obsdata_add_node( data , node );
  return data;
}


local_obsdata_type * local_obsdata_alloc_copy( const local_obsdata_type * src, const char * target_key) {
  local_obsdata_type * target = local_obsdata_alloc( target_key );
  int i;
  for (i=0; i < local_obsdata_get_size( src ); i++ ) {
    const local_obsdata_node_type * src_node = local_obsdata_iget( src  , i );
    local_obsdata_node_type * target_node = local_obsdata_node_alloc_copy( src_node );
    local_obsdata_add_node( target , target_node );
  }
  return target;
}



void local_obsdata_free( local_obsdata_type * data ) {
  vector_free( data->nodes_list );
  hash_free( data->nodes_map );
  free( data->name );
  free( data );
}

void local_obsdata_free__( void * arg) {
  local_obsdata_type * data = local_obsdata_safe_cast( arg );
  return local_obsdata_free( data );
}


const char * local_obsdata_get_name( const local_obsdata_type * data) {
  return data->name;
}


int local_obsdata_get_size( const local_obsdata_type * data ) {
  return vector_get_size( data->nodes_list );
}

/*
  The @data instance will assume ownership of the node; i.e. calling
  scope should NOT call local_obsdata_node_free().
*/

bool local_obsdata_add_node( local_obsdata_type * data , local_obsdata_node_type * node ) {
  const char * key = local_obsdata_node_get_key( node );
  if (local_obsdata_has_node(data , key))
    return false;
  else {
    vector_append_owned_ref( data->nodes_list , node , local_obsdata_node_free__ );
    hash_insert_ref( data->nodes_map , key , node );
    return true;
  }
}

 void local_obsdata_del_node( local_obsdata_type * data  , const char * key) {
   local_obsdata_node_type * node = local_obsdata_get( data , key );
   int index = vector_find( data->nodes_list , node );

   hash_del( data->nodes_map , key );
   vector_idel( data->nodes_list , index );
}


 void local_obsdata_clear( local_obsdata_type * data ) {
   hash_clear( data->nodes_map );
   vector_clear( data->nodes_list );
 }


local_obsdata_node_type * local_obsdata_iget( const local_obsdata_type * data , int index) {
  return vector_iget( data->nodes_list , index );
}


local_obsdata_node_type * local_obsdata_get( const local_obsdata_type * data , const char * key) {
  return hash_get( data->nodes_map , key );
}


bool local_obsdata_has_node( const local_obsdata_type * data , const char * key) {
  return hash_has_key( data->nodes_map , key );
}

void local_obsdata_reset_tstep_list( local_obsdata_type * data , const int_vector_type * step_list) {
  int i;
  for (i=0; i < local_obsdata_get_size( data ); i++ ) {
    local_obsdata_node_type * node = local_obsdata_iget( data  , i );
    local_obsdata_node_reset_tstep_list(node, step_list);
  }
}

active_list_type * local_obsdata_get_node_active_list(const local_obsdata_type * obsdata , const char * obs_key ) {
  local_obsdata_node_type * obsdata_node = local_obsdata_get( obsdata , obs_key );
  active_list_type  * active_list = local_obsdata_node_get_active_list( obsdata_node );
  return active_list;
}

void local_obsdata_summary_fprintf( const local_obsdata_type * obsdata , FILE * stream) {

  fprintf(stream , "LOCAL OBSDATA NAME:%s,LOCAL OBSDATA SIZE:%d,", local_obsdata_get_name(obsdata), local_obsdata_get_size(obsdata) );

  int i;
  for (i = 0; i < local_obsdata_get_size( obsdata ); i++ ) {
    local_obsdata_node_type * node = local_obsdata_iget( obsdata  , i );
    const char * obs_key =  local_obsdata_node_get_key(node);
    fprintf(stream , "OBSERVATION:%s,", obs_key );
  }
}

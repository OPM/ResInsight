/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   
   The file 'well_branch_collection.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>

#include <ert/ecl_well/well_const.h>
#include <ert/ecl_well/well_conn.h>
#include <ert/ecl_well/well_branch_collection.h>


#define WELL_BRANCH_COLLECTION_TYPE_ID 67177087

struct well_branch_collection_struct {
  UTIL_TYPE_ID_DECLARATION;

  vector_type     *  __start_segments;
  int_vector_type *  index_map;
};


UTIL_IS_INSTANCE_FUNCTION( well_branch_collection , WELL_BRANCH_COLLECTION_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( well_branch_collection , WELL_BRANCH_COLLECTION_TYPE_ID )


well_branch_collection_type * well_branch_collection_alloc() {
  well_branch_collection_type * branch_collection = util_malloc( sizeof * branch_collection );
  UTIL_TYPE_ID_INIT( branch_collection , WELL_BRANCH_COLLECTION_TYPE_ID );
  branch_collection->__start_segments = vector_alloc_new();
  branch_collection->index_map = int_vector_alloc(0 , -1 );
  return branch_collection;
}



void well_branch_collection_free( well_branch_collection_type * branches ) {
  vector_free( branches->__start_segments );
  int_vector_free( branches->index_map );
  free( branches );
}



void well_branch_collection_free__( void * arg ) {
  well_branch_collection_type * branches = well_branch_collection_safe_cast( arg );
  well_branch_collection_free( branches );
}


int well_branch_collection_get_size( const well_branch_collection_type * branches ) {
  return vector_get_size( branches->__start_segments );
}


bool well_branch_collection_has_branch( const well_branch_collection_type * branches , int branch_id) {
  if (int_vector_safe_iget( branches->index_map , branch_id) >= 0)
    return true;
  else
    return false;
}



const well_segment_type * well_branch_collection_iget_start_segment( const well_branch_collection_type * branches , int index ) {
  if (index < vector_get_size( branches->__start_segments))
    return vector_iget_const( branches->__start_segments , index);
  else
    return NULL;
}



const well_segment_type * well_branch_collection_get_start_segment( const well_branch_collection_type * branches , int branch_id) {
  int internal_index = int_vector_safe_iget( branches->index_map , branch_id);
  if (internal_index >= 0)
    return well_branch_collection_iget_start_segment( branches , internal_index );
  else
    return NULL;
}


bool well_branch_collection_add_start_segment( well_branch_collection_type * branches , const well_segment_type * start_segment) {
  if ((well_segment_get_link_count( start_segment ) == 0) && (well_segment_get_outlet(start_segment))) {
    int branch_id = well_segment_get_branch_id( start_segment );
    int current_index = int_vector_safe_iget( branches->index_map , branch_id);
    if (current_index >= 0) 
      vector_iset_ref( branches->__start_segments , current_index , start_segment);
    else {
      int new_index = vector_get_size( branches->__start_segments );
      vector_append_ref( branches->__start_segments , start_segment);
      int_vector_iset( branches->index_map , branch_id , new_index);
    }
    
    return true;
  } else
    return false;
}



  

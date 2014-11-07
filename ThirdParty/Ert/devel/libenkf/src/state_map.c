/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   The file 'state_map.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#define  _GNU_SOURCE   /* Must define this to get access to pthread_rwlock_t */
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/state_map.h>


#define STATE_MAP_TYPE_ID 500672132

struct state_map_struct {
  UTIL_TYPE_ID_DECLARATION;
  int_vector_type  * state;
  pthread_rwlock_t   rw_lock;
  bool               read_only;
};


UTIL_IS_INSTANCE_FUNCTION( state_map , STATE_MAP_TYPE_ID )


state_map_type * state_map_alloc( ) {
  state_map_type * map = util_malloc( sizeof * map );
  UTIL_TYPE_ID_INIT( map , STATE_MAP_TYPE_ID );
  map->state = int_vector_alloc( 0 , STATE_UNDEFINED );
  pthread_rwlock_init( &map->rw_lock , NULL);
  map->read_only = false;
  return map;
}


state_map_type * state_map_fread_alloc( const char * filename ) {
  state_map_type * map = state_map_alloc();
  if (util_file_exists( filename )) {
    FILE * stream = util_fopen( filename , "r");
    int_vector_fread( map->state , stream );
    fclose( stream );
  } 
  return map;
}

state_map_type * state_map_fread_alloc_readonly( const char * filename ) {
  state_map_type * map = state_map_fread_alloc(filename);
  map->read_only = true;
  return map;
}


state_map_type * state_map_alloc_copy( state_map_type * map ) {
  state_map_type * copy = state_map_alloc();
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    int_vector_memcpy( copy->state , map->state );
  }
  pthread_rwlock_unlock( &map->rw_lock );
  return copy;
}


void state_map_free( state_map_type * map ) {
  int_vector_free( map->state );
  free( map );
}


int state_map_get_size( state_map_type * map) {
  int size;
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    size = int_vector_size( map->state );
  }
  pthread_rwlock_unlock( &map->rw_lock );
  return size;
}


bool state_map_equal( state_map_type * map1 , state_map_type * map2) {
  bool equal = true;
  pthread_rwlock_rdlock( &map1->rw_lock );
  pthread_rwlock_rdlock( &map2->rw_lock );
  {
    if (int_vector_size( map1->state) != int_vector_size( map2->state))
      equal = false;
    
    if (equal) 
      equal = int_vector_equal( map1->state , map2->state );
  }
  pthread_rwlock_unlock( &map1->rw_lock );
  pthread_rwlock_unlock( &map2->rw_lock );
  return equal;
}


realisation_state_enum state_map_iget( state_map_type * map , int index) {
  realisation_state_enum state;
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    state = int_vector_safe_iget( map->state , index );
  }
  pthread_rwlock_unlock( &map->rw_lock );
  return state;
}

bool state_map_legal_transition( realisation_state_enum state1 , realisation_state_enum state2) {
  int target_mask = 0;

  if (state1 == STATE_UNDEFINED)
    target_mask = STATE_INITIALIZED | STATE_PARENT_FAILURE;
  else if (state1 == STATE_INITIALIZED)
    target_mask = STATE_LOAD_FAILURE | STATE_HAS_DATA | STATE_INITIALIZED | STATE_PARENT_FAILURE;
  else if (state1 == STATE_HAS_DATA)
    target_mask = STATE_INITIALIZED | STATE_LOAD_FAILURE | STATE_HAS_DATA | STATE_PARENT_FAILURE;
  else if (state1 == STATE_LOAD_FAILURE)
    target_mask = STATE_HAS_DATA | STATE_INITIALIZED | STATE_LOAD_FAILURE;
  else if (state1 == STATE_PARENT_FAILURE)
    target_mask = STATE_INITIALIZED | STATE_PARENT_FAILURE;
  
  if (state2 & target_mask)
    return true;
  else
    return false;
}

static void state_map_assert_writable( const state_map_type * map) {
  if (map->read_only)
    util_abort("%s: tried to modify read_only state_map - aborting \n",__func__);
}

static void state_map_iset__( state_map_type * map , int index , realisation_state_enum new_state) {
  realisation_state_enum current_state = int_vector_safe_iget( map->state , index );
  
  if (state_map_legal_transition( current_state , new_state ))
    int_vector_iset( map->state , index , new_state);
  else
    util_abort("%s: illegal state transition for realisation:%d %d -> %d \n" , __func__ , index , current_state , new_state );
}

void state_map_iset( state_map_type * map ,int index , realisation_state_enum state) {
  state_map_assert_writable(map);
  pthread_rwlock_wrlock( &map->rw_lock );
  {
    state_map_iset__( map , index , state );
  }
  pthread_rwlock_unlock( &map->rw_lock );
}


void state_map_update_matching( state_map_type * map , int index , int state_mask , realisation_state_enum new_state) {
  realisation_state_enum current_state = state_map_iget( map , index );
  if (current_state & state_mask)
    state_map_iset( map , index , new_state );
}


void state_map_update_undefined( state_map_type * map , int index , realisation_state_enum new_state) {
  state_map_update_matching( map , index , STATE_UNDEFINED , new_state );
}




void state_map_fwrite( state_map_type * map , const char * filename) {
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    FILE * stream = util_mkdir_fopen( filename , "w");
    if (stream) {
      int_vector_fwrite( map->state , stream );
      fclose( stream );
    } else
      util_abort("%s: failed to open:%s for writing \n",__func__ , filename );
  }
  pthread_rwlock_unlock( &map->rw_lock );
}



bool state_map_fread( state_map_type * map , const char * filename) {
  bool file_exists = false;
  pthread_rwlock_wrlock( &map->rw_lock );
  {
    if (util_file_exists( filename )) {
      FILE * stream = util_fopen( filename , "r");
      if (stream) {
        int_vector_fread( map->state , stream );
        fclose( stream );
      } else
        util_abort("%s: failed to open:%s for reading \n",__func__ , filename );
      file_exists = true;
    } else
      int_vector_reset( map->state );
  }
  pthread_rwlock_unlock( &map->rw_lock );
  return file_exists;
}


static void state_map_select_matching__( state_map_type * map , bool_vector_type * select_target , int select_mask , bool select) {
  state_map_assert_writable(map);
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    {
      const int * map_ptr = int_vector_get_ptr( map->state );
      int size = util_int_min(int_vector_size( map->state ), bool_vector_size(select_target)); 
      for (int i=0; i < size; i++) {
        int state_value = map_ptr[i];
        if (state_value & select_mask) 
          bool_vector_iset( select_target , i , select);
      }
    }
    pthread_rwlock_unlock( &map->rw_lock );
  }
}


void state_map_select_matching( state_map_type * map , bool_vector_type * select_target , int select_mask) {    
  state_map_select_matching__(map , select_target , select_mask , true );
}



 void state_map_deselect_matching( state_map_type * map , bool_vector_type * select_target , int select_mask) {
   state_map_select_matching__(map , select_target , select_mask , false );
}


static void state_map_set_from_mask__( state_map_type * map , const bool_vector_type * mask , realisation_state_enum state, bool invert) {
  const bool * mask_ptr = bool_vector_get_ptr(mask);
  for (int i=0; i < bool_vector_size( mask); i++) {
    if (mask_ptr[i] != invert)
      state_map_iset(map , i , state);
  }
}

void state_map_set_from_inverted_mask( state_map_type * state_map , const bool_vector_type * mask , realisation_state_enum state) {
  state_map_set_from_mask__(state_map , mask , state , true);
}

void state_map_set_from_mask( state_map_type * state_map , const bool_vector_type * mask , realisation_state_enum state) {
  state_map_set_from_mask__(state_map , mask , state , false);
}

bool state_map_is_readonly(const state_map_type * state_map) {
  return state_map->read_only;
}


int state_map_count_matching( state_map_type * state_map , int mask) {
   int count = 0;
   pthread_rwlock_rdlock( &state_map->rw_lock );
   {
     const int * map_ptr = int_vector_get_ptr(state_map->state);
     for (int i=0; i < int_vector_size( state_map->state ); i++) {
       int state_value = map_ptr[i];
       if (state_value & mask)
         count++;
     }
   }
   pthread_rwlock_unlock(&state_map->rw_lock);
   return count;
 }


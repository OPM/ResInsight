/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_state_map.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include <ert/util/test_work_area.h>
#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/bool_vector.h>
#include <ert/util/arg_pack.h>

#include <ert/enkf/state_map.h>
#include <ert/enkf/enkf_types.h>


void create_test() {
  state_map_type * state_map = state_map_alloc();
  test_assert_true( state_map_is_instance( state_map ));
  test_assert_int_equal( 0 , state_map_get_size( state_map ));
  state_map_free( state_map );
}

void get_test( ) {
  state_map_type * state_map = state_map_alloc();
  test_assert_int_equal( STATE_UNDEFINED , state_map_iget( state_map , 0 ));
  test_assert_int_equal( STATE_UNDEFINED , state_map_iget( state_map , 100 ));
  state_map_free( state_map );
}

void set_test( ) {
  state_map_type * state_map = state_map_alloc();
  state_map_iset( state_map , 0 , STATE_INITIALIZED );
  test_assert_int_equal( STATE_INITIALIZED , state_map_iget( state_map , 0 ));

  state_map_iset( state_map , 100 , STATE_INITIALIZED );
  test_assert_int_equal( STATE_INITIALIZED , state_map_iget( state_map , 100 ));

  test_assert_int_equal( STATE_UNDEFINED , state_map_iget( state_map , 50 ));
  test_assert_int_equal( 101 , state_map_get_size( state_map ));
  state_map_free( state_map );
}


void load_empty_test() {
  state_map_type * state_map = state_map_fread_alloc( "File/does/not/exists" );
  test_assert_true( state_map_is_instance( state_map ));
  test_assert_int_equal( 0 , state_map_get_size( state_map ));
  state_map_free( state_map );
}


void test_equal() {
  state_map_type * state_map1 = state_map_alloc();
  state_map_type * state_map2 = state_map_alloc();

  test_assert_true( state_map_equal( state_map1 , state_map2 ));
  for (int i =0; i < 25; i++) {
    state_map_iset( state_map1 , i , STATE_INITIALIZED );
    state_map_iset( state_map2 , i , STATE_INITIALIZED );
  }
  test_assert_true( state_map_equal( state_map1 , state_map2 ));

  state_map_iset( state_map2 , 15 , STATE_HAS_DATA );
  test_assert_false( state_map_equal( state_map1 , state_map2 ));
  state_map_iset( state_map2 , 15 , STATE_LOAD_FAILURE );
  state_map_iset( state_map2 , 15 , STATE_INITIALIZED );
  test_assert_true( state_map_equal( state_map1 , state_map2 ));
  
  state_map_iset( state_map2 , 150 , STATE_INITIALIZED );
  test_assert_false( state_map_equal( state_map1 , state_map2 ));
}


void test_copy() {
  state_map_type * state_map = state_map_alloc();
  state_map_iset( state_map , 0 , STATE_INITIALIZED );
  state_map_iset( state_map , 100 , STATE_INITIALIZED );
  {
    state_map_type * copy = state_map_alloc_copy( state_map );
    test_assert_true( state_map_equal( copy , state_map ));

    state_map_iset( state_map , 10 , STATE_INITIALIZED );
    test_assert_false( state_map_equal( copy , state_map ));                      
    
    state_map_free( copy );
  }
  state_map_free( state_map );
}


void test_io( ) {
  test_work_area_type * work_area = test_work_area_alloc( "enkf-state-map" , false );
  {
    state_map_type * state_map = state_map_alloc();
    state_map_type * copy1 , *copy2;
    state_map_iset( state_map , 0 , STATE_INITIALIZED );
    state_map_iset( state_map , 100 , STATE_INITIALIZED );
    state_map_fwrite( state_map , "map");
    
    copy1 = state_map_fread_alloc( "map" );
    test_assert_true( state_map_equal( state_map , copy1 ));
    
    copy2 = state_map_alloc();
    state_map_fread( copy2 , "map" );
    test_assert_true( state_map_equal( state_map , copy2 ));

    state_map_iset( copy2 , 67 , STATE_INITIALIZED );
    test_assert_false(state_map_equal( state_map , copy2 ));
    
    state_map_fread( copy2 , "map");
    test_assert_true( state_map_equal( state_map , copy2 ));

    state_map_fread( copy2 , "DoesNotExis");
    test_assert_int_equal( 0 , state_map_get_size( copy2 ));
  }
  test_work_area_free( work_area );
}



void test_update_undefined( ) {
  state_map_type * map = state_map_alloc( );
  
  state_map_iset( map , 10 , STATE_INITIALIZED );
  test_assert_int_equal( STATE_UNDEFINED , state_map_iget( map , 5 ) );
  test_assert_int_equal( STATE_INITIALIZED , state_map_iget( map , 10 ) );

  state_map_update_undefined( map , 5 , STATE_INITIALIZED );
  test_assert_int_equal( STATE_INITIALIZED , state_map_iget( map , 5 ) );
  
  state_map_update_undefined( map , 10 , STATE_INITIALIZED );
  test_assert_int_equal( STATE_INITIALIZED , state_map_iget( map , 10 ) );
  
  state_map_free( map );
}


void test_select_matching( ) {
  state_map_type * map = state_map_alloc( );
  bool_vector_type * mask1 = bool_vector_alloc(0 , false);
  bool_vector_type * mask2 = bool_vector_alloc(1000 , true);

  state_map_iset( map , 10 , STATE_INITIALIZED );
  state_map_iset( map , 10 , STATE_HAS_DATA );
  state_map_iset( map , 20 , STATE_INITIALIZED );
  state_map_select_matching( map , mask1 , STATE_HAS_DATA | STATE_INITIALIZED );
  state_map_select_matching( map , mask2 , STATE_HAS_DATA | STATE_INITIALIZED );
  
  test_assert_int_equal( state_map_get_size( map ) , bool_vector_size( mask1 ));
  
  for (int i=0; i < bool_vector_size( mask1 ); i++) {
    if (i==10)
      test_assert_true( bool_vector_iget( mask1 , i ));
    else if (i== 20)
      test_assert_true( bool_vector_iget( mask1 , i ));
    else {
      test_assert_false( bool_vector_iget( mask1 , i ));
      test_assert_true( bool_vector_iget( mask2 , i ));
    }
  }
    
  bool_vector_free( mask1 );
  bool_vector_free( mask2 );
  state_map_free( map );
}


void test_deselect_matching( ) {
  state_map_type * map = state_map_alloc( );
  bool_vector_type * mask1 = bool_vector_alloc(0 , false);
  bool_vector_type * mask2 = bool_vector_alloc(1000 , true);

  state_map_iset( map , 10 , STATE_INITIALIZED );
  state_map_iset( map , 10 , STATE_HAS_DATA );
  state_map_iset( map , 20 , STATE_INITIALIZED );
  state_map_deselect_matching( map , mask1 , STATE_HAS_DATA | STATE_INITIALIZED );
  state_map_deselect_matching( map , mask2 , STATE_HAS_DATA | STATE_INITIALIZED );
  
  test_assert_int_equal( state_map_get_size( map ) , bool_vector_size( mask1 ));
  
  for (int i=0; i < bool_vector_size( mask1 ); i++) {
    if (i==10)
      test_assert_false( bool_vector_iget( mask1 , i ));
    else if (i== 20)
      test_assert_false( bool_vector_iget( mask2 , i ));
    else {
      test_assert_false( bool_vector_iget( mask1 , i ));
      test_assert_true( bool_vector_iget( mask2 , i ));
    }
  }
    
  bool_vector_free( mask1 );
  bool_vector_free( mask2 );
  state_map_free( map );
}


void test_set_from_mask() {
  int i;
  state_map_type * map1 = state_map_alloc();
  state_map_type * map2 = state_map_alloc();
  bool_vector_type * mask = bool_vector_alloc(0, false);
  bool_vector_iset(mask , 10 , true);
  bool_vector_iset(mask , 20 , true);

  state_map_set_from_mask(map1 , mask , STATE_INITIALIZED);
  state_map_set_from_inverted_mask(map2 , mask , STATE_INITIALIZED);
  test_assert_int_equal(21 , state_map_get_size(map1));
  test_assert_int_equal(21 , state_map_get_size(map2));
  for (i = 0; i < state_map_get_size(map1); i++) {
    if (i == 10 || i== 20) {
      test_assert_int_equal( STATE_INITIALIZED , state_map_iget( map1 , i) );
      test_assert_int_equal( STATE_UNDEFINED , state_map_iget(map2 , i));
    }
    else {
      test_assert_int_equal(STATE_UNDEFINED , state_map_iget(map1 , i ));
      test_assert_int_equal( STATE_INITIALIZED , state_map_iget(map2 , i));
    }


  }
}


void test_count_matching() {
  state_map_type * map1 = state_map_alloc();
  state_map_iset(map1 , 10 , STATE_INITIALIZED );

  state_map_iset(map1 , 15 , STATE_INITIALIZED );
  state_map_iset(map1 , 15 , STATE_HAS_DATA );

  state_map_iset(map1 , 16 , STATE_INITIALIZED );
  state_map_iset(map1 , 16 , STATE_HAS_DATA );
  state_map_iset(map1 , 16 , STATE_LOAD_FAILURE );
  
  test_assert_int_equal( 1 , state_map_count_matching( map1 , STATE_HAS_DATA));
  test_assert_int_equal( 2 , state_map_count_matching( map1 , STATE_HAS_DATA | STATE_LOAD_FAILURE));
  test_assert_int_equal( 3 , state_map_count_matching( map1 , STATE_HAS_DATA | STATE_LOAD_FAILURE | STATE_INITIALIZED));

  state_map_free( map1 );
}

int main(int argc , char ** argv) {
  create_test();
  get_test();
  set_test();
  load_empty_test();
  test_equal();
  test_copy();
  test_io();
  test_update_undefined( );
  test_select_matching();
  test_count_matching();
  exit(0);
}


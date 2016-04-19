/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ert_util_struct_vector.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include "ert/util/build_config.h"
#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/struct_vector.h>



struct test_struct {
  int x;
  double y;
  int z;
};



void test_create() {
  struct test_struct d;
  struct_vector_type * struct_vector = struct_vector_alloc( sizeof d );
  test_assert_true( struct_vector_is_instance( struct_vector ));
  test_assert_int_equal( struct_vector_get_size( struct_vector ) , 0 );
  struct_vector_free( struct_vector );
}

void alloc_invalid() {
  struct_vector_alloc( 0 );
}


void test_create_invalid() {
  test_assert_util_abort( "struct_vector_alloc" , alloc_invalid , NULL );
}

void test_append_iget() {
  struct test_struct d1,d2;
  struct_vector_type * struct_vector = struct_vector_alloc( sizeof d1 );
  d1.x = 100;
  d1.y = 99;
  d1.z = 234;

  struct_vector_append( struct_vector , &d1 );
  test_assert_int_equal( struct_vector_get_size( struct_vector ) , 1 );
  
  test_assert_false( d1.x == d2.x );
  test_assert_false( d1.y == d2.y );
  test_assert_false( d1.z == d2.z );

  struct_vector_iget( struct_vector , 0 , &d2);
  test_assert_true( d1.x == d2.x );
  test_assert_true( d1.y == d2.y );
  test_assert_true( d1.z == d2.z );

  struct_vector_reset( struct_vector );
  test_assert_int_equal( struct_vector_get_size( struct_vector ) , 0 );

  struct_vector_free( struct_vector );
}




int main(int argc , char ** argv) {
  test_create();
  test_create_invalid();
  test_append_iget();
}

/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_string_util.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdarg.h>

#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/test_util.h>
#include <ert/util/string_util.h>


void test_int_vector(const int_vector_type * list , int length , ...) {
  va_list ap;
  int i;
  va_start(ap , length);
  test_assert_int_equal( length , int_vector_size( list ));

  for (i =0; i < int_vector_size( list ); i++) {
    int value = va_arg(ap , int);
    test_assert_int_equal( int_vector_iget( list , i ) , value);
  }

  va_end(ap);
}



void test_active_list() {
  int_vector_type * active_list = string_util_alloc_active_list("1,3- 10,15");
  test_assert_true( string_util_init_active_list("1,3- 10,15" , active_list) );
  test_int_vector( active_list , 10 , 1,3,4,5,6,7,8,9,10,15);
  
  test_assert_true( string_util_update_active_list("1,3- 10,15,8" , active_list) );
  test_int_vector( active_list , 10 , 1,3,4,5,6,7,8,9,10,15);

  test_assert_false( string_util_update_active_list("1,X" , active_list) );
  test_int_vector( active_list , 10 , 1,3,4,5,6,7,8,9,10,15);

  test_assert_true( string_util_update_active_list("14-16" , active_list) );
  test_int_vector( active_list , 12 , 1,3,4,5,6,7,8,9,10,14,15,16);
  
  test_assert_true( string_util_update_active_list("0" , active_list) );
  test_int_vector( active_list , 13 , 0,1,3,4,5,6,7,8,9,10,14,15,16);

  test_assert_true( string_util_update_active_list("4-6" , active_list) );
  test_int_vector( active_list , 13 , 0,1,3,4,5,6,7,8,9,10,14,15,16);
}


static void test2( const bool_vector_type * active_mask ) {
  test_assert_int_equal( bool_vector_size( active_mask ), 16 );

  test_assert_true( bool_vector_iget( active_mask , 1 ));
  test_assert_true( bool_vector_iget( active_mask , 3 ));
  test_assert_true( bool_vector_iget( active_mask , 4 ) );
  test_assert_true( bool_vector_iget( active_mask , 9 ));
  test_assert_true( bool_vector_iget( active_mask , 10 ));
  test_assert_true( bool_vector_iget( active_mask , 15 ));
  
  test_assert_false( bool_vector_iget( active_mask , 0 ));
  test_assert_false( bool_vector_iget( active_mask , 2 ));
  test_assert_false( bool_vector_iget( active_mask , 11 ));
  test_assert_false( bool_vector_iget( active_mask , 14 ));
}



void test_active_mask() {
  bool_vector_type * active_mask = string_util_alloc_active_mask("1,3 -6,6-  10, 15");
  
  test2( active_mask );
  test_assert_true( string_util_init_active_mask("1,3- 10,15" , active_mask));
  test2( active_mask );

  test_assert_false( string_util_update_active_mask("11,X" , active_mask));
  test2( active_mask );

  bool_vector_free( active_mask );
}



void test_value_list() {
  {
    int_vector_type * int_vector = string_util_alloc_value_list("1,2,4-7");
    test_int_vector( int_vector , 6 , 1,2,4,5,6,7);
    int_vector_free( int_vector );
  }

  {
    int_vector_type * int_vector = string_util_alloc_value_list("1,2,X");
    test_int_vector( int_vector , 0);
    int_vector_free( int_vector );
  }

  {
    int_vector_type * int_vector = string_util_alloc_value_list("1,2,4-7");
    test_int_vector( int_vector , 6 , 1,2,4,5,6,7);
    test_assert_false( string_util_update_value_list("1,2,X" , int_vector ));
    int_vector_free( int_vector );
  }

  {
    int_vector_type * int_vector = string_util_alloc_value_list("5,5,5,5");
    test_int_vector( int_vector , 4,5,5,5,5);
    test_assert_true( string_util_update_value_list("1-5" , int_vector ));
    test_int_vector( int_vector , 9,5,5,5,5,1,2,3,4,5 );
    test_assert_true( string_util_update_value_list("1-5" , int_vector ));
    test_int_vector( int_vector , 14,5,5,5,5,1,2,3,4,5,1,2,3,4,5 );
    int_vector_free( int_vector );
  }

  {
    int_vector_type * int_vector = int_vector_alloc(0 , 77 );
    int_vector_append(int_vector , 125 );
    string_util_init_value_list("1-5" , int_vector );
    test_int_vector( int_vector , 5,1,2,3,4,5);
    int_vector_free( int_vector );
  }
  
}




int main(int argc , char ** argv) {
  test_active_list();
  test_active_mask();
  test_value_list();
  exit(0);
}

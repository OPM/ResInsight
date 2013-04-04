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

#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/test_util.h>
#include <ert/util/string_util.h>


static void test1( const int_vector_type * active_list ) {
  test_assert_int_equal( int_vector_size( active_list ), 10 );
  test_assert_int_equal( int_vector_iget( active_list , 0 )  , 1 );
  test_assert_int_equal( int_vector_iget( active_list , 1 )  , 3);
  test_assert_int_equal( int_vector_iget( active_list , 8 )  ,10 );
  test_assert_int_equal( int_vector_iget( active_list , 9 )  ,15 );
}


void test_active_list() {
  int_vector_type * active_list = string_util_alloc_active_list("1,3- 10,15");
  test_assert_true( string_util_init_active_list("1,3- 10,15" , active_list) );
  test1( active_list );
  
  test_assert_true( string_util_update_active_list("1,3- 10,15,8" , active_list) );
  test1( active_list );
  test_assert_false( string_util_update_active_list("1,X" , active_list) );
  test1( active_list );

  test_assert_true( string_util_update_active_list("14-16" , active_list) );
  test_assert_int_equal( int_vector_size( active_list )    , 12);
  test_assert_int_equal( int_vector_iget( active_list , 9 )  ,14 );
  test_assert_int_equal( int_vector_iget( active_list , 11 )  ,16 );
  
  test_assert_true( string_util_update_active_list("0" , active_list) );
  test_assert_int_equal( int_vector_size( active_list ) , 13);
  test_assert_int_equal( int_vector_iget( active_list , 0 )  ,0 );
  test_assert_int_equal( int_vector_iget( active_list , 10 )  ,14 );
  test_assert_int_equal( int_vector_iget( active_list , 12 )  ,16 );

  test_assert_true( string_util_update_active_list("4-6" , active_list) );
  test_assert_int_equal( int_vector_size( active_list ) , 13);
  test_assert_int_equal( int_vector_iget( active_list , 0 )  ,0 );
  test_assert_int_equal( int_vector_iget( active_list , 10 )  ,14 );
  test_assert_int_equal( int_vector_iget( active_list , 12 )  ,16 );
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




int main(int argc , char ** argv) {
  test_active_list();
  test_active_mask();
  exit(0);
}

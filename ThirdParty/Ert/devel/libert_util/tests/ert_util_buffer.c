/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'ert_util_buffer.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>
#include <ert/util/buffer.h>



void test_create() {
  buffer_type * buffer = buffer_alloc( 1024 );
  test_assert_true( buffer_is_instance( buffer ));
  buffer_free( buffer );
}


void test_buffer_strchr() {
  buffer_type * buffer = buffer_alloc( 1024 );
  test_assert_false( buffer_strchr( buffer , 'A'));
  buffer_fwrite_char_ptr(buffer , "XX AB AB AB");
  test_assert_false( buffer_strchr( buffer , 'A'));

  buffer_rewind( buffer );
  test_assert_true( buffer_strchr( buffer , 'A')); buffer_fskip(buffer , 1);
  test_assert_true( buffer_strchr( buffer , 'A')); buffer_fskip(buffer , 1);
  test_assert_true( buffer_strchr( buffer , 'A')); buffer_fskip(buffer , 1);
  test_assert_false( buffer_strchr( buffer , 'A'));
  buffer_free( buffer );
}

void test_buffer_strstr() {
  buffer_type * buffer = buffer_alloc( 1024 );
  test_assert_false( buffer_strstr( buffer , "Hello World" ));
  test_assert_false( buffer_strstr( buffer , "" ));

  buffer_fwrite_char_ptr( buffer , "ABC");
  test_assert_int_equal( buffer_get_size( buffer ), 4 );
  test_assert_false( buffer_strstr( buffer , "ABC"));

  buffer_rewind( buffer );
  test_assert_true( buffer_strstr( buffer , "ABC"));
  test_assert_int_equal( buffer_get_offset(buffer) , 0);
  test_assert_string_equal( "ABC" , buffer_get_data(buffer));

  {
    size_t pos = buffer_get_offset( buffer );
    test_assert_false( buffer_strstr( buffer , "ABCD" ));
    test_assert_size_t_equal( pos , buffer_get_offset( buffer ));
  }
  buffer_rewind( buffer );
  test_assert_true( buffer_strstr( buffer , "AB" ));
  test_assert_true( buffer_strstr( buffer , "ABC" ));
  test_assert_true( buffer_strstr( buffer , "BC" ));
  test_assert_false( buffer_strstr( buffer , "ABC" ));
}




void test_buffer_search_replace1() {
  buffer_type * buffer = buffer_alloc( 1024 );
  test_assert_false( buffer_search_replace( buffer , "" , "XYZ"));
  test_assert_false( buffer_search_replace( buffer , "XYZ" , "ABC"));
  test_assert_false( buffer_search_replace( buffer , "XYZ" , ""));

  buffer_fwrite_char_ptr(buffer , "ABC 123");
  buffer_rewind(buffer);
  test_assert_true( buffer_search_replace( buffer, "ABC" , "XYZ" ));
  buffer_rewind( buffer );
  test_assert_string_equal( "XYZ 123" , buffer_get_data( buffer ));

  buffer_rewind( buffer );
  test_assert_true( buffer_search_replace( buffer, "XYZ" , "A"));
  buffer_rewind( buffer );
  test_assert_string_equal( "A 123" , buffer_get_data( buffer ));

  buffer_rewind( buffer );
  test_assert_true( buffer_search_replace( buffer, "A", "XYZ"));
  buffer_rewind( buffer );
  test_assert_string_equal( "XYZ 123" , buffer_get_data( buffer ));

  buffer_free( buffer );
}


void test_buffer_search_replace2() {
  buffer_type * buffer = buffer_alloc( 1024 );
  buffer_fwrite_char_ptr(buffer , "MAGIC_PRINT  magic-list.txt  <ERTCASE>  __MAGIC__");

  buffer_rewind( buffer );
  test_assert_false( buffer_search_replace( buffer , "<CASE>" , "SUPERCase"));
  test_assert_string_equal( "MAGIC_PRINT  magic-list.txt  <ERTCASE>  __MAGIC__" , buffer_get_data( buffer));

  buffer_rewind( buffer );
  test_assert_true( buffer_search_replace( buffer , "<ERTCASE>" , "default"));
  test_assert_string_equal( "MAGIC_PRINT  magic-list.txt  default  __MAGIC__" , buffer_get_data( buffer));


  buffer_free( buffer );
}


void test_char_ptr( ) {
  buffer_type * buffer = buffer_alloc(1024);
  buffer_fwrite_char_ptr( buffer , "Hello World");
  test_assert_size_t_equal( buffer_get_size( buffer ) , 12 );
  test_assert_int_equal( strlen( buffer_get_data( buffer )) , 11);

  buffer_clear( buffer );
  buffer_fwrite_char_ptr(buffer , "Hello" );
  buffer_strcat( buffer , " " );
  buffer_strcat( buffer , "World" );
  test_assert_size_t_equal( buffer_get_size( buffer ) , 12 );
  test_assert_int_equal( strlen( buffer_get_data( buffer )) , 11);

  buffer_free( buffer );
}


int main( int argc , char ** argv) {
  test_create();
  test_char_ptr();
  test_buffer_strchr();
  test_buffer_strstr();
  test_buffer_search_replace1();
  test_buffer_search_replace2();
  exit(0);
}

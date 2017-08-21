/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_hash_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/hash.h>

int main(int argc , char ** argv) {
  
  hash_type * h = hash_alloc();

  test_assert_bool_equal( hash_add_option( h , "Key" ) , false );
  test_assert_false( hash_add_option( h , "Key" ) );

  test_assert_true( hash_add_option( h , "Key1:Value" ) );
  test_assert_true( hash_add_option( h , "Key2:Value1:Value2" ) );
  test_assert_true( hash_add_option( h , "Key3:Value1:value2:Value3" ) );
  
  test_assert_string_equal( hash_get( h , "Key1" ) , "Value" );
  test_assert_string_equal( hash_get( h , "Key2" ) , "Value1:Value2" );
  test_assert_string_equal( hash_get( h , "Key3" ) , "Value1:value2:Value3" );

  test_assert_false( hash_has_key( h , "Key" ));

  hash_free( h );
  exit(0);
}

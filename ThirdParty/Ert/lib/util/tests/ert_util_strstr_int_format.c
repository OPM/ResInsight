/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_str_str_int_format.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/test_util.h>


void test_util_strstr() {
  test_assert_NULL( util_strstr_int_format("StringWithoutIntFormatSpecifier") );
  {
    const char * source = "String with %d ---";
    char * next = util_strstr_int_format( source );
    test_assert_ptr_equal( next , &source[13] );
  }

  {
    const char * source = "String with %04d ---";
    char * next = util_strstr_int_format( source );
    test_assert_ptr_equal( next , &source[15] );
  }


  {
    const char * source = "String with %ld ---";
    test_assert_NULL( util_strstr_int_format( source ));
  }


  {
    const char * source = "String with %3d ---";
    test_assert_NULL( util_strstr_int_format( source ));
  }
}



void test_util_count_int_format() {
  test_assert_int_equal( 0 , util_int_format_count("Abcdddd"));
  test_assert_int_equal( 0 , util_int_format_count("%4d"));
  test_assert_int_equal( 0 , util_int_format_count("%ld"));
  test_assert_int_equal( 1 , util_int_format_count("%04d"));
  test_assert_int_equal( 2 , util_int_format_count("%04dXX%034d"));
  test_assert_int_equal( 4 , util_int_format_count("%04dXX%034d%d%d"));
}

int main(int argc , char ** argv) {
  test_util_strstr();
  test_util_count_int_format();
  exit(0);
}

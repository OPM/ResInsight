/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_sum_case_exists.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/path_stack.h>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_sum.h>


void test_case( const char * sum_case , bool expected_exist) {
  test_assert_bool_equal( expected_exist , ecl_sum_case_exists( sum_case ));
}


void test_case_no_path( const char * sum_case , bool expected_exist) {
  path_stack_type * path_stack = path_stack_alloc();
  path_stack_push_cwd( path_stack );
  {
    char * basename , *path;
    
    util_alloc_file_components( sum_case , &path , &basename , NULL );
    if (path)
      chdir( path );
    test_assert_bool_equal(expected_exist ,  ecl_sum_case_exists( basename ));
    
    util_safe_free( path );
    util_safe_free( basename );
  }
  path_stack_pop( path_stack );
  path_stack_free( path_stack );
}


int main( int argc , char ** argv) {
  const char * existing_case = argv[1];
  const char * missing_header = argv[2];
  const char * missing_data   = argv[3];
  test_assert_false( ecl_sum_case_exists( "/does/not/exist" ));
  
  test_case( existing_case , true);
  test_case_no_path( existing_case , true);

  test_case( missing_header , false);
  test_case_no_path( missing_header , false);

  test_case( missing_data , false);
  test_case_no_path( missing_data , false);
}

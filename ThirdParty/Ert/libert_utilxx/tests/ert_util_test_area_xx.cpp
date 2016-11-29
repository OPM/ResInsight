/*
   Copyright (C) 2016 Statoil ASA, Norway.

   This is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or1
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>

#include <ert/util/TestArea.hpp>

// Warning: this test hardcodes an assumption of the name
// of the executable running as argv[0].
#define LOCAL_ARGV0 "ert_util_test_area_xx"


void test_enter( const char* argv0 ) {
  ERT::TestArea ta;
  test_assert_throw( ta.copyFile( argv0 ) , std::runtime_error );

  ta.enter("test/enter");
  ta.copyFile( argv0 );
  test_assert_true( util_file_exists( LOCAL_ARGV0 ));
}


void test_copy( const char* argv0 ) {
   ERT::TestArea ta("test/copy");

   test_assert_throw( ta.copyFile( "does/not/exist" ) , std::invalid_argument );
   test_assert_throw( ta.copyDirectory( argv0 ) , std::invalid_argument );

   ta.copyFile( argv0 );
   test_assert_true( util_file_exists( LOCAL_ARGV0 ));

   util_unlink_existing( LOCAL_ARGV0 );
   test_assert_false( util_file_exists( LOCAL_ARGV0));

   ta.copyParentContent( argv0 );
   test_assert_true( util_file_exists( LOCAL_ARGV0));

   {
       ERT::TestArea ta2("test2/copy");
       ta2.copyFile( LOCAL_ARGV0 );
       test_assert_true( util_file_exists( LOCAL_ARGV0));
   }
}




void test_create() {
  char * cwd0 = util_alloc_cwd();
  char * cwd1;
  {
    ERT::TestArea ta("test/area");
    cwd1 = util_alloc_cwd();
    test_assert_string_not_equal( cwd0 , cwd1 );
    test_assert_string_equal( cwd1 , ta.getCwd().c_str());
    test_assert_string_equal( cwd0 , ta.getOriginalCwd( ).c_str() );
  }
  test_assert_false( util_is_directory(cwd1) );
  free( cwd1 );
  free( cwd0 );
}


int main(int argc , char **argv) {
  test_create();

  test_assert_true( util_file_exists( argv[0] ));
  test_copy( argv[0] );
  test_enter( argv[0] );
}

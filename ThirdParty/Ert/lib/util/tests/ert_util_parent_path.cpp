/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ert_util_parent_path.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <ert/util/util.h>
#include <ert/util/test_util.hpp>


void test_path(const char * expected_parent , const char * input_path ) {
  char * parent_path = util_alloc_parent_path( input_path );
  test_assert_string_equal( expected_parent , parent_path);
  free( parent_path );
}



int main(int argc , char ** argv) {

  test_path("" , "path");
  test_path(NULL , "");
  test_path(NULL , NULL);

  test_path("path/parent" , "path/parent/leaf");
  test_path("/path/parent" , "/path/parent/leaf");
  test_path("/path/parent" , "/path/parent/leaf/");
  test_path("/path/parent" , "/path/parent/leaf/../leaf");
  test_path("/path" , "/path/parent/leaf/..");

  test_path("path/parent" , "path/parent/leaf/../leaf");
  test_path("path"        , "path/parent/leaf/..");


  exit(0);
}

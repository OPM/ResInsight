/*
   Copyright (C) 2017  Equinor ASA, Norway.

   The file 'ert_util_chdir.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>


void test_chdir() {
  ecl::util::TestArea ta("chdir");
  const char * cwd = ta.test_cwd().c_str();

  test_assert_false( util_chdir_file( "/file/does/not/exist"));
  test_assert_false( util_chdir_file( cwd ));
  {
    FILE * stream = util_mkdir_fopen("path/FILE","w");
    fclose( stream );
  }
  test_assert_true( util_chdir_file( "path/FILE" ));
  {
    char * new_cwd = util_alloc_cwd();
    char * fname = util_alloc_filename(cwd, "path", NULL);
    test_assert_string_equal( new_cwd, fname );
    free(new_cwd);
    free(fname);
  }
}


int main(int argc , char ** argv) {
  test_chdir( );
  exit(0);
}

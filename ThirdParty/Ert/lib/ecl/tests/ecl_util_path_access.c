/*
   Copyright (C) 2018  Statoil ASA, Norway.

   The file 'ecl_util_path_access.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/test_util.h>

#include <ert/ecl/ecl_util.h>


void test_relative_access() {
  test_work_area_type * work_area = test_work_area_alloc("access");
  test_assert_false( ecl_util_path_access("No/directory/does/not/exist"));

  util_make_path("path");
  test_assert_true( ecl_util_path_access("path"));
  test_assert_true( ecl_util_path_access("path/FILE_DOES_NOT_EXIST"));

  {
    FILE * f = util_fopen("path/file", "w");
    fprintf(f,"Hello\n");
    fclose(f);
  }
  test_assert_true( ecl_util_path_access("path/file"));
  chmod("path/file", 0);
  test_assert_false( ecl_util_path_access("path/file"));

  test_assert_true( ecl_util_path_access("ECLIPSE_CASE"));
  test_work_area_free( work_area );
}


int main(int argc, char ** argv) {
  test_relative_access();
}

/*
   Copyright (C) 2018  Statoil ASA, Norway.

   The file 'ert_util_mkdir_p.c' is part of ERT - Ensemble based Reservoir Tool.

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


int main(int argc , char ** argv) {
  test_work_area_type * work_area = test_work_area_alloc("Test_area");

  // Regular use
  test_assert_true( util_mkdir_p("some/path/with/many/levels"));


  // Absolute path where the root exists.
  {
    char * abs_path = util_alloc_abs_path("a/path/with/abs/prefix");
    test_assert_true( util_mkdir_p(abs_path));

    // Already exists:
    test_assert_true( util_mkdir_p(abs_path));
    free(abs_path);
  }

  // Permission denied
  test_assert_true(util_mkdir_p("read_only"));
  chmod("read_only", 0555);
  test_assert_false(util_mkdir_p("read_only/no/not/this"));

  test_work_area_free(work_area);
  exit(0);
}

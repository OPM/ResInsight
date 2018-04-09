/*
   Copyright (C) 2018  Statoil ASA, Norway.

   The file 'ecl_util_filenames.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_work_area.h>
#include <ert/util/test_util.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>


void test_filename_report_nr() {
  test_assert_int_equal(78, ecl_util_filename_report_nr("Path/with/mixedCASE/case.x0078"));
  test_assert_int_equal(78, ecl_util_filename_report_nr("Case.X0078"));
  test_assert_int_equal(ECL_EGRID_FILE, ecl_util_get_file_type("path/WITH/xase/MyGrid.EGrid", NULL, NULL));
}

void test_filename_case() {
  test_assert_NULL( ecl_util_alloc_filename(NULL, "mixedBase", ECL_EGRID_FILE, false, -1));
  test_assert_string_equal( ecl_util_alloc_filename(NULL, "UPPER", ECL_EGRID_FILE, false, -1), "UPPER.EGRID");
  test_assert_string_equal( ecl_util_alloc_filename(NULL , "lower", ECL_EGRID_FILE, false, -1), "lower.egrid");
}


void test_file_list() {
  test_work_area_type * work_area = test_work_area_alloc("RESTART_FILES");
  stringlist_type * s = stringlist_alloc_new();

  for (int i = 0; i < 10; i += 2) {
    char * fname = ecl_util_alloc_filename(NULL, "case", ECL_RESTART_FILE, true, i);
    FILE * stream = util_fopen(fname, "w");
    fclose(stream);
    free( fname);
  }

  for (int i = 0; i < 10; i += 2) {
    char * fname = util_alloc_sprintf("Case.F%04d", i);
    FILE * stream = util_fopen(fname, "w");
    fclose(stream);
    free( fname);
  }


  ecl_util_select_filelist(NULL , "case" , ECL_RESTART_FILE, true, s);
  test_assert_int_equal( stringlist_get_size(s), 5);
  for (int i = 0; i < 5; i++) {
    char * fname = ecl_util_alloc_filename(NULL, "case", ECL_RESTART_FILE, true, 2*i);
    test_assert_string_equal( fname, stringlist_iget(s,i));
    free( fname);
  }

  ecl_util_select_filelist(NULL , "Case" , ECL_RESTART_FILE, true, s);
  test_assert_int_equal( stringlist_get_size(s), 0);


  util_make_path("path");
  for (int i=0; i < 10; i++) {
    char * summary_file1 = ecl_util_alloc_filename("path", "CASE1", ECL_SUMMARY_FILE, false, i );
    char * summary_file2 = ecl_util_alloc_filename("path", "CASE2", ECL_SUMMARY_FILE, true, i);
    char * restart_file1 = ecl_util_alloc_filename("path", "CASE1", ECL_RESTART_FILE, false, i);

    FILE * f1 = fopen(summary_file1, "w");
    fclose(f1);

    FILE * f2 = fopen(summary_file2, "w");
    fclose(f2);

    FILE * f3 = fopen(restart_file1, "w");
    fclose(f3);

    free(summary_file1);
    free(summary_file2);
    free(restart_file1);
  }
  printf("-----------------------------------------------------------------\n");
  ecl_util_select_filelist(NULL, "path/CASE1", ECL_SUMMARY_FILE, false, s);
  test_assert_int_equal(stringlist_get_size(s), 10);

  ecl_util_select_filelist(NULL, "path/CASE1", ECL_SUMMARY_FILE, true, s);
  test_assert_int_equal(stringlist_get_size(s), 0);

  ecl_util_select_filelist(NULL, "path/CASE2", ECL_SUMMARY_FILE, true, s);
  test_assert_int_equal(stringlist_get_size(s), 10);

  ecl_util_select_filelist("path", "CASE1", ECL_SUMMARY_FILE, false, s);
  test_assert_int_equal(stringlist_get_size(s), 10);

  ecl_util_select_filelist("path", "CASE1", ECL_SUMMARY_FILE, true, s);
  test_assert_int_equal(stringlist_get_size(s), 0);

  ecl_util_select_filelist("path", "CASE2", ECL_SUMMARY_FILE, true, s);
  test_assert_int_equal(stringlist_get_size(s), 10);

  stringlist_free(s);
  test_work_area_free(work_area);
}


int main(int argc , char ** argv) {
  test_filename_report_nr();
  test_filename_case();
  test_file_list();
}

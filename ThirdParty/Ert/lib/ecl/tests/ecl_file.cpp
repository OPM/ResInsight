
/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_file.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>

#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_file_view.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_endian_flip.hpp>

void test_writable(size_t data_size) {
  test_work_area_type * work_area = test_work_area_alloc("ecl_file_writable");
  const char * data_file_name = "test_file";

  ecl_kw_type * kw = ecl_kw_alloc("TEST_KW", data_size, ECL_INT);
  for(size_t i = 0; i < data_size; ++i)
    ecl_kw_iset_int(kw, i, ((i*37)+11)%data_size);

  fortio_type * fortio = fortio_open_writer(data_file_name, false, true);
  ecl_kw_fwrite(kw, fortio);
  fortio_fclose(fortio);

  for(int i = 0; i < 4; ++i) {
    ecl_file_type * ecl_file = ecl_file_open(data_file_name, ECL_FILE_WRITABLE);
    ecl_kw_type * loaded_kw = ecl_file_view_iget_kw(
                                  ecl_file_get_global_view(ecl_file),
                                  0);
    test_assert_true(ecl_kw_equal(kw, loaded_kw));

    ecl_file_save_kw(ecl_file, loaded_kw);
    ecl_file_close(ecl_file);
  }

  ecl_kw_free(kw);
  test_work_area_free( work_area );
}

void test_truncated() {
  test_work_area_type * work_area = test_work_area_alloc("ecl_file_truncated" );
  int num_kw;
  {
    ecl_grid_type * grid = ecl_grid_alloc_rectangular(20,20,20,1,1,1,NULL);
    ecl_grid_fwrite_EGRID2( grid , "TEST.EGRID", ECL_METRIC_UNITS );
    ecl_grid_free( grid );
  }
  {
    ecl_file_type * ecl_file = ecl_file_open("TEST.EGRID" , 0 );
    test_assert_true( ecl_file_is_instance( ecl_file ) );
    num_kw = ecl_file_get_size( ecl_file );
    ecl_file_close( ecl_file );
  }

  {
    offset_type file_size = util_file_size( "TEST.EGRID");
    FILE * stream = util_fopen("TEST.EGRID" , "r+");
    util_ftruncate( stream , file_size / 2 );
    fclose( stream );
  }
  {
    ecl_file_type * ecl_file = ecl_file_open("TEST.EGRID" , 0 );
    test_assert_true( ecl_file_get_size( ecl_file) < num_kw );
    ecl_file_close( ecl_file );
  }
  test_work_area_free( work_area );
}


int main( int argc , char ** argv) {
  test_writable(10);
  test_writable(1337);
  test_truncated();
  exit(0);
}

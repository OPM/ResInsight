/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'ecl_kw_init.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/util.h>
#include <ert/util/test_work_area.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fortio.h>


void test_truncated(const char * filename , offset_type truncate_size) {
  {
    FILE * stream = util_fopen(filename , "r+");
    util_ftruncate( stream , truncate_size);
    fclose( stream );
  }
  {
    fortio_type * fortio = fortio_open_reader( filename , false , true );
    ecl_kw_type * kw2 = ecl_kw_fread_alloc( fortio );
    test_assert_NULL( kw2 );
    fortio_fclose(fortio);
  }
}


void test_fread_alloc() {
  test_work_area_type * work_area = test_work_area_alloc("ecl_kw_fread" );
  {
    ecl_kw_type * kw1 = ecl_kw_alloc( "INT" , 100 , ECL_INT );
    int i;
    for (i=0; i < 100; i++)
      ecl_kw_iset_int( kw1 , i , i );
    {
      fortio_type * fortio = fortio_open_writer("INT" , false , true );
      ecl_kw_fwrite( kw1 , fortio );
      fortio_fclose( fortio );
    }
    {
      fortio_type * fortio = fortio_open_reader("INT" , false , true );
      ecl_kw_type * kw2 = ecl_kw_fread_alloc( fortio );
      test_assert_true( ecl_kw_is_instance( kw2 ));
      test_assert_true( ecl_kw_equal( kw1 , kw2 ));
      ecl_kw_free( kw2 );
      fortio_fclose( fortio );
    }

    {
      offset_type file_size = util_file_size("INT");
      test_truncated("INT" , file_size - 4 );
      test_truncated("INT" , file_size - 25 );
      test_truncated("INT" , 5 );
      test_truncated("INT" , 0 );
    }
    ecl_kw_free( kw1 );
  }
  test_work_area_free( work_area );
}


int main(int argc , char ** argv) {
  test_fread_alloc();
  exit(0);
}



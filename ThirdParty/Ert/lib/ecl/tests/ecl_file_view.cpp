/*
   Copyright (C) 2017  Statoil ASA, Norway.

   The file 'ecl_file_view.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/ecl/ecl_file_kw.hpp>

void test_file_kw_equal() {
  ecl_file_kw_type * kw1 = ecl_file_kw_alloc0( "PRESSURE" , ECL_FLOAT, 1000 , 66);
  ecl_file_kw_type * kw2 = ecl_file_kw_alloc0( "PRESSURE" , ECL_FLOAT, 1000 , 66);
  ecl_file_kw_type * kw3 = ecl_file_kw_alloc0( "SWAT" , ECL_FLOAT, 1000 , 66);
  ecl_file_kw_type * kw4 = ecl_file_kw_alloc0( "PRESSURE" , ECL_DOUBLE, 1000 , 66);
  ecl_file_kw_type * kw5 = ecl_file_kw_alloc0( "PRESSURE" , ECL_FLOAT, 10 , 66);
  ecl_file_kw_type * kw6 = ecl_file_kw_alloc0( "PRESSURE" , ECL_FLOAT, 1000 , 67);

  test_assert_true( ecl_file_kw_equal( kw1 , kw1 ));
  test_assert_true( ecl_file_kw_equal( kw1 , kw2 ));
  test_assert_false( ecl_file_kw_equal( kw1 , kw3 ));
  test_assert_false( ecl_file_kw_equal( kw1 , kw4 ));
  test_assert_false( ecl_file_kw_equal( kw1 , kw5 ));
  test_assert_false( ecl_file_kw_equal( kw1 , kw6 ));

  ecl_file_kw_free( kw6 );
  ecl_file_kw_free( kw5 );
  ecl_file_kw_free( kw4 );
  ecl_file_kw_free( kw3 );
  ecl_file_kw_free( kw2 );
  ecl_file_kw_free( kw1 );
}

void test_create_file_kw() {
  ecl_file_kw_type * file_kw0 = ecl_file_kw_alloc0( "PRESSURE" , ECL_FLOAT, 1000 , 66);
  ecl_file_kw_type * file_kw1 = ecl_file_kw_alloc0( "TEST1_KW" , ECL_FLOAT, 2000 , 1066);
  ecl_file_kw_type * file_kw2 = ecl_file_kw_alloc0( "TEST2_KW" , ECL_FLOAT, 3000 , 2066);
  test_assert_string_equal( ecl_file_kw_get_header( file_kw0 ) , "PRESSURE" );
  test_assert_int_equal( ecl_file_kw_get_size( file_kw0 ) , 1000 );
  test_assert_true( ecl_type_is_equal( ecl_file_kw_get_data_type( file_kw0 ) , ECL_FLOAT ));
  {
    test_work_area_type * work_area = test_work_area_alloc("file_kw");
    {
      FILE * ostream = util_fopen("file_kw" , "w");
      ecl_file_kw_fwrite( file_kw0 , ostream );
      fclose( ostream );
    }
    {
      FILE * istream = util_fopen("file_kw" , "r");
      ecl_file_kw_type * disk_kw = ecl_file_kw_fread_alloc( istream );
      test_assert_true( ecl_file_kw_equal( file_kw0 , disk_kw ));

      /* Beyond the end of stream - return NULL */
      test_assert_NULL( ecl_file_kw_fread_alloc( istream ));
      ecl_file_kw_free( disk_kw );
      fclose( istream );
    }

    {
      FILE * ostream = util_fopen("file_kw" , "w");
      ecl_file_kw_fwrite( file_kw0 , ostream );
      ecl_file_kw_fwrite( file_kw1 , ostream );
      ecl_file_kw_fwrite( file_kw2 , ostream );
      fclose( ostream );
    }

    {
      FILE * istream = util_fopen("file_kw" , "r");
      ecl_file_kw_type ** disk_kw = ecl_file_kw_fread_alloc_multiple( istream , 3);
      test_assert_true( ecl_file_kw_equal( file_kw0 , disk_kw[0] ));
      test_assert_true( ecl_file_kw_equal( file_kw1 , disk_kw[1] ));
      test_assert_true( ecl_file_kw_equal( file_kw2 , disk_kw[2] ));

      for (int i=0; i < 3; i++)
        ecl_file_kw_free( disk_kw[i] );
      free( disk_kw );
      fclose( istream );
    }
    {
      FILE * istream = util_fopen("file_kw" , "r");
      test_assert_NULL( ecl_file_kw_fread_alloc_multiple( istream , 10));
      fclose( istream );
    }
    test_work_area_free( work_area );
  }
  ecl_file_kw_free( file_kw0 );
  ecl_file_kw_free( file_kw1 );
  ecl_file_kw_free( file_kw2 );

}


int main( int argc , char ** argv) {
  util_install_signals();
  test_file_kw_equal( );
  test_create_file_kw( );
}

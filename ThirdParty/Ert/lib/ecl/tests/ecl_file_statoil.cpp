
/*
   Copyright (C) 2017  Statoil ASA, Norway.

   The file 'ecl_file_statoil.c' is part of ERT - Ensemble based Reservoir Tool.

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


void test_flags( const char * filename) {
  int FLAG1 = 1;
  int FLAG2 = 2;
  int FLAG3 = 4;
  int FLAG4 = 8;

  int FLAGS = FLAG1 + FLAG2 + FLAG3;


  ecl_file_type * ecl_file = ecl_file_open( filename , FLAGS );

  test_assert_int_equal( ecl_file_get_flags( ecl_file ) , FLAGS );
  test_assert_true( ecl_file_flags_set( ecl_file , FLAG1 ));
  test_assert_true( ecl_file_flags_set( ecl_file , FLAG1 | FLAG2));
  test_assert_true( ecl_file_flags_set( ecl_file , FLAG1 | FLAG3 ));
  test_assert_true( ecl_file_flags_set( ecl_file , FLAG1 | FLAG3 | FLAG2 ));
  test_assert_false( ecl_file_flags_set( ecl_file , FLAG1 | FLAG3 | FLAG4 ));
  ecl_file_close( ecl_file );
}


void test_close_stream2(const char * src_file , const char * target_file ) {
  util_copy_file( src_file , target_file );
  ecl_file_type * ecl_file = ecl_file_open( target_file , ECL_FILE_CLOSE_STREAM );

  ecl_file_load_all( ecl_file );
  unlink( target_file );
  ecl_kw_type * kw2 = ecl_file_iget_kw( ecl_file , 2 );
  test_assert_not_NULL( kw2 );
  ecl_file_close( ecl_file );
}


void test_loadall(const char * src_file , const char * target_file ) {
  util_copy_file( src_file , target_file );
  {
    ecl_file_type * ecl_file = ecl_file_open( target_file , ECL_FILE_CLOSE_STREAM );

    test_assert_true( ecl_file_load_all( ecl_file ) );
    ecl_file_close( ecl_file );
  }

  {
    ecl_file_type * ecl_file = ecl_file_open( target_file , ECL_FILE_CLOSE_STREAM );
    unlink( target_file );

    test_assert_false( ecl_file_load_all( ecl_file ) );
    ecl_file_close( ecl_file );
  }
}



void test_close_stream1(const char * src_file , const char * target_file ) {
  util_copy_file( src_file , target_file );

  ecl_file_type * ecl_file = ecl_file_open( target_file , ECL_FILE_CLOSE_STREAM );
  ecl_kw_type * kw0 = ecl_file_iget_kw( ecl_file , 0 );
  ecl_kw_type * kw1 = ecl_file_iget_kw( ecl_file , 1 );
  unlink( target_file );
  ecl_kw_type * kw1b = ecl_file_iget_kw( ecl_file , 1 );

  test_assert_not_NULL( kw0 );
  test_assert_not_NULL( kw1 );
  test_assert_ptr_equal( kw1 , kw1b );

  ecl_kw_type * kw2 = ecl_file_iget_kw( ecl_file , 2 );
  test_assert_NULL( kw2 );

  test_assert_false( ecl_file_writable( ecl_file ));

  ecl_file_close( ecl_file );

}


void test_writable(const char * src_file ) {
  test_work_area_type * work_area = test_work_area_alloc("ecl_file_writable" );
  char * fname = util_split_alloc_filename( src_file );

  test_work_area_copy_file( work_area , src_file );
  {
    test_flags( fname );
    ecl_file_type * ecl_file = ecl_file_open( fname , ECL_FILE_WRITABLE);
    ecl_kw_type * swat = ecl_file_iget_named_kw( ecl_file , "SWAT" , 0 );
    ecl_kw_type * swat0 = ecl_kw_alloc_copy( swat );
    test_assert_true( ecl_kw_equal( swat , swat0 ));
    ecl_kw_iset_float( swat , 0 , 1000.0 );
    ecl_file_save_kw( ecl_file , swat );
    test_assert_true( ecl_file_writable( ecl_file ));
    ecl_file_close( ecl_file );

    ecl_file = ecl_file_open( fname , 0);
    swat = ecl_file_iget_named_kw( ecl_file , "SWAT" , 0 );
    test_assert_true( util_double_approx_equal( ecl_kw_iget_float( swat , 0 ) , 1000 ));
  }
  test_work_area_free( work_area );
}




int main( int argc , char ** argv) {
  const char * src_file = argv[1];
  const char * target_file = argv[2];

  {
    test_work_area_type * work_area = test_work_area_alloc("ecl_file");

    test_work_area_copy_file( work_area , src_file );
    test_loadall(src_file , target_file );

    test_close_stream1( src_file , target_file);
    test_close_stream2( src_file , target_file);
    test_writable( src_file );

    test_work_area_free( work_area );
  }
  exit(0);
}

/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_fortio.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_endian_flip.h>
                  
void test_existing_read(const char * filename) {
  fortio_type * fortio = fortio_open_reader( filename , false , ECL_ENDIAN_FLIP);
  test_assert_not_NULL( fortio );
  fortio_fclose( fortio );
}


void test_not_existing_read() {
  fortio_type * fortio = fortio_open_reader( "/does/not/exist" , false , ECL_ENDIAN_FLIP);
  test_assert_NULL( fortio );
}


void test_write( const char * filename , bool path_exists) {
  fortio_type * fortio = fortio_open_writer( filename , false , ECL_ENDIAN_FLIP);
  if (path_exists) {
    test_assert_not_NULL( fortio );
    fortio_fclose( fortio );
  } else
    test_assert_NULL( fortio );
}

void test_wrapper( const char * filename ) {
  FILE * stream = util_fopen( filename , "r");
  fortio_type * fortio = fortio_alloc_FILE_wrapper( filename , false , false , stream );
  test_assert_not_NULL( fortio );
  test_assert_false( fortio_fclose_stream( fortio ));
  test_assert_false( fortio_fopen_stream( fortio ));
  test_assert_true( fortio_stream_is_open( fortio ));
  fortio_free_FILE_wrapper( fortio );
  fclose( stream );
}


void test_open_close_read( const char * filename ) {
  fortio_type * fortio = fortio_open_reader( filename , false , ECL_ENDIAN_FLIP);
  test_assert_not_NULL( fortio );

  test_assert_true( fortio_stream_is_open( fortio ));
  test_assert_true( fortio_fclose_stream( fortio ));
  test_assert_false( fortio_stream_is_open( fortio ));
  test_assert_false( fortio_fclose_stream( fortio ));
  test_assert_true( fortio_fopen_stream( fortio ));
  test_assert_true( fortio_stream_is_open( fortio ));
  test_assert_false( fortio_fopen_stream( fortio ));
  
  fortio_fclose( fortio );
}


int main( int argc , char ** argv) {
  const char * file = argv[1];
  
  test_existing_read( file );
  test_not_existing_read( );
  test_open_close_read( file );
  test_wrapper( file );
  test_write( "/tmp/path/does/not/exist" , false );  
  {
    test_work_area_type * work_area = test_work_area_alloc("ecl_fortio.write" );
    util_make_path("path");
    test_write( "path/file.x" , true );
    test_work_area_free( work_area );
  }
  
  exit(0);
}

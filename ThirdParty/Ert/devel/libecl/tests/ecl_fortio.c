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
#include <ert/util/vector.h>
#include <ert/util/test_work_area.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_endian_flip.h>

void test_existing_read(const char * filename) {
  fortio_type * fortio = fortio_open_reader( filename , false , ECL_ENDIAN_FLIP);
  test_assert_not_NULL( fortio );
  fortio_fclose( fortio );
}


void test_fortio_is_instance(const char * filename ) {
  {
    fortio_type * fortio = fortio_open_reader( filename , false , ECL_ENDIAN_FLIP);
    test_assert_not_NULL( fortio );
    test_assert_true(fortio_is_instance(fortio));
    fortio_fclose( fortio );
  }
  {
    vector_type * dummy_vector = vector_alloc_new();
    test_assert_false(fortio_is_instance(dummy_vector));
    vector_free(dummy_vector);
  }
}


void test_fortio_safe_cast(const char * filename ) {
  void  * i_am_a_fortio = fortio_open_reader( filename , false , ECL_ENDIAN_FLIP);
  test_assert_not_NULL( i_am_a_fortio );
  fortio_type * fortio = fortio_safe_cast(i_am_a_fortio);
  test_assert_true(fortio_is_instance(fortio));
  fortio_fclose( fortio );
}


void test_fortio_unsafe_cast(void * arg) {
  void * i_am_not_a_fortio = vector_alloc_new();
  test_assert_not_NULL( i_am_not_a_fortio );
  fortio_safe_cast(i_am_not_a_fortio);
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
  fortio_type * fortio = fortio_alloc_FILE_wrapper( filename , false , false , false , stream );

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


void test_fread_truncated_data() {
  test_work_area_type * work_area = test_work_area_alloc("fortio_truncated" );
  {
    const size_t buffer_size = 1000;
    void * buffer = util_malloc( buffer_size );
    {
      fortio_type * fortio = fortio_open_writer( "PRESSURE" , false , true );

      fortio_fwrite_record( fortio , buffer , buffer_size );
      fortio_fwrite_record( fortio , buffer , buffer_size );

      fortio_fseek( fortio , 0 , SEEK_SET);
      util_ftruncate( fortio_get_FILE(fortio) , 2 * buffer_size  - 100 );
      fortio_fclose( fortio );
    }

    test_assert_long_equal( util_file_size( "PRESSURE") , 2*buffer_size - 100);

    {
      fortio_type * fortio = fortio_open_reader( "PRESSURE" , false , true );
      test_assert_true( fortio_fread_buffer( fortio , buffer , buffer_size ));
      test_assert_false( fortio_fread_buffer( fortio , buffer , buffer_size ));
      fortio_fclose( fortio );
    }
    free( buffer );
  }
  test_work_area_free( work_area );
}

void test_fread_truncated_head() {
  test_work_area_type * work_area = test_work_area_alloc("fortio_truncated" );
  {
    {
      FILE * stream = util_fopen("PRESSURE" , "w");
      fclose( stream );
    }

    {
      fortio_type * fortio = fortio_open_reader( "PRESSURE" , false , true );
      void * buffer = NULL;
      int buffer_size = 10;
      test_assert_false( fortio_fread_buffer( fortio , buffer , buffer_size ));
      test_assert_true( fortio_read_at_eof( fortio ));
      fortio_fclose( fortio );
    }
  }
  test_work_area_free( work_area );
}


void test_fread_truncated_tail() {
  test_work_area_type * work_area = test_work_area_alloc("fortio_truncated2" );
  {
    const size_t buffer_size = 1000;
    void * buffer = util_malloc( buffer_size );
    {
      fortio_type * fortio = fortio_open_writer( "PRESSURE" , false , true );

      fortio_fwrite_record( fortio , buffer , buffer_size );
      fortio_fseek( fortio , 0 , SEEK_SET);
      util_ftruncate( fortio_get_FILE(fortio) , buffer_size + 4);
      fortio_fclose( fortio );
    }

    test_assert_long_equal( util_file_size( "PRESSURE") , buffer_size + 4);

    {
      fortio_type * fortio = fortio_open_reader( "PRESSURE" , false , true );
      test_assert_false( fortio_fread_buffer( fortio , buffer , buffer_size ));
      fortio_fclose( fortio );
    }
    free( buffer );
  }
  test_work_area_free( work_area );
}


void test_fread_invalid_tail() {
  test_work_area_type * work_area = test_work_area_alloc("fortio_invalid" );
  int record_size = 10;
  void * buffer = util_malloc( record_size );
  {
    FILE * stream = util_fopen("PRESSURE" , "w");

    util_fwrite(&record_size , sizeof record_size , 1 , stream , __func__);
    util_fwrite(buffer       , 1                  , record_size , stream , __func__);
    util_fwrite(&record_size , sizeof record_size , 1 , stream , __func__);


    util_fwrite(&record_size , sizeof record_size , 1 , stream , __func__);
    util_fwrite(buffer       , 1 , record_size , stream , __func__);
    record_size += 1;
    util_fwrite(&record_size , sizeof record_size , 1 , stream , __func__);

    fclose(stream);
  }
  {
    fortio_type * fortio = fortio_open_reader( "PRESSURE" , false , false );
    record_size -= 1;
    test_assert_true( fortio_fread_buffer( fortio , buffer , record_size ));
    test_assert_false( fortio_fread_buffer( fortio , buffer , record_size ));
    fortio_fclose( fortio );
  }

  free( buffer );
  test_work_area_free( work_area );
}



void test_at_eof() {
  test_work_area_type * work_area = test_work_area_alloc("fortio_truncated2" );
  {
    fortio_type * fortio = fortio_open_writer("PRESSURE" , false , true);
    void * buffer = util_malloc( 100 );

    fortio_fwrite_record( fortio , buffer , 100);
    free( buffer );

    fortio_fclose( fortio );
  }
  {
    fortio_type * fortio = fortio_open_reader("PRESSURE" , false , true);

    test_assert_false( fortio_read_at_eof( fortio ));
    fortio_fseek( fortio , 50 , SEEK_SET );
    test_assert_false( fortio_read_at_eof( fortio ));
    fortio_fseek( fortio , 0 , SEEK_END );
    test_assert_true( fortio_read_at_eof( fortio ));

    fortio_fclose( fortio );
  }

  test_work_area_free( work_area );
}


void test_fseek() {
  test_work_area_type * work_area = test_work_area_alloc("fortio_fseek" );
  {
    fortio_type * fortio = fortio_open_writer("PRESSURE" , false , true);
    void * buffer = util_malloc( 100 );

    fortio_fwrite_record( fortio , buffer , 100);
    free( buffer );

    fortio_fclose( fortio );
  }
  {
    fortio_type * fortio = fortio_open_reader("PRESSURE" , false , true);


    printf("Starting fssek test \n");
    test_assert_true( fortio_fseek( fortio , 0 , SEEK_SET ));
    test_assert_true( fortio_fseek( fortio , 0 , SEEK_END ));
    test_assert_false( fortio_fseek( fortio , 100000 , SEEK_END));
    test_assert_false( fortio_fseek( fortio , 100000 , SEEK_SET));

    fortio_fclose( fortio );
  }

  test_work_area_free( work_area );
}




int main( int argc , char ** argv) {
  util_install_signals();
  {
    const char * file = argv[1];

    test_fortio_is_instance( file );
    test_fortio_safe_cast( file );
    test_assert_util_abort("fortio_safe_cast", test_fortio_unsafe_cast, NULL);
    test_existing_read( file );
    test_not_existing_read( );
    test_open_close_read( file );
    test_wrapper( file );
    test_fread_truncated_head();
    test_fread_truncated_data();
    test_fread_truncated_tail();
    test_fread_invalid_tail();
    test_fseek();
    test_at_eof();

    test_write( "/tmp/path/does/not/exist" , false );
    {
      test_work_area_type * work_area = test_work_area_alloc("ecl_fortio.write" );
      util_make_path("path");
      test_write( "path/file.x" , true );
      test_work_area_free( work_area );
    }

    exit(0);
  }
}

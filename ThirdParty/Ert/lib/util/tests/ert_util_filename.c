/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ert_util_PATH_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>


void test_dirname() {
  const char * src_file1 = "/some/very/long/path/file.txt";
  const char * src_file2 = "relative/path/file.txt";
  const char * src_file3 = "file.txt";

  char * path1 = util_split_alloc_dirname( src_file1 );
  char * path2 = util_split_alloc_dirname( src_file2 );
  char * path3 = util_split_alloc_dirname( src_file3 );

  test_assert_string_equal( "/some/very/long/path" , path1);
  test_assert_string_equal( "relative/path" , path2);
  test_assert_NULL( path3 );

  free( path1 );
  free( path2 );
}



void test_filename() {
  const char * src_file1 = "/some/very/long/path/file1.txt";
  const char * src_file2 = "relative/path/file2";
  const char * src_file3 = "/tmp";

  char * file1 = util_split_alloc_filename( src_file1 );
  char * file2 = util_split_alloc_filename( src_file2 );
  char * file3 = util_split_alloc_filename( src_file3 );

  test_assert_string_equal( "file1.txt" , file1);
  test_assert_string_equal( "file2" , file2);
  test_assert_NULL( file3 );
  free( file1 );
  free( file2 );

}

void test_alloc_filename_empty_strings() {
  const char * path = "";
  const char * filename = "file";
  const char * extension = "";

  char * alloc_filename = util_alloc_filename( path, filename , extension);
  test_assert_string_equal( alloc_filename , filename );
  free( alloc_filename );

}



int main(int argc , char ** argv) {

  test_dirname();
  test_filename();
  test_alloc_filename_empty_strings();
  exit(0);

}

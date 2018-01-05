/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ert_util_addr2line.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <execinfo.h>
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/util/stringlist.h>
#include <ert/util/util.h>



void test_lookup(bool valid_address, bool change_cwd) {
  const char * file = __FILE__;
  const char * func = __func__;
  int    line;
  const int max_bt = 50;
  void *bt_addr[max_bt];
  int size;
  char * func_name , * file_name;
  int line_nr;

  line = __LINE__ + 2;
  size = backtrace(bt_addr , max_bt);
  test_assert_int_equal( size , 4 );

  if (change_cwd) {
    char * cwd = util_alloc_cwd();
    util_chdir("/tmp");
    if (valid_address) {
      test_assert_false( util_addr2line_lookup( bt_addr[0] , &func_name , &file_name , &line_nr));
      test_assert_string_equal( func_name , func );
      test_assert_string_equal( file_name , NULL );
      test_assert_int_equal( 0 , line_nr);
    } else {
      test_assert_false( util_addr2line_lookup( NULL , &func_name , &file_name , &line_nr));
      test_assert_string_equal( func_name , NULL);
      test_assert_string_equal( file_name , NULL);
      test_assert_int_equal( 0 , line_nr );
    }
    util_chdir(cwd);
    free( cwd );
  } else {
    if (valid_address) {
      test_assert_true( util_addr2line_lookup( bt_addr[0] , &func_name , &file_name , &line_nr));
      test_assert_string_equal( func_name , func );
      test_assert_string_equal( file_name , file );
      test_assert_int_equal( line , line_nr );
    } else {
      test_assert_false( util_addr2line_lookup( NULL , &func_name , &file_name , &line_nr));
      test_assert_string_equal( func_name , NULL);
      test_assert_string_equal( file_name , NULL);
      test_assert_int_equal( 0 , line_nr );
    }
  }


}


int main( int argc , char ** argv) {
  if (util_is_abs_path(argv[0])) {
    char * path;
    char * name;
    char * dot_name;
    /*
       This bisaaarre hoopsing is to be able to emulate the situation
       where addr2line can not find the executable; this behaviour is
       invoked when change_cwd is set to true in the test_lookup()
       call.
    */
    util_alloc_file_components( argv[0] , &path , &name, NULL);

    util_chdir(path);
    dot_name = util_alloc_sprintf("./%s" , name);
    util_spawn_blocking(dot_name, 0, NULL, NULL, NULL);
    exit(0);
  } else {
    printf("Testing internal lookup ....\n");
    test_lookup(true, false);
    test_lookup(false , false);

    test_lookup(true, true);
    test_lookup(false , true);

    printf("Testing external lookup ....\n");
    test_util_addr2line();

    exit(0);
  }
}

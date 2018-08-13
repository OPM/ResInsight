/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ert_util_realpath.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/test_util.hpp>
#include <ert/util/stringlist.hpp>

void test_path(const char * input , const char * expected) {
  char * rpath = util_alloc_realpath__( input );
  if (!test_check_string_equal( rpath , expected ))
    test_error_exit("util_alloc_realpath__(%s) => %s  expected:%s \n",input , rpath , expected);
  else
    printf("OK: %s -> %s \n",input , rpath);

  free( rpath );
}



int main( int argc , char ** argv) {
#ifdef ERT_LINUX

  test_path("/tmp/" , "/tmp" );
  test_path("/tmp/test/normal" , "/tmp/test/normal" );
  test_path("/tmp/test/../test/normal" , "/tmp/test/normal");
  test_path("/tmp/test/../../tmp/test/normal" , "/tmp/test/normal");
  test_path("/tmp/test/../../tmp//test/normal" , "/tmp/test/normal");
  test_path("/tmp/test/../../tmp/./test/normal" , "/tmp/test/normal");
  test_path("/tmp/test/../../tmp/./test/normal/" , "/tmp/test/normal");
  test_path("/tmp/test/../../tmp/other/XX/" , "/tmp/other/XX");

#endif

  exit(0);
}

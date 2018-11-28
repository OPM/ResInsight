/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ert_util_strcat_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>

#include <ert/util/vector.hpp>
#include <ert/util/util.h>
#include <ert/util/test_util.hpp>


void test_strcat(char * s1 , const char *s2 , const char * expected) {
  char * cat = util_strcat_realloc(s1 , s2 );
  if (test_check_string_equal( cat , expected ))
    free( cat );
  else
    test_error_exit("util_strcat_realloc(%s,%s) Got:%s  expected:%s \n",s1,s2,cat , expected);
}


int main(int argc , char ** argv) {
  test_strcat(NULL , NULL , NULL);

  {
    const char * s = "Hei";
    test_strcat(NULL, util_alloc_string_copy(s) , s);
  }
  {
    const char * s = "Hei";
    test_strcat(util_alloc_string_copy(s) , NULL , s);
  }
  {
    char * s1 = util_alloc_string_copy("hei");
    char * s2 = util_alloc_string_copy("-Hei");
    test_strcat(s1,s2 , "hei-Hei");
  }

  printf("Test OK\n");
  exit(0);
}

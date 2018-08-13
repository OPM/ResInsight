/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ert_util_relpath_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <ert/util/vector.hpp>
#include <ert/util/util.h>
#include <ert/util/test_util.hpp>


void test_path(int nr , const char * root , const char * path , const char * true_path) {
  char * rel_path = util_alloc_rel_path( root  , path);

  if (!test_check_string_equal( rel_path , true_path))
    test_error_exit("Case:%d  rel_path(%s,%s) -> %s failed - expected: %s\n" , nr , root , path , rel_path , true_path);
  else
    printf("Case:%d OK \n",nr);

  free( rel_path );
}

int main(int argc , char ** argv) {
#ifdef ERT_LINUX
  const char * root1 = "/tmp/root/path";
  const char * path1 = "/tmp/root/path/relative";
  const char * true1 = "relative";

  const char * root2 = "/tmp/root/path/";
  const char * path2 = "/tmp/root/path/relative";
  const char * true2 = "relative";

  const char * root3 = "/tmp/root/path";
  const char * path3 = "/tmp/root/";
  const char * true3 = "../";

  const char * root4 = "/tmp/root/path";
  const char * path4 = "relative";
  const char * true4 = "relative";

  const char * root5 = "/tmp/root/path";
  const char * path5 = "/tmp/root/pathX/relative";
  const char * true5 = "../pathX/relative";

  const char * root6 = "/tmp/root/path";
  const char * path6 = "/tmpX/root/pathX/relative";
  const char * true6 = "../../../tmpX/root/pathX/relative";

  const char * root7 = "/tmp/root/path";
  const char * path7 = "/tmp/root/path";
  const char * true7 = "";

  const char * root8 = "/tmp";
  const char * path8 = "root/path";
  const char * true8 = "root/path";

#endif

  test_path( 1 , root1 , path1 , true1 );
  test_path( 2 , root2 , path2 , true2 );
  test_path( 3 , root3 , path3 , true3 );
  test_path( 4 , root4 , path4 , true4 );
  test_path( 5 , root5 , path5 , true5 );
  test_path( 6 , root6 , path6 , true6 );
  test_path( 7 , root7 , path7 , true7 );
  {
    util_chdir(root8);
    test_path( 8 , NULL , path8 , true8 );
  }

  exit(0);
}

/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ert_util_path_stack_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>


#include <ert/util/util.h>
#include <ert/util/vector.hpp>
#include <ert/util/path_stack.hpp>
#include <ert/util/test_util.hpp>




int main(int argc , char ** argv) {
  char * cwd     = util_alloc_cwd( );
  char * path1   = argv[1];
  char * path2   = argv[2];
  {
    path_stack_type * path_stack = path_stack_alloc();

    path_stack_push_cwd( path_stack );

    if (path_stack_push( path_stack , "NotExist-1111"))
      test_error_exit("Pushed unexisting path\n");

    if (!path_stack_push( path_stack, path1 ))
      test_error_exit("Failed to push:%s \n",path1 );

    util_chdir( path2 );
    if (util_is_cwd( path1 ))
      test_error_exit("Failed to chdir(%s) \n",path2 );

    if (path_stack_size( path_stack ) != 2)
      test_error_exit("Wrong stack size");

    path_stack_pop( path_stack );
    if (!util_is_cwd( path1 ))
      test_error_exit("path_stack_pop failed \n");

    path_stack_pop( path_stack );
    if (!util_is_cwd( cwd ))
      test_error_exit("path_stack_pop failed \n");

    if (path_stack_size( path_stack ) != 0)
      test_error_exit("Wrong stack size");

    if (!path_stack_push(path_stack , NULL))
      test_error_exit("Hmmm - push(NULL) failed \n");

    if (path_stack_size( path_stack ) != 1)
      test_error_exit("Wrong stack size");

    path_stack_pop( path_stack );
    path_stack_free( path_stack);
  }
  exit(0);
}

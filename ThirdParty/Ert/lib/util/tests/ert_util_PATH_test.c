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
#include <stdbool.h>
#include <string.h>

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/test_util.h>



int main(int argc , char ** argv) {
  unsetenv("PATH");
  {
    char ** path_list = util_alloc_PATH_list();
    if (path_list[0] != NULL)
      test_error_exit("Failed on empty PATH\n");

    util_free_NULL_terminated_stringlist( path_list );
  }


  setenv("PATH" , "/usr/bin:/bin:/usr/local/bin" , 1);
  {
    char ** path_list = util_alloc_PATH_list();
    if (strcmp(path_list[0] , "/usr/bin") != 0)
      test_error_exit("Failed on first path element\n");

    if (strcmp(path_list[1] , "/bin") != 0)
      test_error_exit("Failed on second path element\n");

    if (strcmp(path_list[2] , "/usr/local/bin") != 0)
      test_error_exit("Failed on third  path element\n");

    if (path_list[3] != NULL)
      test_error_exit("Failed termination \n");
    
    util_free_NULL_terminated_stringlist( path_list );
  }
  

  exit(0);
}

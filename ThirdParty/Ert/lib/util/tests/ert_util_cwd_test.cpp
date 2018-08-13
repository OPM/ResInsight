/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ert_util_cwd_test.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/vector.hpp>
#include <ert/util/util.h>
#include <ert/util/test_util.hpp>

int main(int argc , char ** argv) {
  char * cwd = argv[1];
  printf("cwd    :%s\n",util_alloc_cwd());
  printf("argv[1]:%s\n",argv[1]);

  if (!util_is_cwd(cwd))
    test_error_exit("Hmmm did not recognize:%s as cwd\n",cwd);

  if (util_is_cwd("/some/path"))
    test_error_exit("Took /whatver/ as CWD\n");

  exit(0);
}

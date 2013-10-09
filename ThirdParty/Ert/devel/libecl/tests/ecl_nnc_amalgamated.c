/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_nnc_index_list_grid.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/int_vector.h>

#include <ert/ecl/nnc_index_list.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_file.h>



int main( int argc , char ** argv) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( argv[1] );
  ecl_grid_free( ecl_grid );
  exit(0);
}

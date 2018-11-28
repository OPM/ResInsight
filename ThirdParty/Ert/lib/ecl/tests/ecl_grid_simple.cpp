/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_grid_simple.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_grid.hpp>



int main(int argc , char ** argv) {
  const char * grid_file = argv[1];
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_file );

  test_assert_int_equal( ecl_grid_get_nactive_fracture( ecl_grid ) , 0 );
  test_assert_int_equal( ecl_grid_get_active_fracture_index1( ecl_grid , 10 ) , -1 );

  ecl_grid_free( ecl_grid );
  exit(0);
}

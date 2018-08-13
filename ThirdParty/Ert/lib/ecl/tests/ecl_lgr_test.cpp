/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_lgr_test.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>


int main(int argc , char ** argv) {
  const char * grid_file = argv[1];
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_file );
  ecl_file_type * ecl_file = ecl_file_open( grid_file , 0);

  ecl_grid_test_lgr_consistency( ecl_grid );

  if (ecl_file_get_num_named_kw( ecl_file , COORD_KW ))
    test_assert_int_equal( ecl_file_get_num_named_kw( ecl_file , COORD_KW ) - 1, ecl_grid_get_num_lgr( ecl_grid ));

  ecl_grid_free( ecl_grid );
  ecl_file_close( ecl_file);
  exit(0);
}

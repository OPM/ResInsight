/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ecl_grid_copy.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/time_t_vector.hpp>
#include <ert/util/util.h>

#include <ert/ecl/ecl_grid.hpp>


void test_copy_grid( const ecl_grid_type * grid ) {
  ecl_grid_type * grid_copy = ecl_grid_alloc_copy( grid );
  test_assert_true( ecl_grid_compare( grid , grid_copy , true , true ,  true ));
  ecl_grid_free( grid_copy );
}



int main( int argc , char ** argv) {
  ecl_grid_type * grid = ecl_grid_alloc_rectangular( 10,11,12,1,2,3 , NULL);
  test_copy_grid( grid );
  ecl_grid_free( grid );

  exit(0);
}

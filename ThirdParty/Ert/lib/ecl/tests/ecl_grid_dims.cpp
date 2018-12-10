/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_grid_dims.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <signal.h>

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_grid_dims.hpp>



void test_grid( const char * grid_filename , const char * data_filename) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_filename );
  ecl_grid_dims_type * grid_dims = ecl_grid_dims_alloc( grid_filename, data_filename);

  test_assert_not_NULL( grid_dims );
  test_assert_int_equal( ecl_grid_get_num_lgr( ecl_grid ) + 1 , ecl_grid_dims_get_num_grids( grid_dims ));
  for (int i=0; i < ecl_grid_dims_get_num_grids( grid_dims ); i++) {

    grid_dims_type   d1 = ecl_grid_iget_dims( ecl_grid , i);
    const grid_dims_type * d2 = ecl_grid_dims_iget_dims( grid_dims , i );

    test_assert_int_equal( d1.nx , d2->nx );
    test_assert_int_equal( d1.ny , d2->ny );
    test_assert_int_equal( d1.nz , d2->nz );

    if (data_filename)
      test_assert_int_equal( d1.nactive , d2->nactive );
  }
}


void test_dims() {
  grid_dims_type d1;
  grid_dims_type * d2 = grid_dims_alloc( 100 , 100 , 100 , 0);

  grid_dims_init(&d1 , 100 , 100 , 100 , 0 );

  test_assert_int_equal( d1.nx , d2->nx );
  test_assert_int_equal( d1.ny , d2->ny );
  test_assert_int_equal( d1.nz , d2->nz );
  test_assert_int_equal( d1.nactive , d2->nactive );

  grid_dims_free( d2 );
}



int main(int argc , char ** argv) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */

  if (argc == 1) {
    ecl_grid_dims_type * grid_dims = ecl_grid_dims_alloc( argv[0] , NULL );
    test_assert_NULL( grid_dims );
    test_dims();
  } else {
    const char * GRID_file = argv[1];
    char * data_file;

    if (argc == 3)
      data_file = argv[2];
    else
      data_file = NULL;

    test_grid( GRID_file , data_file );
  }

  exit(0);
}

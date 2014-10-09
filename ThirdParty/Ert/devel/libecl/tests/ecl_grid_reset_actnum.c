/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_grid_reset_actnum.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/ecl/ecl_grid.h>





int main(int argc , char ** argv) {
  const int nx = 5;
  const int ny = 4;
  const int nz = 2;
  const int g  = nx*ny*nz;
  const int nactive = g - 9;
  const int actnum1[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const int actnum2[] = {0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0};
  ecl_grid_type * grid = ecl_grid_alloc_rectangular(nx , ny , nz , 1 , 1 , 1 , actnum1 );

  
  test_assert_int_equal( g , ecl_grid_get_nactive( grid ));
  ecl_grid_reset_actnum(grid , actnum2 );
  test_assert_int_equal( nactive , ecl_grid_get_nactive( grid ));

  test_assert_int_equal( -1 , ecl_grid_get_active_index1( grid , 0 ));
  test_assert_int_equal(  0 , ecl_grid_get_active_index1( grid , 1 ));
  test_assert_int_equal( -1 , ecl_grid_get_active_index1( grid , 2 ));
  test_assert_int_equal(  1 , ecl_grid_get_global_index1A( grid , 0 ));
  test_assert_int_equal(  3 , ecl_grid_get_global_index1A( grid , 1 ));
  test_assert_int_equal(  5 , ecl_grid_get_global_index1A( grid , 2 ));


  ecl_grid_reset_actnum(grid , NULL );
  test_assert_int_equal( g , ecl_grid_get_nactive( grid ));
  test_assert_int_equal(  0 , ecl_grid_get_active_index1( grid , 0 ));
  test_assert_int_equal(  1 , ecl_grid_get_active_index1( grid , 1 ));
  test_assert_int_equal(  2 , ecl_grid_get_active_index1( grid , 2 ));
  test_assert_int_equal(  0 , ecl_grid_get_global_index1A( grid , 0 ));
  test_assert_int_equal(  1 , ecl_grid_get_global_index1A( grid , 1 ));
  test_assert_int_equal(  2 , ecl_grid_get_global_index1A( grid , 2 ));

  
  
  ecl_grid_free( grid );
  exit(0);
}

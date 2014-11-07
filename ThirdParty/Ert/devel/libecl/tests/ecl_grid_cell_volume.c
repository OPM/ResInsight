/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_grid_cell_contains.c' is part of ERT - Ensemble based
   Reservoir Tool.
    
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


void test_cells( const ecl_grid_type * grid ) {
  int error_count = 0;
  int global_index;
  for (global_index = 0; global_index < ecl_grid_get_global_size( grid ); global_index++) {
    if ((abs(ecl_grid_get_cell_volume1( grid , global_index)) + abs(ecl_grid_get_cell_volume1_tskille( grid , global_index))) > 1e-9) {
      if (!test_check_double_equal( ecl_grid_get_cell_volume1( grid , global_index) , ecl_grid_get_cell_volume1_tskille( grid , global_index))) {
        fprintf(stderr," Global index:%d \n",global_index);
        error_count += 1;
      }
    }
  }
  test_assert_int_equal(0 , error_count);
}





int main(int argc , char ** argv) {
  ecl_grid_type * grid;

  if (argc == 1) 
    grid = ecl_grid_alloc_rectangular(6,6,6,1,2,3,NULL);
  else
    grid = ecl_grid_alloc( argv[1] );

  test_cells( grid );
  ecl_grid_free( grid );
  exit(0);
}

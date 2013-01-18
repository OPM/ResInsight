/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ecl_coarse_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ecl_grid.h>
#include <ecl_coarse_cell.h>

void assert_equal( bool equal ) {
  if (!equal)
    exit(1);
}


int test_grid( const char * filename) {
  ecl_grid_type * GRID = ecl_grid_alloc( filename );

  assert_equal( ecl_grid_have_coarse_cells( GRID ));
  assert_equal( ecl_grid_get_num_coarse_groups( GRID ) == 3384 );
  return 0;
}




int main(int argc , char ** argv) {
  test_grid( argv[1] );
}

/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_lgr_name.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


int main( int argc , char ** argv) {  
  ecl_grid_type * grid = ecl_grid_alloc( argv[1] );

  int num_lgr = ecl_grid_get_num_lgr( grid );
  int lgr_index;
  for (lgr_index = 0; lgr_index < num_lgr; lgr_index++) {
    ecl_grid_type * lgr_from_index = ecl_grid_iget_lgr( grid, lgr_index );
    int lgr_nr = ecl_grid_get_lgr_nr( lgr_from_index);
    ecl_grid_type * lgr_from_nr = ecl_grid_get_lgr_from_lgr_nr( grid , lgr_nr);
    
    test_assert_ptr_equal( lgr_from_index , lgr_from_nr );
    
    test_assert_string_equal( ecl_grid_get_lgr_name( grid , lgr_nr) ,  ecl_grid_iget_lgr_name( grid , lgr_index));
    printf("Grid[%d:%d] : %s \n",lgr_index , lgr_nr , ecl_grid_iget_lgr_name( grid , lgr_index ));
  }
  ecl_grid_free( grid );

  exit(0);
}

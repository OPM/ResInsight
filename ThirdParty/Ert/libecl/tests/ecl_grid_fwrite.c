/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_kw_fwrite.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/test_work_area.h>

#include <ert/ecl/ecl_grid.h>


void test_fwrite_EGRID(ecl_grid_type * grid ) {
  test_work_area_type * work_area = test_work_area_alloc("grid-has-mapaxes");
  
  ecl_grid_fwrite_EGRID2( grid , "TEST.EGRID", ECL_METRIC_UNITS);
  {
    ecl_grid_type * copy = ecl_grid_alloc( "TEST.EGRID" );
    test_assert_true( ecl_grid_compare( grid , copy , false , false , true ));
    ecl_grid_free( copy );
  }
  test_work_area_free( work_area );
}


int main( int argc , char **argv) {
  const char * src_file = argv[1];
  ecl_grid_type * grid = ecl_grid_alloc( src_file );
  
  test_fwrite_EGRID( grid );

  ecl_grid_free( grid );
}

/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_grid_case.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


void test_grid( const char * input , bool expected) {
  ecl_grid_type * grid = ecl_grid_load_case( input );
  if (expected) {
    test_assert_true( ecl_grid_is_instance( grid ));
    ecl_grid_free( grid );
  } else
    test_assert_NULL( grid );
}


int main(int argc , char ** argv) {
  const char * grid_file = argv[1];
  const char * case_path = argv[2];
  
  test_grid( grid_file , true );
  test_grid( case_path , true );
  test_grid( "/tmp/does/not/exists/file.EGRID" , false );
  test_grid( "/tmp/does/not/exists/CASE" , false );
  
  exit(0);
}

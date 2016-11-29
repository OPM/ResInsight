/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_grid_copy_statoil.c' is part of ERT - Ensemble based Reservoir Tool. 
   
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
#include <ert/util/time_t_vector.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_grid.h>


void test_copy_grid( const char * filename ) {
  {
    ecl_grid_type * src_grid = ecl_grid_alloc( filename );
    ecl_grid_type * copy_grid = ecl_grid_alloc_copy( src_grid );
    test_assert_true( ecl_grid_compare( src_grid , copy_grid , true , true , true ));
    ecl_grid_free( copy_grid );
    ecl_grid_free( src_grid );
  }
}



int main( int argc , char ** argv) {
  int iarg;
  for (iarg = 1; iarg < argc; iarg++)
    test_copy_grid( argv[iarg] );

  exit(0);
}

/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ecl_region.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/ecl/ecl_region.h>

void test_list( int volume , int nactive , ecl_region_type * region ) {
  const int_vector_type * active_list;
  const int_vector_type * global_list;
  active_list = ecl_region_get_active_list( region );
  global_list = ecl_region_get_global_list( region );
  test_assert_int_equal( nactive , int_vector_size( active_list ));
  test_assert_int_equal( volume  , int_vector_size( global_list ));


  ecl_region_deselect_all( region );
  active_list = ecl_region_get_active_list( region );
  global_list = ecl_region_get_global_list( region );
  test_assert_int_equal( 0 , int_vector_size( active_list ));
  test_assert_int_equal( 0 , int_vector_size( global_list ));
}



void test_slice( const ecl_grid_type * grid ) {
  int nx = ecl_grid_get_nx( grid );
  int ny = ecl_grid_get_ny( grid );
  int nz = ecl_grid_get_nz( grid );
  int nactive = ecl_grid_get_nactive( grid );
  ecl_region_type * region = ecl_region_alloc( grid , false );
  
  ecl_region_select_i1i2( region , 0 , nx - 1);
  test_list( nx*ny*nz , nactive , region );
  ecl_region_select_j1j2( region , 0 , ny - 1);
  test_list( nx*ny*nz , nactive , region );
  ecl_region_select_k1k2( region , 0 , nz - 1);
  test_list( nx*ny*nz , nactive , region );
  
  ecl_region_free( region );
}


int main(int argc , char ** argv) {
  const char * grid_file = argv[1];
  ecl_grid_type * grid = ecl_grid_alloc( grid_file );

  test_slice( grid );
  
  ecl_grid_free( grid );
  exit(0);
}

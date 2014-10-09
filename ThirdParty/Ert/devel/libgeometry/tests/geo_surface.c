/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'geo_surface.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/test_work_area.h>

#include <ert/geometry/geo_surface.h>


void test_load(const char * input_file , const char * broken_file) {
  geo_surface_type * surface = geo_surface_fload_alloc_irap( input_file , false );
  double * data = util_calloc( geo_surface_get_size( surface ) , sizeof * data );
  
  test_assert_true( geo_surface_fload_irap_zcoord( surface , input_file , data ));
  test_assert_false( geo_surface_fload_irap_zcoord( surface , "/does/not/exist" , data ));
  test_assert_false( geo_surface_fload_irap_zcoord( surface , broken_file , data ));
  

  free( data );
  geo_surface_free( surface );
}


void test_fprintf(const char * input_file ) {
  geo_surface_type * surface1 = geo_surface_fload_alloc_irap( input_file , true );
  test_work_area_type * work_area = test_work_area_alloc( "SURFACE-FPRINTF" );
  
  geo_surface_fprintf_irap( surface1 , "surface/test/surface.irap");
  {
    geo_surface_type * surface2 = geo_surface_fload_alloc_irap( "surface/test/surface.irap" , true );
    
    test_assert_true( geo_surface_equal( surface1 , surface2 ));
    
    geo_surface_free( surface2 );
  }
  test_work_area_free( work_area );
  geo_surface_free( surface1 );
}



int main( int argc , char ** argv) {
  char * input_file = argv[1];
  char * broken_file1 = argv[2];

  test_load( input_file , broken_file1 );
  test_fprintf( input_file );
  
  exit( 0 );
}

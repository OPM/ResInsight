/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'geo_polygon_collection.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/geometry/geo_polygon.h>
#include <ert/geometry/geo_polygon_collection.h>




void test_create() {
  geo_polygon_collection_type * pc = geo_polygon_collection_alloc();
  test_assert_true( geo_polygon_collection_is_instance( pc ));
  test_assert_int_equal( 0 , geo_polygon_collection_size( pc ));
  geo_polygon_collection_free( pc );
}



int main(int argc , char ** argv) {
  test_create();
  exit(0);
}

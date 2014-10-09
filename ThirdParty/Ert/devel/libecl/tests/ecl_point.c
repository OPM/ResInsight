/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_sum_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/ecl/point.h>


void test_equal() {
  point_type * p1 = point_alloc(1,1,1);
  point_type * p2 = point_alloc(1,1,1);
  point_type * p3 = point_alloc(1,1,1);
  

  test_assert_true( point_equal( p1 , p2 ));
  test_assert_true( point_equal( p1 , p3 ));

  point_shift(p2 , 1e-12 , 0,0);
  test_assert_false( point_equal( p1 , p2 ));
  point_copy_values(p2 , p3);
  test_assert_true( point_equal( p1 , p2 ));

  point_shift(p2 , 0, 1e-12 ,0);
  test_assert_false( point_equal( p1 , p2 ));
  point_copy_values(p2 , p3);
  test_assert_true( point_equal( p1 , p2 ));

  point_shift(p2 , 0, 0,-1e-12 );
  test_assert_false( point_equal( p1 , p2 ));
  point_copy_values(p2 , p3);
  test_assert_true( point_equal( p1 , p2 ));

  point_free( p1 );
  point_free( p2 );
  point_free( p3 );
}



void test_distancez() {
  point_type * p0 = point_alloc(0,0,0);
  point_type * p1 = point_alloc(1,0,0);
  point_type * p2 = point_alloc(0,1,0);
  
  point_type * px = point_alloc(1,1,10);

  test_assert_double_equal( 10 , point3_plane_distance( p0, p1 , p2 , px ));
  px->z = -1;
  test_assert_double_equal( -1 , point3_plane_distance( p0, p1 , p2 , px ));
  
  point_free( p1 );
  point_free( p2 );
  point_free( p0 );
  point_free( px );
}


void test_distancex() {
  point_type * p0 = point_alloc(0,0,0);
  point_type * p1 = point_alloc(0,1,0);
  point_type * p2 = point_alloc(0,0,1);
  
  point_type * px = point_alloc(1,1,10);

  test_assert_double_equal(  1 , point3_plane_distance( p0, p1 , p2 , px ));
  px->x = -7;
  test_assert_double_equal( -7 , point3_plane_distance( p0, p1 , p2 , px ));
  
  point_free( p1 );
  point_free( p2 );
  point_free( p0 );
  point_free( px );
}


void test_distancey() {
  point_type * p0 = point_alloc(0,0,0);
  point_type * p1 = point_alloc(0,0,2);
  point_type * p2 = point_alloc(2,0,0);
  
  point_type * px = point_alloc(1,1,10);

  test_assert_double_equal(  1 , point3_plane_distance( p0, p1 , p2 , px ));
  px->y = -7;
  test_assert_double_equal( -7 , point3_plane_distance( p0, p1 , p2 , px ));
  
  point_free( p1 );
  point_free( p2 );
  point_free( p0 );
  point_free( px );
}


int main(int argc , char ** argv) {
  test_equal();
  test_distancex();
  test_distancey();
  test_distancez();
}

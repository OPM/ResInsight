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

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>

#include <ert/geometry/geo_polygon.hpp>




void test_create() {
  geo_polygon_type * polygon = geo_polygon_alloc( "Name" );
  test_assert_true( geo_polygon_is_instance( polygon ));
  test_assert_string_equal( "Name" , geo_polygon_get_name( polygon ));
  geo_polygon_free( polygon );
}


void test_contains__( double x , double y , int length , const double * data, bool expected) {
  geo_polygon_type * polygon = (geo_polygon_type *) geo_polygon_alloc( NULL );
  int i;
  for (i=0; i < length; i++)
    geo_polygon_add_point( polygon , data[2*i] , data[2*i + 1]);

  test_assert_bool_equal( expected , geo_polygon_contains_point( polygon ,x , y));
  geo_polygon_free( polygon );
}



void test_contains_polygon1( int length , const double * data) {
  test_contains__(0.50 , 0.50 , length , data , true);
  test_contains__(2.00 , 2.00 , length , data , false);
  test_contains__(1.00 , 0.50 , length , data , true);
}


void test_contains_polygon2( int length , const double * data) {
  test_contains__(0.50 , 0.50 , length , data , true);
  test_contains__(2.00 , 2.00 , length , data , false);
  test_contains__(1.00 , 0.50 , length , data , true);
}


void test_contains_polygon3( int length , const double * data) {
  test_contains__(0.50 , 0.51 , length , data , false);
  test_contains__(0.50 , 0.49 , length , data , true);
}



void test_contains() {
  test_contains_polygon1( 4 , (const double[8])  {0,0,1,0,1,1,0,1});
  test_contains_polygon2( 5 , (const double[10]) {0,0,1,0,1,1,0,1,0,0});
  test_contains_polygon3( 6 , (const double[12]) {0,0 , 0,1 , 0.6,0.5 , 0.4,0.5 , 1,1 , 1,0});
}


void test_prepend() {
  geo_polygon_type * polygon = (geo_polygon_type *) geo_polygon_alloc( NULL );
  geo_polygon_add_point(polygon , 1 , 1);
  geo_polygon_add_point_front( polygon , 0,0);
  test_assert_int_equal( 2 , geo_polygon_get_size( polygon ));
  {
    double x,y;

    geo_polygon_iget_xy(polygon , 0 , &x , &y);
    test_assert_double_equal( x , 0 );
    test_assert_double_equal( x , 0 );

    geo_polygon_iget_xy(polygon , 1 , &x , &y);
    test_assert_double_equal( x , 1 );
    test_assert_double_equal( x , 1 );
  }
  geo_polygon_free( polygon );
}



int main(int argc , char ** argv) {
  test_create();
  test_contains();
  test_prepend();
  exit(0);
}

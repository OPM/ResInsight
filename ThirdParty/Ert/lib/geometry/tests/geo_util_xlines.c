/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'geo_util_xlines.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/geometry/geo_util.h>



void test_parallell_lines(double ** points) {
  double x0 = -100;
  double y0 = -100;

  points[0][0] = 0;
  points[1][0] = 10;

  points[0][1] = 1;
  points[1][1] = 1;


  points[2][0] = 0;
  points[3][0] = 10;

  points[2][1] = 4;
  points[3][1] = 4;


  test_assert_int_equal(GEO_UTIL_LINES_PARALLELL , geo_util_xlines( (const double **) points , &x0 , &y0 ));
  test_assert_double_equal( x0 , -100 );
  test_assert_double_equal( y0 , -100 );
}


void test_overlapping_lines_in_contact(double ** points) {
  double x0 = -100;
  double y0 = -100;

  points[0][0] = 0;
  points[1][0] = 1;

  points[0][1] = 0;
  points[1][1] = 1;


  points[2][0] = -1;
  points[3][0] =  2;

  points[2][1] = -1;
  points[3][1] =  2;


  test_assert_int_equal(GEO_UTIL_LINES_OVERLAPPING , geo_util_xlines( (const double **) points , &x0 , &y0 ));
  test_assert_double_equal( x0 , -100 );
  test_assert_double_equal( y0 , -100 );
}


void test_overlapping_lines_not_in_contact(double ** points) {
  double x0 = -100;
  double y0 = -100;

  points[0][0] = 0;
  points[1][0] = 1;

  points[0][1] = 0;
  points[1][1] = 1;


  points[2][0] = 2;
  points[3][0] = 3;

  points[2][1] = 2;
  points[3][1] = 3;


  test_assert_int_equal(GEO_UTIL_NOT_CROSSING , geo_util_xsegments( (const double **) points , &x0 , &y0 ));
  test_assert_double_equal( x0 , -100 );
  test_assert_double_equal( y0 , -100 );
}


void test_overlapping_lines_not_in_contact_horizontal(double ** points) {
  double x0 = -100;
  double y0 = -100;

  points[0][0] = 0;
  points[1][0] = 1;

  points[0][1] = 0;
  points[1][1] = 0;


  points[2][0] = 2;
  points[3][0] = 3;

  points[2][1] = 0;
  points[3][1] = 0;


  test_assert_int_equal(GEO_UTIL_NOT_CROSSING , geo_util_xsegments( (const double **) points , &x0 , &y0 ));
  test_assert_double_equal( x0 , -100 );
  test_assert_double_equal( y0 , -100 );
}


void test_overlapping_lines_not_in_contact_vertical(double ** points) {
  double x0 = -100;
  double y0 = -100;

  points[0][0] = 0;
  points[1][0] = 0;

  points[0][1] = 0;
  points[1][1] = 1;


  points[2][0] = 0;
  points[3][0] = 0;

  points[2][1] = 2;
  points[3][1] = 3;

  test_assert_int_equal(GEO_UTIL_NOT_CROSSING , geo_util_xsegments( (const double **) points , &x0 , &y0 ));
  test_assert_double_equal( x0 , -100 );
  test_assert_double_equal( y0 , -100 );
}



void test_overlapping_lines(double ** points) {
  test_overlapping_lines_in_contact( points );
  test_overlapping_lines_not_in_contact( points );
  test_overlapping_lines_not_in_contact_horizontal( points );
  test_overlapping_lines_not_in_contact_vertical( points );
}


void test_crossing_lines(double ** points) {
  double x0 = -100;
  double y0 = -100;

  points[0][0] = 0;
  points[1][0] = 1;

  points[0][1] = 0;
  points[1][1] = 1;


  points[2][0] = 0;
  points[3][0] = 2;

  points[2][1] = 2;
  points[3][1] = 0;


  test_assert_int_equal(GEO_UTIL_LINES_CROSSING , geo_util_xlines( (const double **) points , &x0 , &y0 ));
  test_assert_double_equal( x0 , 1 );
  test_assert_double_equal( y0 , 1 );


}


void test_vertical_line(double ** points) {
  double x0 = -100;
  double y0 = -100;

  points[0][0] = -1;
  points[1][0] =  1;

  points[0][1] = -1;
  points[1][1] =  1;


  points[2][0] =  0;
  points[3][0] =  0;

  points[2][1] =  1;
  points[3][1] = -1;


  test_assert_int_equal(GEO_UTIL_LINES_CROSSING , geo_util_xlines( (const double **) points , &x0 , &y0 ));
  test_assert_double_equal( x0 , 0 );
  test_assert_double_equal( y0 , 0 );
}



void test_degenerate_line(double ** points) {
  double x0 = -100;
  double y0 = -100;

  points[0][0] = -1;
  points[1][0] = -1;

  points[0][1] = -1;
  points[1][1] = -1;


  points[2][0] =  0;
  points[3][0] =  0;

  points[2][1] =  1;
  points[3][1] = -1;


  test_assert_int_equal(GEO_UTIL_LINES_DEGENERATE , geo_util_xlines( (const double **) points , &x0 , &y0 ));
  test_assert_double_equal( x0 , -100 );
  test_assert_double_equal( y0 , -100 );
}




int main( int argc , char ** argv) {
  double ** points = util_malloc( 4 * sizeof * points);
  int i;
  for (i = 0; i < 4; i++)
    points[i] = util_malloc( 2 * sizeof * points[i]);
  {
    test_parallell_lines(points);
    test_crossing_lines(points);
    test_overlapping_lines(points);
    test_vertical_line( points );
    test_degenerate_line( points );
  }
  for (i = 0; i < 4; i++)
    free(points[i]);

  free( points );
  exit(0);
}

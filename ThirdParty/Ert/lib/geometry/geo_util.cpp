/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'geo_util.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <math.h>

#include <ert/util/util.hpp>

#include <ert/geometry/geo_util.hpp>

#define EPSILON  0.000001


static bool between(double v1, double v2 , double v) {
  return ((( v > v1) && (v < v2)) || (( v < v1) && (v > v2)));
}


static bool interval_overlap(double a1,double a2, double b1, double b2) {
  if (between(a1,a2,b1) || between(a1,a2,b2))
    return true;

  if ((a1 == b1) && (a2 == b2))
    return true;

  return false;
}


static bool on_edge(double _x1 , double _y1 , double _x2 , double _y2 , double x0 , double y0) {
  double x1 = util_double_min( _x1 , _x2 );
  double x2 = util_double_max( _x1 , _x2 );
  double y1 = util_double_min( _y1 , _y2 );
  double y2 = util_double_max( _y1 , _y2 );

  {
    /* Vertical line */
    if (((x1 == x2) && (x0 == x1)) && ((y1 <= y0) && (y0 <= y2)))
      return true;

    /* Horizontal line */
    if (((x1 <= x0) && (x0 <= x2)) && ((y1 == y2) && (y0 == y1)))
      return true;

    /* General slope */
    {
      double a = (y2 - y1) / (x2 - x1);
      double yc = a*(x0 - x1) + y1;

      if (yc == y0) {
        if ((x1 <= x0) && (x0 <= x2))
          return true;
      }
    }
    return false;
  }
}



/*
  If the bool force_edge_inside is set to true, points exactly on the
  edge will be identified as inside. If the force_edge_inside variable
  is set to false the behaviour on the edges is undefined.
*/


bool geo_util_inside_polygon__(const double * xlist , const double * ylist , int num_points , double x0 , double y0 , bool force_edge_inside) {
  bool inside = false;
  int point_num;
  double y = y0;
  double xc = 0;

  for (point_num = 0; point_num < num_points; point_num++) {
    int next_point = ((point_num + 1) % num_points);
    double x1 = xlist[point_num];  double y1 = ylist[point_num];
    double x2 = xlist[next_point]; double y2 = ylist[next_point];

    double ymin = util_double_min(y1,y2);
    double ymax = util_double_max(y1,y2);
    double xmax = util_double_max(x1,x2);

    if (force_edge_inside) {
      if (on_edge(x1,y1,x2,y2,x0,y0)) {
        inside = true;
        break;
      }
    }

    if ((x1 == x2) && (y1 == y2))
      continue;

    if ((y0 > ymin) && (y <= ymax)) {

      if (x0 <= xmax) {
        if (y1 != y2)
          xc = (y0 - y1) * (x2 - x1) / (y2 - y1) + x1;

        if ((x1 == x2) || (x0 <= xc))
          inside = !inside;

      }
    }
  }

  return inside;
}


bool geo_util_inside_polygon(const double * xlist , const double * ylist , int num_points , double x0 , double y0) {
  return geo_util_inside_polygon__(xlist , ylist , num_points , x0 , y0 , false );
}

/*
  This function will cross to infinitely long lines which go through
  the points given by the points pointer; this function does not
  consider line segments of finite length.
*/

geo_util_xlines_status_enum geo_util_xlines( const double ** points , double * x0, double * y0 ) {
  double x1 = points[0][0];
  double x2 = points[1][0];
  double x3 = points[2][0];
  double x4 = points[3][0];

  double y1 = points[0][1];
  double y2 = points[1][1];
  double y3 = points[2][1];
  double y4 = points[3][1];

  double dx1 = (x2 - x1);
  double dx2 = (x4 - x3);
  double dy1 = (y2 - y1);
  double dy2 = (y4 - y3);

  if ((dx1 == 0 && dy1 == 0) ||
      (dx2 == 0 && dy2 == 0))
    return GEO_UTIL_LINES_DEGENERATE;

  if ((dx1 != 0) && (dx2 != 0)) {
    // Both lines have a finite slope.
    double a1 = dy1 / dx1;
    double a2 = dy2 / dx2;

    if (a1 != a2) {
      // The two lines are not parallell and we will have a normal solution.
      double x = ((y3 - y1) - a2*x3 + a1*x1) / (a1 - a2);
      double y = a1*(x - x1) + y1;

      *x0 = x;
      *y0 = y;

      return GEO_UTIL_LINES_CROSSING;
    } else {
      // The two lines are parallell - potentially overlapping.
      double ytest = a1*(x3 - x1) + y1;
      if (ytest == y3)
        return GEO_UTIL_LINES_OVERLAPPING;
      else
        return GEO_UTIL_LINES_PARALLELL;
    }
  } else {
    double x,y;

    if ((dx1 == dx2) && (dx1 == 0)) {
      if (x1 == x3)
        return GEO_UTIL_LINES_OVERLAPPING;
      else
        return GEO_UTIL_LINES_PARALLELL;
    }

    // Only one of the lines is vertical - this will be a normal crossing.
    if (dx1 == 0) {
      double a2 = dy2 / dx2;
      y = y3 + a2*(x1 - x3);
      x = x1;
    } else {
      double a1 = dy1 / dx1;
      y = y1 + a1*(x3 - x1);
      x = x3;
    }
    *x0 = x;
    *y0 = y;
    return GEO_UTIL_LINES_CROSSING;
  }
}


geo_util_xlines_status_enum geo_util_xsegments( const double ** points , double * x0, double * y0 ) {
  double x1 = points[0][0];
  double x2 = points[1][0];
  double x3 = points[2][0];
  double x4 = points[3][0];

  double y1 = points[0][1];
  double y2 = points[1][1];
  double y3 = points[2][1];
  double y4 = points[3][1];

  double denominator = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
  double numerator_a = (x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3);
  double numerator_b = (x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3);

  // coincident?
  if ((fabs(numerator_a) < EPSILON) &&
      (fabs(numerator_b) < EPSILON) &&
      (fabs(denominator) < EPSILON)) {

    if (interval_overlap( x1,x2,x3,x4) && interval_overlap(y1,y2,y3,y4))
      return GEO_UTIL_LINES_OVERLAPPING;
    else
      return GEO_UTIL_NOT_CROSSING;
  }


  // Parallell
  if (fabs(denominator) < EPSILON)
    return GEO_UTIL_NOT_CROSSING;


  // Normal intersection
  {
    double mua = numerator_a / denominator;
    double mub = numerator_b / denominator;

    if ((mua < 0.0) || (mua > 1.0) || (mub < 0.0) || (mub > 1.0))
      return GEO_UTIL_NOT_CROSSING;

    *x0 = x1 + mua * (x2 - x1);
    *y0 = y1 + mua * (y2 - y1 );

    return GEO_UTIL_LINES_CROSSING;
  }
}

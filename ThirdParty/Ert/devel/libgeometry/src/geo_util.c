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

#include <ert/util/util.h>

#include <ert/geometry/geo_util.h>




static bool between(double v1, double v2 , double v) {
  return ((( v > v1) && (v < v2)) || (( v < v1) && (v > v2)));
}


bool geo_util_inside_polygon(const double * xlist , const double * ylist , int num_points , double x0 , double y0) {
  bool inside = false;
  int point_num;
  double y = y0;
  
  for (point_num = 0; point_num < num_points; point_num++) {
    int next_point = ((point_num + 1) % num_points);
    double x1 = xlist[point_num];  double y1 = ylist[point_num];
    double x2 = xlist[next_point]; double y2 = ylist[next_point];
    
    if (between(x1,x2,x0)) {
      double yc = (y2 - y1)/(x2 - x1) * (x0 - x1) + y1;  
      if (y < yc) {
        y = yc;
        inside = !inside;
      }
    }
  }
  return inside;
}


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


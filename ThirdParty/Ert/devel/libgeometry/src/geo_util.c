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
#include <util.h>
#include <geo_util.h>




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

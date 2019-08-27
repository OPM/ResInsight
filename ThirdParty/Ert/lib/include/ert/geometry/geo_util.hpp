/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'geo_util.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_GEO_UTIL_H
#define ERT_GEO_UTIL_H


#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

  typedef enum {
    GEO_UTIL_LINES_CROSSING = 0,
    GEO_UTIL_LINES_PARALLELL = 1,
    GEO_UTIL_LINES_OVERLAPPING = 2,
    GEO_UTIL_LINES_DEGENERATE = 3,
    GEO_UTIL_NOT_CROSSING = 4
  } geo_util_xlines_status_enum;

  bool geo_util_inside_polygon__(const double * xlist , const double * ylist , int num_points , double x0 , double y0 , bool force_edge_inside);
  bool geo_util_inside_polygon(const double * xlist , const double * ylist , int num_points , double x0 , double y0);
  geo_util_xlines_status_enum  geo_util_xlines( const double ** points , double * x0, double * y0 );
  geo_util_xlines_status_enum geo_util_xsegments( const double ** points , double * x0, double * y0 );


#ifdef __cplusplus
}
#endif

#endif

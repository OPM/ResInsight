/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'geo_polygon.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/double_vector.hpp>
#include <ert/util/type_vector_functions.hpp>

#include <ert/geometry/geo_util.hpp>
#include <ert/geometry/geo_polygon.hpp>



#define GEO_POLYGON_TYPE_ID 9951322

struct geo_polygon_struct {
  UTIL_TYPE_ID_DECLARATION;
  double_vector_type * xcoord;
  double_vector_type * ycoord;
  char * name;
};


static UTIL_SAFE_CAST_FUNCTION( geo_polygon , GEO_POLYGON_TYPE_ID );
UTIL_IS_INSTANCE_FUNCTION( geo_polygon , GEO_POLYGON_TYPE_ID);


geo_polygon_type * geo_polygon_alloc(const char * name) {
  geo_polygon_type * polygon = (geo_polygon_type*)util_malloc( sizeof * polygon );

  UTIL_TYPE_ID_INIT( polygon , GEO_POLYGON_TYPE_ID );
  polygon->xcoord = double_vector_alloc( 0 , 0 );
  polygon->ycoord = double_vector_alloc( 0 , 0 );
  polygon->name   = util_alloc_string_copy( name );
  return polygon;
}

void geo_polygon_free( geo_polygon_type * polygon ) {
  double_vector_free( polygon->xcoord );
  double_vector_free( polygon->ycoord );
  free( polygon->name );
  free( polygon );
}



void geo_polygon_free__( void * arg ) {
  geo_polygon_type * polygon = geo_polygon_safe_cast( arg );
  geo_polygon_free( polygon );
}


void geo_polygon_add_point( geo_polygon_type * polygon , double x , double y) {
  double_vector_append( polygon->xcoord , x );
  double_vector_append( polygon->ycoord , y );
}

void geo_polygon_add_point_front( geo_polygon_type * polygon , double x , double y) {
  double_vector_insert( polygon->xcoord , 0 , x );
  double_vector_insert( polygon->ycoord , 0 , y );
}


void geo_polygon_close( geo_polygon_type * polygon) {
  double x = double_vector_get_first( polygon->xcoord );
  double y = double_vector_get_first( polygon->ycoord );
  geo_polygon_add_point( polygon , x , y );
}




bool geo_polygon_contains_point__( const geo_polygon_type * polygon , double x , double y, bool force_edge_inside) {
  return geo_util_inside_polygon__( double_vector_get_const_ptr( polygon->xcoord ) ,
                                    double_vector_get_const_ptr( polygon->ycoord ) ,
                                    double_vector_size( polygon->xcoord ) ,
                                    x , y , force_edge_inside);
}


bool geo_polygon_contains_point( const geo_polygon_type * polygon , double x , double y) {
  return geo_polygon_contains_point__(polygon , x, y , false );
}



static geo_polygon_type * geo_polygon_fload_alloc_xyz( const char * filename , bool irap_format) {
  bool stop_on_999 = irap_format;
  bool skip_last_point = irap_format;

  geo_polygon_type * polygon = geo_polygon_alloc( filename );
  {
    FILE * stream = util_fopen( filename , "r");
    double x , y , z;
    while (true) {
      if (fscanf(stream , "%lg %lg %lg" , &x, &y , &z) == 3) {
        if (stop_on_999 && (x == 999) && (y == 999) && (z == 999))
          break;

        geo_polygon_add_point( polygon , x , y );
      } else
        break;
    }

    fclose( stream );

    if ((double_vector_size( polygon->xcoord ) > 1) && (skip_last_point)) {
      if ((double_vector_get_last(polygon->xcoord) == double_vector_get_first(polygon->xcoord)) &&
          (double_vector_get_last(polygon->ycoord) == double_vector_get_first(polygon->ycoord))) {

        double_vector_pop( polygon->xcoord );
        double_vector_pop( polygon->ycoord );
      }
    }
  }
  return polygon;
}


/*
  The irap format is a polygon which closes on itself by construction,
  and the list of numbers is terminated with (999,999,999). This is
  supported as follows:

    - Reading will stop at (999,999,999) - all points after this
      triplet will be ignored.

    - The polyline will by construction close on itself, i.e. P0 ==
      PN. Iff the last point is identical to the first it will not be
      included.
*/

geo_polygon_type * geo_polygon_fload_alloc_irap( const char * filename ) {
  geo_polygon_type * polygon = geo_polygon_fload_alloc_xyz( filename , true );
  return polygon;
}



void geo_polygon_reset(geo_polygon_type * polygon ) {
  double_vector_reset( polygon->xcoord );
  double_vector_reset( polygon->ycoord );
}



void geo_polygon_shift(geo_polygon_type * polygon , double x0 , double y0) {
  double_vector_shift( polygon->xcoord , x0 );
  double_vector_shift( polygon->ycoord , y0 );
}


int geo_polygon_get_size(const geo_polygon_type * polygon ) {
  return double_vector_size( polygon->xcoord );
}


void geo_polygon_fprintf(const geo_polygon_type * polygon , FILE * stream) {
  int i;
  for (i=0; i < double_vector_size( polygon->xcoord ); i++)
    fprintf(stream , "%10.3f  %10.3f \n", double_vector_iget( polygon->xcoord , i ) , double_vector_iget( polygon->ycoord , i));
}


void geo_polygon_iget_xy(const geo_polygon_type * polygon , int index , double *x , double *y) {
  *x = double_vector_iget( polygon->xcoord , index );
  *y = double_vector_iget( polygon->ycoord , index );
}


bool geo_polygon_segment_intersects(const geo_polygon_type * polygon , double x1 , double y1 , double x2 , double y2) {
  bool intersects = false;
  double ** points = (double**)util_malloc( 4 * sizeof * points);
  {
    int i;
    for (i = 0; i < 4; i++)
      points[i] = (double*)util_malloc( 2 * sizeof * points[i]);
  }

  points[0][0] = x1;
  points[1][0] = x2;
  points[0][1] = y1;
  points[1][1] = y2;

  {
    int index = 0;
    while (true) {

      if (index >= (geo_polygon_get_size( polygon ) - 1))
        break;

      {
        double xc,yc;

        points[2][0] = double_vector_iget( polygon->xcoord , index );
        points[3][0] = double_vector_iget( polygon->xcoord , index + 1);
        points[2][1] = double_vector_iget( polygon->ycoord , index );
        points[3][1] = double_vector_iget( polygon->ycoord , index + 1);

        {
          geo_util_xlines_status_enum xline_status = geo_util_xsegments(( const double **) points , &xc , &yc);
          if ((xline_status == GEO_UTIL_LINES_CROSSING) ||
              (xline_status == GEO_UTIL_LINES_OVERLAPPING))
            intersects = true;
        }

        if (intersects)
          break;

      }

      index++;
    }
  }

  {
    int i;
    for (i = 0; i < 4; i++)
      free(points[i]);

    free( points );
  }

  return intersects;
}


const char* geo_polygon_get_name( const geo_polygon_type * polygon ) {
  return polygon->name;
}

void geo_polygon_set_name( geo_polygon_type * polygon , const char * name) {
  polygon->name = util_realloc_string_copy( polygon->name , name );
}


double geo_polygon_get_length( geo_polygon_type * polygon ) {
  if (double_vector_size( polygon->xcoord) == 1)
    return 0;
  else {
    double length = 0;
    double x0 = double_vector_iget( polygon->xcoord , 0 );
    double y0 = double_vector_iget( polygon->ycoord , 0 );
    int i;

    for (i = 1; i < double_vector_size( polygon->xcoord ); i++) {
      double x1 = double_vector_iget( polygon->xcoord , i );
      double y1 = double_vector_iget( polygon->ycoord , i );

      length += sqrt( (x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0));
      x0 = x1;
      y0 = y1;
    }
    return length;
  }
}


/*
  Name is ignored in the comparison.
*/
bool geo_polygon_equal( const geo_polygon_type * polygon1 , const geo_polygon_type * polygon2 ) {
  bool equal =
    double_vector_equal( polygon1->xcoord , polygon2->xcoord) &&
    double_vector_equal( polygon1->ycoord , polygon2->ycoord);


  if (!equal) {
    equal =
      double_vector_approx_equal( polygon1->xcoord , polygon2->xcoord , 1e-8) &&
      double_vector_approx_equal( polygon1->ycoord , polygon2->ycoord , 1e-8);
  }

  return equal;
}

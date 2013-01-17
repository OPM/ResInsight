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
#include <util.h>
#include <geo_util.h>
#include <geo_polygon.h>        
#include <double_vector.h>


#define GEO_POLYGON_TYPE_ID 9951322

struct geo_polygon_struct {
  UTIL_TYPE_ID_DECLARATION;
  double_vector_type * xcoord;
  double_vector_type * ycoord;
};


static UTIL_SAFE_CAST_FUNCTION( geo_polygon , GEO_POLYGON_TYPE_ID );

geo_polygon_type * geo_polygon_alloc() {
  geo_polygon_type * polygon = util_malloc( sizeof * polygon );
  
  UTIL_TYPE_ID_INIT( polygon , GEO_POLYGON_TYPE_ID );
  polygon->xcoord = double_vector_alloc( 0 , 0 );
  polygon->ycoord = double_vector_alloc( 0 , 0 );

  return polygon;
}

void geo_polygon_free( geo_polygon_type * polygon ) {
  double_vector_free( polygon->xcoord );
  double_vector_free( polygon->ycoord );
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


bool geo_polygon_contains_point( const geo_polygon_type * polygon , double x , double y) {
  return geo_util_inside_polygon( double_vector_get_const_ptr( polygon->xcoord ) , 
                                  double_vector_get_const_ptr( polygon->ycoord ) ,
                                  double_vector_size( polygon->xcoord ) , 
                                  x , y );
}



geo_polygon_type * geo_polygon_fload_alloc_irap( const char * filename ) {
  geo_polygon_type * polygon = geo_polygon_alloc();
  {
    FILE * stream = util_fopen( filename , "r");
    double x , y , z;
    while (true) {
      if (fscanf(stream , "%lg %lg %lg" , &x, &y , &z) == 3) 
        geo_polygon_add_point( polygon , x , y );
      else
        break;
    } 
    fclose( stream );
    /*
      The irap format is a polygon which closes on itself by
      construction; i.e. the last point from file is not added to the
      polygon data structure. In addition the final '999' termination
      is not included.
    */
    double_vector_pop( polygon->xcoord );
    double_vector_pop( polygon->xcoord );
    double_vector_pop( polygon->ycoord );
    double_vector_pop( polygon->ycoord );
  }
  return polygon;
}

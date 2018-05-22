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

#include <ert/util/util.hpp>
#include <ert/util/type_macros.hpp>
#include <ert/util/vector.hpp>
#include <ert/util/hash.hpp>

#include <ert/geometry/geo_polygon.hpp>        
#include <ert/geometry/geo_polygon_collection.hpp>        



#define GEO_POLYGON_COLLECTION_TYPE_ID 95721327

struct geo_polygon_collection_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type * polygon_list;
  hash_type   * polygon_map;
};


UTIL_IS_INSTANCE_FUNCTION( geo_polygon_collection , GEO_POLYGON_COLLECTION_TYPE_ID)

geo_polygon_collection_type * geo_polygon_collection_alloc( ) {
   geo_polygon_collection_type * polygons = (geo_polygon_collection_type*)util_malloc( sizeof * polygons );
   UTIL_TYPE_ID_INIT( polygons , GEO_POLYGON_COLLECTION_TYPE_ID );
   polygons->polygon_list = vector_alloc_new();
   polygons->polygon_map = hash_alloc();
   return polygons;
}


int geo_polygon_collection_size( const geo_polygon_collection_type * polygons ) {
   return vector_get_size( polygons->polygon_list );
}


geo_polygon_type * geo_polygon_collection_create_polygon( geo_polygon_collection_type * polygons , const char * name ) {
  geo_polygon_type * polygon = NULL;
  bool create_polygon = true;

  if (name && geo_polygon_collection_has_polygon( polygons , name ))
    create_polygon = false;

  if (create_polygon) {
    polygon = geo_polygon_alloc( name );
    geo_polygon_collection_add_polygon( polygons , polygon , true );
  }

  return polygon;
} 


bool geo_polygon_collection_add_polygon( geo_polygon_collection_type * polygons , geo_polygon_type * polygon , bool polygon_owner ) {
  const char * name = geo_polygon_get_name( polygon );
  if (geo_polygon_collection_has_polygon( polygons , name ))
    return false;
  else {
    if (polygon_owner)
      vector_append_owned_ref( polygons->polygon_list , polygon , geo_polygon_free__);
    else
      vector_append_ref( polygons->polygon_list , polygon );
    
    if (name)
      hash_insert_ref( polygons->polygon_map , name , polygon );
    
    return true;
  }
}


bool geo_polygon_collection_has_polygon( const geo_polygon_collection_type * polygons , const char * name) {
  if (name)
    return hash_has_key( polygons->polygon_map , name );
  else
    return false;
}


void geo_polygon_collection_free( geo_polygon_collection_type * polygons ) {
  vector_free( polygons->polygon_list );
  hash_free( polygons->polygon_map );
  free( polygons );
}



geo_polygon_type * geo_polygon_collection_iget_polygon(const geo_polygon_collection_type * polygons , int index) {
  return (geo_polygon_type*)vector_iget( polygons->polygon_list , index );
}


geo_polygon_type * geo_polygon_collection_get_polygon(const geo_polygon_collection_type * polygons , const char * polygon_name) {
  return (geo_polygon_type*)hash_get( polygons->polygon_map , polygon_name );
}

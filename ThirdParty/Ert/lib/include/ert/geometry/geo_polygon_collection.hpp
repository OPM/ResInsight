/*
   Copyright (C) 2014  Equinor ASA, Norway.

   The file 'geo_polygon_collection.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_GEO_POLYGON_COLLECTION_H
#define ERT_GEO_POLYGON_COLLECTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/type_macros.hpp>

#include <ert/geometry/geo_polygon.hpp>



  typedef struct geo_polygon_collection_struct geo_polygon_collection_type;

  geo_polygon_collection_type * geo_polygon_collection_alloc( );
  void                          geo_polygon_collection_free( geo_polygon_collection_type * polygons );
  int                           geo_polygon_collection_size( const geo_polygon_collection_type * polygons );
  geo_polygon_type            * geo_polygon_collection_create_polygon( geo_polygon_collection_type * polygons , const char * name );
  bool                          geo_polygon_collection_has_polygon( const geo_polygon_collection_type * polygons , const char * name);
  bool                          geo_polygon_collection_add_polygon( geo_polygon_collection_type * polygons , geo_polygon_type * polygon , bool polygon_owner );
  geo_polygon_type            * geo_polygon_collection_iget_polygon(const geo_polygon_collection_type * polygons , int index);
  geo_polygon_type            * geo_polygon_collection_get_polygon(const geo_polygon_collection_type * polygons , const char * polygon_name);

  UTIL_IS_INSTANCE_HEADER( geo_polygon_collection );

#ifdef __cplusplus
}
#endif
#endif

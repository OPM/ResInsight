/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'geo_region.h' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/int_vector.hpp>

#include <ert/geometry/geo_pointset.hpp>
#include <ert/geometry/geo_polygon.hpp>

#ifndef ERT_GEO_REGION_H
#define ERT_GEO_REGION_H


#ifdef __cplusplus
extern "C" {
#endif



  typedef struct geo_region_struct geo_region_type;

  geo_region_type *       geo_region_alloc( const geo_pointset_type * pointset , bool preselect);
  void                    geo_region_free( geo_region_type * region );
  void                    geo_region_free__( void * arg );
  void                    geo_region_reset( geo_region_type * region );
  const int_vector_type * geo_region_get_index_list( geo_region_type * region );

  void                    geo_region_select_inside_polygon( geo_region_type * region   , const geo_polygon_type * polygon);
  void                    geo_region_select_outside_polygon( geo_region_type * region  , const geo_polygon_type * polygon);
  void                    geo_region_deselect_inside_polygon( geo_region_type * region  , const geo_polygon_type * polygon);
  void                    geo_region_deselect_outside_polygon( geo_region_type * region , const geo_polygon_type * polygon);

  void                    geo_region_select_above_line( geo_region_type * region, const double xcoords[2] , const double ycoords[2]);
  void                    geo_region_select_below_line( geo_region_type * region, const double xcoords[2] , const double ycoords[2]);
  void                    geo_region_deselect_above_line( geo_region_type * region, const double xcoords[2] , const double ycoords[2]);
  void                    geo_region_deselect_below_line( geo_region_type * region, const double xcoords[2] , const double ycoords[2]);

#ifdef __cplusplus
}
#endif
#endif

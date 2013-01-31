/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'local_context.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef __LOCAL_CONTEXT_H__
#define __LOCAL_CONTEXT_H__

#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdbool.h>

#include <ert/geometry/geo_polygon.h>
#include <ert/geometry/geo_surface.h>
#include <ert/geometry/geo_region.h>

#include <ert/ecl/ecl_region.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_grid.h>

#define  GLOBAL_GRID  "GLOBAL_GRID"
  
  typedef struct local_context_struct local_context_type;


  local_context_type * local_context_alloc( const ecl_grid_type * ecl_grid );
  void                 local_context_free( local_context_type * context );
  
  ecl_region_type    * local_context_get_ecl_region( local_context_type * context , const char * region_name);
  void                 local_context_create_ecl_region( local_context_type * context , const char * grid_name , const char * region_name , bool preselect );
  
  void                 local_context_load_file( local_context_type * context , const char * filename , const char * file_key );
  ecl_file_type      * local_context_get_file( local_context_type * context , const char * file_key );

  void                 local_context_add_polygon( local_context_type * context , const char * polygon_name);
  geo_polygon_type   * local_context_get_polygon( local_context_type * context , const char * polygon_name );
  void                 local_context_load_polygon( local_context_type * context , const char * polygon_name , const char * polygon_file);

  void                 local_context_load_surface( local_context_type * context , const char * surface_name , const char * surface_file);
  geo_surface_type   * local_context_get_surface( local_context_type * context , const char * surface_name );

  void                 local_context_create_surface_region( local_context_type * context , const char * surface_name , const char * region_name , bool preselect );
  geo_region_type    * local_context_get_surface_region( local_context_type * context , const char * region_name);

#ifdef __cplusplus
}
#endif
#endif

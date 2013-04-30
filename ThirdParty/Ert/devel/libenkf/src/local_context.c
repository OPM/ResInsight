/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'local_context.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>

#include <ert/geometry/geo_polygon.h>
#include <ert/geometry/geo_surface.h>
#include <ert/geometry/geo_region.h>

#include <ert/ecl/ecl_grid.h>

#include <ert/enkf/local_context.h>

struct local_context_struct {
  hash_type * ecl_regions;
  hash_type * files;
  hash_type * polygons;
  hash_type * grids;
  hash_type * surfaces;
  hash_type * surface_regions;
};



local_context_type * local_context_alloc( const ecl_grid_type * ecl_grid ) {
  local_context_type * context = util_malloc( sizeof * context );
  context->surface_regions = hash_alloc();
  context->ecl_regions   = hash_alloc();
  context->files     = hash_alloc();
  context->polygons  = hash_alloc();
  context->grids     = hash_alloc();
  context->surfaces  = hash_alloc();
  
  hash_insert_ref( context->grids , GLOBAL_GRID , ecl_grid );
  return context;
}


void local_context_free( local_context_type * context) {
  hash_free( context->ecl_regions );
  hash_free( context->files );
  hash_free( context->polygons );
  hash_free( context->grids );
  hash_free( context->surfaces );
  hash_free( context->surface_regions );
}



ecl_region_type * local_context_get_ecl_region( local_context_type * context , const char * region_name) {
  return hash_get( context->ecl_regions , region_name );
}

void local_context_create_ecl_region( local_context_type * context , const char * grid_name , const char * region_name , bool preselect ) {
  ecl_grid_type * grid         = hash_get( context->grids , grid_name);
  ecl_region_type * new_region = ecl_region_alloc( grid , preselect );
  hash_insert_hash_owned_ref( context->ecl_regions , region_name , new_region , ecl_region_free__ );
}

/*************************/

geo_region_type * local_context_get_surface_region( local_context_type * context , const char * region_name) {
  return hash_get( context->surface_regions , region_name );
}

void local_context_create_surface_region( local_context_type * context , const char * surface_name , const char * region_name , bool preselect ) {
  geo_surface_type * base_surface  = hash_get( context->surfaces , surface_name );
  geo_region_type * new_region = geo_region_alloc( geo_surface_get_pointset( base_surface ) , preselect );
  hash_insert_hash_owned_ref( context->surface_regions , region_name , new_region , geo_region_free__ );
}

/*************************/

void local_context_load_file( local_context_type * context , const char * filename , const char * file_key ) {
  ecl_file_type * ecl_file = ecl_file_open( filename , 0);
  hash_insert_hash_owned_ref( context->files , file_key , ecl_file , ecl_file_free__);
}


ecl_file_type * local_context_get_file( local_context_type * context , const char * file_key ) {
  return hash_get( context->files , file_key );
}

/*************************/

static void local_context_add_polygon__( local_context_type * context , const char * polygon_name , geo_polygon_type * polygon)  {
  hash_insert_hash_owned_ref( context->polygons , polygon_name , polygon , geo_polygon_free__);
}

void local_context_add_polygon( local_context_type * context , const char * polygon_name )  {
  geo_polygon_type * polygon = geo_polygon_alloc( );
  local_context_add_polygon__(context , polygon_name , polygon );
}

void local_context_load_polygon( local_context_type * context , const char * polygon_name , const char * polygon_file)  {
  geo_polygon_type * polygon = geo_polygon_fload_alloc_irap( polygon_file );
  hash_insert_hash_owned_ref( context->polygons , polygon_name , polygon , geo_polygon_free__);
}


geo_polygon_type * local_context_get_polygon( local_context_type * context , const char * polygon_name ) {
  return hash_get( context->polygons , polygon_name );
}

/*************************/

void local_context_load_surface( local_context_type * context , const char * surface_name , const char * surface_file) {
  geo_surface_type * surface = geo_surface_fload_alloc_irap( surface_file  , true );
  hash_insert_hash_owned_ref( context->surfaces , surface_name , surface , geo_surface_free__);
}

geo_surface_type * local_context_get_surface( local_context_type * context , const char * surface_name) {
  return hash_get( context->surfaces , surface_name );
}

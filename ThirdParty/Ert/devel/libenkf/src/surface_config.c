/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'surface_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <ert/util/util.h>

#include <ert/geometry/geo_surface.h>

#include <ert/enkf/surface_config.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_types.h>

#define SURFACE_CONFIG_TYPE_ID 853317

struct surface_config_struct {
  UTIL_TYPE_ID_DECLARATION;
  geo_surface_type * base_surface;
};



surface_config_type  * surface_config_alloc_empty( ) {
  surface_config_type * config = util_malloc( sizeof * config );
  UTIL_TYPE_ID_INIT( config , SURFACE_CONFIG_TYPE_ID );
  config->base_surface  = NULL;
  return config;
}



void surface_config_free( surface_config_type * config ) {
  if (config->base_surface != NULL)
    geo_surface_free( config->base_surface );
  
  free( config );
}


void surface_config_set_base_surface( surface_config_type * config , const char * base_surface ) {
  if (config->base_surface != NULL) 
    geo_surface_free( config->base_surface );
  config->base_surface = geo_surface_fload_alloc_irap( base_surface , false );
}


const geo_surface_type * surface_config_get_base_surface( const surface_config_type * config ) {
  return config->base_surface;
}


int surface_config_get_data_size( const surface_config_type * config ) {
  return geo_surface_get_size( config->base_surface );
}

void surface_config_ecl_write( const surface_config_type * config , const char * filename , const double * zcoord) {
  geo_surface_fprintf_irap_external_zcoord(  config->base_surface , filename , zcoord );
}


/*****************************************************************/
UTIL_SAFE_CAST_FUNCTION(surface_config , SURFACE_CONFIG_TYPE_ID)
UTIL_SAFE_CAST_FUNCTION_CONST(surface_config , SURFACE_CONFIG_TYPE_ID)
VOID_GET_DATA_SIZE(surface)
VOID_CONFIG_FREE(surface)



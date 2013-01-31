/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'surface_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SURFACE_CONFIG_H__
#define __SURFACE_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/geometry/geo_surface.h>

#include <ert/enkf/enkf_macros.h>

  typedef struct surface_config_struct surface_config_type;

  void                     surface_config_ecl_write( const surface_config_type * config , const char * filename , const double * zcoord);
  const geo_surface_type * surface_config_get_base_surface( const surface_config_type * config );
  void                     surface_config_free( surface_config_type * config );
  int                      surface_config_get_data_size(const surface_config_type * config );
  surface_config_type    * surface_config_alloc_empty( );
  void                     surface_config_set_base_surface( surface_config_type * config , const char * base_surface );

  UTIL_SAFE_CAST_HEADER(surface_config);
  UTIL_SAFE_CAST_HEADER_CONST(surface_config);
  GET_DATA_SIZE_HEADER(surface);
  VOID_GET_DATA_SIZE_HEADER(surface);
  VOID_CONFIG_FREE_HEADER(surface);


#ifdef __cplusplus
}
#endif

#endif

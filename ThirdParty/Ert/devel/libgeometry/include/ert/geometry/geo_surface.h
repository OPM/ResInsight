/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'geo_surface.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __GEO_SURFACE_H__
#define __GEO_SURFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/geometry/geo_pointset.h>


  typedef struct geo_surface_struct geo_surface_type;


  void                geo_surface_free( geo_surface_type * geo_surface );
  void                geo_surface_free__( void * arg);
  geo_pointset_type * geo_surface_get_pointset( const geo_surface_type * surface );
  geo_surface_type  * geo_surface_fload_alloc_irap( const char * filename , bool loadz);
  void                geo_surface_fload_irap_zcoord( const geo_surface_type * surface, const char * filename, double *zlist);
  int                 geo_surface_get_size( const geo_surface_type * surface );
  void                geo_surface_fprintf_irap( const geo_surface_type * surface, const char * filename );
  void                geo_surface_fprintf_irap_external_zcoord( const geo_surface_type * surface, const char * filename , const double * zcoord);

#ifdef __cplusplus
}
#endif
#endif

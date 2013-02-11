/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'geo_polygon.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __GEO_POLYGON_H__
#define __GEO_POLYGON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

  typedef struct geo_polygon_struct geo_polygon_type;
  
  geo_polygon_type * geo_polygon_alloc( );
  void               geo_polygon_free( geo_polygon_type * polygon );
  void               geo_polygon_free__( void * arg );
  void               geo_polygon_add_point( geo_polygon_type * polygon , double x , double y );
  geo_polygon_type * geo_polygon_fload_alloc_irap( const char * filename );
  bool               geo_polygon_contains_point( const geo_polygon_type * polygon , double x , double y);

#ifdef __cplusplus
}
#endif
#endif

/*
   Copyright (C) 2011  Equinor ASA, Norway.

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


#ifndef ERT_GEO_SURFACE_H
#define ERT_GEO_SURFACE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <ert/geometry/geo_pointset.hpp>


  typedef struct geo_surface_struct geo_surface_type;

  bool                geo_surface_equal_header( const geo_surface_type * surface1 , const geo_surface_type * surface2 );
  bool                geo_surface_equal( const geo_surface_type * surface1 , const geo_surface_type * surface2);
  void                geo_surface_free( geo_surface_type * geo_surface );
  void                geo_surface_free__( void * arg);
  geo_pointset_type * geo_surface_get_pointset( const geo_surface_type * surface );
  geo_surface_type  * geo_surface_fload_alloc_irap( const char * filename , bool loadz);
  geo_surface_type  * geo_surface_alloc_new( int nx, int ny, double xinc, double yinc, double xstart, double ystart, double angle );
  bool                geo_surface_fload_irap_zcoord( const geo_surface_type * surface, const char * filename, double *zlist);
  double              geo_surface_iget_zvalue(const geo_surface_type * surface, int index);
  int                 geo_surface_get_size( const geo_surface_type * surface );
  void                geo_surface_fprintf_irap( const geo_surface_type * surface, const char * filename );
  void                geo_surface_fprintf_irap_external_zcoord( const geo_surface_type * surface, const char * filename , const double * zcoord);
  int                 geo_surface_get_nx( const geo_surface_type * surface );
  int                 geo_surface_get_ny( const geo_surface_type * surface );
  void                geo_surface_iget_xy( const geo_surface_type* surface, int index, double* x, double* y);

  void                geo_surface_shift( const geo_surface_type * src , double value);
  void                geo_surface_scale( const geo_surface_type * src , double value);
  void                geo_surface_isub( geo_surface_type * self , const geo_surface_type * other);
  void                geo_surface_iset_zvalue(geo_surface_type * surface, int index , double value);
  void                geo_surface_assign_value( const geo_surface_type * src , double value);
  geo_surface_type  * geo_surface_alloc_copy( const geo_surface_type * src , bool copy_zdata);
  void                geo_surface_iadd( geo_surface_type * self , const geo_surface_type * other);
  void                geo_surface_imul( geo_surface_type * self , const geo_surface_type * other);
  void                geo_surface_isqrt( geo_surface_type * surface );

#ifdef __cplusplus
}
#endif
#endif

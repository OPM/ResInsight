/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'geo_pointset.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_GEO_POINTSET_H
#define ERT_GEO_POINTSET_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct geo_pointset_struct geo_pointset_type;


  geo_pointset_type * geo_pointset_alloc( bool external_z );
  void                geo_pointset_free( geo_pointset_type * pointset );
  void                geo_pointset_add_xyz( geo_pointset_type * pointset , double x , double y, double z);
  int                 geo_pointset_get_size( const geo_pointset_type * pointset );
  void                geo_pointset_iget_xy( const geo_pointset_type * pointset , int index , double * x , double * y);
  const double      * geo_pointset_get_zcoord( const geo_pointset_type * pointset );
  bool                geo_pointset_equal( const geo_pointset_type * pointset1 , const geo_pointset_type * pointset2);
  double              geo_pointset_iget_z( const geo_pointset_type * pointset , int index );
  void                geo_pointset_iset_z( geo_pointset_type * pointset , int index , double value);
  void                geo_pointset_memcpy( const geo_pointset_type * src, geo_pointset_type * target , bool copy_zdata);
  void                geo_pointset_shift_z( geo_pointset_type * pointset , double value );
  void                geo_pointset_assign_z( geo_pointset_type * pointset , double value );
  void                geo_pointset_scale_z( geo_pointset_type * pointset , double value );
  void                geo_pointset_imul( geo_pointset_type * pointset , const geo_pointset_type * other );
  void                geo_pointset_iadd( geo_pointset_type * pointset , const geo_pointset_type * other );
  void                geo_pointset_isub( geo_pointset_type * self , const geo_pointset_type * other );
  void                geo_pointset_isqrt( geo_pointset_type * pointset );

#ifdef __cplusplus
}
#endif
#endif

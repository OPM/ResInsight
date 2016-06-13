/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'point.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_POINT_H
#define ERT_POINT_H
#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdbool.h>
#include <stdio.h>
  
  typedef struct point_struct  point_type;
  
  struct point_struct {
    double x;
    double y;
    double z;
  };
  
  void         point_mapaxes_invtransform( point_type * p , const double origo[2], const double unit_x[2] , const double unit_y[2]);
  void         point_mapaxes_transform( point_type * p , const double origo[2] , const double unit_x[2] , const double unit_y[2]);
  point_type * point_alloc_empty( );
  void         point_inplace_sub(point_type * point , const point_type * sub);
  void         point_inplace_add(point_type * point , const point_type * add);
  void         point_inplace_scale(point_type * point , double scale_factor);
  bool         point_equal( const point_type *p1 , const point_type * p2);
  void         point_compare( const point_type *p1 , const point_type * p2, bool * equal);
  void         point_dump( const point_type * p , FILE * stream);
  void         point_dump_ascii( const point_type * p , FILE * stream , const double * offset);
  void         point_fprintf( const point_type * p , FILE * stream );
  void         point_free( point_type * p);
  void         point_set( point_type *p , double x , double y , double z);
  void         point_shift(point_type * p , double dx , double dy , double dz);
  point_type * point_alloc( double x , double y , double z);
  point_type * point_copyc( const point_type * p);
  void         point_copy_values(point_type * p , const point_type * src);
  void         point_vector_cross(point_type * A , const point_type * B , const point_type * C);
  double       point_dot_product( const point_type * v1 , const point_type * v2);
  void         point_normal_vector(point_type * n, const point_type * p0, const point_type * p1 , const point_type * p2);
  double       point_plane_distance(const point_type * p , const point_type * n , const point_type * plane_point);
  double       point3_plane_distance(const point_type * p0 , const point_type * p1 , const point_type * p2 , const point_type * x);

#ifdef __cplusplus
}
#endif
#endif

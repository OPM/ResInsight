/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'point.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <ert/util/util.h>

#include <ert/ecl/point.h>


void point_mapaxes_transform( point_type * p , const double origo[2], const double unit_x[2] , const double unit_y[2]) {
  double new_x =  origo[0] + p->x*unit_x[0] + p->y*unit_y[0];
  double new_y =  origo[1] + p->x*unit_x[1] + p->y*unit_y[1];
    
  p->x = new_x;
  p->y = new_y;
}


void point_mapaxes_invtransform( point_type * p , const double origo[2], const double unit_x[2] , const double unit_y[2]) {
  double norm   =  1.0 / (unit_x[0]*unit_y[1] - unit_x[1] * unit_y[0]);
  double dx     = p->x - origo[0];
  double dy     = p->y - origo[1];


  double org_x  =  ( dx*unit_y[1] - dy*unit_y[0]) * norm;
  double org_y  =  (-dx*unit_x[1] + dy*unit_x[0]) * norm;
  
  p->x = org_x;
  p->y = org_y;
}



void point_compare( const point_type *p1 , const point_type * p2, bool * equal) {
  const double tolerance = 0.001;
  
  double diff_x = (fabs(p1->x - p2->x) / fabs(p1->x + p2->x + 1));
  double diff_y = (fabs(p1->y - p2->y) / fabs(p1->y + p2->y + 1));
  double diff_z = (fabs(p1->z - p2->z) / fabs(p1->z + p2->z + 1));
  
  if (diff_x + diff_y + diff_z > tolerance)
    *equal = false;
}

bool point_equal( const point_type *p1 , const point_type * p2) {
  return (memcmp( p1 , p2 , sizeof * p1 ) == 0);
}


void point_dump( const point_type * p , FILE * stream) {
  util_fwrite_double( p->x , stream );
  util_fwrite_double( p->y , stream );
  util_fwrite_double( p->z , stream );
}


void point_dump_ascii( const point_type * p , FILE * stream , const double * offset) {
  if (offset)
    fprintf(stream , "(%7.2f, %7.2f, %7.2f) " , p->x - offset[0], p->y - offset[1] , p->z - offset[2]);
  else
    fprintf(stream , "(%7.2f, %7.2f, %7.2f) " , p->x , p->y , p->z);

}



void point_set(point_type * p , double x , double y , double z) {
  p->x = x;
  p->y = y;
  p->z = z;
}


void point_shift(point_type * p , double dx , double dy , double dz) {
  p->x += dx;
  p->y += dy;
  p->z += dz;
}


void point_copy_values(point_type * p , const point_type * src) {
  point_set(p , src->x , src->y , src->z);
}


point_type * point_alloc_empty( ) {
  point_type * p = util_malloc( sizeof * p );
  return p;
}

point_type * point_alloc( double x , double y , double z) {
  point_type * p = point_alloc_empty( );
  point_set( p , x , y , z );
  return p;
}



point_type * point_copyc( const point_type * p) {
  point_type * copy = util_alloc_copy( p , sizeof * p );
  return copy;
}


point_type * point_alloc_diff( const point_type * p1 , const point_type * p0) {
  point_type * diff = point_copyc( p1 );
  point_inplace_sub( diff , p0 );
  return diff;
}



void point_free( point_type * p) {
  free( p );
}


void point_fprintf( const point_type * p , FILE * stream ) {
  fprintf(stream , "%g %g %g ",p->x , p->y , p->z);
}

/*****************************************************************/
/* Math operations */

void point_inplace_add(point_type * point , const point_type * add) {
  point->x += add->x;
  point->y += add->y;
  point->z += add->z;
}


void point_inplace_sub(point_type * point , const point_type * sub) {
  point->x -= sub->x;
  point->y -= sub->y;
  point->z -= sub->z;
}


void point_inplace_scale(point_type * point , double scale_factor) {
  point->x *= scale_factor;
  point->y *= scale_factor;
  point->z *= scale_factor;
}


/**
   Will compute the vector cross product B x C and store the result in A.
*/

void point_vector_cross(point_type * A , const point_type * B , const point_type * C) {
  A->x =   B->y*C->z - B->z*C->y;
  A->y = -(B->x*C->z - B->z*C->x);
  A->z =   B->x*C->y - B->y*C->x;
}

double point_dot_product( const point_type * v1 , const point_type * v2) {
  return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}



/**
   Computes the normal vector spanned by the two vectors: v1 = (p1 - p0) and v2 = (p2 - p0). The
   result is stored in n.
*/

void point_normal_vector(point_type * n, const point_type * p0, const point_type * p1 , const point_type * p2) {
  point_type * v1 = point_alloc_diff( p1 , p0 );
  point_type * v2 = point_alloc_diff( p2 , p0 );

  point_vector_cross( n , v1 , v2 );

  free( v1 );
  free( v2 );
}



/**
   This function calculates the (signed) distance from point 'p' to
   the plane specifed by the plane vector 'n' and the point
   'plane_point' which is part of the plane.
*/

double point_plane_distance(const point_type * p , const point_type * n , const point_type * plane_point) {
  point_type * diff = point_alloc_diff( p , plane_point );
  double d = point_dot_product( n , diff );
  
  free( diff );
  return d;
}


double point3_plane_distance(const point_type * p0 , const point_type * p1 , const point_type * p2 , const point_type * x) {
  point_type n;
  point_normal_vector( &n , p0 , p1 , p2 );
  return point_plane_distance( x , &n , p0 ) / sqrt( n.x*n.x + n.y*n.y + n.z*n.z);
}

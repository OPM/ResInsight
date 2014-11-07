/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'tetrahedron.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/matrix.h>

#include <ert/ecl/point.h>
#include <ert/ecl/tetrahedron.h>



void tetrahedron_init( tetrahedron_type * tet , const point_type * p0 , const point_type * p1 , const point_type * p2 , const point_type * p3) {
  tet->p0 = p0;
  tet->p1 = p1;
  tet->p2 = p2;
  tet->p3 = p3;
}





/**
        |a·(b x c)| 
  V  =  -----------
            6   


  Wherea a,b and c are assumed to have origo as reference.
*/          


double tetrahedron_volume( const tetrahedron_type * tet ) {
  point_type a; 
  point_type b;
  point_type c;
  point_type b_x_c;

  point_copy_values(&a , tet->p0);
  point_copy_values(&b , tet->p1);
  point_copy_values(&c , tet->p2);
  
  point_inplace_sub(&a , tet->p3);
  point_inplace_sub(&b , tet->p3);
  point_inplace_sub(&c , tet->p3);

  point_vector_cross( &b_x_c , &b , &c);
  
  return point_dot_product( &a , &b_x_c)  / 6.0;
}




/**
   The __sign() function will return 0 for x ==== 0 - it is not
   exactly zero measure....
*/

static int __sign( double x) {
  const double zero_tol = 1e-8;
  if (fabs(x) < zero_tol)
    return 0;
  else if (x > 0) 
    return 1;
  else
    return -1;
  
}






/**
   When it comes to points which are exactly on one (or more ...) of
   the surfaces it is a mess. The function will return true if exact
   equality is found, unfortunately this implies that the function
   finding the correct cell for a xyz location is not 100% stable:
   
   When it comes to a point on the surface between two cells the first
   cell which queries about the point will be assigned this point.
*/

/**
Point in Tetrahedron TestNewsgroups: comp.graphics,comp.graphics.algorithms
From: herron@cs.washington.edu (Gary Herron)
Subject: Re: point within a tetrahedron
Date: Wed, 23 Feb 94 21:52:45 GMT 


Let the tetrahedron have vertices 
        V1 = (x1, y1, z1)
        V2 = (x2, y2, z2)
        V3 = (x3, y3, z3)
        V4 = (x4, y4, z4)
and your test point be 
        P = (x, y, z).

Then the point P is in the tetrahedron if following five determinants all have 
the same sign. 

             |x1 y1 z1 1|
        D0 = |x2 y2 z2 1|
             |x3 y3 z3 1|
             |x4 y4 z4 1|

             |x  y  z  1|
        D1 = |x2 y2 z2 1|
             |x3 y3 z3 1|
             |x4 y4 z4 1|

             |x1 y1 z1 1|
        D2 = |x  y  z  1|
             |x3 y3 z3 1|
             |x4 y4 z4 1|

             |x1 y1 z1 1|
        D3 = |x2 y2 z2 1|
             |x  y  z  1|
             |x4 y4 z4 1|

             |x1 y1 z1 1|
        D4 = |x2 y2 z2 1|
             |x3 y3 z3 1|
             |x  y  z  1|


  Some additional notes: 
  If by chance the D0=0, then your tetrahedron is degenerate (the points are 
  coplanar). 
  If any other Di=0, then P lies on boundary i (boundary i being that boundary 
  formed by the three points other than Vi). 
  If the sign of any Di differs from that of D0 then P is outside boundary i. 
  If the sign of any Di equals that of D0 then P is inside boundary i. 
  If P is inside all 4 boundaries, then it is inside the tetrahedron. 
  As a check, it must be that D0 = D1+D2+D3+D4. 
  The pattern here should be clear; the computations can be extended to 
  simplicies of any dimension. (The 2D and 3D case are the triangle and the 
  tetrahedron). 
  If it is meaningful to you, the quantities bi = Di/D0 are the usual 
  barycentric coordinates. 
  Comparing signs of Di and D0 is only a check that P and Vi are on the same 
  side of boundary i. 

*/



bool tetrahedron_contains__( const tetrahedron_type * tet , const point_type * p, matrix_type * D) {
  const point_type * p1 = tet->p0;
  const point_type * p2 = tet->p1;
  const point_type * p3 = tet->p2; 
  const point_type * p4 = tet->p3;
  

  int current_sign , sign;
  /*
    Special casing around the vertex points should be handled prior to
    calling this function.
  */

  /*****************************************************************/
  matrix_iset( D , 0 , 0 , p1->x);
  matrix_iset( D , 0 , 1 , p1->y);
  matrix_iset( D , 0 , 2 , p1->z);
  matrix_iset( D , 0 , 3 , 1);
  
  matrix_iset( D , 1 , 0 , p2->x);
  matrix_iset( D , 1 , 1 , p2->y);
  matrix_iset( D , 1 , 2 , p2->z);
  matrix_iset( D , 1 , 3 , 1);

  matrix_iset( D , 2 , 0 , p3->x);
  matrix_iset( D , 2 , 1 , p3->y);
  matrix_iset( D , 2 , 2 , p3->z);
  matrix_iset( D , 2 , 3 , 1);

  matrix_iset( D , 3 , 0 , p4->x);
  matrix_iset( D , 3 , 1 , p4->y);
  matrix_iset( D , 3 , 2 , p4->z);
  matrix_iset( D , 3 , 3 , 1);
  
  {
    double D0 = matrix_det4( D );
    current_sign = __sign( D0 );
    if (current_sign == 0)
      return false; /* A zero volume cell. */
  }

  /*****************************************************************/
  matrix_iset( D , 0 , 0 , p->x);
  matrix_iset( D , 0 , 1 , p->y);
  matrix_iset( D , 0 , 2 , p->z);
  matrix_iset( D , 0 , 3 , 1);
  
  matrix_iset( D , 1 , 0 , p2->x);
  matrix_iset( D , 1 , 1 , p2->y);
  matrix_iset( D , 1 , 2 , p2->z);
  matrix_iset( D , 1 , 3 , 1);

  matrix_iset( D , 2 , 0 , p3->x);
  matrix_iset( D , 2 , 1 , p3->y);
  matrix_iset( D , 2 , 2 , p3->z);
  matrix_iset( D , 2 , 3 , 1);

  matrix_iset( D , 3 , 0 , p4->x);
  matrix_iset( D , 3 , 1 , p4->y);
  matrix_iset( D , 3 , 2 , p4->z);
  matrix_iset( D , 3 , 3 , 1);
  
  {
    double D1 = matrix_det4( D );
    sign = __sign( D1 );
    if ((sign != 0) && (sign != current_sign)) 
      return false;
  }

  /*****************************************************************/
  matrix_iset( D , 0 , 0 , p1->x);
  matrix_iset( D , 0 , 1 , p1->y);
  matrix_iset( D , 0 , 2 , p1->z);
  matrix_iset( D , 0 , 3 , 1);
  
  matrix_iset( D , 1 , 0 , p->x);
  matrix_iset( D , 1 , 1 , p->y);
  matrix_iset( D , 1 , 2 , p->z);
  matrix_iset( D , 1 , 3 , 1);

  matrix_iset( D , 2 , 0 , p3->x);
  matrix_iset( D , 2 , 1 , p3->y);
  matrix_iset( D , 2 , 2 , p3->z);
  matrix_iset( D , 2 , 3 , 1);

  matrix_iset( D , 3 , 0 , p4->x);
  matrix_iset( D , 3 , 1 , p4->y);
  matrix_iset( D , 3 , 2 , p4->z);
  matrix_iset( D , 3 , 3 , 1);
  
  {
    double D2 = matrix_det4( D );
    sign = __sign( D2 );
    if ((sign != 0) && (sign != current_sign)) 
      return false;
  }
  /*****************************************************************/

  matrix_iset( D , 0 , 0 , p1->x);
  matrix_iset( D , 0 , 1 , p1->y);
  matrix_iset( D , 0 , 2 , p1->z);
  matrix_iset( D , 0 , 3 , 1);
  
  matrix_iset( D , 1 , 0 , p2->x);
  matrix_iset( D , 1 , 1 , p2->y);
  matrix_iset( D , 1 , 2 , p2->z);
  matrix_iset( D , 1 , 3 , 1);

  matrix_iset( D , 2 , 0 , p->x);
  matrix_iset( D , 2 , 1 , p->y);
  matrix_iset( D , 2 , 2 , p->z);
  matrix_iset( D , 2 , 3 , 1);

  matrix_iset( D , 3 , 0 , p4->x);
  matrix_iset( D , 3 , 1 , p4->y);
  matrix_iset( D , 3 , 2 , p4->z);
  matrix_iset( D , 3 , 3 , 1);

  {
    double D3 = matrix_det4( D );
    sign = __sign( D3 );
    if ((sign != 0) && (sign != current_sign)) 
      return false;
  }
  /*****************************************************************/

  matrix_iset( D , 0 , 0 , p1->x);
  matrix_iset( D , 0 , 1 , p1->y);
  matrix_iset( D , 0 , 2 , p1->z);
  matrix_iset( D , 0 , 3 , 1);
  
  matrix_iset( D , 1 , 0 , p2->x);
  matrix_iset( D , 1 , 1 , p2->y);
  matrix_iset( D , 1 , 2 , p2->z);
  matrix_iset( D , 1 , 3 , 1);

  matrix_iset( D , 2 , 0 , p3->x);
  matrix_iset( D , 2 , 1 , p3->y);
  matrix_iset( D , 2 , 2 , p3->z);
  matrix_iset( D , 2 , 3 , 1);

  matrix_iset( D , 3 , 0 , p->x);
  matrix_iset( D , 3 , 1 , p->y);
  matrix_iset( D , 3 , 2 , p->z);
  matrix_iset( D , 3 , 3 , 1);
  
  {
    double D4 = matrix_det4( D );
    sign = __sign( D4 );
    if ((sign != 0) && (sign != current_sign)) 
      return false;
  }
  return true;
}


bool tetrahedron_contains( const tetrahedron_type * tet , const point_type * p) {
  matrix_type * D = matrix_alloc(4 , 4);
  bool contains = tetrahedron_contains__( tet , p , D );
  matrix_free( D );
  return contains;
}


void tetrahedron_fprintf( const tetrahedron_type * tet , FILE * stream , const double* offset) {
  fprintf(stream , "P0: "); point_dump_ascii( tet->p0 , stream , offset); fprintf(stream , "\n");
  fprintf(stream , "P1: "); point_dump_ascii( tet->p1 , stream , offset); fprintf(stream , "\n");
  fprintf(stream , "P2: "); point_dump_ascii( tet->p2 , stream , offset); fprintf(stream , "\n");
  fprintf(stream , "P3: "); point_dump_ascii( tet->p3 , stream , offset); fprintf(stream , "\n");
}

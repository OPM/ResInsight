/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'tetrahedron.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_TETRAHEDRON_H
#define ERT_TETRAHEDRON_H


typedef struct tetrahedron_struct tetrahedron_type;

struct tetrahedron_struct {
  const point_type * p0;
  const point_type * p1;
  const point_type * p2;
  const point_type * p3; 
};



void               tetrahedron_init( tetrahedron_type * tet , const point_type * p0 , const point_type * p1 , const point_type * p2 , const point_type * p3);
double             tetrahedron_volume( const tetrahedron_type * tet );
bool               tetrahedron_contains( const tetrahedron_type * tet , const point_type * p);
void               tetrahedron_fprintf( const tetrahedron_type * tet , FILE * stream , const double * offset);
#endif



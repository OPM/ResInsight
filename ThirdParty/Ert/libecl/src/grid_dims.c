/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'grid_dims.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/ecl/grid_dims.h>


void grid_dims_init( grid_dims_type * dims , int nx, int ny , int nz , int nactive) {
  dims->nx = nx;
  dims->ny = ny;
  dims->nz = nz;
  dims->nactive = nactive;
}



grid_dims_type * grid_dims_alloc( int nx, int ny , int nz , int nactive) {
  grid_dims_type * dims = util_malloc( sizeof * dims );
  grid_dims_init( dims , nx , ny , nz , nactive );
  return dims;
}



void grid_dims_free( grid_dims_type * dims ) {
  free( dims );
}


void grid_dims_free__( void * arg) {
  grid_dims_type * grid_dims = (grid_dims_type * ) arg;
  grid_dims_free( grid_dims );
}

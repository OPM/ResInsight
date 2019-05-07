/*
   Copyright (C) 2013  Equinor ASA, Norway.

   The file 'grid_dims.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_GRID_DIMS_H
#define ERT_GRID_DIMS_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int nx,ny,nz,nactive;
} grid_dims_type;

  void              grid_dims_init( grid_dims_type * dims , int nx, int ny , int nz , int nactive);
  grid_dims_type *  grid_dims_alloc( int nx, int ny , int nz , int nactive);
  void              grid_dims_free( grid_dims_type * dims );
  void              grid_dims_free__( void * arg);

#ifdef __cplusplus
}
#endif
#endif

/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_grid_cache.c' is part of ERT - Ensemble based
   Reservoir Tool.

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
#include <stdbool.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_grid_cache.h>




/**
   The ecl_grid_cache_struct data structure internalizes the world
   position of all the active cells. This is just a minor
   simplification to speed up repeated calls to get the true world
   coordinates of a cell.
*/

struct ecl_grid_cache_struct {
  int                   size;         /* The length of the vectors, equal to the number of active elements in the grid. */
  double              * xpos;
  double              * ypos;
  double              * zpos;
  double              * volume;       /* Will be initialized on demand. */
  int                 * global_index; /* Maps from active index (i.e. natural index in this context) - to the corresponding global index. */
  const ecl_grid_type * grid;
};





ecl_grid_cache_type * ecl_grid_cache_alloc( const ecl_grid_type * grid ) {
  ecl_grid_cache_type * grid_cache = util_malloc( sizeof * grid_cache );

  grid_cache->grid          = grid;
  grid_cache->volume        = NULL;
  grid_cache->size          = ecl_grid_get_active_size( grid );
  grid_cache->xpos          = util_calloc( grid_cache->size , sizeof * grid_cache->xpos );
  grid_cache->ypos          = util_calloc( grid_cache->size , sizeof * grid_cache->ypos );
  grid_cache->zpos          = util_calloc( grid_cache->size , sizeof * grid_cache->zpos );
  grid_cache->global_index  = util_calloc( grid_cache->size , sizeof * grid_cache->global_index );
  {
    int active_index;


    /* Go trough all the active cells and extract the cell center
       position and store it in xpos/ypos/zpos. */

    for (active_index = 0; active_index < grid_cache->size; active_index++) {
      int global_index = ecl_grid_get_global_index1A( grid , active_index );
      grid_cache->global_index[ active_index ] = global_index;
      ecl_grid_get_xyz1( grid , global_index ,
                         &grid_cache->xpos[ active_index ] ,
                         &grid_cache->ypos[ active_index ] ,
                         &grid_cache->zpos[ active_index ]);
    }

  }
  return grid_cache;
}

int ecl_grid_cache_get_size( const ecl_grid_cache_type * grid_cache ) {
  return grid_cache->size;
}

int ecl_grid_cache_iget_global_index( const ecl_grid_cache_type * grid_cache , int active_index) {
  return grid_cache->global_index[ active_index ];
}


const int * ecl_grid_cache_get_global_index( const ecl_grid_cache_type * grid_cache) {
  return grid_cache->global_index;
}

const double * ecl_grid_cache_get_xpos( const ecl_grid_cache_type * grid_cache ) {
  return grid_cache->xpos;
}

const double * ecl_grid_cache_get_ypos( const ecl_grid_cache_type * grid_cache ) {
  return grid_cache->ypos;
}

const double * ecl_grid_cache_get_zpos( const ecl_grid_cache_type * grid_cache ) {
  return grid_cache->zpos;
}

const double * ecl_grid_cache_get_volume( const ecl_grid_cache_type * grid_cache ) {

  if (!grid_cache->volume) {
    // C++ style const cast.
    ecl_grid_cache_type * gc = (ecl_grid_cache_type *) grid_cache;
    gc->volume = util_calloc( gc->size , sizeof * gc->volume );
    for (int active_index = 0; active_index < grid_cache->size; active_index++)
      gc->volume[active_index] = ecl_grid_get_cell_volume1A( gc->grid , active_index );
  }

  return grid_cache->volume;
}


void ecl_grid_cache_free( ecl_grid_cache_type * grid_cache ) {
  free( grid_cache->xpos );
  free( grid_cache->ypos );
  free( grid_cache->zpos );
  free( grid_cache->global_index );
  free( grid_cache->volume );
  free( grid_cache );
}

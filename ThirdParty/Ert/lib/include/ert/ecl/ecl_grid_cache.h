/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_grid_cache.h' is part of ERT - Ensemble based
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

#ifndef ERT_ECL_GRID_CACHE_H
#define ERT_ECL_GRID_CACHE_H

#include <ert/ecl/ecl_grid.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct ecl_grid_cache_struct ecl_grid_cache_type;


  ecl_grid_cache_type  * ecl_grid_cache_alloc( const ecl_grid_type * grid );
  int                    ecl_grid_cache_get_size( const ecl_grid_cache_type * grid_cache );
  int                    ecl_grid_cache_iget_global_index( const ecl_grid_cache_type * grid_cache , int active_index);
  const int            * ecl_grid_cache_get_global_index( const ecl_grid_cache_type * grid_cache );
  const double         * ecl_grid_cache_get_xpos( const ecl_grid_cache_type * grid_cache );
  const double         * ecl_grid_cache_get_ypos( const ecl_grid_cache_type * grid_cache );
  const double         * ecl_grid_cache_get_zpos( const ecl_grid_cache_type * grid_cache );
  const double         * ecl_grid_cache_get_volume( const ecl_grid_cache_type * grid_cache );
  void                   ecl_grid_cache_free( ecl_grid_cache_type * grid_cache );


#ifdef __cplusplus
}
#endif

#endif

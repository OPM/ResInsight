/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_grav_common.h' is part of ERT - Ensemble based
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

#ifndef ERT_ECL_GRAV_COMMON_H
#define ERT_ECL_GRAV_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/ecl/ecl_grid_cache.h>
#include <ert/ecl/ecl_file.h>

  bool   * ecl_grav_common_alloc_aquifer_cell( const ecl_grid_cache_type * grid_cache , const ecl_file_type * init_file);
  double   ecl_grav_common_eval_biot_savart( const ecl_grid_cache_type * grid_cache , ecl_region_type * region , const bool * aquifer , const double * weight ,  double utm_x , double utm_y , double depth);
  double ecl_grav_common_eval_geertsma( const ecl_grid_cache_type * grid_cache , ecl_region_type * region , const bool * aquifer , const double * weight , double utm_x , double utm_y , double depth, double poisson_ratio, double seabed);

#ifdef __cplusplus
}

#endif
#endif

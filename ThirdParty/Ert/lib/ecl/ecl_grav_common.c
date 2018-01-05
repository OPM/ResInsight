/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_grav_common.c' is part of ERT - Ensemble based
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
#include <stdbool.h>
#include <math.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_region.h>
#include <ert/ecl/ecl_grid_cache.h>
#include <ert/ecl/ecl_kw_magic.h>

/*
  This file contains code which is common to both the ecl_grav
  implementation for gravity changes, and the ecl_subsidence
  implementation for changes in subsidence.
*/

bool * ecl_grav_common_alloc_aquifer_cell( const ecl_grid_cache_type * grid_cache , const ecl_file_type * init_file) {
  bool * aquifer_cell = util_calloc( ecl_grid_cache_get_size( grid_cache ) , sizeof * aquifer_cell  );

  for (int active_index = 0; active_index < ecl_grid_cache_get_size( grid_cache ); active_index++)
    aquifer_cell[ active_index ] = false;

  if (ecl_file_has_kw( init_file , AQUIFER_KW)) {
    ecl_kw_type * aquifer_kw = ecl_file_iget_named_kw( init_file , AQUIFER_KW , 0);
    const int * aquifer_data = ecl_kw_get_int_ptr( aquifer_kw );

    for (int active_index = 0; active_index < ecl_grid_cache_get_size( grid_cache ); active_index++) {
      if (aquifer_data[ active_index ] < 0)
        aquifer_cell[ active_index ] = true;
    }
  }

  return aquifer_cell;
}



double ecl_grav_common_eval_biot_savart( const ecl_grid_cache_type * grid_cache , ecl_region_type * region , const bool * aquifer , const double * weight , double utm_x , double utm_y , double depth) {
  const double * xpos      = ecl_grid_cache_get_xpos( grid_cache );
  const double * ypos      = ecl_grid_cache_get_ypos( grid_cache );
  const double * zpos      = ecl_grid_cache_get_zpos( grid_cache );
  double sum = 0;
  if (region == NULL) {
    const int      size   = ecl_grid_cache_get_size( grid_cache );
    int index;
    for ( index = 0; index < size; index++) {
      if (!aquifer[index]) {
        double dist_x  = (xpos[index] - utm_x );
        double dist_y  = (ypos[index] - utm_y );
        double dist_z  = (zpos[index] - depth );
        double dist    = sqrt( dist_x*dist_x + dist_y*dist_y + dist_z*dist_z );

        /**
            For numerical precision it might be benficial to use the
            util_kahan_sum() function to do a Kahan summation.
        */
        sum += weight[index] * dist_z/(dist * dist * dist );
      }
    }
  } else {
    const int_vector_type * index_vector = ecl_region_get_active_list( region );
    const int size = int_vector_size( index_vector );
    const int * index_list = int_vector_get_const_ptr( index_vector );
    int i, index;
    for (i = 0; i < size; i++) {
      index = index_list[i];
      if (!aquifer[index]) {
        double dist_x  = (xpos[index] - utm_x );
        double dist_y  = (ypos[index] - utm_y );
        double dist_z  = (zpos[index] - depth );
        double dist    = sqrt( dist_x*dist_x + dist_y*dist_y + dist_z*dist_z );

        sum += weight[index] * dist_z/(dist * dist * dist );
      }
    }
  }
  return sum;
}


static inline double ecl_grav_common_eval_geertsma_kernel(int index, const double * xpos, const double * ypos, const double * zpos , double utm_x , double utm_y , double depth, double poisson_ratio, double seabed) {
  double z = zpos[index];
  z -= seabed;
  double dist_x  = xpos[index] - utm_x;
  double dist_y  = ypos[index] - utm_y;

  double dist_z1 = z - depth;
  double dist_z2 = dist_z1 - 2*z;

  double dist1   = sqrt( dist_x*dist_x + dist_y*dist_y + dist_z1*dist_z1 );
  double dist2   = sqrt( dist_x*dist_x + dist_y*dist_y + dist_z2*dist_z2 );

  double cube_dist1 = dist1*dist1*dist1;
  double cube_dist2 = dist2*dist2*dist2;

  double displacement =
    dist_z1 / cube_dist1 +
    (3 - 4*poisson_ratio)*dist_z2 / cube_dist2 -
    6*depth * (z + depth) * dist_z2 / (dist2*dist2*cube_dist2) +
    2*((3 - 4*poisson_ratio)*(z + depth) - depth)/cube_dist2 ;

  return displacement;
}


double ecl_grav_common_eval_geertsma( const ecl_grid_cache_type * grid_cache , ecl_region_type * region , const bool * aquifer , const double * weight , double utm_x , double utm_y , double depth, double poisson_ratio, double seabed) {
  const double * xpos      = ecl_grid_cache_get_xpos( grid_cache );
  const double * ypos      = ecl_grid_cache_get_ypos( grid_cache );
  const double * zpos      = ecl_grid_cache_get_zpos( grid_cache );
  double sum = 0;
  if (region == NULL) {
    const int      size = ecl_grid_cache_get_size( grid_cache );
    int index;
    for ( index = 0; index < size; index++) {
      if (!aquifer[index]) {

        double displacement = ecl_grav_common_eval_geertsma_kernel( index, xpos , ypos , zpos, utm_x, utm_y , depth, poisson_ratio, seabed);

        /**
            For numerical precision it might be benficial to use the
            util_kahan_sum() function to do a Kahan summation.
        */
        sum += weight[index] * displacement;
      }
    }
  } else {
    const int_vector_type * index_vector = ecl_region_get_active_list( region );
    const int size = int_vector_size( index_vector );
    const int * index_list = int_vector_get_const_ptr( index_vector );
    int i, index;
    for (i = 0; i < size; i++) {
      index = index_list[i];
      if (!aquifer[index]) {
        double displacement = ecl_grav_common_eval_geertsma_kernel( index, xpos , ypos , zpos, utm_x, utm_y , depth , poisson_ratio, seabed);
        sum += weight[index] * displacement;
      }
    }
  }
  return sum;
}



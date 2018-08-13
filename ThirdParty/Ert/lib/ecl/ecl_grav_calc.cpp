/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_grav.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_grav.hpp>



/**
   This file contains one function, ecl_grav_phase_deltag() which
   calculates the change in local gravitational strength (in units of
   micro Gal) in a point, based on base and monitor values for pore
   volume, saturation and density. Should typically be called several
   times, one time for each phase.
*/

/**
   Check that the rporv values are in the right ballpark.  For
   ECLIPSE version 2008.2 they are way fucking off. Check PORV
   versus RPORV for ten 'random' locations in the grid.
*/


static void ecl_grav_check_rporv(const ecl_grid_type * ecl_grid , const ecl_kw_type * rporv1_kw , const ecl_kw_type * rporv2_kw , const ecl_kw_type * init_porv_kw) {
  int    active_index;
  int    active_delta;
  int    active_size;

  ecl_grid_get_dims( ecl_grid , NULL , NULL , NULL , &active_size );
  active_delta = active_size / 12;
  for (active_index = active_delta; active_index < active_size; active_index += active_delta) {
    int    global_index = ecl_grid_get_global_index1A( ecl_grid , active_index );
    double init_porv    = ecl_kw_iget_as_double( init_porv_kw , global_index );   /* NB - this uses global indexing. */
    double rporv1       = ecl_kw_iget_as_double( rporv1_kw ,  active_index );
    double rporv2       = ecl_kw_iget_as_double( rporv2_kw ,  active_index );
    double rporv12      = 0.5 * ( rporv1 + rporv2 );
    double fraction     = util_double_min( init_porv , rporv12 ) / util_double_max( init_porv , rporv12 );

    if (fraction  < 0.50) {
      fprintf(stderr,"-----------------------------------------------------------------\n");
      fprintf(stderr,"INIT PORV: %g \n",init_porv);
      fprintf(stderr,"RPORV1   : %g \n",rporv1);
      fprintf(stderr,"RPORV2   : %g \n",rporv2);
      fprintf(stderr,"Hmmm - the RPORV values extracted from the restart file seem to be \n");
      fprintf(stderr,"veeery different from the initial rporv value. This might indicate\n");
      fprintf(stderr,"an ECLIPSE bug. Version 2007.2 is known to be ok in this respect, \n");
      fprintf(stderr,"whereas version 2008.2 is known to have a bug. \n");
      fprintf(stderr,"-----------------------------------------------------------------\n");
      exit(1);
    }
  }
}



double ecl_grav_phase_deltag( double utm_x ,
                              double utm_y ,
                              double tvd,
                              const ecl_grid_type * grid,
                              const ecl_file_type * init_file,
                              const ecl_kw_type   * sat1_kw,
                              const ecl_kw_type   * rho1_kw,
                              const ecl_kw_type   * porv1_kw,
                              const ecl_kw_type   * sat2_kw,
                              const ecl_kw_type   * rho2_kw,
                              const ecl_kw_type   * porv2_kw) {

  double deltag = 0;
  const int * aquifern      = NULL;

  const float * rho1    = ecl_kw_get_float_ptr( rho1_kw );
  const float * rho2    = ecl_kw_get_float_ptr( rho2_kw );
  const float * sat1    = ecl_kw_get_float_ptr( sat1_kw );
  const float * sat2    = ecl_kw_get_float_ptr( sat2_kw );
  const float * porv1   = ecl_kw_get_float_ptr( porv1_kw );
  const float * porv2   = ecl_kw_get_float_ptr( porv2_kw );

  if (ecl_file_has_kw( init_file , "AQUIFERN")) {
    const ecl_kw_type * aquifern_kw = ecl_file_iget_named_kw( init_file , "AQUIFERN" , 0);
    aquifern = ecl_kw_get_int_ptr( aquifern_kw );
  }

  {
    const ecl_kw_type * init_porv_kw = ecl_file_iget_named_kw( init_file , "PORV" , 0);
    ecl_grav_check_rporv( grid , porv1_kw , porv2_kw , init_porv_kw);
  }

  {
    int active_index;
    for (active_index = 0; active_index < ecl_grid_get_active_size( grid ); active_index++) {
      if (aquifern != NULL && aquifern[ active_index ] != 0)
        continue; /* This is a numerical aquifer cell - skip it. */
      else {
        double  mas1 , mas2;
        double  xpos , ypos , zpos;

        mas1 = rho1[ active_index ] * porv1[active_index] * sat1[active_index];
        mas2 = rho2[ active_index ] * porv2[active_index] * sat2[active_index];

        ecl_grid_get_xyz1A(grid , active_index , &xpos , &ypos , &zpos);
        {
          double dist_x   = xpos - utm_x;
          double dist_y   = ypos - utm_y;
          double dist_z   = zpos - tvd;
          double dist_sq  = dist_x*dist_x + dist_y*dist_y + dist_z*dist_z;

          if(dist_sq == 0)
            exit(1);


          /**
             The Gravitational constant is 6.67E-11 N (m/kg)^2, we
             return the result in microGal, i.e. we scale with 10^2 *
             10^6 => 6.67E-3.
          */
          deltag += 6.67E-3*(mas2 - mas1) * dist_z/pow(dist_sq , 1.5);
        }
      }
    }
  }

  return deltag;
}


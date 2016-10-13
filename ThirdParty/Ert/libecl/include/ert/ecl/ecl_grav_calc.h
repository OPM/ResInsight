/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_grav.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef  ERT_ECL_GRAV_CALC_H
#define  ERT_ECL_GRAV_CALC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <etr/ecl/ecl_kw.h>
#include <etr/ecl/ecl_grid.h>
#include <etr/ecl/ecl_file.h>

double ecl_grav_phase_deltag( double utm_x ,
                              double utm_y ,
                              double tvd,
                              const ecl_grid_type * grid,
                              const ecl_file_type * init_file ,
                              const ecl_kw_type   * sat_kw1,
                              const ecl_kw_type   * rho_kw1,
                              const ecl_kw_type   * porv_kw1,
                              const ecl_kw_type   * sat_kw2,
                              const ecl_kw_type   * rho_kw2,
                              const ecl_kw_type   * porv_kw2);



#ifdef __cplusplus
}
#endif
#endif


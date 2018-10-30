/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_grid_dims.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <signal.h>
#include <math.h>

#include <string>

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_file.hpp>

double err(double a, double b) {
  return (a - b) / a;
}



void test_dxdydz(const std::string& grid_fname, const std::string& init_fname) {
  double eps_x = 1e-4;
  double eps_y = 1e-4;
  double eps_z = 1e-3;
  ecl_grid_type * grid = ecl_grid_alloc( grid_fname.c_str() );
  ecl_file_type * init_file = ecl_file_open( init_fname.c_str(), 0);
  ecl_kw_type * dx = ecl_file_iget_named_kw( init_file, "DX", 0);
  ecl_kw_type * dy = ecl_file_iget_named_kw( init_file, "DY", 0);
  ecl_kw_type * dz = ecl_file_iget_named_kw( init_file, "DZ", 0);
  for(int a=0; a < ecl_grid_get_active_size(grid); a+= 100) {
    int g = ecl_grid_get_global_index1A(grid, a);

    double dxg = ecl_grid_get_cell_dx1(grid, g);
    double dyg = ecl_grid_get_cell_dy1(grid, g);
    double dzg = ecl_grid_get_cell_dz1(grid, g);

    double dxi = ecl_kw_iget_float(dx, a);
    double dyi = ecl_kw_iget_float(dy, a);
    double dzi = ecl_kw_iget_float(dz, a);

    double err_x = fabs(err(dxg, dxi));
    double err_y = fabs(err(dyg, dyi));
    double err_z = fabs(err(dzg, dzi));

    test_assert_true( err_x < eps_x );
    test_assert_true( err_y < eps_y );
    test_assert_true( err_z < eps_z );

  }
  ecl_file_close(init_file);
  ecl_grid_free( grid );
}


int main(int argc , char ** argv) {
  const std::string ecl_case = argv[1];
  std::string grid_file = ecl_case + ".EGRID";
  std::string init_file = ecl_case + ".INIT";

  test_dxdydz(grid_file, init_file);

  exit(0);
}

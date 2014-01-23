/*
   Copyright (C) 2013 Andreas Lauser
   Copyright (C) 2013 Statoil ASA, Norway.

   The file 'ecl_alloc_grid_dxv_dyv_dzv.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/ecl_grid.h>
#include <ert/util/test_util.h>

void test_grid() {
  const int nx = 5, ny = 6, nz = 7;
  double dxv[nx];
  double dyv[ny];
  double dzv[nz];

  double width = 1.0;
  for (int i = 0; i < nx; ++i) {
      dxv[i] = width;
      width += 1.0;
  };
  for (int i = 0; i < ny; ++i) {
      dyv[i] = width;
      width += 1.0;
  };
  for (int i = 0; i < nz; ++i) {
      dzv[i] = width;
      width += 1.0;
  };

  ecl_grid_type * ecl_grid = ecl_grid_alloc_dxv_dyv_dzv(nx,ny,nz,
                                                        dxv, dyv, dzv,
                                                        /*actnum=*/NULL);
  for (int i = 0; i < nx; ++ i) {
      for (int j = 0; j < ny; ++ j) {
          for (int k = 0; k < nz; ++ k) {
              double volume_ert = ecl_grid_get_cell_volume3(ecl_grid, i, j, k);

              test_assert_double_equal(volume_ert, dxv[i]*dyv[j]*dzv[k]);
          }
      }
  }
}

int main(int argc , char ** argv) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */

  test_grid();

  exit(0);
}

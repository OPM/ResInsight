/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ecl_grid_create.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>

#include <ert/ecl/ecl_grid.hpp>



void test_create2() {
  int nx = 10;
  int ny = 8;
  int nz = 5;
  double * dx   = (double *) util_calloc( nx*ny*nz , sizeof * dx );
  double * dy   = (double *) util_calloc( nx*ny*nz , sizeof * dy );
  double * dz   = (double *) util_calloc( nx*ny*nz , sizeof * dz );
  double * tops = (double *) util_calloc( nx*ny*nz , sizeof * dz );
  for (int k=0; k< nz; k++) {
    for (int j=0; j < ny; j++) {
      for (int i=0; i < nx; i++) {
        int g = k*nx*ny + j*nx + i;

        dx[g]   = (i+1);
        dy[g]   = (j+1);
        dz[g]   = (k+1);
        if (k == 0)
          tops[g] = 77;
        else
          tops[g] = tops[g - nx*ny] + dz[g];
      }
    }
  }

  {
    ecl_grid_type * grid = ecl_grid_alloc_dx_dy_dz_tops( nx , ny , nz , dx , dy , dz , tops , NULL );

    test_assert_int_equal( nx*ny*nz , ecl_grid_get_global_size( grid ));
    test_assert_int_equal( nx*ny*nz , ecl_grid_get_active_size( grid ));

    for (int k=0; k< nz; k++) {
      for (int j=0; j < ny; j++) {
        for (int i=0; i < nx; i++) {
          int g = k*nx*ny + j*nx + i;

          test_assert_double_equal( ecl_grid_get_cell_volume1( grid , g ) , dx[g] * dy[g] * dz[g]);
        }
      }
    }
    {
      double x,y,z;

      ecl_grid_get_xyz1(grid , 0 , &x , &y , &z);
      test_assert_double_equal( x , dx[0] * 0.5 );
      test_assert_double_equal( y , dy[0] * 0.5 );
      test_assert_double_equal( z , dz[0] * 0.5 + tops[0]);
    }
    ecl_grid_free( grid );
  }
  free( tops );
  free( dx );
  free( dy );
  free( dz );
}


void test_create1() {
  int nx = 10;
  int ny = 10;
  int nz = 10;
  double * dx   = (double *) util_calloc( nx*ny*nz , sizeof * dx );
  double * dy   = (double *) util_calloc( nx*ny*nz , sizeof * dy );
  double * dz   = (double *) util_calloc( nx*ny*nz , sizeof * dz );
  double * tops = (double *) util_calloc( nx*ny*nz , sizeof * tops );
  for (int k=0; k< nz; k++) {
    for (int j=0; j < ny; j++) {
      for (int i=0; i < nx; i++) {
        int g = k*nx*ny + j*nx + i;

        dx[g]   = 1;
        dy[g]   = 1;
        dz[g]   = 1;
        if (k == 0)
          tops[g] = 0;
        else
          tops[g] = tops[g - nx*ny] + dz[g];
      }
    }
  }

  {
    ecl_grid_type * grid = ecl_grid_alloc_dx_dy_dz_tops( nx , ny , nz , dx , dy , dz , tops , NULL );

    test_assert_int_equal( nx*ny*nz , ecl_grid_get_global_size( grid ));
    test_assert_int_equal( nx*ny*nz , ecl_grid_get_active_size( grid ));

    for (int k=0; k< nz; k++) {
      for (int j=0; j < ny; j++) {
        for (int i=0; i < nx; i++) {
          int g = k*nx*ny + j*nx + i;

          test_assert_double_equal( ecl_grid_get_cell_volume1( grid , g ) , dx[g] * dy[g] * dz[g]);

          {
            double x,y,z;
            ecl_grid_get_xyz1(grid , g , &x , &y , &z);
            test_assert_double_equal( x , i + 0.5);
            test_assert_double_equal( y , j + 0.5);
            test_assert_double_equal( z , k + 0.5);
          }
        }
      }
    }
    ecl_grid_free( grid );
  }
  free( tops );
  free( dx );
  free( dy );
  free( dz );
}



int main(int argc , char ** argv) {
  test_create1();
  test_create2();
}

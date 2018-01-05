/*
   Copyright (C) 2017 Statoil ASA, Norway.

   The file 'ecl_alloc_cpgrid.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/util/test_util.h>
#include <ert/ecl/ecl_type.h>


void test_grid(int nx, int ny, int nz) {
  ecl_kw_type * coord_kw = ecl_kw_alloc( COORD_KW , ECL_GRID_COORD_SIZE( nx , ny ) , ECL_FLOAT );
  ecl_kw_type * zcorn_kw = ecl_kw_alloc( ZCORN_KW , ECL_GRID_ZCORN_SIZE( nx , ny , nz) , ECL_FLOAT );
  int i,j,k;
  double a = 1.0;
  for (j= 0; j < ny; j++) {
    for (i = 0; i < nx; i++) {
      int offset = 6*(i + j*nx);
      ecl_kw_iset_float( coord_kw , offset    , a*i);
      ecl_kw_iset_float( coord_kw , offset + 1, a*j);
      ecl_kw_iset_float( coord_kw , offset + 2,  -1);

      ecl_kw_iset_float( coord_kw , offset + 3, a*i);
      ecl_kw_iset_float( coord_kw , offset + 4, a*j);
      ecl_kw_iset_float( coord_kw , offset + 5,  -1);

      for (k=0; k < nz; k++) {
        for (int c = 0; c < 4; c++) {
          int zi1 = ecl_grid_zcorn_index__( nx , ny , i , j , k , c);
          int zi2 = ecl_grid_zcorn_index__( nx , ny , i , j , k , c + 4);

          double z1 = k*a;
          double z2 = (k + 1) * a;

          ecl_kw_iset_float( zcorn_kw , zi1 , z1 );
          ecl_kw_iset_float( zcorn_kw , zi2 , z2 );
        }
      }
    }
  }


  {
    ecl_grid_type * grid = ecl_grid_alloc_GRDECL_kw( nx,ny,nz, zcorn_kw , coord_kw ,  NULL, NULL );
    test_assert_int_equal( ecl_grid_get_cell_twist3( grid , 0,0,0) , 0 );
    ecl_grid_free( grid );
  }


  {
    int zi1 = ecl_grid_zcorn_index__( nx , ny , 0 , 0 , 0 , 0);
    int zi2 = ecl_grid_zcorn_index__( nx , ny , 0 , 0 , 0 , 4);

    double z1 = 0;
    double z2 = -0.25;

    ecl_kw_iset_float( zcorn_kw , zi1 , z1 );
    ecl_kw_iset_float( zcorn_kw , zi2 , z2 );

    {
      ecl_grid_type * grid = ecl_grid_alloc_GRDECL_kw( nx,ny,nz, zcorn_kw , coord_kw ,  NULL, NULL );
      test_assert_int_equal( ecl_grid_get_cell_twist3( grid , 0,0,0) , 1 );
      ecl_grid_free( grid );
    }

    zi1 = ecl_grid_zcorn_index__( nx , ny , 0 , 0 , 0 , 3);
    zi2 = ecl_grid_zcorn_index__( nx , ny , 0 , 0 , 0 , 7);

    ecl_kw_iset_float( zcorn_kw , zi1 , z1 );
    ecl_kw_iset_float( zcorn_kw , zi2 , z2 );

    {
      ecl_grid_type * grid = ecl_grid_alloc_GRDECL_kw( nx,ny,nz, zcorn_kw , coord_kw ,  NULL, NULL );
      test_assert_int_equal( ecl_grid_get_cell_twist3( grid , 0,0,0) , 2 );
      ecl_grid_free( grid );
    }
  }


  ecl_kw_free( coord_kw );
  ecl_kw_free( zcorn_kw );
}


int main(int argc , char ** argv) {
  int nx = 10;
  int ny = 7;
  int nz = 8;

  test_grid( nx,ny,nz );

  exit(0);
}

/*
   Copyright (C) 2017  Statoil ASA, Norway.

   The file 'ecl_nnc_geometry.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>

#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_nnc_geometry.h>


void test_create_empty() {
  ecl_grid_type * grid = ecl_grid_alloc_rectangular(10,10,10,1,1,1,NULL);
  ecl_nnc_geometry_type * nnc_geo = ecl_nnc_geometry_alloc( grid );
  test_assert_true( ecl_nnc_geometry_is_instance( nnc_geo ));
  test_assert_int_equal( ecl_nnc_geometry_size( nnc_geo ) , 0 );
  ecl_nnc_geometry_free( nnc_geo );
  ecl_grid_free( grid );
}

void test_create_simple() {
  test_work_area_type * work_area = test_work_area_alloc("nnc-INIT");
  {
    int nx = 10;
    int ny = 10;
    int nz = 10;
    ecl_grid_type * grid0 = ecl_grid_alloc_rectangular(nx,ny,nz,1,1,1,NULL);

    ecl_grid_add_self_nnc(grid0, 0 ,nx*ny + 0, 0 );
    ecl_grid_add_self_nnc(grid0, 1 ,nx*ny + 1, 1 );
    ecl_grid_add_self_nnc(grid0, 2 ,nx*ny + 2, 2 );
    {
      ecl_nnc_geometry_type * nnc_geo = ecl_nnc_geometry_alloc( grid0 );
      test_assert_int_equal( ecl_nnc_geometry_size( nnc_geo ) , 3 );

      /*
        Create a dummy INIT file which *ony* contains a TRANNC keyword with the correct size.
      */
      {
        ecl_kw_type * trann_nnc = ecl_kw_alloc(TRANNNC_KW , ecl_nnc_geometry_size( nnc_geo ), ECL_FLOAT);
        fortio_type * f = fortio_open_writer( "TEST.INIT" , false, ECL_ENDIAN_FLIP );

        for (int i=0; i < ecl_kw_get_size( trann_nnc); i++)
          ecl_kw_iset_float( trann_nnc , i , i*1.0 );

        ecl_kw_fwrite( trann_nnc , f );
        fortio_fclose( f );
        ecl_kw_free( trann_nnc );
      }
    }
    ecl_grid_free( grid0 );
  }
  test_work_area_free( work_area );
}


int main(int argc , char ** argv) {
  util_install_signals( );
  test_create_empty( );
  test_create_simple( );
}

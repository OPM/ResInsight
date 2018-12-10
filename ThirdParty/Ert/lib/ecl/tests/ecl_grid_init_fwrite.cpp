/*
   Copyright (C) 2016  Statoil ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>

#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_endian_flip.hpp>



void test_write_depth(const ecl_grid_type * grid) {
  test_work_area_type * test_area = test_work_area_alloc("write_depth");
  {
    fortio_type * init_file = fortio_open_writer( "INIT" , false , ECL_ENDIAN_FLIP );
    ecl_grid_fwrite_depth( grid , init_file , ECL_METRIC_UNITS);
    fortio_fclose( init_file );
  }
  {
    ecl_file_type * init_file = ecl_file_open( "INIT" , 0 );
    ecl_kw_type * depth = ecl_file_iget_named_kw( init_file , "DEPTH" , 0 );

    test_assert_int_equal( ecl_kw_get_size( depth ) , ecl_grid_get_nactive(grid));
    for (int i=0; i < ecl_grid_get_nactive(grid); i++)
      test_assert_double_equal( ecl_kw_iget_as_double(depth , i) , ecl_grid_get_cdepth1A( grid , i ));

    ecl_file_close(init_file);
  }
  test_work_area_free( test_area );
}


void test_write_dims(const ecl_grid_type * grid) {
  test_work_area_type * test_area = test_work_area_alloc("write_dims");
  {
    fortio_type * init_file = fortio_open_writer( "INIT" , false , ECL_ENDIAN_FLIP );
    ecl_grid_fwrite_dims( grid , init_file , ECL_METRIC_UNITS );
    fortio_fclose( init_file );
  }
  {
    ecl_file_type * init_file = ecl_file_open( "INIT" , 0 );
    ecl_kw_type * DX = ecl_file_iget_named_kw( init_file , "DX" , 0 );
    ecl_kw_type * DY = ecl_file_iget_named_kw( init_file , "DY" , 0 );
    ecl_kw_type * DZ = ecl_file_iget_named_kw( init_file , "DZ" , 0 );

    test_assert_int_equal( ecl_kw_get_size( DX ) , ecl_grid_get_nactive(grid));
    test_assert_int_equal( ecl_kw_get_size( DY ) , ecl_grid_get_nactive(grid));
    test_assert_int_equal( ecl_kw_get_size( DZ ) , ecl_grid_get_nactive(grid));
    for (int i=0; i < ecl_grid_get_nactive(grid); i++) {
      test_assert_double_equal( ecl_kw_iget_as_double(DX , i) , ecl_grid_get_cell_dx1A( grid , i ));
      test_assert_double_equal( ecl_kw_iget_as_double(DY , i) , ecl_grid_get_cell_dy1A( grid , i ));
      test_assert_double_equal( ecl_kw_iget_as_double(DZ , i) , ecl_grid_get_cell_dz1A( grid , i ));
    }
    ecl_file_close(init_file);
  }
  test_work_area_free( test_area );
}




ecl_grid_type * create_grid( ) {
  int nx = 10;
  int ny = 10;
  int nz = 8;
  int_vector_type * actnum = int_vector_alloc( nx*ny*nz , 1 );

  ecl_grid_type * ecl_grid = ecl_grid_alloc_rectangular(nx, ny, nz, 1, 1, 1, int_vector_get_ptr( actnum ));
  int_vector_free( actnum );
  return ecl_grid;
}



int main( int argc , char **argv) {
  util_install_signals();
  {
    ecl_grid_type * grid = create_grid( );

    test_write_depth( grid );
    test_write_dims( grid );
    ecl_grid_free( grid );
  }
}

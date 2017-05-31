/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'ecl_init_file.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_init_file.h>
#include <ert/ecl/ecl_type.h>


void test_write_header() {
  int nx = 10;
  int ny = 10;
  int nz = 5;

  int_vector_type * actnum = int_vector_alloc( nx*ny*nz , 1 );
  test_work_area_type * test_area = test_work_area_alloc( "ecl_init_file" );
  time_t start_time = util_make_date_utc(15 , 12 , 2010 );
  ecl_grid_type * ecl_grid;

  int_vector_iset( actnum , 10 , 0 );
  int_vector_iset( actnum , 100 , 0 );

  ecl_grid = ecl_grid_alloc_rectangular(nx, ny, nz, 1, 1, 1, int_vector_get_ptr( actnum ));

  // Write poro with global size.
  {
    fortio_type * f = fortio_open_writer( "FOO1.INIT" , false , ECL_ENDIAN_FLIP );
    ecl_kw_type * poro = ecl_kw_alloc( "PORO" , ecl_grid_get_global_size( ecl_grid ) , ECL_FLOAT);
    ecl_kw_scalar_set_float( poro , 0.10 );
    ecl_init_file_fwrite_header( f , ecl_grid , poro , ECL_FIELD_UNITS, 7 , start_time );
    ecl_kw_free( poro );
    fortio_fclose( f );
  }


  // Write poro with nactive size.
  {
    fortio_type * f = fortio_open_writer( "FOO2.INIT" , false , ECL_ENDIAN_FLIP );
    ecl_kw_type * poro = ecl_kw_alloc( "PORO" , ecl_grid_get_global_size( ecl_grid ) , ECL_FLOAT);
    ecl_kw_scalar_set_float( poro , 0.10 );
    ecl_init_file_fwrite_header( f , ecl_grid , poro , ECL_FIELD_UNITS, 7 , start_time );
    ecl_kw_free( poro );
    fortio_fclose( f );
  }
  {
    ecl_file_type * file1 = ecl_file_open( "FOO1.INIT" , 0 );
    ecl_file_type * file2 = ecl_file_open( "FOO2.INIT" , 0 );

    test_assert_true( ecl_kw_equal( ecl_file_iget_named_kw( file1 , "PORV" , 0 ) ,
                                    ecl_file_iget_named_kw( file2 , "PORV" , 0)));

    ecl_file_close( file2 );
    ecl_file_close( file1 );
  }


  // Poro == NULL
  {
    fortio_type * f = fortio_open_writer( "FOO3.INIT" , false , ECL_ENDIAN_FLIP );
    ecl_init_file_fwrite_header( f , ecl_grid , NULL , ECL_METRIC_UNITS, 7 , start_time );
    fortio_fclose( f );
  }
  test_work_area_free( test_area );
}



int main( int argc , char ** argv) {
  test_write_header();
  exit(0);
}

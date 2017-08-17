/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ecl_grid_export.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>


void export_actnum( const ecl_grid_type * ecl_grid , ecl_file_type * ecl_file ) {
  ecl_kw_type * actnum_kw = ecl_file_iget_named_kw( ecl_file , "ACTNUM" , 0 );
  int * actnum = util_malloc( ecl_kw_get_size( actnum_kw ) * sizeof * actnum );

  ecl_grid_init_actnum_data( ecl_grid , actnum );
  for (int i=0; i < ecl_kw_get_size( actnum_kw); i++)
    test_assert_int_equal( actnum[i] , ecl_kw_iget_int( actnum_kw , i ));

  free( actnum );
}


void export_coord( const ecl_grid_type * grid , ecl_file_type * ecl_file ) {
  ecl_kw_type * coord_kw = ecl_file_iget_named_kw( ecl_file , "COORD" , 0);
  test_assert_int_equal( ecl_kw_get_size( coord_kw ) , ecl_grid_get_coord_size( grid ));
  {
    float * coord_float = util_malloc( ecl_grid_get_coord_size( grid ) * sizeof * coord_float );
    double * coord_double = util_malloc( ecl_grid_get_coord_size( grid ) * sizeof * coord_double );

    ecl_grid_init_coord_data( grid , coord_float );
    ecl_grid_init_coord_data_double( grid , coord_double );

    for (int i=0; i < ecl_grid_get_coord_size( grid ); i++)
      test_assert_double_equal( coord_double[i] , coord_float[i]);

    free( coord_float );
    free( coord_double );
  }
}


void export_zcorn( const ecl_grid_type * grid , ecl_file_type * ecl_file ) {
  ecl_kw_type * zcorn_kw = ecl_file_iget_named_kw( ecl_file , "ZCORN" , 0);
  test_assert_int_equal( ecl_kw_get_size( zcorn_kw ) , ecl_grid_get_zcorn_size( grid ));
  {
    float * zcorn_float = util_malloc( ecl_grid_get_zcorn_size( grid ) * sizeof * zcorn_float );
    double * zcorn_double = util_malloc( ecl_grid_get_zcorn_size( grid ) * sizeof * zcorn_double );

    ecl_grid_init_zcorn_data( grid , zcorn_float );
    ecl_grid_init_zcorn_data_double( grid , zcorn_double );

    for (int i=0; i < ecl_grid_get_zcorn_size( grid ); i++) {
      test_assert_double_equal( zcorn_double[i] , zcorn_float[i]);
      test_assert_float_equal( zcorn_float[i] , ecl_kw_iget_float( zcorn_kw , i ));
    }


    free( zcorn_float );
    free( zcorn_double );
  }
}


void copy_processed( const ecl_grid_type * src ) {
  {
    ecl_grid_type * copy = ecl_grid_alloc_processed_copy( src , NULL , NULL );
    test_assert_true( ecl_grid_compare(src, copy,true,true,false) );
    ecl_grid_free( copy );
  }


  {
    int * actnum = util_malloc( ecl_grid_get_global_size( src ) * sizeof * actnum );
    int index = 0;
    ecl_grid_init_actnum_data( src , actnum );

    while (true) {
      if (actnum[index] == 1) {
        actnum[index] = 0;
        break;
      }
      index++;
    }

    {
      ecl_grid_type * copy = ecl_grid_alloc_processed_copy( src , NULL , actnum );
      test_assert_int_equal( 1 , ecl_grid_get_active_size(src) - ecl_grid_get_active_size( copy ));
      ecl_grid_free( copy );
    }
    free( actnum );
  }


  {
    double * zcorn_double = util_malloc( ecl_grid_get_zcorn_size( src ) * sizeof * zcorn_double );
    int i = 0;
    int j = 0;
    int k = 0;

    ecl_grid_init_zcorn_data_double( src , zcorn_double );
    {
      ecl_grid_type * copy = ecl_grid_alloc_processed_copy( src , zcorn_double , NULL );
      test_assert_double_equal( ecl_grid_get_cell_volume3(src,i,j,k) , ecl_grid_get_cell_volume3( copy , i , j , k ));
      ecl_grid_free( copy );
    }

    
    for (int c = 0; c < 4; c++) {
      double dz = zcorn_double[ ecl_grid_zcorn_index( src , i , j , k , c + 4 ) ] - zcorn_double[ ecl_grid_zcorn_index( src , i , j , k , c ) ];
      zcorn_double[ ecl_grid_zcorn_index( src , i , j , k , c + 4 ) ] += dz;
    }
    {
      ecl_grid_type * copy = ecl_grid_alloc_processed_copy( src , zcorn_double , NULL );
      test_assert_double_equal( ecl_grid_get_cell_volume3(src,i,j,k) * 2 , ecl_grid_get_cell_volume3( copy , i , j , k ));
      ecl_grid_free( copy );
    }

    free( zcorn_double );
  }
}


void export_mapaxes( const ecl_grid_type * grid , ecl_file_type * ecl_file ) {
  if (ecl_file_has_kw(ecl_file , "MAPAXES")) {
    ecl_kw_type * mapaxes_kw = ecl_file_iget_named_kw( ecl_file , "MAPAXES" , 0);
    double mapaxes[6];
    int i;

    test_assert_true( ecl_grid_use_mapaxes( grid ));
    ecl_grid_init_mapaxes_data_double( grid , mapaxes );
    for (i= 0; i < 6; i++)
      test_assert_double_equal( ecl_kw_iget_float( mapaxes_kw , i) , mapaxes[i]);
  }
}



int main(int argc , char ** argv) {
  test_work_area_type * work_area = test_work_area_alloc("grid_export");
  {
    char * test_grid = "TEST.EGRID";
    char * grid_file;
    if (argc == 1) {
      ecl_grid_type * grid = ecl_grid_alloc_rectangular(4,4,2,1,1,1,NULL);
      grid_file = test_grid;
      ecl_grid_fwrite_EGRID( grid , grid_file , true );
      ecl_grid_free( grid );
    } else
      grid_file = argv[1];

    {
      ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_file );
      ecl_file_type * ecl_file = ecl_file_open( grid_file , 0) ;

      export_actnum( ecl_grid , ecl_file );
      export_coord( ecl_grid , ecl_file );
      export_zcorn( ecl_grid , ecl_file );
      export_mapaxes( ecl_grid , ecl_file );
      copy_processed( ecl_grid );
      ecl_file_close( ecl_file );
      ecl_grid_free( ecl_grid );
    }
  }
  test_work_area_free( work_area );
}

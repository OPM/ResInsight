/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ecl_coarse_test.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_coarse_cell.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_kw.hpp>



void test_coarse_cell(const ecl_grid_type * grid , ecl_coarse_cell_type * cell ) {
  const int_vector_type * global_index_list = ecl_coarse_cell_get_index_vector(cell );
  const int * ijk = ecl_coarse_cell_get_box_ptr( cell );
  int c;
  int prev_active = 0;

  for (c=0; c < ecl_coarse_cell_get_size( cell ); c++) {
    int gi = int_vector_iget( global_index_list , c );
    int i,j,k;

    /* The coordinates are right */
    ecl_grid_get_ijk1( grid , gi , &i , &j , &k);
    if ((i < ijk[0]) || (i > ijk[1]))
      test_error_exit("i:%d not inside range [%d,%d] \n",i , ijk[0] , ijk[1]);

    if ((j < ijk[2]) || (j > ijk[3]))
      test_error_exit("j:%d not inside range [%d,%d] \n",j , ijk[2] , ijk[3]);

    if ((k < ijk[4]) || (k > ijk[5]))
      test_error_exit("k:%d not inside range [%d,%d] \n",k , ijk[4] , ijk[4]);

    if (c == 0)
      prev_active = ecl_grid_get_active_index1( grid , gi );
    else {
      /* All the cells have the same active value */
      int this_active = ecl_grid_get_active_index1( grid , gi );
      test_assert_int_equal( prev_active , this_active );
      prev_active = this_active;
    }
  }
}




int main(int argc , char ** argv) {
  const char * case_path = argv[1];
  char * egrid_file  = ecl_util_alloc_filename( NULL , case_path , ECL_EGRID_FILE , false , 0 );
  char * rst_file    = ecl_util_alloc_filename( NULL , case_path , ECL_RESTART_FILE , false , 0 );
  char * init_file   = ecl_util_alloc_filename( NULL , case_path , ECL_INIT_FILE , false , 0 );

  ecl_grid_type * GRID      = ecl_grid_alloc(egrid_file );
  ecl_file_type * RST_file  = ecl_file_open( rst_file , 0);
  ecl_file_type * INIT_file = ecl_file_open( init_file , 0);

  {
    test_assert_true( ecl_grid_have_coarse_cells( GRID ) );
    test_assert_int_equal( ecl_grid_get_num_coarse_groups( GRID ) , 3384);
  }

  {
    const ecl_kw_type * swat0 = ecl_file_iget_named_kw( RST_file , "SWAT" , 0 );
    const ecl_kw_type * porv  = ecl_file_iget_named_kw( INIT_file , "PORV" , 0 );

    test_assert_int_equal( ecl_kw_get_size( swat0 ) , ecl_grid_get_active_size( GRID ) );
    test_assert_int_equal( ecl_kw_get_size( porv )  , ecl_grid_get_global_size( GRID ) );
  }

  {
    int ic;
    for (ic = 0; ic < ecl_grid_get_num_coarse_groups(GRID); ic++) {
      ecl_coarse_cell_type * coarse_cell = ecl_grid_iget_coarse_group( GRID , ic );
      test_coarse_cell( GRID , coarse_cell );
    }
  }



  ecl_file_close( INIT_file );
  ecl_file_close( RST_file );
  ecl_grid_free( GRID );
  exit(0);
}

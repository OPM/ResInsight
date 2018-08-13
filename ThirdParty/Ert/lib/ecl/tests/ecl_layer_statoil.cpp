/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ecl_layer_statoil.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/layer.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_kw.hpp>


ecl_kw_type * alloc_faultblock_kw( const char * filename, int grid_size) {
  FILE * stream = util_fopen( filename , "r");
  ecl_kw_type * kw = ecl_kw_fscanf_alloc_grdecl( stream , "FAULTBLK" , grid_size , ECL_INT );
  fclose( stream );

  return kw;
}


void test_layer( const ecl_grid_type * ecl_grid , const ecl_kw_type * faultblock_kw , int k) {
  int nx = ecl_grid_get_nx(ecl_grid );
  int ny = ecl_grid_get_ny(ecl_grid );
  layer_type * layer = layer_alloc( nx , ny );
  int i,j;

  for (j=0; j < ny; j++)
    for (i=0; i < nx; i++) {
      int g = ecl_grid_get_global_index3( ecl_grid , i , j , k );
      int fblk = ecl_kw_iget_int( faultblock_kw , g );
      layer_iset_cell_value( layer , i , j , fblk );
    }

  {
    struct_vector_type * corner_list = struct_vector_alloc( sizeof(int_point2d_type) );
    int_vector_type * i_list = int_vector_alloc(0,0);
    int_vector_type * j_list = int_vector_alloc(0,0);
    int_vector_type * cell_list = int_vector_alloc(0,0);

    for (j=0; j < ny; j++) {
      for (i=0; i < nx; i++) {
        int cell_value = layer_iget_cell_value( layer , i , j );
        if (cell_value != 0) {
          test_assert_true( layer_trace_block_edge( layer , i , j , cell_value , corner_list , cell_list));
          test_assert_true( layer_trace_block_content( layer  , true , i,j,cell_value , i_list , j_list ));
        }
      }
    }
    test_assert_int_equal( 0 , layer_get_cell_sum( layer ));
    struct_vector_free( corner_list );
    int_vector_free( i_list );
    int_vector_free( j_list );
    int_vector_free( cell_list );
  }

  layer_free( layer );
}




int main(int argc , char ** argv) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc( argv[1] );
  ecl_kw_type * faultblock_kw = alloc_faultblock_kw( argv[2] , ecl_grid_get_global_size( ecl_grid ));
  int k;

  for (k=0; k < ecl_grid_get_nz( ecl_grid ); k++)
    test_layer( ecl_grid , faultblock_kw , k );


  ecl_kw_free( faultblock_kw );
  ecl_grid_free( ecl_grid );
  exit(0);
}

/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ecl_fault_block_collection_statoil.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <unistd.h>

#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/fault_block_collection.hpp>



void test_create( const ecl_grid_type * grid , const ecl_kw_type * fault_block_kw) {
  fault_block_collection_type * fault_blocks = fault_block_collection_alloc( grid  );
  test_assert_true( fault_block_collection_is_instance( fault_blocks ));
  test_assert_int_equal( ecl_grid_get_nz( grid ) , fault_block_collection_num_layers( fault_blocks ));
  fault_block_collection_free( fault_blocks );
}



void test_get_layer( const ecl_grid_type * grid , const ecl_kw_type * fault_block_kw) {
  fault_block_collection_type * fault_blocks = fault_block_collection_alloc( grid );

  test_assert_NULL( fault_block_collection_get_layer( fault_blocks , -1 ));
  test_assert_NULL( fault_block_collection_get_layer( fault_blocks , ecl_grid_get_nz( grid ) ));
  {
    int k;
    for (k = 0; k < ecl_grid_get_nz( grid ); k++) {
      fault_block_layer_type * layer = fault_block_collection_get_layer( fault_blocks  , k );
      test_assert_true( fault_block_layer_is_instance( layer ));
    }
  }
  fault_block_collection_free( fault_blocks );
}






int main(int argc , char ** argv) {
  const char * grid_file = argv[1];
  const char * fault_blk_file = argv[2];
  ecl_grid_type * ecl_grid = ecl_grid_alloc( grid_file );
  ecl_kw_type * fault_blk_kw;
  {
    FILE * stream = util_fopen( fault_blk_file , "r");
    fault_blk_kw = ecl_kw_fscanf_alloc_grdecl( stream , "FAULTBLK" , ecl_grid_get_global_size( ecl_grid ) , ECL_INT_TYPE);
    fclose( stream );
  }


  test_create( ecl_grid , fault_blk_kw );
  test_get_layer( ecl_grid , fault_blk_kw );


  ecl_grid_free( ecl_grid );
  ecl_kw_free( fault_blk_kw );
  exit(0);
}

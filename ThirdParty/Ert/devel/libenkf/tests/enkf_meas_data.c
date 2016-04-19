/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'enkf_meas_data.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdio.h>
#include <unistd.h>

#include "ert/util/build_config.h"

#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>
#include <ert/util/int_vector.h>
#include <ert/util/type_vector_functions.h>

#include <ert/enkf/meas_data.h>



void meas_block_iset_abort(void * arg) {
  meas_block_type * block = meas_block_safe_cast( arg );
  meas_block_iset( block , 0 , 0 , 100);
}


void meas_block_iget_abort(void * arg) {
  meas_block_type * block = meas_block_safe_cast( arg );
  meas_block_iget( block , 0 , 0 );
}



void create_test() {
  int_vector_type * ens_active_list = int_vector_alloc(0 , false);
  bool_vector_type * ens_mask;
  int_vector_append( ens_active_list , 10 );
  int_vector_append( ens_active_list , 20 );
  int_vector_append( ens_active_list , 30 );

  ens_mask = int_vector_alloc_mask(ens_active_list);
  {
    meas_data_type * meas_data = meas_data_alloc( ens_mask );
    test_assert_int_equal( 3 , meas_data_get_active_ens_size( meas_data ));

    {
      meas_block_type * block = meas_data_add_block(meas_data , "OBS" , 10 , 10);

      meas_block_iset(block , 10 , 0 , 100);
      test_assert_double_equal( 100 , meas_block_iget( block , 10 , 0 ));

      test_assert_bool_equal( true  , meas_block_iens_active( block , 10 ));
      test_assert_bool_equal( false , meas_block_iens_active( block , 11 ));

      test_assert_util_abort( "meas_block_assert_iens_active" ,  meas_block_iset_abort , block);
      test_assert_util_abort( "meas_block_assert_iens_active" ,  meas_block_iget_abort , block);
    }
    meas_data_free( meas_data );
  }


  bool_vector_free( ens_mask );
  int_vector_free( ens_active_list );
}



int main(int argc , char ** argv) {
  create_test();
  exit(0);
}


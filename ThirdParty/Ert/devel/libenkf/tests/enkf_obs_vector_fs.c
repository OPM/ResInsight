/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_obs_vector_fs.c' is part of ERT - Ensemble based
   Reservoir Tool.

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
#include <stdio.h>

#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/ert_test_context.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/obs_vector.h>

#include <ert/util/test_util.h>
#include <ert/util/type_vector_functions.h>


void test_valid_obs_vector( enkf_main_type * enkf_main , const char * obs_key) {
  enkf_fs_type * fs = enkf_main_get_fs(enkf_main);
  enkf_obs_type * enkf_obs = enkf_main_get_obs( enkf_main );
  obs_vector_type * obs_vector = enkf_obs_get_vector( enkf_obs , obs_key );
  bool_vector_type * active_mask = bool_vector_alloc( enkf_main_get_ensemble_size( enkf_main ) , true);

  test_assert_true( obs_vector_has_data( obs_vector , active_mask , fs ));
  bool_vector_free( active_mask );
}


/*
  This test will modify the enkf_obs container with invalid data; must
  be the last test.
*/

void test_invalid_obs_vector( enkf_main_type * enkf_main , const char * obs_key) {
  enkf_fs_type * fs = enkf_main_get_fs(enkf_main);
  enkf_obs_type * enkf_obs = enkf_main_get_obs( enkf_main );
  obs_vector_type * obs_vector = enkf_obs_get_vector( enkf_obs , obs_key );
  bool_vector_type * active_mask = bool_vector_alloc( enkf_main_get_ensemble_size( enkf_main ) , true);

  test_assert_false( obs_vector_has_data( obs_vector , active_mask , fs ));
  bool_vector_free( active_mask );
}


void test_container( ert_test_context_type * test_context ) {
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  enkf_obs_type * enkf_obs = enkf_main_get_obs( enkf_main );
  obs_vector_type * rft_obs = enkf_obs_get_vector( enkf_obs , "RFT_TEST");
  enkf_fs_type * fs = enkf_main_get_fs( enkf_main );
  bool_vector_type * active_mask = bool_vector_alloc( enkf_main_get_ensemble_size( enkf_main ) , true );

  test_assert_true( obs_vector_has_data( rft_obs , active_mask , fs ));
  bool_vector_free( active_mask );
}



void test_measure( ert_test_context_type * test_context ) {
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  enkf_fs_type * fs = enkf_main_get_fs( enkf_main );
  enkf_obs_type * enkf_obs = enkf_main_get_obs( enkf_main );
  obs_vector_type * rft_obs = enkf_obs_get_vector( enkf_obs , "RFT_TEST");
  int_vector_type * ens_active_list = int_vector_alloc(0,0);
  active_list_type * active_list = active_list_alloc( );
  meas_data_type * meas_data_RFT;

  for (int i=0; i < enkf_main_get_ensemble_size( enkf_main ); i++)
    int_vector_append( ens_active_list , i );

  {
    bool_vector_type * ens_mask;
    ens_mask = int_vector_alloc_mask( ens_active_list );
    meas_data_RFT = meas_data_alloc( ens_mask );
    bool_vector_free( ens_mask );
  }

  obs_vector_measure( rft_obs , fs , 20 , ens_active_list , meas_data_RFT , active_list );

  int_vector_free( ens_active_list );
  active_list_free( active_list );
  meas_data_free( meas_data_RFT );
}




int main(int argc , char ** argv) {
  const char * config_file = argv[1];
  ert_test_context_type * context = ert_test_context_alloc( "OBS_VECTOR_FS" , config_file);
  enkf_main_type * enkf_main = ert_test_context_get_main( context );

  {
    test_valid_obs_vector( enkf_main , "WWCT:OP_3");
    test_container( context );
    test_measure( context );
    test_invalid_obs_vector( enkf_main , "GOPT:OP");
  }
  ert_test_context_free( context );
}


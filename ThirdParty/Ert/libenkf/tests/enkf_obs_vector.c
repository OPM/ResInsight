/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'enkf_obs_vector.c' is part of ERT - Ensemble based
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

#include <ert/util/test_util.h>
#include <ert/util/type_vector_functions.h>

#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/ert_test_context.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/obs_vector.h>
#include <ert/enkf/summary_obs.h>



void test_create(enkf_config_node_type * config_node ) {
  obs_vector_type * obs_vector = obs_vector_alloc( SUMMARY_OBS , "OBS" , config_node , 100 );
  test_assert_true( obs_vector_is_instance( obs_vector ));
  {
    const int_vector_type * step_list = obs_vector_get_step_list( obs_vector );

    {
      summary_obs_type * obs_node = summary_obs_alloc( "FOPT" , "FOPT" , 10 , 1 , NULL , 0);
      obs_vector_install_node( obs_vector , 10 , obs_node );
      test_assert_int_equal( 1 , int_vector_size( step_list ));
      test_assert_int_equal( 10 , int_vector_iget( step_list , 0));
    }

    {
      summary_obs_type * obs_node = summary_obs_alloc( "FOPT" , "FOPT" , 10 , 1 , NULL , 0);
      obs_vector_install_node( obs_vector , 10 , obs_node );
      test_assert_int_equal( 1 , int_vector_size( step_list ));
      test_assert_int_equal( 10 , int_vector_iget( step_list , 0));
    }

    {
      summary_obs_type * obs_node = summary_obs_alloc( "FOPT" , "FOPT" , 10 , 1 , NULL , 0);
      obs_vector_install_node( obs_vector , 5 , obs_node );
      test_assert_int_equal( 2 , int_vector_size( step_list ));
      test_assert_int_equal( 5 , int_vector_iget( step_list , 0));
      test_assert_int_equal( 10 , int_vector_iget( step_list , 1));
    }

    {
      summary_obs_type * obs_node = summary_obs_alloc( "FOPT" , "FOPT" , 10 , 1 , NULL , 0);
      obs_vector_install_node( obs_vector , 15 , obs_node );
      test_assert_int_equal( 3 , int_vector_size( step_list ));
      test_assert_int_equal( 5 , int_vector_iget( step_list , 0));
      test_assert_int_equal( 10 , int_vector_iget( step_list , 1));
      test_assert_int_equal( 15 , int_vector_iget( step_list , 2));
    }
  }
  obs_vector_free( obs_vector );
}



int main(int argc , char ** argv) {
  enkf_config_node_type * config_node = enkf_config_node_alloc_summary("FOPR" , LOAD_FAIL_EXIT);
  {
    test_create( config_node );
  }
  enkf_config_node_free( config_node );
}


/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'enkf_plot_gendata_fs.c' is part of ERT - Ensemble based Reservoir Tool.
    
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

#include <ert/util/test_util.h>

#include <ert/ecl/ecl_grid.h>

#include <ert/enkf/block_obs.h>
#include <ert/enkf/ert_test_context.h>
#include <ert/enkf/enkf_plot_gendata.h>
#include <ert/enkf/gen_obs.h>




void test_gendata( enkf_main_type * enkf_main , const char * obs_key , int report_step ) {
  enkf_obs_type * enkf_obs = enkf_main_get_obs( enkf_main );

  obs_vector_type * obs_vector = enkf_obs_get_vector( enkf_obs , obs_key);

  {
    enkf_plot_gendata_type * gen_data = enkf_plot_gendata_alloc_from_obs_vector( obs_vector );

    enkf_fs_type * fs = enkf_main_get_fs( enkf_main );
    gen_obs_type * gen_obs = obs_vector_iget_node( obs_vector , report_step );

    {

        double  value;
        double  std;
        bool valid;
        gen_obs_user_get_with_data_index(gen_obs , "0" , &value , &std , &valid );
        test_assert_double_equal( 0.143841 , value );
        test_assert_double_equal( 0.0300 ,  std );
        test_assert_true( valid );

    }

    enkf_plot_gendata_load(gen_data, fs, report_step, NULL);

    test_assert_int_equal( enkf_main_get_ensemble_size( enkf_main ) , enkf_plot_gendata_get_size( gen_data ));

    {
      enkf_plot_genvector_type * vector = enkf_plot_gendata_iget( gen_data , 24);
      test_assert_true( enkf_plot_genvector_is_instance( vector ));
      test_assert_double_equal(  0.675537 , enkf_plot_genvector_iget( vector , 0 ));
      test_assert_double_equal( 0.682635 , enkf_plot_genvector_iget( vector , 1 ));
      test_assert_double_equal( 0.616371 , enkf_plot_genvector_iget( vector , 2 ));


    }

    {
      enkf_plot_genvector_type * vector = enkf_plot_gendata_iget( gen_data , 9 );
      test_assert_true( enkf_plot_genvector_is_instance( vector ));
      test_assert_double_equal( -0.515033 , enkf_plot_genvector_iget( vector , 0 ));
      test_assert_double_equal( -0.507350 , enkf_plot_genvector_iget( vector , 1 ));
      test_assert_double_equal( -0.541030 , enkf_plot_genvector_iget( vector , 2 ));
    }


    enkf_plot_gendata_free( gen_data );
  }


}



int main( int argc , char ** argv) {
  const char * config_file = argv[1];
  util_install_signals();
  ert_test_context_type * test_context = ert_test_context_alloc("GENDATA" , config_file );
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  
  test_gendata( enkf_main , "GEN_TIMESHIFT" , 60);

  ert_test_context_free( test_context );
  exit(0);
}

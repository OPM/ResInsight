/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'enkf_plot_data_fs.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdbool.h>

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>

#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_plot_tvector.h>



void test_load_GEN_KW( enkf_main_type * enkf_main , const char * key , const char * index_key) {
  ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config( enkf_main );
  const enkf_config_node_type * config_node = ensemble_config_get_node( ensemble_config , key );
  enkf_plot_data_type * plot_data = enkf_plot_data_alloc( config_node );

  {
    enkf_fs_type * enkf_fs = enkf_main_mount_alt_fs( enkf_main , "enkf" , true );

    enkf_plot_data_load( plot_data , enkf_fs , index_key ,  NULL );
    test_assert_int_equal( 25 , enkf_plot_data_get_size( plot_data ));
    {
      enkf_plot_tvector_type * plot_vector = enkf_plot_data_iget( plot_data , 10 );
      test_assert_true( enkf_plot_tvector_is_instance( plot_vector ));
      test_assert_int_equal( 63 , enkf_plot_tvector_size( plot_vector   ));

      test_assert_true( enkf_plot_tvector_iget_active( plot_vector ,  0 ));
      test_assert_true( enkf_plot_tvector_iget_active( plot_vector , 10 ));
      test_assert_true( enkf_plot_tvector_iget_active( plot_vector , 20 ));
      test_assert_true( enkf_plot_tvector_iget_active( plot_vector , 30 ));

      test_assert_false( enkf_plot_tvector_iget_active( plot_vector ,  1 ));
      test_assert_false( enkf_plot_tvector_iget_active( plot_vector , 11 ));
      test_assert_false( enkf_plot_tvector_iget_active( plot_vector , 21 ));
      test_assert_false( enkf_plot_tvector_iget_active( plot_vector , 31 ));
    }
    enkf_fs_decref( enkf_fs );
  }
  enkf_plot_data_free( plot_data );
}



void test_load_summary( enkf_main_type * enkf_main , const char * summary_key) {
  ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config( enkf_main );
  const enkf_config_node_type * config_node = ensemble_config_get_node( ensemble_config , summary_key );
  enkf_plot_data_type * plot_data = enkf_plot_data_alloc( config_node );

  {
    enkf_fs_type * enkf_fs = enkf_main_mount_alt_fs( enkf_main , "enkf" , true );
    enkf_plot_data_load( plot_data , enkf_fs , NULL , NULL );
    test_assert_int_equal( 25 , enkf_plot_data_get_size( plot_data ));
    {
      enkf_plot_tvector_type * plot_vector = enkf_plot_data_iget( plot_data , 10 );
      test_assert_true( enkf_plot_tvector_is_instance( plot_vector ));
      test_assert_false( enkf_plot_tvector_iget_active( plot_vector , 0 ));
      test_assert_int_equal( 63 , enkf_plot_tvector_size( plot_vector ));
    }
    enkf_fs_decref( enkf_fs );
  }

  {
    enkf_fs_type * enkf_fs = enkf_main_mount_alt_fs( enkf_main , "default" , true  );
    enkf_plot_data_load( plot_data , enkf_fs , NULL ,  NULL );
    test_assert_int_equal( 25 , enkf_plot_data_get_size( plot_data ));
    {
      enkf_plot_tvector_type * plot_vector = enkf_plot_data_iget( plot_data , 0 );
      test_assert_true( enkf_plot_tvector_is_instance( plot_vector ));
      test_assert_false( enkf_plot_tvector_iget_active( plot_vector , 0 ));
      test_assert_int_equal( 63 , enkf_plot_tvector_size( plot_vector ));

      plot_vector = enkf_plot_data_iget( plot_data , 1 );
      test_assert_true( enkf_plot_tvector_is_instance( plot_vector ));
      test_assert_int_equal( 0 , enkf_plot_tvector_size( plot_vector ));
    }
    enkf_fs_decref( enkf_fs );
  }
  enkf_plot_data_free( plot_data );
}





int main(int argc, char ** argv) {
  util_install_signals();
  {
    const char * config_file = argv[1];
    test_work_area_type * work_area = test_work_area_alloc( "enkf_main_fs" );
    char * model_config;
    util_alloc_file_components( config_file , NULL , &model_config , NULL);
    test_work_area_set_store( work_area , true );
    test_work_area_copy_parent_content( work_area , config_file );
    {
      enkf_main_type * enkf_main = enkf_main_bootstrap( model_config , false , false );

      test_load_summary(enkf_main , "WWCT:OP_3");
      test_load_GEN_KW( enkf_main , "MULTFLT" , "F3");
      enkf_main_free( enkf_main );
    }
    test_work_area_free( work_area );
    exit(0);
  }
}

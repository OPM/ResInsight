/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_forward_init_GEN_KW.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>

#include <ert/enkf/enkf_main.h>


void create_runpath(enkf_main_type * enkf_main ) {
  const int ens_size         = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive = bool_vector_alloc(0,false);

  state_enum init_state    = ANALYZED; 
  int start_report         = 0;
  int init_step_parameters = 0;
  bool_vector_iset( iactive , ens_size - 1 , true );
  enkf_main_run_exp(enkf_main , iactive , false , init_step_parameters , start_report , init_state, true);
  bool_vector_free(iactive);
}



int main(int argc , char ** argv) {
  enkf_main_install_SIGNALS();
  const char * root_path = argv[1];
  const char * config_file = argv[2];
  const char * forward_init_string = argv[3];
  test_work_area_type * work_area = test_work_area_alloc(config_file , false);
  test_work_area_copy_directory_content( work_area , root_path );
  {  
    bool forward_init;
    bool strict = true;
    enkf_main_type * enkf_main;
    
    test_assert_true( util_sscanf_bool( forward_init_string , &forward_init));
    
    util_clear_directory( "Storage" , true , true );
    enkf_main = enkf_main_bootstrap( NULL , config_file , strict , true );
    {
      enkf_state_type * state   = enkf_main_iget_state( enkf_main , 0 );
      enkf_node_type * gen_kw_node = enkf_state_get_node( state , "MULTFLT" );
      {
        const enkf_config_node_type * gen_kw_config_node = enkf_node_get_config( gen_kw_node );
        char * init_file1 = enkf_config_node_alloc_initfile( gen_kw_config_node , NULL , 0);
        char * init_file2 = enkf_config_node_alloc_initfile( gen_kw_config_node , "/tmp", 0);
        
        test_assert_bool_equal( enkf_config_node_use_forward_init( gen_kw_config_node ) , forward_init );
        test_assert_string_equal( init_file1 , "MULTFLT_INIT");
        test_assert_string_equal( init_file2 , "/tmp/MULTFLT_INIT");
        
        free( init_file1 );
        free( init_file2 );
      }
      
      test_assert_bool_equal( enkf_node_use_forward_init( gen_kw_node ) , forward_init );
      if (forward_init)
        test_assert_bool_not_equal( enkf_node_initialize( gen_kw_node , 0 , enkf_state_get_rng( state )) , forward_init);
      // else hard_failure()
    }
    test_assert_bool_equal( forward_init, ensemble_config_have_forward_init( enkf_main_get_ensemble_config( enkf_main )));
    
    if (forward_init) {
      enkf_state_type * state   = enkf_main_iget_state( enkf_main , 0 );
      enkf_fs_type * fs = enkf_main_get_fs( enkf_main );
      enkf_node_type * gen_kw_node = enkf_state_get_node( state , "MULTFLT" );
      node_id_type node_id = {.report_step = 0 ,  
                              .iens = 0,
                              .state = ANALYZED };
      
      create_runpath( enkf_main );
      test_assert_true( util_is_directory( "simulations/run0" ));
      
      {
        int error = 0;
        stringlist_type * msg_list = stringlist_alloc_new();
        
        {
          run_mode_type run_mode = ENSEMBLE_EXPERIMENT; 
          enkf_main_init_run(enkf_main , run_mode);     /* This is ugly */
        }
        
        
        test_assert_false( enkf_node_has_data( gen_kw_node , fs, node_id ));
        util_unlink_existing( "simulations/run0/MULTFLT_INIT" );
        

        test_assert_false( enkf_node_forward_init( gen_kw_node , "simulations/run0" , 0 ));
        enkf_state_forward_init( state , fs , &error );
        test_assert_true(LOAD_FAILURE & error);
        
        error = 0;
        {
          enkf_fs_type * fs = enkf_main_get_fs( enkf_main );
          state_map_type * state_map = enkf_fs_get_state_map(fs);
          state_map_iset(state_map , 0 , STATE_INITIALIZED);
        }
        enkf_state_load_from_forward_model( state , fs , &error , false , msg_list );
        stringlist_free( msg_list );
        test_assert_true(LOAD_FAILURE & error);
      }
      
      
      
      {
        FILE * stream = util_fopen("simulations/run0/MULTFLT_INIT" , "w");
        fprintf(stream , "123456.0\n" );
        fclose( stream );
      }
      
      {
        int error = 0;
        stringlist_type * msg_list = stringlist_alloc_new();

        {
          run_mode_type run_mode = ENSEMBLE_EXPERIMENT; 
          enkf_main_init_run(enkf_main , run_mode);     /* This is ugly */
        }
        

        test_assert_true( enkf_node_forward_init( gen_kw_node , "simulations/run0" , 0 ));
        enkf_state_forward_init( state , fs , &error );
        test_assert_int_equal(0, error);
        enkf_state_load_from_forward_model( state , fs , &error , false , msg_list );
       
        stringlist_free( msg_list );
        test_assert_int_equal(0, error);

        {
          double value;
          test_assert_true( enkf_node_user_get( gen_kw_node , fs , "MULTFLT" , node_id , &value)); 
          test_assert_double_equal( 123456.0 , value);
        }
      }
      util_clear_directory( "simulations" , true , true );
      create_runpath( enkf_main );
      test_assert_true( util_is_directory( "simulations/run0" ));
      test_assert_true( util_is_file( "simulations/run0/MULTFLT.INC" ));
      {
        FILE * stream = util_fopen("simulations/run0/MULTFLT.INC" , "r");
        double value;
        fscanf(stream , "%lg" , &value);
        fclose( stream );
        test_assert_double_equal( 123456.0 , value);
      }
      util_clear_directory( "simulations" , true , true );
    }
    enkf_main_free( enkf_main );
  }
  test_work_area_free( work_area );
}

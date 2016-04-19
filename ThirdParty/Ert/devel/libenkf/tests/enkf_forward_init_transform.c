/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_forward_init_transform.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/ecl/ecl_kw_magic.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/run_arg.h>
#include <ert/enkf/enkf_config_node.h>






void create_runpath(enkf_main_type * enkf_main ) {
  const int ens_size         = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive = bool_vector_alloc(0,false);

  bool_vector_iset( iactive , ens_size - 1 , true );
  enkf_main_run_exp(enkf_main , iactive , false );
  bool_vector_free(iactive);
}


bool check_original_exported_data_equal(const enkf_node_type * field_node) {
  FILE * original_stream = util_fopen( "petro.grdecl" , "r");
  ecl_kw_type * kw_original = ecl_kw_fscanf_alloc_grdecl_dynamic( original_stream , "PORO" , ECL_DOUBLE_TYPE );

  enkf_node_ecl_write(field_node, "tmp", NULL, 0); 
  FILE * exported_stream = util_fopen( "tmp/PORO.grdecl" , "r");
  ecl_kw_type * kw_exported = ecl_kw_fscanf_alloc_grdecl_dynamic( exported_stream , "PORO" , ECL_DOUBLE_TYPE );

  bool ret = ecl_kw_numeric_equal(kw_original, kw_exported, 1e-5 , 1e-5);

  util_fclose(original_stream); 
  util_fclose(exported_stream); 
  ecl_kw_free(kw_original); 
  ecl_kw_free(kw_exported); 
  
  return ret; 
}


int main(int argc , char ** argv) {
  enkf_main_install_SIGNALS();
  const char * root_path   = argv[1];
  const char * config_file = argv[2];
  const char * init_file   = argv[3];
  const char * forward_init_string = argv[4];
  
  test_work_area_type * work_area = test_work_area_alloc(config_file );
  test_work_area_copy_directory_content( work_area , root_path );
  test_work_area_install_file( work_area , init_file );
  test_work_area_set_store(work_area, true); 
  
  bool strict = true;
  enkf_main_type * enkf_main = enkf_main_bootstrap( config_file , strict , true );
  enkf_fs_type * init_fs = enkf_main_get_fs(enkf_main);
  enkf_state_type * state = enkf_main_iget_state( enkf_main , 0 );
  run_arg_type * run_arg = run_arg_alloc_ENSEMBLE_EXPERIMENT( init_fs , 0 ,0 , "simulations/run0");
  enkf_node_type * field_node = enkf_state_get_node( state , "PORO" );
  
  bool forward_init;
  test_assert_true( util_sscanf_bool( forward_init_string , &forward_init));
  test_assert_bool_equal( enkf_node_use_forward_init( field_node ) , forward_init );
  test_assert_bool_equal( forward_init, ensemble_config_have_forward_init( enkf_main_get_ensemble_config( enkf_main )));
  
  util_clear_directory( "Storage" , true , true );
  
  create_runpath( enkf_main );
  test_assert_true( util_is_directory( "simulations/run0" ));

  if (forward_init)
    util_copy_file( init_file , "simulations/run0/petro.grdecl");

  {
    bool_vector_type * iactive = bool_vector_alloc( enkf_main_get_ensemble_size(enkf_main) , true);
    int error;
    stringlist_type * msg_list = stringlist_alloc_new();  
    error = enkf_state_load_from_forward_model( state , run_arg ,  msg_list );
    stringlist_free( msg_list );
    bool_vector_free( iactive );
    test_assert_int_equal(error, 0); 
  }

  test_assert_true(check_original_exported_data_equal(field_node));

  run_arg_free( run_arg );
  enkf_main_free(enkf_main);    
  test_work_area_free(work_area); 
}


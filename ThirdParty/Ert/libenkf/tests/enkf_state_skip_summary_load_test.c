/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_state_no_summary_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/run_arg.h>


bool check_ecl_sum_loaded(const enkf_main_type * enkf_main)
{
  enkf_fs_type    * fs       = enkf_main_get_fs( enkf_main );
  stringlist_type * msg_list = stringlist_alloc_new();
  enkf_state_type * state1   = enkf_main_iget_state( enkf_main , 0 );
  run_arg_type * run_arg1    = run_arg_alloc_ENSEMBLE_EXPERIMENT( fs , 0 , 0 , "simulations/run0");
  enkf_state_type * state2   = enkf_main_iget_state( enkf_main , 1 );
  run_arg_type * run_arg2    = run_arg_alloc_ENSEMBLE_EXPERIMENT( fs , 0 , 0 , "simulations/run1");
  

  state_map_type * state_map = enkf_fs_get_state_map(fs);
  state_map_iset(state_map, 0, STATE_INITIALIZED);
  
  int error = enkf_state_load_from_forward_model( state1 , run_arg1 ,  msg_list );

  
  state_map_iset(state_map, 1, STATE_INITIALIZED);
  error = enkf_state_load_from_forward_model( state2 , run_arg2 , msg_list );
  
  stringlist_free( msg_list );
  return (0 == error); 
}



int main(int argc , char ** argv) {
  enkf_main_install_SIGNALS();
  const char * root_path      = argv[1];
  const char * config_file    = argv[2];
      
  test_work_area_type * work_area = test_work_area_alloc(config_file );
  test_work_area_copy_directory_content( work_area , root_path );
  
  bool strict = true;
  enkf_main_type * enkf_main = enkf_main_bootstrap( config_file , strict , true );
  
  test_assert_true( check_ecl_sum_loaded(enkf_main) );
  
  enkf_main_free( enkf_main );
  test_work_area_free(work_area); 
}

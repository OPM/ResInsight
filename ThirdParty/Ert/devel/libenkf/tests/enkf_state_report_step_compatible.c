/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_state_report_step_compatible.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


bool check_ecl_sum_compatible(const enkf_main_type * enkf_main)
{
  stringlist_type * msg_list = stringlist_alloc_new();
  enkf_state_type * state    = enkf_main_iget_state( enkf_main , 0 );
  enkf_fs_type    * fs       = enkf_main_get_fs( enkf_main );
  
  state_map_type * state_map = enkf_fs_get_state_map(fs);
  state_map_iset(state_map, 0, STATE_INITIALIZED);
  
  int error = 0; 

  enkf_state_load_from_forward_model( state , fs , &error , false , msg_list );
  
  stringlist_free( msg_list );
  return (REPORT_STEP_INCOMPATIBLE & error) ? false : true; 
}



int main(int argc , char ** argv) {
  enkf_main_install_SIGNALS();
  const char * root_path      = argv[1];
  const char * config_file    = argv[2];
  const char * compatible_str = argv[3]; 
  bool check_compatible; 
  
  test_assert_true( util_sscanf_bool( compatible_str , &check_compatible));
  
  test_work_area_type * work_area = test_work_area_alloc(config_file );
  test_work_area_copy_directory_content( work_area , root_path );
  
  bool strict = true;
  enkf_main_type * enkf_main = enkf_main_bootstrap( NULL , config_file , strict , true );
  
  {
    run_mode_type run_mode = ENSEMBLE_EXPERIMENT; 
    enkf_main_init_run(enkf_main , run_mode);     /* This is ugly */
    
    enkf_state_type * state = enkf_main_iget_state( enkf_main , 0 );
    bool active = true;
    int max_internal_sumbit = 1; 
    int init_step_parameter = 1; 
    state_enum init_state_parameter = FORECAST; 
    state_enum init_state_dynamic = FORECAST; 
    int load_start = 1; 
    int step1 = 1; 
    int step2 = 1; 
    
    enkf_state_init_run(state, run_mode, active, max_internal_sumbit, init_step_parameter, init_state_parameter, init_state_dynamic, load_start, step1, step2); 
  }

  test_assert_bool_equal(check_compatible, check_ecl_sum_compatible(enkf_main));
  
  enkf_main_free( enkf_main );
  test_work_area_free(work_area); 
}

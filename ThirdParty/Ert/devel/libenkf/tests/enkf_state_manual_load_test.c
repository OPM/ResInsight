/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_state_manual_load_test.c' is part of ERT - Ensemble based Reservoir Tool.
    
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


int test_load_manually_to_new_case(enkf_main_type * enkf_main) {
    const char * casename = "new_case";
    enkf_main_select_fs( enkf_main , casename );

    enkf_fs_type * fs = enkf_main_get_fs( enkf_main );
    int result = 0;
    int step1 = 1;
    int step2 = 1;

    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , enkf_main_iget_state(enkf_main, 0));                /* 0: */
    arg_pack_append_ptr( arg_pack , fs );                                               /* 1: */
    arg_pack_append_int( arg_pack , step1 );                                            /* 2: This will be the load start parameter for the run_info struct. */
    arg_pack_append_int( arg_pack , step1 );                                            /* 3: Step1 */
    arg_pack_append_int( arg_pack , step2 );                                            /* 4: Step2 For summary data it will load the whole goddamn thing anyway.*/
    arg_pack_append_bool( arg_pack , true );                                            /* 5: Interactive */
    arg_pack_append_owned_ptr( arg_pack , stringlist_alloc_new() , stringlist_free__);  /* 6: List of interactive mode messages. */
    arg_pack_append_bool( arg_pack, true );                                             /* 7: Manual load */
    arg_pack_append_int( arg_pack , 0 );                                                /* 8: iter */
    arg_pack_append_ptr( arg_pack, &result );                                           /* 9: Result */
    
    enkf_state_load_from_forward_model_mt(arg_pack);

    arg_pack_free(arg_pack);
    return result;
}



void initialize(enkf_main_type * enkf_main) {

  run_mode_type run_mode = ENSEMBLE_EXPERIMENT;
  bool_vector_type * iactive = bool_vector_alloc( enkf_main_get_ensemble_size(enkf_main) , true);
  enkf_main_init_run(enkf_main , iactive , run_mode , INIT_NONE);     /* This is ugly */

  int step1 = 1;
  int step2 = 1;
  enkf_state_type * state = enkf_main_iget_state( enkf_main , 0 );
  bool active = true;
  int max_internal_sumbit = 1;
  int init_step_parameter = 1;
  state_enum init_state_parameter = FORECAST;
  state_enum init_state_dynamic = FORECAST;
  int load_start = 1;

  enkf_state_init_run(state, run_mode, active, max_internal_sumbit, init_step_parameter, init_state_parameter, init_state_dynamic, load_start, 0, step1, step2);
  
  bool_vector_free( iactive );
}



int main(int argc , char ** argv) {
  enkf_main_install_SIGNALS();
  const char * root_path   = argv[1];
  const char * config_file = argv[2];

  test_work_area_type * work_area = test_work_area_alloc(config_file);
  test_work_area_copy_directory_content( work_area , root_path );

  bool strict = true;
  enkf_main_type * enkf_main = enkf_main_bootstrap( NULL , config_file , strict , true );

  initialize(enkf_main);

  test_assert_int_equal(test_load_manually_to_new_case(enkf_main), 0);

  enkf_main_free( enkf_main );
  test_work_area_free(work_area);

  exit(0);
}


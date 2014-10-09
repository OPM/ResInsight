/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_state_runpath_test.c' is part of ERT - Ensemble based Reservoir Tool.
    
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



int main(int argc , char ** argv) {

  enkf_main_install_SIGNALS();
  const char * root_path = argv[1];
  const char * config_file_single_run = argv[2];
  const char * config_file_multiple_iter = argv[3];

  test_work_area_type * work_area = test_work_area_alloc("enkf_state_runpath_test");
  test_work_area_copy_directory_content( work_area , root_path );
  test_work_area_set_store(work_area, true);


  {
    enkf_state_type * state = NULL;
    run_mode_type run_mode = ENSEMBLE_EXPERIMENT;
    bool strict = true;

    bool active = true;
    int max_internal_sumbit = 1;
    int init_step_parameter = 1;
    state_enum init_state_parameter = FORECAST;
    state_enum init_state_dynamic = FORECAST;
    int load_start = 1;
    int step1 = 1;
    int step2 = 1;

    const char * cwd = test_work_area_get_cwd(work_area);

    {
      enkf_main_type * enkf_main_single_run = enkf_main_bootstrap( NULL , config_file_single_run , strict , true );
      bool_vector_type * iactive = bool_vector_alloc( enkf_main_get_ensemble_size( enkf_main_single_run ) , true );
      enkf_main_init_run(enkf_main_single_run , iactive , run_mode , INIT_NONE);     /* This is ugly */

      state = enkf_main_iget_state( enkf_main_single_run , 0 );
      enkf_state_init_run(state, run_mode, active, max_internal_sumbit, init_step_parameter, init_state_parameter, init_state_dynamic, load_start, 0, step1, step2);
      char * path_single_run0 = util_alloc_sprintf("%s/sim/run0", cwd);
      test_assert_string_equal(enkf_state_get_run_path(state), path_single_run0);

      state = enkf_main_iget_state( enkf_main_single_run, 1);
      enkf_state_init_run(state, run_mode, active, max_internal_sumbit, init_step_parameter, init_state_parameter, init_state_dynamic, load_start, 0, step1, step2);
      char * path_single_run1 = util_alloc_sprintf("%s/sim/run1", cwd);
      test_assert_string_equal(enkf_state_get_run_path(state), path_single_run1);

      free(path_single_run0);
      free(path_single_run1);
      bool_vector_free( iactive );
      enkf_main_free(enkf_main_single_run);
    }


    {
      enkf_main_type * enkf_main_iter = enkf_main_bootstrap( NULL , config_file_multiple_iter , strict , true );
      bool_vector_type * iactive = bool_vector_alloc( enkf_main_get_ensemble_size( enkf_main_iter ) , true );
      enkf_main_init_run(enkf_main_iter , iactive , run_mode , INIT_NONE);     /* This is ugly */
      state = enkf_main_iget_state( enkf_main_iter , 0 );

      enkf_state_init_run(state, run_mode, active, max_internal_sumbit, init_step_parameter, init_state_parameter, init_state_dynamic, load_start, 0, step1, step2);
      char * path_iter0_run0  = util_alloc_sprintf("%s/sim/run0/iter0", cwd);
      test_assert_string_equal(enkf_state_get_run_path(state), path_iter0_run0);

      state = enkf_main_iget_state( enkf_main_iter , 1 );
      enkf_state_init_run(state, run_mode, active, max_internal_sumbit, init_step_parameter, init_state_parameter, init_state_dynamic, load_start, 1, step1, step2);
      char * path_iter1_run1  = util_alloc_sprintf("%s/sim/run1/iter1", cwd);
      test_assert_string_equal(enkf_state_get_run_path(state), path_iter1_run1);

      free(path_iter0_run0);
      free(path_iter1_run1);
      bool_vector_free( iactive );
      enkf_main_free(enkf_main_iter);
    }
  }


  test_work_area_free(work_area);
  exit(0);
}

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
#include <ert/enkf/run_arg.h>


int test_load_manually_to_new_case(enkf_main_type * enkf_main) {
  int result = 0;
  int iens = 0;
  int iter = 0;
  const char * casename = "new_case";
  enkf_main_select_fs( enkf_main , casename );


  enkf_fs_type * fs = enkf_main_get_fs( enkf_main );
  run_arg_type * run_arg = run_arg_alloc_ENSEMBLE_EXPERIMENT(fs , iens , iter , "simulations/run0");
  {
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , enkf_main_iget_state(enkf_main, 0));
    arg_pack_append_ptr( arg_pack , run_arg );
    arg_pack_append_owned_ptr( arg_pack , stringlist_alloc_new() , stringlist_free__);
    arg_pack_append_bool( arg_pack, true );
    arg_pack_append_ptr( arg_pack, &result );

    enkf_state_load_from_forward_model_mt(arg_pack);
    arg_pack_free(arg_pack);
  }

  return result;
}




int main(int argc , char ** argv) {
  enkf_main_install_SIGNALS();
  const char * root_path   = argv[1];
  const char * config_file = argv[2];

  test_work_area_type * work_area = test_work_area_alloc(config_file);
  test_work_area_copy_directory_content( work_area , root_path );
  {
    bool strict = true;
    enkf_main_type * enkf_main = enkf_main_bootstrap( config_file , strict , true );

    test_assert_int_equal( 0 , test_load_manually_to_new_case(enkf_main));

    enkf_main_free( enkf_main );
  }
  test_work_area_free(work_area);

  exit(0);
}


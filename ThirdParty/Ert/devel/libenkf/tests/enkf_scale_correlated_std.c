/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'enkf_scale_correlated_std.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/enkf/ert_test_context.h>

#include <ert/util/util.h>
#include <ert/util/string_util.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_main_jobs.h>




void test_scaling( ert_test_context_type * test_context , int nobs, const char** obs_keys ) {
  stringlist_type * args = stringlist_alloc_new();
  for (int iobs=0; iobs < nobs; iobs++)
    stringlist_append_ref( args , obs_keys[iobs]);

  test_assert_true( ert_test_context_run_worklow_job( test_context , "STD_SCALE" , args) );
  stringlist_free( args );
}



int main(int argc , const char ** argv) {
  const char * config_file = argv[1];
  const char * workflow_job_file = argv[2];
  enkf_main_install_SIGNALS();
  {
    ert_test_context_type * test_context = ert_test_context_alloc("std_scale_test" , config_file);

    ert_test_context_install_workflow_job( test_context , "STD_SCALE" , workflow_job_file );
    test_scaling(test_context , 1 , ( const char *[1] ) {"WWCT:OP_1"});
    test_scaling(test_context , 2 , ( const char *[2] ) {"WWCT:OP_1", "WWCT:OP_2"});
    test_scaling(test_context , 8 , ( const char *[8] ) {"RPR2_1", "RPR2_2","RPR2_3","RPR2_4","RPR2_5","RPR2_6","RPR2_7","RPR2_8"});

    ert_test_context_free( test_context );
  }
  exit(0);
}

/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_magic_string_in_workflows.c' is part of ERT - Ensemble based Reservoir Tool.

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


void test_magic_strings( ert_test_context_type * test_context ) {
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  test_assert_true( ert_test_context_run_worklow( test_context , "MAGIC_PRINT") );
  test_assert_true( util_file_exists( "magic-list.txt") );

  {
    FILE * stream = util_fopen("magic-list.txt" , "r");
    char string[128];

    fscanf( stream , "%s" , string);
    test_assert_string_equal( string , enkf_fs_get_case_name( enkf_main_get_fs( enkf_main )));

    fscanf( stream , "%s" , string);
    test_assert_string_equal( string , "MagicAllTheWayToWorkFlow");

    fclose( stream );
  }
}


void test_has_job(ert_test_context_type * test_context ) {
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  ert_workflow_list_type * workflows = enkf_main_get_workflow_list( enkf_main );
  test_assert_true( ert_workflow_list_has_job( workflows , "MAGIC_PRINT" ));
}


int main( int argc , char ** argv) {
  const char * model_config = argv[1];
  ert_test_context_type * test_context = ert_test_context_alloc( "MAGIC-STRINGS" , model_config);
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );

  {
    test_has_job( test_context );

    enkf_main_select_fs(enkf_main , "default");
    test_assert_string_equal( "default" , enkf_fs_get_case_name( enkf_main_get_fs( enkf_main )));
    test_magic_strings( test_context );

    enkf_main_select_fs(enkf_main , "extraCase");
    test_assert_string_equal( "extraCase" , enkf_fs_get_case_name( enkf_main_get_fs( enkf_main )));
    test_magic_strings( test_context );
  }
  ert_test_context_free( test_context );
}

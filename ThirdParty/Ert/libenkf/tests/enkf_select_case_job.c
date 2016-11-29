/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_select_case_job.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/string_util.h>
#include <ert/util/bool_vector.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_main_jobs.h>
#include <ert/enkf/ert_test_context.h>


int main(int argc , const char ** argv) {
  enkf_main_install_SIGNALS();
  
  const char * config_file  = argv[1];
  const char * select_case_job = argv[2];

  ert_test_context_type * test_context = ert_test_context_alloc("SELECT_CASE" , config_file );
  
  test_assert_true( ert_test_context_install_workflow_job( test_context , "SELECT_CASE" , select_case_job));
  {
    enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
    stringlist_type * args = stringlist_alloc_new();
    stringlist_append_copy( args , "OtherCase");
    
    test_assert_string_not_equal( "OtherCase" , enkf_main_get_current_fs( enkf_main ));
    ert_test_context_run_worklow_job( test_context , "SELECT_CASE" , args);
    test_assert_true( ert_test_context_run_worklow_job( test_context , "SELECT_CASE" , args) );
    test_assert_string_equal( "OtherCase" , enkf_main_get_current_fs( enkf_main ));
    
    stringlist_free( args );
  }
  ert_test_context_free( test_context );
  exit(0);
}

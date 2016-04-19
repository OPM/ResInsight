/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_analysis_update_job.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/ui_return.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/ert_test_context.h>


void test_update_default(const char * config_file , const char * job_file) {
  ert_test_context_type * test_context = ert_test_context_alloc("AnalysisJob0" , config_file);

  stringlist_type * args = stringlist_alloc_new();
  test_assert_true( ert_test_context_install_workflow_job( test_context , "JOB" , job_file ));
  test_assert_true( ert_test_context_run_worklow_job( test_context , "JOB" , args) );
  stringlist_free( args );

  ert_test_context_free( test_context );
}


void test_update_new_case(const char * config_file , const char * job_file) {
  ert_test_context_type * test_context = ert_test_context_alloc("AnalysisJob1" , config_file);

  stringlist_type * args = stringlist_alloc_new();
  stringlist_append_copy( args , "NewCase" );
  ert_test_context_install_workflow_job( test_context , "JOB" , job_file );
  test_assert_true( ert_test_context_run_worklow_job( test_context , "JOB" , args) );
  stringlist_free( args );
  
  ert_test_context_free( test_context );
}


void test_update_new_case_step(const char * config_file , const char * job_file) {
  ert_test_context_type * test_context = ert_test_context_alloc("AnalysisJob2" , config_file);

  stringlist_type * args = stringlist_alloc_new();
  stringlist_append_copy( args , "NewCase" );
  stringlist_append_copy( args , "20" );
  ert_test_context_install_workflow_job( test_context , "JOB" , job_file );
  test_assert_true( ert_test_context_run_worklow_job( test_context , "JOB" , args) );
  stringlist_free( args );
  
  ert_test_context_free( test_context );
}


void test_update_new_case_step_selected(const char * config_file , const char * job_file) {
  ert_test_context_type * test_context = ert_test_context_alloc("AnalysisJob2" , config_file );

  stringlist_type * args = stringlist_alloc_new();
  stringlist_append_copy( args , "NewCase" );
  stringlist_append_copy( args , "20" );
  stringlist_append_copy( args , "10" );
  stringlist_append_copy( args , ",20" );
  stringlist_append_copy( args , ",30-50" );
  ert_test_context_install_workflow_job( test_context , "JOB" , job_file );
  test_assert_true( ert_test_context_run_worklow_job( test_context , "JOB" , args) );
  stringlist_free( args );
  
  ert_test_context_free( test_context );
}



int main(int argc , char ** argv) {
  const char * config_file = argv[1];
  const char * job_file = argv[2];
  
  test_update_default( config_file , job_file);
  test_update_new_case( config_file , job_file );
  test_update_new_case_step( config_file , job_file );
  test_update_new_case_step_selected( config_file , job_file );

  exit(0);
}


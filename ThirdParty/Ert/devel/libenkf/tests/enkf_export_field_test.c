/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_export_field_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/enkf/ert_test_context.h>

#include <ert/util/util.h>
#include <ert/util/string_util.h>



void test_export_field(ert_test_context_type * test_context , const char * job_name , const char * job_file) {

  test_assert_true( ert_test_context_install_workflow_job( test_context , job_name , job_file ));
  {
    stringlist_type * args = stringlist_alloc_new();
    
    stringlist_append_copy(args, "PERMZ");
    stringlist_append_copy(args, "TEST_EXPORT/test_export_field/PermZ%d.grdecl");
    stringlist_append_copy(args, "0");
    stringlist_append_copy(args, "FORECAST");
    stringlist_append_copy(args, "0, 2");
    
    test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );
    stringlist_free( args );
  }
  test_assert_true( util_file_exists("TEST_EXPORT/test_export_field/PermZ0.grdecl") );
  test_assert_true( util_file_exists("TEST_EXPORT/test_export_field/PermZ2.grdecl") );
}


void job_file_export_field_ecl_grdecl(ert_test_context_type * test_context , const char * job_name , const char * job_file) {
  ert_test_context_install_workflow_job( test_context , job_name , job_file );
  {
    stringlist_type * args = stringlist_alloc_new();
    
    stringlist_append_copy(args, "PERMX");
    stringlist_append_copy(args, "TEST_EXPORT/test_export_field_ecl_grdecl/PermX%d.grdecl");
    stringlist_append_copy(args, "0");
    stringlist_append_copy(args, "ANALYZED");
    test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );
    
    stringlist_clear(args);
    stringlist_append_copy(args, "PERMZ");
    stringlist_append_copy(args, "TEST_EXPORT/test_export_field_ecl_grdecl/PermZ%d");
    stringlist_append_copy(args, "0");
    stringlist_append_copy(args, "FORECAST");
    stringlist_append_copy(args, "0-1");
    test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );
    
    
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_ecl_grdecl/PermX0.grdecl"));
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_ecl_grdecl/PermX1.grdecl"));
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_ecl_grdecl/PermX2.grdecl"));
    
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_ecl_grdecl/PermZ0"));
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_ecl_grdecl/PermZ1"));
    
    stringlist_free( args );
  }
}


void job_file_export_field_rms_roff(ert_test_context_type * test_context , const char * job_name , const char * job_file) {
  test_assert_true( ert_test_context_install_workflow_job( test_context , job_name , job_file ) );
  {
    stringlist_type * args = stringlist_alloc_new();
    
    stringlist_append_copy(args, "PERMZ");
    stringlist_append_copy(args, "TEST_EXPORT/test_export_field_rms_roff/PermZ%d");
    stringlist_append_copy(args, "0");
    stringlist_append_copy(args, "ANALYZED");
    test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );
    
    stringlist_clear(args);
    stringlist_append_copy(args, "PERMX");
    stringlist_append_copy(args, "TEST_EXPORT/test_export_field_rms_roff/PermX%d.roff");
    stringlist_append_copy(args, "0");
    stringlist_append_copy(args, "FORECAST");
    test_assert_true( ert_test_context_run_worklow_job( test_context , job_name , args) );
    
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_rms_roff/PermZ0"));
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_rms_roff/PermZ1"));
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_rms_roff/PermZ2"));
    
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_rms_roff/PermX0.roff"));
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_rms_roff/PermX1.roff"));
    test_assert_true(util_file_exists("TEST_EXPORT/test_export_field_rms_roff/PermX2.roff"));
    
    stringlist_free( args );
  }
}



int main(int argc , const char ** argv) {
  enkf_main_install_SIGNALS();

  const char * config_file                      = argv[1];
  const char * job_file_export_field            = argv[2];
  const char * job_file_export_field_ecl_grdecl = argv[3];
  const char * job_file_export_field_rms_roff   = argv[4];

  ert_test_context_type * test_context = ert_test_context_alloc("ExportFieldsJobs" , config_file);
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );

  enkf_main_select_fs( enkf_main , "default" );
  {
    test_export_field(test_context, "JOB1" , job_file_export_field);
    test_export_field(test_context, "JOB2" , job_file_export_field_ecl_grdecl);
    test_export_field(test_context, "JOB3" , job_file_export_field_rms_roff);
  }
  ert_test_context_free( test_context );
  exit(0);
}

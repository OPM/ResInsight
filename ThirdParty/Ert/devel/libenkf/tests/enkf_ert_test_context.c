/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'enkf_ert_test_context.c' is part of ERT - Ensemble based
   Reservoir Tool.
    
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




#include <ert/util/test_util.h>
#include <ert/enkf/ert_test_context.h>




void test_create_invalid(const char * config_file) {
  char * cwd0 = util_alloc_cwd();
  ert_test_context_type * test_context = ert_test_context_alloc("CREATE_CONTEXT" , config_file );
  test_assert_true( ert_test_context_is_instance( test_context ));
  test_assert_NULL( ert_test_context_get_main( test_context ));
  {
    char * cwd1 = util_alloc_cwd();
    test_assert_string_equal(cwd1 , cwd0);
    free( cwd1 );
  }
  free( cwd0 );
  ert_test_context_free( test_context );
}



void test_create_valid( const char * config_file ) {
  char * cwd0 = util_alloc_cwd();
  ert_test_context_type * test_context = ert_test_context_alloc("CREATE_CONTEXT" , config_file );
  test_assert_true( ert_test_context_is_instance( test_context ));
  test_assert_true( enkf_main_is_instance( ert_test_context_get_main( test_context )));
  {
    char * cwd1 = util_alloc_cwd();
    test_assert_string_not_equal(cwd1 , cwd0);
    free( cwd1 );
  }
  free( cwd0 );
  ert_test_context_free( test_context );
}



void test_install_job( const char * config_file, const char * job_file_OK , const char * job_file_ERROR) {
  ert_test_context_type * test_context = ert_test_context_alloc("CREATE_CONTEXT_JOB" , config_file );

  test_assert_false( ert_test_context_install_workflow_job( test_context , "JOB" , "File/does/not/exist"));
  test_assert_false( ert_test_context_install_workflow_job( test_context , "ERROR" , job_file_ERROR));
  test_assert_true( ert_test_context_install_workflow_job( test_context , "OK" , job_file_OK));
  
  ert_test_context_free( test_context );
}



void test_run_workflow_job( const char * config_file , const char * job_file ) {
  ert_test_context_type * test_context = ert_test_context_alloc("CREATE_CONTEXT_JOB" , config_file  );
  stringlist_type * args0 = stringlist_alloc_new( );
  stringlist_type * args1 = stringlist_alloc_new( );

  stringlist_append_ref( args1 , "NewCase");
  test_assert_false( ert_test_context_run_worklow_job( test_context , "NO-this-does-not-exist" , args1));
  ert_test_context_install_workflow_job( test_context , "JOB" , job_file );
  
  test_assert_false( ert_test_context_run_worklow_job( test_context , "JOB" , args0));
  test_assert_true( ert_test_context_run_worklow_job( test_context , "JOB" , args1));
  
  stringlist_free( args0 );
  stringlist_free( args1 );
  ert_test_context_free( test_context );
}


void test_install_workflow( const char * config_file , const char * job_file ) {
  ert_test_context_type * test_context = ert_test_context_alloc("INSTALL_WORKFLOW" , config_file  );
  const char * wf_file = "WFLOW";

  ert_test_context_install_workflow_job( test_context , "JOB" , job_file );
  {
    FILE * stream = util_fopen( wf_file , "w");
    stringlist_type * args = stringlist_alloc_new( );
    stringlist_append_ref( args , "NewCase");
    ert_test_context_fwrite_workflow_job( stream , "JOB" , args);
    stringlist_free( args );
    fclose( stream );
  }
  test_assert_true( ert_test_context_install_workflow( test_context , "WFLOW" , wf_file ));
  ert_test_context_free( test_context );
}


void test_run_workflow(const char * config_file , const char * job_file) {
  ert_test_context_type * test_context = ert_test_context_alloc("INSTALL_WORKFLOW" , config_file );
  test_assert_false( ert_test_context_run_worklow( test_context , "No-does.not.exist"));
  
  ert_test_context_install_workflow_job( test_context , "JOB" , job_file );
  {
    FILE * stream1 = util_fopen( "WFLOW1", "w");
    FILE * stream2 = util_fopen( "WFLOW2", "w");
    stringlist_type * args = stringlist_alloc_new( );
    ert_test_context_fwrite_workflow_job( stream1 , "JOB" , args);
    stringlist_append_ref( args , "NewCase");
    ert_test_context_fwrite_workflow_job( stream2 , "JOB" , args);
        
    stringlist_free( args );
    fclose( stream1 );
    fclose( stream2 );
  }
  test_assert_true( ert_test_context_install_workflow( test_context , "WFLOW1" , "WFLOW1"));
  test_assert_true( ert_test_context_install_workflow( test_context , "WFLOW2" , "WFLOW2"));
  
  test_assert_true( ert_test_context_run_worklow( test_context , "WFLOW2"));
  test_assert_false( ert_test_context_run_worklow( test_context , "WFLOW1"));

  ert_test_context_free( test_context );
}






int main( int argc , char ** argv) {
  char * config_file = argv[1];
  char * wf_job_fileOK = argv[2];
  char * wf_job_fileERROR = argv[3];

  test_create_invalid( "DoesNotExist" );
  test_create_valid( config_file );
  test_install_job( config_file , wf_job_fileOK, wf_job_fileERROR );
  test_install_workflow( config_file , wf_job_fileOK);
  test_run_workflow( config_file , wf_job_fileOK);
  test_run_workflow_job( config_file , wf_job_fileOK);
}



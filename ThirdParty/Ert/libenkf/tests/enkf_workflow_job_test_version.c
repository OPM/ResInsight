/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'enkf_workflow_job_test_version.c' is part of ERT -
   Ensemble based Reservoir Tool.

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

#include <ert/util/ert_version.h>
#include <ert/util/test_util.h>
#include <ert/enkf/ert_test_context.h>

/*
  These version functions are overrideed here to get a consistent
  version mapping.
*/

int version_get_major_ert_version( ) {
  return 1;
}


int version_get_minor_ert_version( ) {
  return 2;
}


const char* version_get_micro_ert_version( ) {
  return "3";
}


void test_version() {
  test_assert_int_equal( version_get_major_ert_version( ) , 1 );
  test_assert_int_equal( version_get_minor_ert_version( ) , 2 );
  test_assert_string_equal( version_get_micro_ert_version( ) , "3" );
}


int main(int argc , const char ** argv) {
  enkf_main_install_SIGNALS();
  test_version( );
  {
    const char * path = argv[1];
    ert_workflow_list_type * workflows = ert_workflow_list_alloc( NULL );
    ert_workflow_list_add_jobs_in_directory( workflows , path );
    
    // The CONF1 only exists as default - unversioned
    test_assert_true( ert_workflow_list_has_job( workflows , "CONF1"));
    
    // The CONF2 exists as the default - which is invalid and will not load,
    // and CONF2@1 - which should load.
    test_assert_false( ert_workflow_list_has_job( workflows , "CONF2@1"));
    test_assert_true( ert_workflow_list_has_job( workflows , "CONF2"));

    // The CONF3 only exists as a fully versioned CONF3@1.2.3 - which should load.
    test_assert_true( ert_workflow_list_has_job( workflows , "CONF3"));

    // The CONF4 only exists as a fully versioned CONF4@1.2.0 - which should not load.
    test_assert_false( ert_workflow_list_has_job( workflows , "CONF4"));

    // The CONF5 exists as a fully versioned CONF5@1.2.0 - which should not load and
    // CONF@1.2 which should load.
    test_assert_true( ert_workflow_list_has_job( workflows , "CONF5"));

    ert_workflow_list_free( workflows );
  }
  exit(0);
}

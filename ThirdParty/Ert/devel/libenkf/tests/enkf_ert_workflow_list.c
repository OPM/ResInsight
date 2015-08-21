/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_ert_workflow_list.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/enkf/ert_workflow_list.h>



void test_create_workflow_list() {
   ert_workflow_list_type * wf_list = ert_workflow_list_alloc( NULL );
   test_assert_true( ert_workflow_list_is_instance( wf_list ));
   ert_workflow_list_free( wf_list );
}



void test_add_alias( const char * job) {
  test_work_area_type * work_area = test_work_area_alloc( "workflow_list/alias" );
  ert_workflow_list_type * wf_list = ert_workflow_list_alloc( NULL );
  ert_workflow_list_add_job( wf_list , "JOB" , job );

  {
    FILE * stream = util_fopen("WF1" , "w");
    fprintf(stream , "SCALE_STD 0.25\n");
    fclose(stream);
  }


  {
    FILE * stream = util_fopen("WF2" , "w");
    fprintf(stream , "SCALE_STD 0.25\n");
    fclose(stream);
  }

  test_assert_true( workflow_is_instance( ert_workflow_list_add_workflow( wf_list , "WF1" , "WF")));
  test_assert_int_equal( 1 , ert_workflow_list_get_size( wf_list ));
  test_assert_false( ert_workflow_list_has_workflow( wf_list , "WF1"));
  test_assert_true( ert_workflow_list_has_workflow( wf_list , "WF"));

  ert_workflow_list_add_alias( wf_list , "WF" , "alias");
  test_assert_int_equal( 2 , ert_workflow_list_get_size( wf_list ));
  test_assert_true( ert_workflow_list_has_workflow( wf_list , "WF"));
  test_assert_true( ert_workflow_list_has_workflow( wf_list , "alias"));
  test_assert_true( workflow_is_instance( ert_workflow_list_get_workflow( wf_list , "WF")));
  test_assert_true( workflow_is_instance( ert_workflow_list_get_workflow( wf_list , "alias")));

  test_assert_true( workflow_is_instance( ert_workflow_list_add_workflow( wf_list , "WF2" , "WF")));
  test_assert_int_equal( 2 , ert_workflow_list_get_size( wf_list ));
  test_assert_true( ert_workflow_list_has_workflow( wf_list , "WF"));
  test_assert_true( ert_workflow_list_has_workflow( wf_list , "alias"));
  test_assert_true( workflow_is_instance( ert_workflow_list_get_workflow( wf_list , "WF")));
  test_assert_true( workflow_is_instance( ert_workflow_list_get_workflow( wf_list , "alias")));

  test_work_area_free( work_area );
}


int main(int argc , char ** argv) {
  const char * job = argv[1];
  test_create_workflow_list();
  test_add_alias(job);
  exit(0);
}


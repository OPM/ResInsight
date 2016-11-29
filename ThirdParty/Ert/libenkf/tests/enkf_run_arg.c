/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ert_run_context.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include "ert/util/build_config.h"

#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>
#include <ert/util/path_fmt.h>
#include <ert/util/subst_list.h>
#include <ert/util/test_work_area.h>

#include <ert/enkf/ert_run_context.h>
#include <ert/enkf/run_arg.h>
#include <ert/enkf/enkf_fs.h>


void call_get_queue_index( void * arg ) {
  run_arg_type * run_arg = run_arg_safe_cast( arg );
  run_arg_get_queue_index( run_arg );
}

void call_set_queue_index( void * arg ) {
  run_arg_type * run_arg = run_arg_safe_cast( arg );
  run_arg_set_queue_index( run_arg , 88 );
}


void test_queue_index() {
  test_work_area_type * test_area = test_work_area_alloc("run_arg/ENS");
  {
    enkf_fs_type * fs   = enkf_fs_create_fs("sim" , BLOCK_FS_DRIVER_ID , NULL , true);
    run_arg_type * run_arg = run_arg_alloc_ENSEMBLE_EXPERIMENT(fs , 0 , 6 , "path");

    test_assert_false( run_arg_is_submitted( run_arg ) );
    test_assert_util_abort("run_arg_get_queue_index" , call_get_queue_index , run_arg );

    run_arg_set_queue_index(run_arg, 78);
    test_assert_true( run_arg_is_submitted( run_arg ) );
    test_assert_int_equal( 78 , run_arg_get_queue_index( run_arg ));

    test_assert_util_abort("run_arg_set_queue_index" , call_set_queue_index , run_arg );
    run_arg_free( run_arg );
    enkf_fs_decref( fs );
  }
  test_work_area_free( test_area );
}

void call_get_result_fs( void * arg ) {
  run_arg_type * run_arg = run_arg_safe_cast( arg );
  run_arg_get_result_fs( run_arg );
}


void call_get_update_target_fs( void * arg ) {
  run_arg_type * run_arg = run_arg_safe_cast( arg );
  run_arg_get_update_target_fs( run_arg );
}



void test_SMOOTHER_RUN( ) {
  test_work_area_type * test_area = test_work_area_alloc("run_arg/SMOOTHER");
  {
    enkf_fs_type * sim_fs    = enkf_fs_create_fs("sim" , BLOCK_FS_DRIVER_ID , NULL , true);
    enkf_fs_type * target_fs = enkf_fs_create_fs("target" , BLOCK_FS_DRIVER_ID , NULL , true);
    run_arg_type * run_arg = run_arg_alloc_SMOOTHER_RUN(sim_fs , target_fs , 0 , 6 , "path");
    test_assert_true( run_arg_is_instance( run_arg ));
    test_assert_ptr_equal( run_arg_get_init_fs( run_arg ) , sim_fs );
    test_assert_ptr_equal( run_arg_get_result_fs( run_arg ) , sim_fs );
    test_assert_ptr_equal( run_arg_get_update_target_fs( run_arg ) , target_fs );
    run_arg_free( run_arg );

    enkf_fs_decref( sim_fs );
    enkf_fs_decref( target_fs );
  }
  test_work_area_free( test_area );
}


void alloc_invalid_run_arg(void *arg) {
  test_work_area_type * test_area = test_work_area_alloc("run_arg/invalid");
  {
    enkf_fs_type * fs    = enkf_fs_create_fs("fs" , BLOCK_FS_DRIVER_ID , NULL , true);
    run_arg_type * run_arg = run_arg_alloc_SMOOTHER_RUN(fs , fs , 0 , 6 , "path"); // This should explode ...
    run_arg_free( run_arg );
    enkf_fs_decref( fs );
  }
  test_work_area_free( test_area );
}


void test_invalid_update_on_self( ) {
  test_assert_util_abort( "run_arg_alloc" , alloc_invalid_run_arg , NULL);
}


void test_INIT_ONLY( ) {
  test_work_area_type * test_area = test_work_area_alloc("run_arg/INIT");
  {
    enkf_fs_type * init_fs   = enkf_fs_create_fs("sim" , BLOCK_FS_DRIVER_ID , NULL , true);

    run_arg_type * run_arg = run_arg_alloc_INIT_ONLY(init_fs , 0 , 6 , "path");
    test_assert_true( run_arg_is_instance( run_arg ));
    test_assert_ptr_equal( run_arg_get_init_fs( run_arg ) , init_fs );

    test_assert_util_abort( "run_arg_get_result_fs" , call_get_result_fs , run_arg );
    test_assert_util_abort( "run_arg_get_update_target_fs" , call_get_update_target_fs , run_arg );
    run_arg_free( run_arg );

    enkf_fs_decref( init_fs );
  }
  test_work_area_free( test_area );
}


void test_ENSEMBLE_EXPERIMENT( ) {
  test_work_area_type * test_area = test_work_area_alloc("run_arg/ENS");
  {
    enkf_fs_type * fs   = enkf_fs_create_fs("sim" , BLOCK_FS_DRIVER_ID , NULL , true);

    run_arg_type * run_arg = run_arg_alloc_ENSEMBLE_EXPERIMENT(fs , 0 , 6 , "path");
    test_assert_true( run_arg_is_instance( run_arg ));

    test_assert_ptr_equal( run_arg_get_init_fs( run_arg ) , fs );
    test_assert_ptr_equal( run_arg_get_result_fs( run_arg ) , fs );
    test_assert_util_abort( "run_arg_get_update_target_fs" , call_get_update_target_fs , run_arg );

    run_arg_free( run_arg );
    enkf_fs_decref( fs );
  }
  test_work_area_free( test_area );
}


int main(int argc , char ** argv) {
  test_queue_index();
  test_SMOOTHER_RUN();
  test_INIT_ONLY();
  test_ENSEMBLE_EXPERIMENT();
  test_invalid_update_on_self();
  exit(0);
}

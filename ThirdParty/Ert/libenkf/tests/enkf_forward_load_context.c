/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'forward_load_context.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>

#include <ert/enkf/forward_load_context.h>
#include <ert/enkf/run_arg.h>


void test_update_result() {
  forward_load_context_type * load_context = forward_load_context_alloc( NULL , false , NULL , NULL , NULL);
  test_assert_int_equal( forward_load_context_get_result( load_context ) , 0 );
  forward_load_context_update_result( load_context , 1 );
  test_assert_int_equal( forward_load_context_get_result( load_context ) , 1 );

  forward_load_context_update_result( load_context , 1 );
  test_assert_int_equal( forward_load_context_get_result( load_context ) , 1 );

  forward_load_context_update_result( load_context , 2 );
  test_assert_int_equal( forward_load_context_get_result( load_context ) , 3 );

  forward_load_context_update_result( load_context , 5 );
  test_assert_int_equal( forward_load_context_get_result( load_context ) , 7 );

  forward_load_context_free( load_context );
}

void test_create() {
  forward_load_context_type * load_context = forward_load_context_alloc( NULL , false , NULL , NULL ,  NULL);
  test_assert_true( forward_load_context_is_instance( load_context ));
  forward_load_context_free( load_context );
}

void test_load_restart1() {
  run_arg_type * run_arg = run_arg_alloc_ENSEMBLE_EXPERIMENT(NULL , 0 , 0 , "run");
  ecl_config_type * ecl_config = ecl_config_alloc( );
  forward_load_context_type * load_context = forward_load_context_alloc( run_arg , false , ecl_config , "BASE" , NULL );

  test_assert_false( forward_load_context_load_restart_file( load_context , 10 ));

  forward_load_context_free( load_context );
  ecl_config_free( ecl_config );
  run_arg_free( run_arg );
}


void make_restart_mock( const char * path , const char * eclbase , int report_step) {
  char * filename = ecl_util_alloc_filename( path , eclbase , ECL_RESTART_FILE , false , report_step );
  ecl_kw_type * kw = ecl_kw_alloc( "KW" , 100 , ECL_FLOAT_TYPE);
  fortio_type * f = fortio_open_writer( filename , false , true );
  ecl_kw_fwrite( kw , f );
  fortio_fclose( f );
  ecl_kw_free( kw );
  free( filename );
}

void test_load_restart2() {
  test_work_area_type * work_area = test_work_area_alloc("forward_load");
  {
    run_arg_type * run_arg = run_arg_alloc_ENSEMBLE_EXPERIMENT(NULL , 0 , 0 , "run");
    ecl_config_type * ecl_config = ecl_config_alloc( );
    forward_load_context_type * load_context = forward_load_context_alloc( run_arg , false , ecl_config , "BASE" , NULL );
    util_make_path("run");
    make_restart_mock( "run" , "BASE" , 1 );
    make_restart_mock( "run" , "BASE" , 3 );

    test_assert_false( forward_load_context_load_restart_file( load_context , 0 ));
    test_assert_true( forward_load_context_load_restart_file( load_context  , 1 ));
    test_assert_false( forward_load_context_load_restart_file( load_context , 2 ));
    test_assert_true( forward_load_context_load_restart_file( load_context  , 3 ));

    forward_load_context_free( load_context );
    ecl_config_free( ecl_config );
    run_arg_free( run_arg );
  }
  test_work_area_free( work_area );
}



void test_add_message() {
  {
    forward_load_context_type * load_context = forward_load_context_alloc( NULL , false , NULL , NULL , NULL);
    forward_load_context_add_message( load_context , "MESSAGE" );
    test_assert_false( forward_load_context_accept_messages( load_context ));
    forward_load_context_free( load_context );
  }

  {
    stringlist_type * message_list = stringlist_alloc_new( );
    forward_load_context_type * load_context = forward_load_context_alloc( NULL , false , NULL , NULL , message_list );

    test_assert_true( forward_load_context_accept_messages( load_context ));
    forward_load_context_add_message( load_context , "MESSAGE1" );
    forward_load_context_add_message( load_context , "MESSAGE2" );
    forward_load_context_free( load_context );

    test_assert_int_equal( 2 , stringlist_get_size( message_list ));
    test_assert_string_equal( stringlist_iget( message_list , 0 ) , "MESSAGE1" );
    test_assert_string_equal( stringlist_iget( message_list , 1 ) , "MESSAGE2" );
    stringlist_free( message_list );
  }
}




int main(int argc , char ** argv) {
  util_install_signals();
  test_create();
  test_load_restart1();
  test_load_restart2();
  test_add_message();
  test_update_result();
  exit(0);
}

/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'job_status_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <pthread.h>

#include "ert/util/build_config.h"

#include <ert/job_queue/job_queue_status.h>
#include <ert/job_queue/queue_driver.h>
#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>


void call_get_status( void * arg ) {
  job_queue_status_type * job_status = job_queue_status_safe_cast( arg );
  job_queue_status_get_count( job_status , JOB_QUEUE_DONE + JOB_QUEUE_USER_EXIT);
}



void test_create() {
  job_queue_status_type * status = job_queue_status_alloc();
  test_assert_true( job_queue_status_is_instance( status ));
  test_assert_int_equal( job_queue_status_get_count( status , JOB_QUEUE_DONE ) , 0 );
  test_assert_util_abort( "STATUS_INDEX" , call_get_status , status );
  job_queue_status_free( status );
}


void * add_sim( void * arg ) {
   job_queue_status_type * job_status = job_queue_status_safe_cast( arg );
   job_queue_status_inc( job_status , JOB_QUEUE_WAITING );
   return NULL;
}


void * user_exit( void * arg ) {
   job_queue_status_type * job_status = job_queue_status_safe_cast( arg );
   job_queue_status_transition( job_status , JOB_QUEUE_WAITING  , JOB_QUEUE_USER_EXIT);
   return NULL;
}


void * user_done( void * arg ) {
   job_queue_status_type * job_status = job_queue_status_safe_cast( arg );
   job_queue_status_transition( job_status , JOB_QUEUE_WAITING  , JOB_QUEUE_DONE);
   return NULL;
}



void test_update() {
  int N = 15000;
  pthread_t * thread_list = util_malloc( 2*N*sizeof * thread_list);
  int num_exit_threads = 0;
  int num_done_threads = 0;
  job_queue_status_type * status = job_queue_status_alloc();

  test_assert_int_equal( 0 , job_queue_status_get_total_count( status ));
  for (int i=0; i < 2*N; i++)
    add_sim( status );
  test_assert_int_equal( 2*N , job_queue_status_get_count( status , JOB_QUEUE_WAITING ));

  {
    int i = 0;
    while (true) {
      int thread_status;

      if ((i % 2) == 0) {
        thread_status = pthread_create( &thread_list[i] , NULL , user_exit , status );
        if (thread_status == 0)
          num_exit_threads++;
        else
          break;
      }  else {
        thread_status = pthread_create( &thread_list[i] , NULL , user_done , status );
        if (thread_status == 0)
          num_done_threads++;
        else
          break;
      }

      i++;
      if (i == N)
        break;
    }
  }
  if ((num_done_threads + num_exit_threads) == 0) {
    fprintf(stderr, "Hmmm - not a single thread created - very suspicious \n");
    exit(1);
  }

  for (int i=0; i < num_done_threads + num_exit_threads; i++)
    pthread_join( thread_list[i] , NULL );

  test_assert_int_equal( 2*N - num_done_threads - num_exit_threads , job_queue_status_get_count( status , JOB_QUEUE_WAITING ));
  test_assert_int_equal( num_exit_threads , job_queue_status_get_count( status , JOB_QUEUE_USER_EXIT ));
  test_assert_int_equal( num_done_threads , job_queue_status_get_count( status , JOB_QUEUE_DONE ));

  test_assert_int_equal( 2*N , job_queue_status_get_total_count( status ));
  job_queue_status_free( status );
}


void test_name() {
  test_assert_string_equal( job_queue_status_name( JOB_QUEUE_NOT_ACTIVE ) ,
                            "JOB_QUEUE_NOT_ACTIVE" );
  test_assert_string_equal( job_queue_status_name( JOB_QUEUE_EXIT ) ,
                            "JOB_QUEUE_EXIT" );
  test_assert_string_equal( job_queue_status_name( JOB_QUEUE_FAILED ) ,
                            "JOB_QUEUE_FAILED" );
}


int main( int argc , char ** argv) {
  util_install_signals();
  test_create();
  test_update();
  test_name();
}

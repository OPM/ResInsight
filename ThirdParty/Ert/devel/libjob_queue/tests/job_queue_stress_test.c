/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'job_queue_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <time.h>
#include <unistd.h>

#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>

#include <ert/util/test_util.h>
#include <ert/util/arg_pack.h>
#include <ert/util/test_work_area.h>
#include <ert/util/rng.h>
#include <ert/util/type_macros.h>

#include <ert/job_queue/job_queue.h>
#include <ert/job_queue/job_queue_manager.h>

#define JOB_TYPE_ID 77539
typedef struct {
  UTIL_TYPE_ID_DECLARATION;
  char * run_path;
  bool callback_run;
  int queue_index;
  int submit_usleep;
  int callback_usleep;
  int run_usleep;
  int argc;
  char ** argv;
  const char * cmd;
} job_type;



UTIL_SAFE_CAST_FUNCTION( job , JOB_TYPE_ID )

job_type * alloc_job( rng_type * rng , const char * cmd) {
  const int second = 1000000;
  const int submit_min = 0;
  const int submit_max = 10 * second;

  const int callback_min = 0.5 * second;
  const int callback_max = 2 * second;

  const int run_min = 2 * second;
  const int run_max = 10 * second;

  job_type * job = util_malloc( sizeof * job );
  UTIL_TYPE_ID_INIT( job , JOB_TYPE_ID )
  job->callback_run = false;
  job->queue_index = -1;
  job->submit_usleep    = submit_min  + rng_get_int( rng , (submit_max - submit_min ));
  job->callback_usleep = callback_min + rng_get_int( rng , (callback_max - callback_min ));
  job->run_usleep      = run_min      + rng_get_int( rng , (run_max - run_min ));
  job->run_path        = util_alloc_sprintf("%08d", rng_get_int(rng , 100000000));
  job->cmd = cmd;
  job->argc = 4;

  job->argv = util_malloc( 4 * sizeof * job->argv );
  job->argv[0] = job->run_path;
  job->argv[1] = "RUNNING";
  job->argv[2] = "OK";
  job->argv[3] = util_alloc_sprintf("%d", job->run_usleep);

  util_make_path( job->run_path );
  return job;
}


job_type ** alloc_jobs( rng_type * rng , int num_jobs , const char * cmd) {
  job_type ** jobs = util_malloc( num_jobs * sizeof * jobs );
  for (int i=0; i < num_jobs; i++) {
    job_type * job = alloc_job( rng , cmd);
    job_safe_cast( job );
    jobs[i] = job;
  }
  return jobs;
}



bool callback( void * arg ) {
  job_type * job = job_safe_cast( arg );
  usleep( job->callback_usleep );
  job->callback_run = true;
  return true;
}


void * submit_job__( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  job_type * job = job_safe_cast( arg_pack_iget_ptr( arg_pack , 0 ) );
  job_queue_type * queue = arg_pack_iget_ptr( arg_pack , 1 );
  job->queue_index = job_queue_add_job( queue  , job->cmd , callback , NULL , NULL , job , 1 , job->run_path , job->run_path , job->argc , (const char **) job->argv );
  usleep( job->submit_usleep );
  return NULL;
}


void submit_jobs( job_queue_type * queue , int num_jobs , job_type ** jobs , thread_pool_type * tp) {
  for (int i=0; i < num_jobs; i++) {
    job_type * job = jobs[i];
    arg_pack_type * arg = arg_pack_alloc();
    arg_pack_append_ptr( arg , job );
    arg_pack_append_ptr( arg , queue );
    thread_pool_add_job(tp , submit_job__ , arg );
  }
}


void check_jobs( int num_jobs , job_type ** jobs ) {
  for (int i=0; i < num_jobs; i++) {
    job_type * job = jobs[i];
    if (!job->callback_run)
      fprintf(stderr,"The callback has not been registered on job:%d/%d \n",i,job->queue_index);
    test_assert_true( job->callback_run );
  }
}


void * global_status( void * arg ) {
  job_queue_type * job_queue = job_queue_safe_cast( arg );
  int counter = 0;
  while (true) {
    util_usleep(100000);

    if (job_queue_get_num_complete(job_queue) == job_queue_get_active_size(job_queue))
      break;

    if ((counter % 10) == 0)
      printf("Waiting:%03d   Running:%03d   Callback:%03d  Complete:%03d \n",
             job_queue_get_num_waiting(job_queue) ,
             job_queue_get_num_running(job_queue),
             job_queue_get_num_callback( job_queue ) ,
             job_queue_get_num_complete(job_queue));

    counter++;
  }
  return NULL;
}


void * status_job__( void * arg ) {
  const int usleep_time = 10000;
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  job_type * job = job_safe_cast( arg_pack_iget_ptr( arg_pack , 0 ) );
  job_queue_type * queue = arg_pack_iget_ptr( arg_pack , 1 );
  char * run_file = util_alloc_filename( job->run_path , "RUNNING" , NULL);

  while (true) {
    if (job->queue_index >= 0) {
      job_status_type status;
      if (util_is_file(run_file)) {
        status = job_queue_iget_job_status(queue, job->queue_index);
        if (util_is_file(run_file))
          test_assert_true( (status == JOB_QUEUE_RUNNING) || (status == JOB_QUEUE_SUBMITTED) );
      }
      status = job_queue_iget_job_status(queue, job->queue_index);
      if (status == JOB_QUEUE_SUCCESS)
        break;
    }
    usleep( usleep_time );
  }

  free( run_file );
  return NULL;
}



void status_jobs( job_queue_type * queue , int num_jobs , job_type ** jobs , thread_pool_type * tp) {
  for (int i=0; i < num_jobs; i++) {
    job_type * job = jobs[i];
    arg_pack_type * arg = arg_pack_alloc();
    arg_pack_append_ptr( arg , job );
    arg_pack_append_ptr( arg , queue );
    thread_pool_add_job(tp , status_job__ , arg );
  }
  thread_pool_add_job( tp , global_status , queue );
}


/*
  The purpose of this test is to stress the queue system with a
  massively multithreaded workload. The test will submit jobs, let
  them run and run a callback. The various elements are pimped with
  usleep() calls to ensure that all of these actions:

   1. Submit
   2. Run callback
   3. Check status

  Are performed concurrently. The total runtime of the test should be
  ~ 120 seconds.
*/


int main(int argc , char ** argv) {
  const int queue_timeout =  180;
  const int submit_timeout = 180;
  const int status_timeout = 180;
  const int number_of_jobs = 250;
  const int submit_threads = number_of_jobs / 10 ;
  const int status_threads = number_of_jobs + 1;
  const char * job = util_alloc_abs_path(argv[1]);
  rng_type * rng = rng_alloc( MZRAN , INIT_CLOCK );
  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_type **jobs = alloc_jobs( rng , number_of_jobs , job);

  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_manager_type * queue_manager = job_queue_manager_alloc( queue );

  job_queue_set_driver(queue, driver);
  job_queue_manager_start_queue(queue_manager, 0, false , true);

  {
    thread_pool_type * status_pool = thread_pool_alloc( status_threads , true );
    thread_pool_type * submit_pool = thread_pool_alloc( submit_threads , true );

    submit_jobs( queue , number_of_jobs , jobs , submit_pool );
    status_jobs( queue , number_of_jobs , jobs , status_pool );

    if (!thread_pool_try_join( submit_pool , submit_timeout ))
      util_exit("Joining submit pool failed \n");
    thread_pool_free( submit_pool );

    job_queue_submit_complete(queue);

    if (!thread_pool_try_join( status_pool , status_timeout))
      util_exit("Joining status pool failed \n");
    thread_pool_free( status_pool );
  }

  if (!job_queue_manager_try_wait(queue_manager , queue_timeout))
    util_exit("job_queue never completed \n");

  job_queue_manager_free(queue_manager);
  job_queue_free(queue);
  queue_driver_free(driver);
  check_jobs( number_of_jobs , jobs );
  test_work_area_free(work_area);
  rng_free( rng );
}

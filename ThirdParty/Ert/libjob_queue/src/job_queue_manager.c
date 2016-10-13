/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'job_queue_manager.c' is part of ERT - Ensemble based Reservoir Tool.

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

#define  _GNU_SOURCE   /* Must define this to get access to pthread_rwlock_t */
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include <ert/util/type_macros.h>

#include <ert/job_queue/job_queue.h>
#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/job_queue_manager.h>

#define JOB_QUEUE_MANAGER_TYPE_ID 81626006



struct job_queue_manager_struct {
  UTIL_TYPE_ID_DECLARATION;
  pthread_t queue_thread;
  job_queue_type * job_queue;
};


UTIL_IS_INSTANCE_FUNCTION( job_queue_manager , JOB_QUEUE_MANAGER_TYPE_ID )


job_queue_manager_type * job_queue_manager_alloc( job_queue_type * job_queue ) {
  job_queue_manager_type * manager = util_malloc( sizeof * manager );
  UTIL_TYPE_ID_INIT( manager , JOB_QUEUE_MANAGER_TYPE_ID );
  manager->job_queue = job_queue;
  return manager;
}


void job_queue_manager_free( job_queue_manager_type * manager) {
  free( manager );
}


void job_queue_manager_start_queue( job_queue_manager_type * manager , int num_total_run , bool verbose , bool reset_queue) {
  if (reset_queue)
    job_queue_reset( manager->job_queue );

  job_queue_start_manager_thread( manager->job_queue , &manager->queue_thread , num_total_run , verbose );
}



void job_queue_manager_wait( job_queue_manager_type * manager) {
  pthread_join( manager->queue_thread , NULL );
}


bool job_queue_manager_try_wait( job_queue_manager_type * manager , int timeout_seconds) {
  struct timespec ts;
  time_t timeout_time = time( NULL );

  util_inplace_forward_seconds_utc(&timeout_time , timeout_seconds );
  ts.tv_sec = timeout_time;
  ts.tv_nsec = 0;

#ifdef HAVE_TIMEDJOIN
  {
    int join_return = pthread_timedjoin_np( manager->queue_thread , NULL , &ts);  /* Wait for the main thread to complete. */
    if (join_return == 0)
      return true;
    else
      return false;
  }
#else
    while(true) {
        if (pthread_kill(manager->queue_thread, 0) == 0){
            util_yield();
        } else {
            return true;
        }

        time_t now = time(NULL);

        if(util_difftime_seconds(now, timeout_time) <= 0) {
            return false;
        }
    }

#endif
}



bool job_queue_manager_is_running( const job_queue_manager_type * manager) {
  return job_queue_is_running( manager->job_queue );
}


int job_queue_manager_get_num_waiting( const job_queue_manager_type * manager) {
    return job_queue_get_num_waiting( manager->job_queue );
}


int job_queue_manager_get_num_running( const job_queue_manager_type * manager) {
  return job_queue_get_num_running( manager->job_queue );
}


int job_queue_manager_get_num_success( const job_queue_manager_type * manager) {
  return job_queue_get_num_complete( manager->job_queue );
}

int job_queue_manager_get_num_failed( const job_queue_manager_type * manager) {
  return job_queue_get_num_failed( manager->job_queue );
}



bool job_queue_manager_job_complete( const job_queue_manager_type * manager , int job_index) {
  job_status_type status = job_queue_iget_job_status( manager->job_queue , job_index );
  if (status & JOB_QUEUE_COMPLETE_STATUS)
    return true;
  else
    return false;
}


bool job_queue_manager_job_waiting( const job_queue_manager_type * manager , int job_index) {
  job_status_type status = job_queue_iget_job_status( manager->job_queue , job_index );
  if (status & JOB_QUEUE_WAITING_STATUS)
    return true;
  else
    return false;
}

bool job_queue_manager_job_running( const job_queue_manager_type * manager , int job_index) {
  job_status_type status = job_queue_iget_job_status( manager->job_queue , job_index );
  if (status == JOB_QUEUE_RUNNING)
    return true;
  else
    return false;
}


bool job_queue_manager_job_failed( const job_queue_manager_type * manager , int job_index) {
    job_status_type status = job_queue_iget_job_status( manager->job_queue , job_index );
    if (status == JOB_QUEUE_FAILED)
      return true;
    else
      return false;
}


bool job_queue_manager_job_success( const job_queue_manager_type * manager , int job_index) {
  job_status_type status = job_queue_iget_job_status( manager->job_queue , job_index );
  if (status == JOB_QUEUE_SUCCESS)
    return true;
  else
    return false;
}

job_status_type job_queue_manager_iget_job_status(const job_queue_manager_type * manager,  int job_index) {
    return job_queue_iget_job_status(manager->job_queue, job_index);
}


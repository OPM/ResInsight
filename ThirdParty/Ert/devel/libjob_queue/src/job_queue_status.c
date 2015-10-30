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
#include <pthread.h>

#include <ert/util/type_macros.h>
#include <ert/util/util.h>

#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/job_queue_status.h>

#define JOB_QUEUE_STATUS_TYPE_ID 777620306

struct job_queue_status_struct {
  UTIL_TYPE_ID_DECLARATION;
  int status_list[JOB_QUEUE_MAX_STATE];
  pthread_mutex_t update_mutex;
};

static const int status_index[] = {  JOB_QUEUE_NOT_ACTIVE ,  // Initial, allocated job state, job not added                                - controlled by job_queue
                                     JOB_QUEUE_WAITING    ,  // The job is ready to be started                                             - controlled by job_queue
                                     JOB_QUEUE_SUBMITTED  ,  // Job is submitted to driver - temporary state                               - controlled by job_queue
                                     JOB_QUEUE_PENDING    ,  // Job is pending, before actual execution                                    - controlled by queue_driver
                                     JOB_QUEUE_RUNNING    ,  // Job is executing                                                           - controlled by queue_driver
                                     JOB_QUEUE_DONE       ,  // Job is done (sucessful or not), temporary state                            - controlled/returned by by queue_driver
                                     JOB_QUEUE_EXIT       ,  // Job is done, with exit status != 0, temporary state                        - controlled/returned by by queue_driver
                                     JOB_QUEUE_USER_EXIT  ,  // User / queue system has requested killing of job                           - controlled by job_queue / external scope
                                     JOB_QUEUE_USER_KILLED,  // Job has been killed, due to JOB_QUEUE_USER_EXIT, FINAL STATE               - controlled by job_queue
                                     JOB_QUEUE_SUCCESS    ,  // All good, comes after JOB_QUEUE_DONE, with additional checks, FINAL STATE  - controlled by job_queue
                                     JOB_QUEUE_RUNNING_CALLBACK, // Temporary state, while running requested callbacks after an ended job  - controlled by job_queue
                                     JOB_QUEUE_FAILED };     // Job has failed, no more retries, FINAL STATE

static const char* status_name[] = { "JOB_QUEUE_NOT_ACTIVE" ,
                                     "JOB_QUEUE_WAITING"    ,
                                     "JOB_QUEUE_SUBMITTED"  ,
                                     "JOB_QUEUE_PENDING"    ,
                                     "JOB_QUEUE_RUNNING"    ,
                                     "JOB_QUEUE_DONE"       ,
                                     "JOB_QUEUE_EXIT"       ,
                                     "JOB_QUEUE_USER_KILLED" ,
                                     "JOB_QUEUE_USER_EXIT"   ,
                                     "JOB_QUEUE_SUCCESS"    ,
                                     "JOB_QUEUE_RUNNING_CALLBACK",
                                     "JOB_QUEUE_FAILED" };

static int STATUS_INDEX( job_status_type status ) {
  int index = 0;

  while (true) {
    if (status_index[index] == status)
      return index;

    index++;
    if (index == JOB_QUEUE_MAX_STATE)
      util_abort("%s: failed to get index from status:%d \n",__func__ , status);
  }
}


UTIL_IS_INSTANCE_FUNCTION( job_queue_status , JOB_QUEUE_STATUS_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION( job_queue_status , JOB_QUEUE_STATUS_TYPE_ID )


job_queue_status_type * job_queue_status_alloc() {
  job_queue_status_type * status = util_malloc( sizeof * status );
  UTIL_TYPE_ID_INIT( status ,   JOB_QUEUE_STATUS_TYPE_ID );
  pthread_mutex_init( &status->update_mutex , NULL );
  job_queue_status_clear( status );
  return status;
}


void job_queue_status_free( job_queue_status_type * status ) {
  free( status );
}


void job_queue_status_clear( job_queue_status_type * status ) {
  int index;
  for (index = 0; index < JOB_QUEUE_MAX_STATE; index++)
    status->status_list[ index ] = 0;
}


int job_queue_status_get_count( job_queue_status_type * status_count , job_status_type status_type) {
  int index = STATUS_INDEX( status_type );
  int count;

  count = status_count->status_list[index];

  return count;
}


void job_queue_status_inc( job_queue_status_type * status_count , job_status_type status_type) {
  int index = STATUS_INDEX( status_type );

  pthread_mutex_lock( &status_count->update_mutex );
  {
    int count = status_count->status_list[index];
    status_count->status_list[index] = count + 1;
  }
  pthread_mutex_unlock( &status_count->update_mutex );
}


static void job_queue_status_dec( job_queue_status_type * status_count , job_status_type status_type) {
  int index = STATUS_INDEX( status_type );

  pthread_mutex_lock( &status_count->update_mutex );
  {
    int count = status_count->status_list[index];
    status_count->status_list[index] = count - 1;
  }
  pthread_mutex_unlock( &status_count->update_mutex );
}


/*
  The important point is that each individual ++ and -- operation is
  atomic, if the different status counts do not add upp perfectly at
  all times that is ok.
*/


bool job_queue_status_transition( job_queue_status_type * status_count , job_status_type src_status , job_status_type target_status) {
  if (src_status != target_status) {
    job_queue_status_dec( status_count , src_status );
    job_queue_status_inc( status_count , target_status );
    return true;
  } else
    return false;
}


int job_queue_status_get_total_count( const job_queue_status_type * status ) {
  int total_count = 0;
  for (int index = 0; index < JOB_QUEUE_MAX_STATE; index++)
    total_count += status->status_list[ index ];
  return total_count;
}


const char * job_queue_status_name( job_status_type status ) {
  int index = STATUS_INDEX( status );
  return status_name[index];
}

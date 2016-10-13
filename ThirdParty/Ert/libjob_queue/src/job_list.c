/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'job_list.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <pthread.h>
#include <unistd.h>

#include <ert/util/msg.h>
#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>

#include <ert/job_queue/job_node.h>
#include <ert/job_queue/job_list.h>


#define JOB_LIST_TYPE_ID 8154222

struct job_list_struct {
  UTIL_TYPE_ID_DECLARATION;
  int active_size;
  int alloc_size;
  job_queue_node_type ** jobs;
  pthread_rwlock_t       lock;
};


UTIL_IS_INSTANCE_FUNCTION( job_list , JOB_LIST_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION( job_list , JOB_LIST_TYPE_ID )

job_list_type * job_list_alloc() {
  job_list_type * job_list = util_malloc( sizeof * job_list );
  UTIL_TYPE_ID_INIT( job_list , JOB_LIST_TYPE_ID );
  job_list->active_size = 0;
  job_list->alloc_size = 0;
  job_list->jobs = NULL;
  pthread_rwlock_init( &job_list->lock , NULL);
  return job_list;
}


void job_list_reset( job_list_type * job_list ) {
  int queue_index;
  for (queue_index = 0; queue_index < job_list->active_size; queue_index++) {
    job_queue_node_type * node = job_list_iget_job( job_list , queue_index );
    job_queue_node_free( node );
    job_list->jobs[queue_index] = NULL;
  }
  job_list->active_size = 0;
}


int job_list_get_size( const job_list_type * job_list ) {
  return job_list->active_size;
}


/*
  This takes ownership to the job node instance.
*/
void job_list_add_job( job_list_type * job_list , job_queue_node_type * job_node ) {
  if (job_list->alloc_size == job_list->active_size) {

#ifdef QUEUE_DEBUG
    int new_alloc_size = job_list->alloc_size + 1;
    job_queue_node_type ** new_jobs = util_malloc( sizeof * new_jobs * new_alloc_size );
    memcpy( new_jobs , job_list->jobs , sizeof * new_jobs * job_list->active_size );
    free( job_list->jobs );
    job_list->jobs = new_jobs;
#else
    int new_alloc_size = util_int_max( 16 , job_list->alloc_size * 2);
    job_list->jobs = util_realloc( job_list->jobs , sizeof * job_list->jobs * new_alloc_size );
#endif

    job_list->alloc_size = new_alloc_size;
  }

  {
    int queue_index = job_list_get_size( job_list );
    job_queue_node_set_queue_index(job_node, queue_index );
    job_list->jobs[queue_index] = job_node;
  }
  job_list->active_size++;

}


job_queue_node_type * job_list_iget_job( const job_list_type * job_list , int queue_index) {
  if (queue_index >= 0 && queue_index < job_list->active_size)
    return job_list->jobs[queue_index];
  else {
    util_abort("%s: invalid queue_index:%d  Valid range: [0,%d) \n",__func__ , queue_index , queue_index);
    return NULL;
  }
}


void job_list_free( job_list_type * job_list ) {
  if (job_list->alloc_size > 0) {
    job_list_reset( job_list );
    free( job_list->jobs );
  }
  free( job_list );
}



void job_list_get_wrlock( job_list_type * list) {
  pthread_rwlock_wrlock( &list->lock );
}

void job_list_get_rdlock( job_list_type * list) {
  pthread_rwlock_rdlock( &list->lock );
}


void job_list_unlock( job_list_type * list) {
  pthread_rwlock_unlock( &list->lock );
}


void job_list_reader_wait( job_list_type * list, int usleep_time1, int usleep_time2)  {
  if (pthread_rwlock_tryrdlock( &list->lock ) == 0) {
    // Seems to be no writers waiting - take a short sleep and return.
    pthread_rwlock_unlock( &list->lock );
    usleep( usleep_time1 );
  } else
    // A writer already has the lock - let more writers get access; sleep longer.
    usleep( usleep_time2 );

}

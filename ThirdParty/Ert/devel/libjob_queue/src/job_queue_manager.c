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
#include <pthread.h>
#include <unistd.h>

#include <ert/util/type_macros.h>

#include <ert/job_queue/job_queue.h>
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


void job_queue_manager_start_queue( job_queue_manager_type * manager , int num_total_run , bool verbose) {
  job_queue_start_manager_thread( manager->job_queue , &manager->queue_thread , num_total_run , verbose );
}



void job_queue_manager_wait( job_queue_manager_type * manager) {
  pthread_join( manager->queue_thread , NULL );   
}


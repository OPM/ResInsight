/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include <ert/util/util.h>
#include <ert/util/arg_pack.h>

#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/local_driver.h>





struct local_job_struct {
  UTIL_TYPE_ID_DECLARATION;
  bool            active;
  job_status_type status;
  pthread_t       run_thread;
  pid_t           child_process;
};


#define LOCAL_DRIVER_TYPE_ID 66196305
#define LOCAL_JOB_TYPE_ID    63056619

struct local_driver_struct {
  UTIL_TYPE_ID_DECLARATION;
  pthread_attr_t     thread_attr;
  pthread_mutex_t    submit_lock;
};

/*****************************************************************/


static UTIL_SAFE_CAST_FUNCTION( local_driver , LOCAL_DRIVER_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION( local_job    , LOCAL_JOB_TYPE_ID    )


local_job_type * local_job_alloc() {
  local_job_type * job;
  job = util_malloc(sizeof * job );
  UTIL_TYPE_ID_INIT( job , LOCAL_JOB_TYPE_ID );
  job->active = false;
  job->status = JOB_QUEUE_WAITING;
  return job;
}

void local_job_free(local_job_type * job) {
  if (job->active) {
    /* Thread clean up */
  }
  free(job);
}



job_status_type local_driver_get_job_status(void * __driver, void * __job) {
  if (__job == NULL) 
    /* The job has not been registered at all ... */
    return JOB_QUEUE_NOT_ACTIVE;
  else {
    local_job_type * job = local_job_safe_cast( __job );
    {
      if (job->active == false) {
        util_abort("%s: internal error - should not query status on inactive jobs \n" , __func__);
        return JOB_QUEUE_NOT_ACTIVE; /* Dummy */
      } else 
        return job->status;
    }
  }
}



void local_driver_free_job( void * __job ) {
  local_job_type    * job    = local_job_safe_cast( __job );
  local_job_free(job);
}


void local_driver_kill_job( void * __driver , void * __job) {
  local_job_type    * job  = local_job_safe_cast( __job );
  
  kill( job->child_process , SIGSTOP );
  if (job->active) 
    pthread_cancel( job->run_thread );
  
}








void * submit_job_thread__(void * __arg) {
  arg_pack_type * arg_pack = arg_pack_safe_cast(__arg);
  const char * executable  = arg_pack_iget_const_ptr(arg_pack , 0);
  /*
    The arg_pack contains a run_path field as the second argument,
    it has therefor been left here as a comment:
    
    const char * run_path    = arg_pack_iget_const_ptr(arg_pack , 1);   
  */
  int          argc        = arg_pack_iget_int(arg_pack , 2);
  char ** argv             = arg_pack_iget_ptr(arg_pack , 3);
  local_job_type * job     = arg_pack_iget_ptr(arg_pack , 4);
  
  job->child_process = util_fork_exec(executable , argc , (const char **) argv , false , NULL , NULL /* run_path */ , NULL , NULL , NULL); 
  waitpid(job->child_process , NULL , 0);
  job->status = JOB_QUEUE_DONE;
  pthread_exit(NULL);
  util_free_stringlist( argv , argc );
  return NULL;
}



void * local_driver_submit_job(void * __driver           , 
                               const char *  submit_cmd  , 
                               int           num_cpu     , /* Ignored */
                               const char *  run_path    , 
                               const char *  job_name    ,
                               int           argc        ,
                               const char ** argv ) {
  local_driver_type * driver = local_driver_safe_cast( __driver );
  {
    local_job_type * job    = local_job_alloc();
    arg_pack_type  * arg_pack = arg_pack_alloc();
    arg_pack_append_const_ptr( arg_pack , submit_cmd);
    arg_pack_append_const_ptr( arg_pack , run_path );
    arg_pack_append_int( arg_pack , argc );
    arg_pack_append_ptr( arg_pack , util_alloc_stringlist_copy( argv , argc ));   /* Due to conflict with threads and python GC we take a local copy. */
    arg_pack_append_ptr( arg_pack , job );
    
    pthread_mutex_lock( &driver->submit_lock );
    job->active = true;
    job->status = JOB_QUEUE_RUNNING;
    
    if (pthread_create( &job->run_thread , &driver->thread_attr , submit_job_thread__ , arg_pack) != 0) 
      util_abort("%s: failed to create run thread - aborting \n",__func__);
    
    pthread_mutex_unlock( &driver->submit_lock );
    return job;
  }
}



void local_driver_free(local_driver_type * driver) {
  pthread_attr_destroy ( &driver->thread_attr );
  free(driver);
  driver = NULL;
}


void local_driver_free__(void * __driver) {
  local_driver_type * driver = local_driver_safe_cast( __driver );
  local_driver_free( driver );
}


void * local_driver_alloc() {
  local_driver_type * local_driver = util_malloc(sizeof * local_driver );
  UTIL_TYPE_ID_INIT( local_driver , LOCAL_DRIVER_TYPE_ID);
  pthread_mutex_init( &local_driver->submit_lock , NULL );
  pthread_attr_init( &local_driver->thread_attr );
  pthread_attr_setdetachstate( &local_driver->thread_attr , PTHREAD_CREATE_DETACHED );
  
  return local_driver;
}


bool local_driver_set_option( void * __driver , const char * option_key , const void * value){ 
  return false;
}

#undef LOCAL_DRIVER_ID  
#undef LOCAL_JOB_ID    

/*****************************************************************/


/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'job_queue.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/job_queue/job_queue.h>
#include <ert/job_queue/job_node.h>
#include <ert/job_queue/job_list.h>
#include <ert/job_queue/job_queue_status.h>
#include <ert/job_queue/queue_driver.h>


/**

   The running of external jobs is handled thruogh an abstract
   job_queue implemented in this file; the job_queue then contains a
   'driver' which actually runs the job. All drivers must support the
   following functions

     submit: This will submit a job, and return a pointer to a
             newly allocated queue_job instance.

     clean:  This will clear up all resources used by the job.

     abort:  This will stop the job, and then call clean.

     status: This will get the status of the job.


   When calling the various driver functions the queue layer needs to
   dereference the driver structures, i.e. to get access to the
   driver->submit_jobs function. This is currently (rather clumsily??
   implemented like this):

        When implementing a driver the driver struct MUST start like
        this:

        struct some_driver {
            UTIL_TYPE_ID_DECLARATION
            QUEUE_DRIVER_FUNCTIONS
            ....
            ....
        }

        The function allocating a driver instance will just return a
        (void *) however in the queue layer the driver is stored as a
        queue_driver_type instance which is a struct like this:

        struct queue_driver_struct {
            UTIL_TYPE_ID_DECLARATION
            QUEUE_DRIVER_FIELDS
        }

        I.e. it only contains the pointers common to all the driver
        implementations. When calling a driver function the spesific
        driver will cast to it's datatype.

   Observe that this library also contains the files ext_joblist and
   ext_job, those files implement a particular way of dispatching
   external jobs in a series; AFTER THEY HAVE BEEN SUBMITTED. So seen
   from the this scope those files do not provide any particluar
   functionality; there is no compile-time dependencies either.
*/



/*
  Some words about status
  =======================

  The status of a particular job is given by the job_status field of
  the job_queue_node_type, the possible values are given by the enum
  job_status_type, defined in queue_driver.h.

  To actually __GET__ the status of a job we use the driver->status()
  function which will invoke a driver specific function and return the
  new status.

    1. The driver->status() function is invoked by the
       job_queue_update_status() function. This should be invoked by
       the same thread as is running the main queue management in
       job_queue_run_jobs().


    2. The actual change of status is handled by the function
       job_queue_change_node_status(); arbitrary assignments of the
       type job->status = new_status is STRICTLY ILLEGAL.


    3. When external functions query about the status of a particular
       job they get the status value currently stored (i.e. cached) in
       the job_node; external scope can NOT initiate a
       driver->status() function call.

       This might result in external scope getting a outdated status -
       live with it.


    4. The name 'status' indicates that this is read-only property;
       that is actually not the case. In the main manager function
       job_queue_run_jobs() action is taken based on the value of the
       status field, and to initiate certain action on jobs the queue
       system (and also external scope) can explicitly set the status
       of a job (by using the job_queue_change_node_status() function).

       The most promiment example of this is when we want to run a
       certain job again, that is achieved with:

           job_queue_node_change_status( queue , node , JOB_QUEUE_WAITING );

       When the queue manager subsequently finds the job with status
       'JOB_QUEUE_WAITING' it will (re)submit this job.
*/



/*
  Communicating success/failure between the job_script and the job_queue:
  =======================================================================

  The system for communicatin success/failure between the queue system
  (i.e. this file) and the job script is quite elaborate. There are
  essentially three problems which make this complicated:

   1. The exit status of the jobs is NOT reliably captured - the job
      might very well fail without us detecing it with the exit
      status.

   2. Syncronizing of disks can be quite slow, so altough a job has
      completede successfully the files we expect to find might not
      present.

   3. There is layer upon layer here - this file scope (i.e. the
      internal queue_system) spawns external jobs in the form of a job
      script. This script again spawns a series of real external jobs
      like e.g. ECLIPSE and RMS. The job_script does not reliably
      capture the exit status of the external programs.


  The approach to this is as follows:

   1. If the job (i.e. the job script) finishes with a failure status
      we communicate the failure back to the calling scope with no
      more ado.

   2. When a job has finished (seemingly OK) we try hard to determine
      whether the job has failed or not. This is based on the
      following tests:

      a) If the job has produced an EXIT file it has failed.

      b) If the job has produced an OK file it has succeeded.

      c) If neither EXIT nor OK files have been produced we spin for a
         while waiting for one of the files, if none turn up we will
         eventually mark the job as failed.

*/





/**
   This struct holds the job_queue information about one job. Observe
   the following:

    1. This struct is purely static - i.e. it is invisible outside of
       this file-scope.

    2. Typically the driver would like to store some additional
       information, i.e. the PID of the running process for the local
       driver; that is stored in a (driver specific) struct under the
       field job_data.

    3. If the driver detects that a job has failed it leaves an EXIT
       file, the exit status is (currently) not reliably transferred
       back to to the job_queue layer.

*/

/*****************************************************************/

#define JOB_QUEUE_TYPE_ID 665210

struct job_queue_struct {
  UTIL_TYPE_ID_DECLARATION;
  job_list_type            * job_list;
  job_queue_status_type    * status;
  char                     * exit_file;                         /* The queue will look for the occurence of this file to detect a failure. */
  char                     * ok_file;                           /* The queue will look for this file to verify that the job was OK - can be NULL - in which case it is ignored. */
  queue_driver_type        * driver;                            /* A pointer to a driver instance (LSF|LOCAL|RSH) which actually 'does it'. */

  bool                       open;                              /* True if the queue has been reset and is ready for use, false if the queue has been used and not reset */
  bool                       user_exit;                         /* If there comes an external signal to abondond the whole thing user_exit will be set to true, and things start to dwindle down. */
  bool                       running;
  bool                       pause_on;
  bool                       submit_complete;

  int                        max_submit;                        /* The maximum number of submit attempts for one job. */
  int                        max_ok_wait_time;                  /* Seconds to wait for an OK file - when the job itself has said all OK. */
  int                        max_duration;                      /* Maximum allowed time for a job to run, 0 = unlimited */
  time_t                     stop_time;                         /* A job is only allowed to run until this time. 0 = no time set, ignore stop_time */
  unsigned long              usleep_time;                       /* The sleep time before checking for updates. */
  pthread_mutex_t            run_mutex;                         /* This mutex is used to ensure that ONLY one thread is executing the job_queue_run_jobs(). */
  thread_pool_type         * work_pool;
};






/*
  Must hold on to:

   1. A write lock for the job node.
   3. A read lock for the job_list

*/
static bool job_queue_change_node_status(job_queue_type * queue , job_queue_node_type * node , job_status_type new_status) {
  return job_queue_node_status_transition( node , queue->status , new_status );
}




/*****************************************************************/







/**
   Observe that this function should only query the driver for state
   change when the job is currently in one of the states:

     JOB_QUEUE_WAITING || JOB_QUEUE_PENDING || JOB_QUEUE_RUNNING

   The other state transitions are handled by the job_queue itself,
   without consulting the driver functions.
*/

/*
  Will return true if there is any status change. Must already hold
  on to joblist readlock
*/

static bool job_queue_update_status(job_queue_type * queue ) {
  bool update = false;
  int ijob;

  for (ijob = 0; ijob < job_list_get_size( queue->job_list ); ijob++) {
    job_queue_node_type * node = job_list_iget_job( queue->job_list , ijob );
    bool node_update = job_queue_node_update_status( node , queue->status , queue->driver );
    if (node_update)
      update = true;
  }
  return update;
}

/*
  Must hold on to joblist readlock
*/

static submit_status_type job_queue_submit_job(job_queue_type * queue , int queue_index) {
  submit_status_type submit_status;
  if (queue->user_exit || queue->pause_on)
    submit_status = SUBMIT_QUEUE_CLOSED;   /* The queue is currently not accepting more jobs. */
  else {
    {
      job_queue_node_type * node = job_list_iget_job( queue->job_list , queue_index );
      submit_status = job_queue_node_submit( node , queue->status , queue->driver );
    }
  }
  return submit_status;
}









/**
   Will return the number of jobs with status @status.

      #include <queue_driver.h>

      printf("Running jobs...: %03d \n", job_queue_iget_status_summary( queue , JOB_QUEUE_RUNNING ));
      printf("Waiting jobs:..: %03d \n", job_queue_iget_status_summary( queue , JOB_QUEUE_WAITING ));

   Observe that if this function is called repeatedly the status might change between
   calls, with the consequence that the total number of jobs does not add up
   properly. The handles itself autonomously so as long as the return value from this
   function is only used for information purposes this does not matter. Alternatively
   the function job_queue_export_status_summary(), which does proper locking, can be
   used.
*/

int job_queue_iget_status_summary( const job_queue_type * queue , job_status_type status) {
  return job_queue_status_get_count(queue->status, status);
}

int job_queue_get_num_callback( const job_queue_type * queue) {
  return job_queue_iget_status_summary( queue , JOB_QUEUE_RUNNING_CALLBACK );
}

int job_queue_get_num_running( const job_queue_type * queue) {
  return job_queue_iget_status_summary( queue , JOB_QUEUE_RUNNING );
}

int job_queue_get_num_pending( const job_queue_type * queue) {
  return job_queue_iget_status_summary( queue , JOB_QUEUE_PENDING );
}

int job_queue_get_num_waiting( const job_queue_type * queue) {
  return job_queue_iget_status_summary( queue , JOB_QUEUE_WAITING );
}

int job_queue_get_num_complete( const job_queue_type * queue) {
  return job_queue_iget_status_summary( queue , JOB_QUEUE_SUCCESS );
}

int job_queue_get_num_failed( const job_queue_type * queue) {
  return job_queue_iget_status_summary( queue , JOB_QUEUE_FAILED );
}

int job_queue_get_num_killed( const job_queue_type * queue) {
  return job_queue_iget_status_summary( queue , JOB_QUEUE_USER_KILLED );
}

int job_queue_get_active_size( const job_queue_type * queue ) {
  return job_list_get_size( queue->job_list );
}

void job_queue_set_max_job_duration(job_queue_type * queue, int max_duration_seconds) {
  queue->max_duration = max_duration_seconds;
}

int job_queue_get_max_job_duration(const job_queue_type * queue) {
  return queue->max_duration;
}

void job_queue_set_job_stop_time(job_queue_type * queue, time_t time) {
  queue->stop_time = time;
}

time_t job_queue_get_job_stop_time(const job_queue_type * queue) {
  return queue->stop_time;
}

void job_queue_set_auto_job_stop_time(job_queue_type * queue) {
  time_t sum_run_time_succeded_jobs = 0;
  int num_succeded_jobs = 0;

  for (int i = 0; i < job_list_get_size( queue->job_list ); i++) {
    if (job_queue_iget_job_status(queue,i) == JOB_QUEUE_SUCCESS) {
      sum_run_time_succeded_jobs += difftime(job_queue_iget_sim_end(queue, i), job_queue_iget_sim_start(queue, i));
      num_succeded_jobs++;
    }
  }

  if (num_succeded_jobs > 0) {
    time_t avg_run_time_succeded_jobs = sum_run_time_succeded_jobs / num_succeded_jobs;
    time_t stop_time = time(NULL) + (avg_run_time_succeded_jobs * 0.25);
    job_queue_set_job_stop_time(queue, stop_time);
  }
}

/**
   Observe that jobs with status JOB_QUEUE_WAITING can also be killed; for those
   jobs the kill should be interpreted as "Forget about this job for now and set
   the status JOB_QUEUE_USER_KILLED", however it is important that we not call
   the driver->kill() function on it because the job slot will have no data
   (i.e. LSF jobnr), and the driver->kill() function will fail if presented with
   such a job.

   Only jobs which have a status matching "JOB_QUEUE_CAN_KILL" can be
   killed; if the job is not in a killable state the function will do
   nothing. This includes trying to kill a job which is not even
   found.

   Observe that jobs (slots) with status JOB_QUEUE_NOT_ACTIVE can NOT be
   meaningfully killed; that is because these jobs have not yet been submitted
   to the queue system, and there is not yet established a mapping between
   external id and queue_index.

   Must hold on to joblist:read lock.
*/

static bool job_queue_kill_job_node( job_queue_type * queue , job_queue_node_type * node) {
  bool result = job_queue_node_kill( node , queue->status , queue->driver );
  return result;
}

#define ASSIGN_LOCKED_ATTRIBUTE( var , func , ...)                               \
job_list_get_rdlock( queue->job_list );                                          \
{                                                                                \
  job_queue_node_type * node = job_list_iget_job( queue->job_list , job_index ); \
  var = func(__VA_ARGS__);                                                       \
}                                                                                \
job_list_unlock( queue->job_list );



bool job_queue_kill_job( job_queue_type * queue , int job_index) {
  bool result;
  ASSIGN_LOCKED_ATTRIBUTE( result , job_queue_kill_job_node , queue , node);
  return result;
}


time_t job_queue_iget_sim_start( job_queue_type * queue, int job_index) {
  time_t sim_start;
  ASSIGN_LOCKED_ATTRIBUTE( sim_start , job_queue_node_get_sim_start , node );
  return sim_start;
}


time_t job_queue_iget_sim_end( job_queue_type * queue, int job_index) {
  time_t sim_end;
  ASSIGN_LOCKED_ATTRIBUTE( sim_end , job_queue_node_get_sim_end , node );
  return sim_end;
}


time_t job_queue_iget_submit_time( job_queue_type * queue, int job_index) {
  time_t submit_time;
  ASSIGN_LOCKED_ATTRIBUTE( submit_time , job_queue_node_get_submit_time , node );
  return submit_time;
}

const char * job_queue_iget_run_path( job_queue_type * queue , int job_index) {
  const char * run_path;
  ASSIGN_LOCKED_ATTRIBUTE(run_path, job_queue_node_get_run_path, node );
  return run_path;
}


const char * job_queue_iget_failed_job( job_queue_type * queue , int job_index) {
  const char * failed_job;
  ASSIGN_LOCKED_ATTRIBUTE(failed_job, job_queue_node_get_failed_job, node );
  return failed_job;
}


const char * job_queue_iget_error_reason( job_queue_type * queue , int job_index) {
  const char * error_reason;
  ASSIGN_LOCKED_ATTRIBUTE(error_reason, job_queue_node_get_error_reason, node );
  return error_reason;
}


const char * job_queue_iget_stderr_capture(  job_queue_type * queue , int job_index) {
  const char * stderr_capture;
  ASSIGN_LOCKED_ATTRIBUTE(stderr_capture, job_queue_node_get_stderr_capture, node );
  return stderr_capture;
}


const char * job_queue_iget_stderr_file( job_queue_type * queue , int job_index) {
  const char * stderr_file;
  ASSIGN_LOCKED_ATTRIBUTE(stderr_file, job_queue_node_get_stderr_file, node );
  return stderr_file;
}



job_status_type job_queue_iget_job_status( job_queue_type * queue , int job_index) {
  job_status_type job_status;
  ASSIGN_LOCKED_ATTRIBUTE(job_status, job_queue_node_get_status , node );
  return job_status;
}







/**
   The external scope asks the queue to restart the the job; we reset
   the submit counter to zero. This function should typically be used
   in combination with resampling, however that is the responsability
   of the calling scope.
*/

void job_queue_iset_external_restart(job_queue_type * queue , int job_index) {
  job_list_get_rdlock( queue->job_list );
  {
    job_queue_node_type * node = job_list_iget_job( queue->job_list , job_index );
    job_queue_node_restart(node,queue->status);
  }
  job_list_unlock( queue->job_list );
}


/**
   The queue system has said that the job completed OK, however the
   external scope failed to load all the results and are using this
   function to inform the queue system that the job has indeed
   failed. The queue system will then either retry the job, or switch
   status to JOB_QUEUE_RUN_FAIL.


   This is a bit dangerous beacuse the queue system has said that the
   job was all hunkadory, and freed the driver related resources
   attached to the job; it is therefor essential that the
   JOB_QUEUE_EXIT code explicitly checks the status of the job node's
   driver specific data before dereferencing.
*/

void job_queue_iset_external_fail(job_queue_type * queue , int job_index) {
  job_list_get_rdlock( queue->job_list );
  {
    job_queue_node_type * node = job_list_iget_job( queue->job_list , job_index );
    job_queue_node_status_transition(node,queue->status,JOB_QUEUE_EXIT);
  }
  job_list_unlock( queue->job_list );
}






static void job_queue_update_spinner( int * phase ) {
  const char * spinner = "-\\|/";
  int spinner_length   = strlen( spinner );

  printf("%c\b" , spinner[ (*phase % spinner_length) ]);
  fflush(stdout);
  (*phase) += 1;
}


static void job_queue_print_summary(job_queue_type *queue, bool status_change ) {
  const char * status_fmt = "Waiting: %3d    Pending: %3d    Running: %3d    Checking/Loading: %3d    Failed: %3d    Complete: %3d   [ ]\b\b";
  int string_length       = 105;

  if (status_change) {
    for (int i=0; i < string_length; i++)
      printf("\b");

    {
      int waiting  = job_queue_status_get_count( queue->status , JOB_QUEUE_WAITING );
      int pending  = job_queue_status_get_count( queue->status , JOB_QUEUE_PENDING );

      /*
         EXIT and DONE are included in "xxx_running", because the target
         file has not yet been checked.
      */
      int running  = job_queue_status_get_count( queue->status , JOB_QUEUE_RUNNING ) +
                     job_queue_status_get_count( queue->status , JOB_QUEUE_DONE ) +
                     job_queue_status_get_count( queue->status , JOB_QUEUE_EXIT );
      int complete = job_queue_status_get_count( queue->status , JOB_QUEUE_SUCCESS );
      int failed   = job_queue_status_get_count( queue->status , JOB_QUEUE_FAILED );
      int loading  = job_queue_status_get_count( queue->status , JOB_QUEUE_RUNNING_CALLBACK );

      printf(status_fmt , waiting , pending , running , loading , failed , complete);
    }
  }
}





/**
    This function goes through all the nodes and call finalize on
    them. What about jobs which were NOT in a CAN_KILL state when the
    killing was done, i.e. jobs which are in one of the intermediate
    load like states?? They
*/

void job_queue_reset(job_queue_type * queue) {
  job_list_get_wrlock( queue->job_list );
  job_list_reset( queue->job_list );
  job_list_unlock( queue->job_list );
  job_queue_status_clear(queue->status);

  /*
      Be ready for the next run
  */
  queue->submit_complete = false;
  queue->pause_on        = false;
  queue->user_exit       = false;
  queue->open            = true;
  queue->stop_time       = 0;
}


bool job_queue_is_running( const job_queue_type * queue ) {
  return queue->running;
}


static void job_queue_user_exit__( job_queue_type * queue ) {
  int queue_index;
  for (queue_index = 0; queue_index < job_list_get_size( queue->job_list ); queue_index++) {
    job_queue_node_type * node = job_list_iget_job( queue->job_list , queue_index );
    job_queue_node_status_transition(node,queue->status,JOB_QUEUE_USER_EXIT);
  }
}


static bool job_queue_check_node_status_files( const job_queue_type * job_queue , job_queue_node_type * node) {
  const char * exit_file = job_queue_node_get_exit_file( node );
  if ((exit_file != NULL) && util_file_exists(exit_file))
    return false;                /* It has failed. */
  else {
    const char * ok_file = job_queue_node_get_ok_file( node );
    if (ok_file == NULL)
      return true;               /* If the ok-file has not been set we just return true immediately. */
    else {
      int ok_sleep_time    =  1; /* Time to wait between checks for OK|EXIT file.                         */
      int  total_wait_time =  0;

      while (true) {
        if (util_file_exists( ok_file )) {
          return true;
          break;
        } else {
          if (total_wait_time <  job_queue->max_ok_wait_time) {
            sleep( ok_sleep_time );
            total_wait_time += ok_sleep_time;
          } else {
            /* We have waited long enough - this does not seem to give any OK file. */
            return false;
            break;
          }
        }
      }
    }
  }
}


static void * job_queue_run_DONE_callback( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  job_queue_type * job_queue = arg_pack_iget_ptr( arg_pack , 0 );
  int queue_index = arg_pack_iget_int( arg_pack , 1 );
  job_list_get_rdlock( job_queue->job_list );
  {
    job_queue_node_type * node = job_list_iget_job( job_queue->job_list , queue_index );
    bool OK = job_queue_check_node_status_files( job_queue , node );

    if (OK)
      OK = job_queue_node_run_DONE_callback( node );

    if (OK)
      job_queue_change_node_status( job_queue , node , JOB_QUEUE_SUCCESS );
    else
      job_queue_change_node_status( job_queue , node , JOB_QUEUE_EXIT );

    job_queue_node_free_driver_data( node , job_queue->driver );
  }
  job_list_unlock(job_queue->job_list );
  arg_pack_free( arg_pack );
  return NULL;
}


static void job_queue_handle_DONE( job_queue_type * queue , job_queue_node_type * node) {
  job_queue_change_node_status(queue , node , JOB_QUEUE_RUNNING_CALLBACK );
  {
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , queue );
    arg_pack_append_int( arg_pack , job_queue_node_get_queue_index(node));
    thread_pool_add_job( queue->work_pool , job_queue_run_DONE_callback , arg_pack );
  }
}


static void * job_queue_run_EXIT_callback( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  job_queue_type * job_queue = arg_pack_iget_ptr( arg_pack , 0 );
  int queue_index = arg_pack_iget_int( arg_pack , 1 );

  job_list_get_rdlock( job_queue->job_list );
  {
    job_queue_node_type * node = job_list_iget_job( job_queue->job_list , queue_index );

    if (job_queue_node_get_submit_attempt( node ) < job_queue->max_submit)
      job_queue_change_node_status( job_queue , node , JOB_QUEUE_WAITING );  /* The job will be picked up for antother go. */
    else {
      bool retry = job_queue_node_run_RETRY_callback( node );

      if (retry) {
        /* OK - we have invoked the retry_callback() - and that has returned true;
           giving this job a brand new start. */
        job_queue_node_reset_submit_attempt( node );
        job_queue_change_node_status(job_queue , node , JOB_QUEUE_WAITING);
      } else {
        // It's time to call it a day

        job_queue_node_run_EXIT_callback( node );
        job_queue_change_node_status(job_queue , node , JOB_QUEUE_FAILED);
      }
    }
    job_queue_node_free_driver_data( node , job_queue->driver );
  }
  job_list_unlock(job_queue->job_list );
  arg_pack_free( arg_pack );

  return NULL;
}


static void * job_queue_run_USER_EXIT_callback( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  job_queue_type * job_queue = arg_pack_iget_ptr( arg_pack , 0 );
  int queue_index = arg_pack_iget_int( arg_pack , 1 );

  job_list_get_rdlock( job_queue->job_list );
  {
    job_queue_node_type * node = job_list_iget_job( job_queue->job_list , queue_index );
    job_queue_node_free_driver_data( node , job_queue->driver );

    // It's time to call it a day
    job_queue_node_run_EXIT_callback( node );
    job_queue_change_node_status(job_queue, node, JOB_QUEUE_USER_KILLED);
  }
  job_list_unlock(job_queue->job_list );
  arg_pack_free( arg_pack );
  return NULL;
}




static void job_queue_handle_USER_EXIT( job_queue_type * queue , job_queue_node_type * node) {
  job_queue_kill_job_node(queue, node);
  job_queue_change_node_status(queue , node , JOB_QUEUE_RUNNING_CALLBACK );
  {
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , queue );
    arg_pack_append_int( arg_pack , job_queue_node_get_queue_index(node));
    thread_pool_add_job( queue->work_pool , job_queue_run_USER_EXIT_callback , arg_pack );
  }
}

static void job_queue_handle_EXIT( job_queue_type * queue , job_queue_node_type * node) {
  job_queue_change_node_status(queue , node , JOB_QUEUE_RUNNING_CALLBACK );
  {
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , queue );
    arg_pack_append_int( arg_pack , job_queue_node_get_queue_index(node));
    thread_pool_add_job( queue->work_pool , job_queue_run_EXIT_callback , arg_pack );
  }
}



/*****************************************************************/

static void job_queue_check_expired(job_queue_type * queue) {
  if ((job_queue_get_max_job_duration(queue) <= 0) && (job_queue_get_job_stop_time(queue) <= 0))
    return;

  for (int i = 0; i < job_list_get_size( queue->job_list ); i++) {
    job_queue_node_type * node = job_list_iget_job( queue->job_list , i );

    if (job_queue_node_get_status(node) == JOB_QUEUE_RUNNING) {
      time_t now = time(NULL);
      if ( job_queue_get_max_job_duration(queue) > 0) {
        double elapsed = difftime(now, job_queue_node_get_sim_start( node ));
        if (elapsed > job_queue_get_max_job_duration(queue))
          job_queue_change_node_status(queue, node, JOB_QUEUE_USER_EXIT);
      }
      if (job_queue_get_job_stop_time(queue) > 0) {
        if (now >= job_queue_get_job_stop_time(queue))
          job_queue_change_node_status(queue, node, JOB_QUEUE_USER_EXIT);
      }
    }
  }
}

bool job_queue_get_open(const job_queue_type * job_queue) {
  return job_queue->open;
}

void job_queue_check_open(job_queue_type* queue) {
  if (!job_queue_get_open(queue))
    util_abort("%s: queue not open and not ready for use; method job_queue_reset must be called before using the queue - aborting\n", __func__ );
}

/**
   If the total number of jobs is not known in advance the job_queue_run_jobs
   function can be called with @num_total_run == 0. In that case it is paramount
   to call the function job_queue_submit_complete() whan all jobs have been submitted.

   Observe that this function is assumed to have ~exclusive access to
   the jobs array; meaning that:

     1. The jobs array is read without taking a reader lock.

     2. Other functions accessing the jobs array concurrently must
        take a read lock.

     3. This function should be the *only* function modifying
        the jobs array, and that is done *with* the write lock.

*/

void job_queue_run_jobs(job_queue_type * queue , int num_total_run, bool verbose) {
  int trylock = pthread_mutex_trylock( &queue->run_mutex );
  if (trylock != 0)
    util_abort("%s: another thread is already running the queue_manager\n",__func__);
  else {
    /* OK - we have got an exclusive lock to the run_jobs code. */

    //Check if queue is open. Fails hard if not open
    job_queue_check_open(queue);

    const int NUM_WORKER_THREADS = 16;
    queue->running = true;
    queue->work_pool = thread_pool_alloc( NUM_WORKER_THREADS , true );
    {
      bool new_jobs         = false;
      bool cont             = true;
      int  phase = 0;

      do {
        bool local_user_exit = false;
        job_list_get_rdlock( queue->job_list );
        /*****************************************************************/
        if (queue->user_exit)  {/* An external thread has called the job_queue_user_exit() function, and we should kill
                                   all jobs, do some clearing up and go home. Observe that we will go through the
                                   queue handling codeblock below ONE LAST TIME before exiting. */
          job_queue_user_exit__( queue );
          local_user_exit = true;
        }

        job_queue_check_expired(queue);

        /*****************************************************************/
        {
          bool update_status = job_queue_update_status( queue );
          if (verbose) {
            if (update_status || new_jobs)
              job_queue_print_summary(queue , update_status );
            job_queue_update_spinner( &phase );
          }


          {
            int num_complete = job_queue_status_get_count(queue->status, JOB_QUEUE_SUCCESS) +
                               job_queue_status_get_count(queue->status, JOB_QUEUE_FAILED) +
                               job_queue_status_get_count(queue->status, JOB_QUEUE_USER_KILLED);

            if ((num_total_run > 0) && (num_total_run == num_complete))
              /* The number of jobs completed is equal to the number
                 of jobs we have said we want to run; so we are finished.
              */
              cont = false;
            else {
              if (num_total_run == 0) {
                /* We have not informed about how many jobs we will
                   run. To check if we are complete we perform the two
                   tests:

                     1. All the jobs which have been added with
                        job_queue_add_job() have completed.

                     2. The user has used job_queue_complete_submit()
                        to signal that no more jobs will be forthcoming.
                */
                if ((num_complete == job_list_get_size( queue->job_list )) && queue->submit_complete)
                  cont = false;
              }
            }
          }

          if (cont) {
            /* Submitting new jobs */
            int max_submit     = 5; /* This is the maximum number of jobs submitted in one while() { ... } below.
                                       Only to ensure that the waiting time before a status update is not too long. */
            int total_active   = job_queue_status_get_count(queue->status, JOB_QUEUE_PENDING) + job_queue_status_get_count(queue->status, JOB_QUEUE_RUNNING);
            int num_submit_new;

            {
              int max_running = job_queue_get_max_running( queue );
              if (max_running > 0)
                num_submit_new = util_int_min( max_submit ,  max_running - total_active );
              else
                /*
                   If max_running == 0 that should be interpreted as no limit; i.e. the queue layer will
                   attempt to send an unlimited number of jobs to the driver - the driver can reject the jobs.
                */
                num_submit_new = util_int_min( max_submit , job_queue_status_get_count(queue->status, JOB_QUEUE_WAITING));
            }

            new_jobs = false;
            if (job_queue_status_get_count(queue->status, JOB_QUEUE_WAITING) > 0)   /* We have waiting jobs at all           */
              if (num_submit_new > 0)                                               /* The queue can allow more running jobs */
                new_jobs = true;

            if (new_jobs) {
              int submit_count = 0;
              int queue_index  = 0;

              while ((queue_index < job_list_get_size( queue->job_list )) && (num_submit_new > 0)) {
                job_queue_node_type * node = job_list_iget_job( queue->job_list , queue_index );
                if (job_queue_node_get_status(node) == JOB_QUEUE_WAITING) {
                  {
                    submit_status_type submit_status = job_queue_submit_job(queue , queue_index);

                    if (submit_status == SUBMIT_OK) {
                      num_submit_new--;
                      submit_count++;
                    } else if ((submit_status == SUBMIT_DRIVER_FAIL) || (submit_status == SUBMIT_QUEUE_CLOSED))
                      break;
                  }
                }
                queue_index++;
              }
            }


            {
              /*
                Checking for complete / exited / overtime jobs
               */
              int queue_index;
              for (queue_index = 0; queue_index < job_list_get_size( queue->job_list ); queue_index++) {
                job_queue_node_type * node = job_list_iget_job( queue->job_list , queue_index );

                switch (job_queue_node_get_status(node)) {
                  case(JOB_QUEUE_DONE):
                    job_queue_handle_DONE(queue, node);
                    break;
                  case(JOB_QUEUE_EXIT):
                    job_queue_handle_EXIT(queue, node);
                    break;
                  case(JOB_QUEUE_USER_EXIT):
                    job_queue_handle_USER_EXIT(queue, node);
                    break;
                  default:
                    break;
                }


              }
            }
          } else
            /* print an updated status to stdout before exiting. */
            if (verbose)
              job_queue_print_summary(queue , true);
        }
        job_list_unlock( queue->job_list );
        if (local_user_exit)
          cont = false;    /* This is how we signal that we want to get out . */
        else {
          util_yield();
          job_list_reader_wait( queue->job_list , queue->usleep_time , 8 * queue->usleep_time);
        }
      } while ( cont );
      queue->running = false;
    }
    if (verbose)
      printf("\n");
    thread_pool_join( queue->work_pool );
    thread_pool_free( queue->work_pool );
  }

  /*
    Set the queue's "open" flag to false to signal that the queue is
    not ready to be used in a new job_queue_run_jobs or
    job_queue_add_job method call as it has not been reset yet. Not
    resetting the queue here implies that the queue object is still
    available for queries after this method has finished
  */
  queue->open = false;
  pthread_mutex_unlock( &queue->run_mutex );
}





void * job_queue_run_jobs__(void * __arg_pack) {
  arg_pack_type * arg_pack = arg_pack_safe_cast(__arg_pack);
  job_queue_type * queue   = arg_pack_iget_ptr(arg_pack , 0);
  int num_total_run        = arg_pack_iget_int(arg_pack , 1);
  bool verbose             = arg_pack_iget_bool(arg_pack , 2);

  job_queue_run_jobs(queue , num_total_run , verbose);
  arg_pack_free( arg_pack );
  return NULL;
}


void job_queue_start_manager_thread( job_queue_type * job_queue , pthread_t * queue_thread , int job_size , bool verbose) {

  arg_pack_type  * queue_args = arg_pack_alloc(); /* This arg_pack will be freed() in the job_que_run_jobs__() */
  arg_pack_append_ptr(queue_args  , job_queue);
  arg_pack_append_int(queue_args  , job_size);
  arg_pack_append_bool(queue_args , verbose);

  /*
    The running status of the job is set to true here; this is to
    guarantee that if calling scope queries the status of the queue
    before queue_thread has actually started running the queue.
  */
  job_queue->running = true;
  pthread_create( queue_thread , NULL , job_queue_run_jobs__ , queue_args);
}




/**
   The most flexible use scenario is as follows:

     1. The job_queue_run_jobs() is run by one thread.
     2. Jobs are added asyncronously with job_queue_add_job() from othread threads(s).


   Unfortunately it does not work properly (i.e. Ctrl-C breaks) to use a Python
   thread to invoke the job_queue_run_jobs() function; and this function is
   mainly a workaround around that problem. The function will create a new
   thread and run job_queue_run_jobs() in that thread; the calling thread will
   just return.

   No reference is retained to the thread actually running the
   job_queue_run_jobs() function.
*/


void job_queue_run_jobs_threaded(job_queue_type * queue , int num_total_run, bool verbose) {
  pthread_t        queue_thread;
  job_queue_start_manager_thread( queue , &queue_thread , num_total_run , verbose );
  pthread_detach( queue_thread );             /* Signal that the thread resources should be cleaned up when
                                                 the thread has exited. */
}



/*****************************************************************/
/* Adding new jobs - it is complicated ... */


/**
   This initializes the non-driver-spesific fields of a job, i.e. the
   name, runpath and so on, and sets the job->status ==
   JOB_QUEUE_WAITING. This status means the job is ready to be
   submitted proper to one of the drivers (when a slot is ready).
   When submitted the job will get (driver specific) job_data != NULL
   and status SUBMITTED.
*/




int job_queue_add_job(job_queue_type * queue ,
                      const char * run_cmd ,
                      job_callback_ftype * done_callback,
                      job_callback_ftype * retry_callback,
                      job_callback_ftype * exit_callback,
                      void * callback_arg ,
                      int num_cpu ,
                      const char * run_path ,
                      const char * job_name ,
                      int argc ,
                      const char ** argv) {

  //Fail hard if queue is not open
  job_queue_check_open(queue);
  if (!queue->user_exit) {/* We do not accept new jobs if a user-shutdown has been iniated. */
    int queue_index;
    job_queue_node_type * node = job_queue_node_alloc( job_name ,
                                                       run_path ,
                                                       run_cmd ,
                                                       argc ,
                                                       argv ,
                                                       num_cpu ,
                                                       queue->ok_file ,
                                                       queue->exit_file,
                                                       done_callback ,
                                                       retry_callback ,
                                                       exit_callback ,
                                                       callback_arg );
    if (node) {
      job_list_get_wrlock( queue->job_list );
      {
        job_list_add_job( queue->job_list , node );
        queue_index = job_queue_node_get_queue_index(node);
        job_queue_change_node_status(queue , node , JOB_QUEUE_WAITING);
      }
      job_list_unlock( queue->job_list );
      return queue_index;   /* Handle used by the calling scope. */
    } else {
      char * cwd = util_alloc_cwd();
      util_abort("%s: failed to create job %s in path: %s cwd:%s \n",__func__ , job_name , run_path , cwd);
      return -1;
    }
  } else
    return -1;
}


UTIL_SAFE_CAST_FUNCTION( job_queue , JOB_QUEUE_TYPE_ID)


/**
   Observe that the job_queue returned by this function is NOT ready
   for use; a driver must be set explicitly with a call to
   job_queue_set_driver() first.
*/

job_queue_type * job_queue_alloc(int  max_submit               ,
                                 const char * ok_file ,
                                 const char * exit_file ) {



  job_queue_type * queue  = util_malloc(sizeof * queue );
  UTIL_TYPE_ID_INIT( queue , JOB_QUEUE_TYPE_ID);
  queue->usleep_time      = 250000; /* 1000000 : 1 second */
  queue->max_ok_wait_time = 60;
  queue->max_duration     = 0;
  queue->stop_time        = 0;
  queue->max_submit       = max_submit;
  queue->driver           = NULL;
  queue->ok_file          = util_alloc_string_copy( ok_file );
  queue->exit_file        = util_alloc_string_copy( exit_file );
  queue->open             = true;
  queue->user_exit        = false;
  queue->pause_on         = false;
  queue->running          = false;
  queue->submit_complete  = false;
  queue->work_pool        = NULL;
  queue->job_list         = job_list_alloc(  );
  queue->status           = job_queue_status_alloc( );

  pthread_mutex_init( &queue->run_mutex    , NULL );

  return queue;
}


/**
   When the job_queue_run_jobs() has been called with @total_num_jobs
   == 0 that means that the total number of jobs to run is not known
   in advance. In that case it is essential to signal the queue when
   we will not submit any more jobs, so that it can finalize and
   return. That is done with the function job_queue_submit_complete()
*/

void job_queue_submit_complete( job_queue_type * queue ){
  queue->submit_complete = true;
}



/**
   The calling scope must retain a handle to the current driver and
   free it.  Should (in principle) be possible to change driver on a
   running system whoaaa. Will read and update the max_running value
   from the driver.
*/

void job_queue_set_driver(job_queue_type * queue , queue_driver_type * driver) {
  queue->driver = driver;
}


bool job_queue_has_driver(const job_queue_type * queue ) {
  if (queue->driver == NULL)
    return false;
  else
    return true;
}


void job_queue_set_max_submit( job_queue_type * job_queue , int max_submit ) {
  job_queue->max_submit = max_submit;
}


int job_queue_get_max_submit(const job_queue_type * job_queue ) {
  return job_queue->max_submit;
}


/**
   Returns true if the queue is currently paused, which means that no
   more jobs are submitted.
*/

bool job_queue_get_pause( const job_queue_type * job_queue ) {
  return job_queue->pause_on;
}


void job_queue_set_pause_on( job_queue_type * job_queue) {
  job_queue->pause_on = true;
}


void job_queue_set_pause_off( job_queue_type * job_queue) {
  job_queue->pause_on = false;
}

/*
  An external thread sets the user_exit flag to true, then subsequently the
  thread managing the queue will see this, and close down the queue.
*/

void job_queue_user_exit( job_queue_type * queue) {
  queue->user_exit = true;
}



void job_queue_free(job_queue_type * queue) {
  util_safe_free( queue->ok_file );
  util_safe_free( queue->exit_file );
  job_list_free( queue->job_list );
  job_queue_status_free( queue->status );
  free(queue);
}






int job_queue_get_max_running_option(queue_driver_type * driver) {
  char * max_running_string = (char*)queue_driver_get_option(driver, MAX_RUNNING);
  int max_running;
  if (!util_sscanf_int(max_running_string, &max_running)) {
    fprintf(stderr, "%s: Unable to parse option MAX_RUNNING with value %s to an int", __func__, max_running_string);
  }
  return max_running;
}


void job_queue_set_max_running_option(queue_driver_type * driver, int max_running) {
  char * max_running_string = util_alloc_sprintf("%d", max_running);
  queue_driver_set_option(driver, MAX_RUNNING, max_running_string);
  free(max_running_string);
}


/**
   Observe that if the max number of running jobs is decreased,
   nothing will be done to reduce the number of jobs currently
   running; but no more jobs will be submitted until the number of
   running has fallen below the new limit.

   The updated value will also be pushed down to the current driver.

   NOTE: These next three *max_running functions should not be used, rather
   use the set_option feature, with MAX_RUNNING. They are (maybe) used by python
   therefore not removed.
*/
int job_queue_get_max_running( const job_queue_type * queue ) {
  return job_queue_get_max_running_option(queue->driver);
}

void job_queue_set_max_running( job_queue_type * queue , int max_running ) {
  job_queue_set_max_running_option(queue->driver, max_running);
}

/*
  The return value is the new value for max_running.
*/
int job_queue_inc_max_runnning( job_queue_type * queue, int delta ) {
  job_queue_set_max_running( queue , job_queue_get_max_running( queue ) + delta );
  return job_queue_get_max_running( queue );
}



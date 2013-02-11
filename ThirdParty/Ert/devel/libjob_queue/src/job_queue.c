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
#include <ert/job_queue/queue_driver.h>



#define JOB_QUEUE_START_SIZE 16

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
  Threads and such.
  =================

  The job_queue is executed with mulitple threads, and the potential for
  thread-related fuckups is immense. There are essentially two different scopes
  which acces the internal state of the queue concurrently:

    1. The function job_queue_run_jobs() is the main function administrating the
       queue, this includes starting and stopping jobs, and collecting the
       status of the various jobs. The thread running this function is the
       default 'owner' of the information in the job_queue instance.

    2. External scope can:
    
       o Query the status of the queue / individual jobs.        [Read access]

       o Issue commands to make the queue resubmit/kill/wait/... [Write access]
       
  Observe that there can be maaany concurrent invokations of the second
  type. Data structures which can change must obviously be protected with
  read/write locking, however scalars are NOT protected, i.e the two code blocks:


     ...
     state = new_value;
     ...

  and
  
     ...
     return state;

  can run concurrently. In principle we might risk that the return value from
  "return state;" is inconsistent, i.e. different from both new_value and the
  value state had prior to the statement "state = new_value;" - however all
  tests should be explicit so that such an inconsistency is actually OK.

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



typedef enum {SUBMIT_OK           = 0 , 
              SUBMIT_JOB_FAIL     = 1 , /* Typically no more attempts. */
              SUBMIT_DRIVER_FAIL  = 2 , /* The driver would not take the job - for whatever reason?? */ 
              SUBMIT_QUEUE_CLOSED = 3 } /* The queue is currently not accepting more jobs - either (temporarilty)
                                           because of pause or it is going down. */   submit_status_type;



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

struct job_queue_node_struct {
  job_status_type        job_status;      /* The current status of the job. */
  int                    submit_attempt;  /* Which attempt is this ... */
  int                    num_cpu;         /* How many cpu's will this job need - the driver is free to ignore if not relevant. */
  char                  *run_cmd;         /* The path to the actual executable. */
  char                  *exit_file;       /* The queue will look for the occurence of this file to detect a failure. */
  char                  *ok_file;         /* The queue will look for this file to verify that the job was OK - can be NULL - in which case it is ignored. */
  char                  *job_name;        /* The name of the job. */
  char                  *run_path;        /* Where the job is run - absolute path. */
  /*-----------------------------------------------------------------*/
  char                  *failed_job;      /* Name of the job (in the chain) which has failed. */
  char                  *error_reason;    /* The error message from the failed job. */
  char                  *stderr_capture;
  char                  *stderr_file;     /* Name of the file containing stderr information. */
  /*-----------------------------------------------------------------*/
  void                  *job_data;        /* Driver specific data about this job - fully handled by the driver. */
  int                    argc;            /* The number of commandline arguments to pass when starting the job. */ 
  char                 **argv;            /* The commandline arguments. */
  time_t                 submit_time;     /* When was the job added to job_queue - the FIRST TIME. */
  time_t                 sim_start;       /* When did the job change status -> RUNNING - the LAST TIME. */
  pthread_rwlock_t       job_lock;        /* This lock provides read/write locking of the job_data field. */ 
  job_callback_ftype    *done_callback;
  job_callback_ftype    *retry_callback;
  void                  *callback_arg;
};

static const int status_index[] = {  JOB_QUEUE_NOT_ACTIVE ,         
                                     JOB_QUEUE_WAITING    ,        
                                     JOB_QUEUE_SUBMITTED  ,        
                                     JOB_QUEUE_PENDING    ,         
                                     JOB_QUEUE_RUNNING    ,        
                                     JOB_QUEUE_DONE       ,        
                                     JOB_QUEUE_EXIT       ,        
                                     JOB_QUEUE_USER_KILLED ,        
                                     JOB_QUEUE_USER_EXIT   ,        
                                     JOB_QUEUE_SUCCESS    ,        
                                     JOB_QUEUE_RUNNING_CALLBACK,   
                                     JOB_QUEUE_FAILED };  

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



/*****************************************************************/


/**
   
   This is the struct for a whole queue. Observe the following:
   
    1. The number of elements is specified at the allocation time, and
       all nodes are allocated then; i.e. when xx_insert_job() is called
       from external scope a new node is not actaully created
       internally, it is just an existing node which is initialized.

    2. The queue can start running before all jobs are added.

*/

struct job_queue_struct {
  int                        active_size;                       /* The current number of job slots in the queue. */
  int                        alloc_size;                        /* The current allocated size of jobs array. */
  int                        max_submit;                        /* The maximum number of submit attempts for one job. */
  char                     * exit_file;                         /* The queue will look for the occurence of this file to detect a failure. */
  char                     * ok_file;                           /* The queue will look for this file to verify that the job was OK - can be NULL - in which case it is ignored. */
  job_queue_node_type     ** jobs;                              /* A vector of job nodes .*/
  queue_driver_type       * driver;                             /* A pointer to a driver instance (LSF|LOCAL|RSH) which actually 'does it'. */
  int                        status_list[JOB_QUEUE_MAX_STATE];  /* The number of jobs in the different states. */
  int                        old_status_list[JOB_QUEUE_MAX_STATE]; /* Should the display be updated ?? */

  bool                       user_exit;                         /* If there comes an external signal to abondond the whole thing user_exit will be set to true, and things start to dwindle down. */ 
  bool                       running;
  bool                       pause_on;
  bool                       submit_complete;
  bool                       grow;                              /* The function adding new jobs is requesting the job_queue function to grow the jobs array. */
  int                        max_ok_wait_time;                  /* Seconds to wait for an OK file - when the job itself has said all OK. */
  unsigned long              usleep_time;                       /* The sleep time before checking for updates. */
  pthread_mutex_t            status_mutex;                      /* This mutex ensure that the status-change code is only run by one thread. */
  pthread_mutex_t            run_mutex;                         /* This mutex is used to ensure that ONLY one thread is executing the job_queue_run_jobs(). */
  pthread_mutex_t            queue_mutex;
  thread_pool_type         * work_pool;
};

/*****************************************************************/

static void job_queue_grow( job_queue_type * queue );





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

/*
  int status_index = 0;
  int status = input_status;
  while ( (status != 1) && (status_index < JOB_QUEUE_MAX_STATE)) {
  status >>= 1;
  status_index++;
  }
  if (status != 1)
  util_abort("%s: failed to get index from status:%d \n",__func__ , status);
  return status_index;
}
*/



/*****************************************************************/

/*
  When the job script has detected failure it will create a "EXIT"
  file in the runpath directory; this function will inspect the EXIT
  file and determine which job has failed, the reason the job script
  has given to fail the job (typically missing TARGET_FILE) and
  capture the stderr from the job.  

  The file is XML formatted:

  ------------------------------------------------
  <error>
     <time>HH:MM:SS</time>
     <job> Name of job </job>
     <reason> Reason why the job failed </reason>
     <stderr>
        Capture of stderr from the job, can typically be
        a multiline string.
     </stderr> 
  </error>
  ------------------------------------------------

  This format is written by the dump_EXIT_file() function in the
  job_dispatch.py script.
*/

/* 
   This extremely half-assed XML "parsing" should of course be kept a
   secret...  
*/

static char * __alloc_tag_content( const char * xml_buffer , const char * tag) {
  char * open_tag    = util_alloc_sprintf("<%s>"  , tag);
  char * close_tag   = util_alloc_sprintf("</%s>" , tag);
  
  char * start_ptr   = strstr( xml_buffer , open_tag );
  char * end_ptr     = strstr( xml_buffer , close_tag );
  char * tag_content = NULL;

  if ((start_ptr != NULL) && (end_ptr != NULL)) {
    int length;
    start_ptr += strlen(open_tag);
    
    length = end_ptr - start_ptr;
    tag_content = util_alloc_substring_copy( start_ptr , 0 , length );
  } 

  free( open_tag );
  free( close_tag );
  return tag_content;
}


static void job_queue_node_free_error_info( job_queue_node_type * node ) {
  util_safe_free(node->error_reason);  
  util_safe_free(node->stderr_capture);  
  util_safe_free(node->stderr_file);  
  util_safe_free(node->failed_job);  
}



/**
   This code is meant to capture which of the jobs has failed; why it
   has failed and the stderr stream of the failing job. Depending on
   the failure circumstances the EXIT file might not be around.  
*/

static void job_queue_node_fscanf_EXIT( job_queue_node_type * node ) {
  job_queue_node_free_error_info( node );
  if (util_file_exists( node->exit_file )) {
    char * xml_buffer = util_fread_alloc_file_content( node->exit_file, NULL);
    
    node->failed_job     = __alloc_tag_content( xml_buffer , "job" );
    node->error_reason   = __alloc_tag_content( xml_buffer , "reason" );
    node->stderr_capture = __alloc_tag_content( xml_buffer , "stderr");
    node->stderr_file    = __alloc_tag_content( xml_buffer , "stderr_file");

    free( xml_buffer );
  } else
    node->failed_job = util_alloc_sprintf("EXIT file:%s not found - load failure?" , node->exit_file);
}



static void job_queue_node_clear_error_info(job_queue_node_type * node) {
  node->failed_job     = NULL;
  node->error_reason   = NULL;
  node->stderr_capture = NULL;  
  node->stderr_file    = NULL;  
  node->run_path       = NULL;
}



static void job_queue_node_clear(job_queue_node_type * node) {
  node->job_status          = JOB_QUEUE_NOT_ACTIVE;
  node->submit_attempt      = 0;
  node->job_name            = NULL;
  node->job_data            = NULL;
  node->exit_file           = NULL;
  node->ok_file             = NULL;
  node->run_cmd             = NULL;
  node->argc                = 0;
  node->argv                = NULL;
  
  node->retry_callback      = NULL;
  node->done_callback       = NULL;
  node->callback_arg        = NULL;
}


static job_queue_node_type * job_queue_node_alloc( ) {
  job_queue_node_type * node = util_malloc(sizeof * node );
  
  job_queue_node_clear(node);
  job_queue_node_clear_error_info(node);
  pthread_rwlock_init( &node->job_lock , NULL);

  return node;
}



static void job_queue_node_set_status(job_queue_node_type * node, job_status_type status) {
  node->job_status = status;
}

/*
  The error information is retained even after the job has completede
  completely, so that calling scope can ask for it - that is the
  reason there are separte free() and clear functions for the error related fields. 
*/
  
static void job_queue_node_free_data(job_queue_node_type * node) {
  util_safe_free( node->job_name );  
  util_safe_free( node->exit_file ); 
  util_safe_free( node->ok_file );   
  util_safe_free( node->run_cmd );   
  util_free_stringlist( node->argv , node->argc );
  if (node->job_data != NULL) 
    util_abort("%s: internal error - driver spesific job data has not been freed - will leak.\n",__func__);
}


static void job_queue_node_free(job_queue_node_type * node) {
  job_queue_node_free_data(node);
  job_queue_node_free_error_info(node);
  util_safe_free(node->run_path);  
  free(node);
}


static job_status_type job_queue_node_get_status(const job_queue_node_type * node) {
  return node->job_status;
}




static void job_queue_node_finalize(job_queue_node_type * node) {
  job_queue_node_free_data(node);
  job_queue_node_clear(node);
}



/*****************************************************************/

static bool job_queue_change_node_status(job_queue_type *  , job_queue_node_type *  , job_status_type );


static void job_queue_initialize_node(job_queue_type * queue , 
                                      const char * run_cmd , 
                                      job_callback_ftype * done_callback, 
                                      job_callback_ftype * retry_callback, 
                                      void * callback_arg , 
                                      int num_cpu , 
                                      const char * run_path , 
                                      const char * job_name , 
                                      int job_index , 
                                      int argc , 
                                      const char ** argv) {
  
  job_queue_node_type * node = queue->jobs[job_index];
  node->submit_attempt = 0;
  node->num_cpu        = num_cpu;
  node->job_name       = util_alloc_string_copy( job_name );
  node->job_data       = NULL;                                    /* The allocation is run in single thread mode - we assume. */
  node->argc           = argc;
  node->argv           = util_alloc_stringlist_copy( argv , argc );

  util_safe_free(node->run_path); // Might have a value from previous run.
  if (util_is_abs_path(run_path)) 
    node->run_path = util_alloc_string_copy( run_path );
  else
    node->run_path = util_alloc_realpath( run_path );
  
  if ( !util_is_directory(node->run_path) ) 
    util_abort("%s: the run_path: %s does not exist - aborting \n",__func__ , node->run_path);

  if (queue->exit_file != NULL)
      node->exit_file   = util_alloc_filename(node->run_path , queue->exit_file , NULL);
  if (queue->ok_file != NULL)
    node->ok_file     = util_alloc_filename(node->run_path , queue->ok_file   , NULL);
  node->run_cmd = util_alloc_string_copy( run_cmd );

  node->retry_callback = retry_callback;
  node->done_callback  = done_callback;
  node->callback_arg   = callback_arg;
  node->sim_start      = -1;
  node->submit_time    = time( NULL );

  /* Now the job is ready to be picked by the queue manager. */
  job_queue_change_node_status(queue , node , JOB_QUEUE_WAITING);   
}




static void job_queue_assert_queue_index(const job_queue_type * queue , int queue_index) {
  if (queue_index < 0 || queue_index >= queue->active_size) 
    util_abort("%s: invalid queue_index - internal error - aborting \n",__func__);
}



/**
   This function WILL be called by several threads concurrently; both
   directly from the thread running the job_queue_run_jobs() function,
   and indirectly thorugh exported functions like:

      job_queue_set_external_restart();
      job_queue_set_external_fail();
      ...

   It is therefor essential that only one thread is running this code
   at time.
*/

static bool job_queue_change_node_status(job_queue_type * queue , job_queue_node_type * node , job_status_type new_status) {
  bool status_change = false;
  pthread_mutex_lock( &queue->status_mutex );
  {
    job_status_type old_status = job_queue_node_get_status( node );
    
    if (new_status != old_status) {
      job_queue_node_set_status(node , new_status);
      queue->status_list[ STATUS_INDEX(old_status) ]--;
      queue->status_list[ STATUS_INDEX(new_status) ]++;
      
      if (new_status == JOB_QUEUE_RUNNING) 
        node->sim_start = time( NULL );
      status_change = true;

      if (new_status == JOB_QUEUE_FAILED)
        job_queue_node_fscanf_EXIT( node );
    }
  }
  pthread_mutex_unlock( &queue->status_mutex );
  return status_change;
}



/* 
   This frees the storage allocated by the driver - the storage
   allocated by the queue layer is retained. 

   In the case of jobs which are first marked as successfull by the
   queue layer, and then subsequently set to status EXIT by the
   DONE_callback this function will be called twice; i.e. we must
   protect against a double free.
*/

static void job_queue_free_job_driver_data(job_queue_type * queue , job_queue_node_type * node) {
  pthread_rwlock_wrlock( &node->job_lock );
  {
    if (node->job_data != NULL) 
      queue_driver_free_job( queue->driver , node->job_data );
    node->job_data = NULL;
  }
  pthread_rwlock_unlock( &node->job_lock );
}



/**
   Observe that this function should only query the driver for state
   change when the job is currently in one of the states: 

     JOB_QUEUE_WAITING || JOB_QUEUE_PENDING || JOB_QUEUE_RUNNING 

   The other state transitions are handled by the job_queue itself,
   without consulting the driver functions.
*/

/* 
   Will return true if the status has changed since the last time.
*/

static bool job_queue_update_status(job_queue_type * queue ) {
  bool update = false;
  queue_driver_type *driver  = queue->driver;
  int ijob;

  
  for (ijob = 0; ijob < queue->active_size; ijob++) {
    job_queue_node_type * node = queue->jobs[ijob];

    pthread_rwlock_rdlock( &node->job_lock );
    {
      if (node->job_data != NULL) {
        job_status_type current_status = job_queue_node_get_status(node);
        if (current_status & JOB_QUEUE_CAN_UPDATE_STATUS) {
          job_status_type new_status = queue_driver_get_status( driver , node->job_data);
          job_queue_change_node_status(queue , node , new_status);
        }
      }
    }
    pthread_rwlock_unlock( &node->job_lock );
  }
  
  /* Has the net status changed? */
  {
    int istat;
    for (istat = 0; istat  < JOB_QUEUE_MAX_STATE; istat++) {
      if (queue->old_status_list[istat] != queue->status_list[istat]) 
        update = true;
      queue->old_status_list[istat] = queue->status_list[istat];
    }
  }
  return update;
}



static submit_status_type job_queue_submit_job(job_queue_type * queue , int queue_index) {
  submit_status_type submit_status;
  if (queue->user_exit || queue->pause_on)
    submit_status = SUBMIT_QUEUE_CLOSED;   /* The queue is currently not accepting more jobs. */
  else {
    job_queue_assert_queue_index(queue , queue_index);
    {
      job_queue_node_type * node = queue->jobs[queue_index];
      void * job_data = queue_driver_submit_job( queue->driver  ,  
                                                 node->run_cmd  , 
                                                 node->num_cpu  , 
                                                 node->run_path , 
                                                 node->job_name , 
                                                 node->argc     , 
                                                 (const char **) node->argv );
      
      if (job_data != NULL) {
        pthread_rwlock_wrlock( &node->job_lock );
        {
          node->job_data = job_data;
          node->submit_attempt++;
          job_queue_change_node_status(queue , node , JOB_QUEUE_SUBMITTED ); 
          submit_status = SUBMIT_OK;
          /* 
             The status JOB_QUEUE_SUBMITTED is internal, and not exported anywhere. The job_queue_update_status() will
             update this to PENDING or RUNNING at the next call. The important difference between SUBMITTED and WAITING
             is that SUBMITTED have job_data != NULL and the job_queue_node free function must be called on it.
          */
        }
        pthread_rwlock_unlock( &node->job_lock );
      } else
        submit_status = SUBMIT_DRIVER_FAIL;
    }
  }
  return submit_status;
}






const char * job_queue_iget_run_path( const job_queue_type * queue , int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  return node->run_path;
}


const char * job_queue_iget_failed_job( const job_queue_type * queue , int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  return node->failed_job;
}


const char * job_queue_iget_error_reason( const job_queue_type * queue , int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  return node->error_reason;
}


const char * job_queue_iget_stderr_capture( const job_queue_type * queue , int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  return node->stderr_capture;
}


const char * job_queue_iget_stderr_file( const job_queue_type * queue , int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  return node->stderr_file;
}




job_status_type job_queue_iget_job_status(const job_queue_type * queue , int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  return node->job_status;
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
  return queue->status_list[ STATUS_INDEX( status ) ];
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

int job_queue_get_active_size( const job_queue_type * queue ) {
  return queue->active_size;
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
*/

bool job_queue_kill_job( job_queue_type * queue , int job_index) {
  bool result = false;
  job_queue_node_type * node = queue->jobs[job_index];
  pthread_rwlock_wrlock( &node->job_lock );
  {
    if (node->job_status & JOB_QUEUE_CAN_KILL) {
      queue_driver_type * driver = queue->driver;
      /* 
         Jobs with status JOB_QUEUE_WAITING are killable - in the sense that status should be set to
         JOB_QUEUE_USER_KILLED; but they do not have any driver specific job_data, and the driver->kill_job() function
         can NOT be called.
      */
      if (node->job_status != JOB_QUEUE_WAITING) { 
        queue_driver_kill_job( driver , node->job_data );
        queue_driver_free_job( driver , node->job_data );
        node->job_data = NULL;
      }
      job_queue_change_node_status( queue , node , JOB_QUEUE_USER_KILLED );
      result = true;
    }
  }
  pthread_rwlock_unlock( &node->job_lock );
  return result;
}



/**
   The external scope asks the queue to restart the the job; we reset
   the submit counter to zero. This function should typically be used
   in combination with resampling, however that is the responsability
   of the calling scope.
*/
   
void job_queue_iset_external_restart(job_queue_type * queue , int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  node->submit_attempt       = 0;
  job_queue_change_node_status( queue , node , JOB_QUEUE_WAITING );
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
  job_queue_node_type * node = queue->jobs[job_index];
  job_queue_change_node_status( queue , node , JOB_QUEUE_EXIT);
}



time_t job_queue_iget_sim_start( job_queue_type * queue, int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  return node->sim_start;
}

time_t job_queue_iget_submit_time( job_queue_type * queue, int job_index) {
  job_queue_node_type * node = queue->jobs[job_index];
  return node->submit_time;
}



static void job_queue_update_spinner( int * phase ) {
  const char * spinner = "-\\|/";
  int spinner_length   = strlen( spinner );

  printf("%c\b" , spinner[ (*phase % spinner_length) ]);
  fflush(stdout);
  (*phase) += 1;
}


static void job_queue_print_summary(const job_queue_type *queue, bool status_change ) {
  const char * status_fmt = "Waiting: %3d    Pending: %3d    Running: %3d    Checking/Loading: %3d    Failed: %3d    Complete: %3d   [ ]\b\b";
  int string_length       = 105;

  if (status_change) {
    for (int i=0; i < string_length; i++)
      printf("\b");
    {
      int waiting  = queue->status_list[ STATUS_INDEX(JOB_QUEUE_WAITING) ];
      int pending  = queue->status_list[ STATUS_INDEX(JOB_QUEUE_PENDING) ];
      
      /* 
         EXIT and DONE are included in "xxx_running", because the target
         file has not yet been checked.
      */
      int running  = queue->status_list[ STATUS_INDEX(JOB_QUEUE_RUNNING) ] + queue->status_list[ STATUS_INDEX(JOB_QUEUE_DONE) ] + queue->status_list[ STATUS_INDEX(JOB_QUEUE_EXIT) ];
      int complete = queue->status_list[ STATUS_INDEX(JOB_QUEUE_SUCCESS) ];
      int failed   = queue->status_list[ STATUS_INDEX(JOB_QUEUE_FAILED) ];
      int loading  = queue->status_list[ STATUS_INDEX(JOB_QUEUE_RUNNING_CALLBACK) ];  
      
      printf(status_fmt , waiting , pending , running , loading , failed , complete);
    }
  }
}






static void job_queue_clear_status( job_queue_type * queue ) {
  for (int i=0; i < JOB_QUEUE_MAX_STATE; i++) {
    queue->status_list[i] = 0;
    queue->old_status_list[i] = 0;
  }
}


/** 
    This function goes through all the nodes and call finalize on
    them. hat about jobs which were NOT in a CAN_KILL state when the
    killing was done, i.e. jobs which are in one of the intermediate
    load like states?? They 
*/

static void job_queue_finalize(job_queue_type * queue) {
  int i;
  
  for (i=0; i < queue->active_size; i++) 
    job_queue_node_finalize(queue->jobs[i]);
  
  job_queue_clear_status( queue );
  
  /*
      Be ready for the next run 
  */
  queue->grow            = false;
  queue->submit_complete = false;
  queue->pause_on        = false;
  queue->user_exit       = false;
  queue->active_size     = 0;
}


bool job_queue_is_running( const job_queue_type * queue ) {
  return queue->running;
}



static void job_queue_user_exit__( job_queue_type * queue ) {
  int queue_index;

  
  for (queue_index = 0; queue_index < queue->active_size; queue_index++) {
    job_queue_kill_job( queue , queue_index );
    job_queue_change_node_status( queue , queue->jobs[queue_index] , JOB_QUEUE_USER_EXIT);
  }
}


static bool job_queue_check_node_status_files( const job_queue_type * job_queue , job_queue_node_type * node) {
  if ((node->exit_file != NULL) && util_file_exists(node->exit_file)) 
    return false;                /* It has failed. */
  else {
    if (node->ok_file == NULL) 
      return true;               /* If the ok-file has not been set we just return true immediately. */
    else {
      int ok_sleep_time    =  1; /* Time to wait between checks for OK|EXIT file.                         */
      int  total_wait_time =  0;
      
      while (true) {
        if (util_file_exists( node->ok_file )) {
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
  job_queue_type * job_queue;
  job_queue_node_type * node;
  {
    arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
    job_queue = arg_pack_iget_ptr( arg_pack , 0 );
    node = arg_pack_iget_ptr( arg_pack , 1 );
    arg_pack_free( arg_pack );
  }
  job_queue_free_job_driver_data( job_queue , node );
  {
    bool OK = job_queue_check_node_status_files( job_queue , node );
    
    if (OK)
      if (node->done_callback != NULL)
        OK = node->done_callback( node->callback_arg );
    
    if (OK)
      job_queue_change_node_status( job_queue , node , JOB_QUEUE_SUCCESS );
    else
      job_queue_change_node_status( job_queue , node , JOB_QUEUE_EXIT );
  }
  return NULL;
}


static void job_queue_handle_DONE( job_queue_type * queue , job_queue_node_type * node) {
  job_queue_change_node_status(queue , node , JOB_QUEUE_RUNNING_CALLBACK );
  {
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , queue );
    arg_pack_append_ptr( arg_pack , node );
    thread_pool_add_job( queue->work_pool , job_queue_run_DONE_callback , arg_pack );
  }
}



static void * job_queue_run_EXIT_callback( void * arg ) {
  job_queue_type * job_queue;
  job_queue_node_type * node;
  {
    arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
    job_queue = arg_pack_iget_ptr( arg_pack , 0 );
    node = arg_pack_iget_ptr( arg_pack , 1 );
    arg_pack_free( arg_pack );
  }
  job_queue_free_job_driver_data( job_queue , node );

  if (node->submit_attempt < job_queue->max_submit) 
    job_queue_change_node_status( job_queue , node , JOB_QUEUE_WAITING );  /* The job will be picked up for antother go. */
  else {
    bool retry = false;
    if (node->retry_callback != NULL)
      retry = node->retry_callback( node->callback_arg );
    
    if (retry) {
      /* OK - we have invoked the retry_callback() - and that has returned true;
         giving this job a brand new start. */
      node->submit_attempt = 0;
      job_queue_change_node_status(job_queue , node , JOB_QUEUE_WAITING);
    } else
      job_queue_change_node_status(job_queue , node , JOB_QUEUE_FAILED);
  }
  return NULL;
}



static void job_queue_handle_EXIT( job_queue_type * queue , job_queue_node_type * node) {
  job_queue_change_node_status(queue , node , JOB_QUEUE_RUNNING_CALLBACK );
  {
    arg_pack_type * arg_pack = arg_pack_alloc();
    arg_pack_append_ptr( arg_pack , queue );
    arg_pack_append_ptr( arg_pack , node );
    thread_pool_add_job( queue->work_pool , job_queue_run_EXIT_callback , arg_pack );
  }
}



  
/**
   If the total number of jobs is not known in advance the job_queue_run_jobs
   function can be called with @num_total_run == 0. In that case it is paramount
   to call the function job_queue_submit_complete() whan all jobs have been submitted.
*/

void job_queue_run_jobs(job_queue_type * queue , int num_total_run, bool verbose) {
  int trylock = pthread_mutex_trylock( &queue->run_mutex );
  if (trylock != 0)
    util_abort("%s: another thread is already running the queue_manager\n",__func__);
  else {
    /* OK - we have got an exclusive lock to the run_jobs code. */
    const int NUM_WORKER_THREADS = 16;
    queue->running = true;
    queue->work_pool = thread_pool_alloc( NUM_WORKER_THREADS , true );
    {
      bool new_jobs         = false;
      bool cont             = true;
      int  phase = 0;
      
      do {
        bool local_user_exit = false;
        /*****************************************************************/
        if (queue->user_exit)  {/* An external thread has called the job_queue_user_exit() function, and we should kill
                                   all jobs, do some clearing up and go home. Observe that we will go through the
                                   queue handling codeblock below ONE LAST TIME before exiting. */
          job_queue_user_exit__( queue ); 
          local_user_exit = true;
        }
        /*****************************************************************/
        {
          bool update_status = job_queue_update_status( queue );
          if (verbose) {
            if (update_status || new_jobs)
              job_queue_print_summary(queue , update_status );
            job_queue_update_spinner( &phase );
          }
          
        
          {
            int num_complete = queue->status_list[ STATUS_INDEX(JOB_QUEUE_SUCCESS)   ] +   
                               queue->status_list[ STATUS_INDEX(JOB_QUEUE_FAILED)    ] +
                               queue->status_list[ STATUS_INDEX(JOB_QUEUE_USER_EXIT) ];

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
                if ((num_complete == queue->active_size) && queue->submit_complete)
                  cont = false;
              }
            }
          }
          
          if (cont) {
            /* Submitting new jobs */
            int max_submit     = 5; /* This is the maximum number of jobs submitted in one while() { ... } below. 
                                       Only to ensure that the waiting time before a status update is not too long. */
            int total_active   = queue->status_list[ STATUS_INDEX(JOB_QUEUE_PENDING) ] + queue->status_list[ STATUS_INDEX(JOB_QUEUE_RUNNING) ];
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
                num_submit_new = util_int_min( max_submit , queue->status_list[ STATUS_INDEX( JOB_QUEUE_WAITING )]);
            }
            
            new_jobs = false;
            if (queue->status_list[ STATUS_INDEX(JOB_QUEUE_WAITING) ] > 0)   /* We have waiting jobs at all           */
              if (num_submit_new > 0)                                        /* The queue can allow more running jobs */
                new_jobs = true;

            if (new_jobs) {
              int submit_count = 0;
              int queue_index  = 0;
            
              while ((queue_index < queue->active_size) && (num_submit_new > 0)) {
                job_queue_node_type * node = queue->jobs[queue_index];
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
                Checking for complete / exited jobs.
              */
              int queue_index;
              for (queue_index = 0; queue_index < queue->active_size; queue_index++) {
                job_queue_node_type * node = queue->jobs[queue_index];
                switch ( job_queue_node_get_status(node) ) {
                case(JOB_QUEUE_DONE):
                  job_queue_handle_DONE( queue , node );
                  break;
                case(JOB_QUEUE_EXIT):
                  job_queue_handle_EXIT( queue , node );
                  break;
                default:
                  break;
                }
              }
            }
            
            if (local_user_exit)
              cont = false;    /* This is how we signal that we want to get out . */

            if (queue->grow) 
              /* 
                 The add_job function has signaled that it needs more
                 job slots. We must grow the jobs array.
              */
              job_queue_grow( queue );
            else 
              if (!new_jobs && cont)
                util_usleep(queue->usleep_time);
          }
        }
        
      } while ( cont );
      queue->running = false;
    }
    if (verbose) 
      printf("\n");
    thread_pool_join( queue->work_pool );
    thread_pool_free( queue->work_pool );
  }
  job_queue_finalize( queue );
  pthread_mutex_unlock( &queue->run_mutex );
}



/*
  An external thread sets the user_exit flag to true, then subsequently the
  thread managing the queue will see this, and close down the queue.
*/

void job_queue_user_exit( job_queue_type * queue) {
  queue->user_exit = true;
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



/**
   The most flexible use scenario is as follows:

     1. The job_queue_run_jobs() is run by one thread.
     2. Jobs are added asyncronously with job_queue_add_job_mt() from othread threads(s).
     

   Unfortunately it does not work properly (i.e. Ctrl-C breaks) to use a Python
   thread to invoke the job_queue_run_jobs() function; and this function is
   mainly a workaround around that problem. The function will create a new
   thread and run job_queue_run_jobs() in that thread; the calling thread will
   just return.

   No reference is retained to the thread actually running the
   job_queue_run_jobs() function. 
*/

void job_queue_run_jobs_threaded(job_queue_type * queue , int num_total_run, bool verbose) {
  arg_pack_type * arg_pack = arg_pack_alloc();   /* The arg_pack will be freed in the job_queue_run_jobs__() function. */
  arg_pack_append_ptr( arg_pack , queue );
  arg_pack_append_int( arg_pack , num_total_run );
  arg_pack_append_bool( arg_pack , verbose );
  {
    pthread_t        queue_thread;
    pthread_create( &queue_thread , NULL , job_queue_run_jobs__ , arg_pack);
    pthread_detach( queue_thread );             /* Signal that the thread resources should be cleaned up when
                                                   the thread has exited. */
  }
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

   The internal data structure jobs will grow as needed when new jobs
   are added. Exactly how this growth takes place is regulated by the
   @mt parameter, and is very important to get right:

     mt == true: This means that we are running in multi threaded
        mode, and in particular another thread is already running the
        job_queue_run_jobs() function. In this case the
        job_queue_add_job__() function will only signal that it needs
        to grow the jobs array with the grow flag, and then block
        until the job_queue_run_jobs() function actually expands the
        array.

     mt == false: There is no other thread running the
        job_queue_run_jobs() function and it is safe for the
        job_queue_add_job__() function to manipulate the jobs array
        itself.
   
   Other thread running job_queue_run_jobs()      |   mt == true     |  Result
   ---------------------------------------------------------------------------
              Yes                                 |   Yes            | OK
              Yes                                 |   No             | Crash and burn  
              No                                  |   Yes            | Deadlock
              No                                  |   No             | OK
     ---------------------------------------------------------------------------
*/




static int job_queue_add_job__(job_queue_type * queue , 
                               const char * run_cmd , 
                               job_callback_ftype * done_callback, 
                               job_callback_ftype * retry_callback,
                               void * callback_arg , 
                               int num_cpu , 
                               const char * run_path , 
                               const char * job_name , 
                               int argc , 
                               const char ** argv, 
                               bool mt) {
  
  if (!queue->user_exit) {/* We do not accept new jobs if a user-shutdown has been iniated. */
    int job_index;        // This should be better protected lockwise
    
    pthread_mutex_lock( &queue->queue_mutex );
    {
      if (queue->active_size == queue->alloc_size) {
        if (mt) {
          queue->grow = true;  /* Signal to the thread running the queue that we need more job slots.
                                  Wait for the queue_size to increase; this will off course deadlock hard
                                  unless another thread is ready to pick up the signal to grow. */
          while (queue->active_size == queue->alloc_size) {
            sleep( 1 );
          }
        } else 
          /* 
             The function is called in single threaded mode, and we
             are certain that is safe for this thread to manipulate
             the jobs array directly.
          */
          job_queue_grow( queue );
      }

      job_index = queue->active_size;
      queue->active_size++;
    }
    pthread_mutex_unlock( &queue->queue_mutex );
    
    job_queue_initialize_node(queue , run_cmd , done_callback , retry_callback , callback_arg , num_cpu , run_path , job_name , job_index , argc , argv);
    return job_index;   /* Handle used by the calling scope. */
  } else
    return -1;
}


/**
   Adding a new job in multi-threaded mode, i.e. another thread is
   running the job_queue_run_jobs() function. 
*/ 
int job_queue_add_job_mt(job_queue_type * queue , 
                         const char * run_cmd , 
                         job_callback_ftype * done_callback, 
                         job_callback_ftype * retry_callback, 
                         void * callback_arg , 
                         int num_cpu , 
                         const char * run_path , 
                         const char * job_name , 
                         int argc , 
                         const char ** argv) { 
  return job_queue_add_job__(queue , run_cmd , done_callback, retry_callback , callback_arg , num_cpu , run_path , job_name , argc , argv , true); 
}


/**
   Adding a new job in single-threaded mode, i.e. no another thread is
   accessing the queue. 
*/ 

int job_queue_add_job_st(job_queue_type * queue , 
                         const char * run_cmd , 
                         job_callback_ftype * done_callback, 
                         job_callback_ftype * retry_callback, 
                         void * callback_arg , 
                         int num_cpu , 
                         const char * run_path , 
                         const char * job_name , 
                         int argc , 
                         const char ** argv) {
  return job_queue_add_job__(queue , run_cmd , done_callback , retry_callback , callback_arg , num_cpu , run_path , job_name , argc , argv , false);
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



/**
   Observe that if the max number of running jobs is decreased,
   nothing will be done to reduce the number of jobs currently
   running; but no more jobs will be submitted until the number of
   running has fallen below the new limit.

   The updated value will also be pushed down to the current driver.
*/

void job_queue_set_max_running( job_queue_type * queue , int max_running ) {
  queue_driver_set_max_running( queue->driver , max_running );
}

/*
  The return value is the new value for max_running.
*/
int job_queue_inc_max_runnning( job_queue_type * queue, int delta ) {
  job_queue_set_max_running( queue , job_queue_get_max_running( queue ) + delta );
  return job_queue_get_max_running( queue );
}

int job_queue_get_max_running( const job_queue_type * queue ) {
  return queue_driver_get_max_running( queue->driver );
}


/*****************************************************************/


job_driver_type job_queue_lookup_driver_name( const char * driver_name ) {
  if (strcmp( driver_name , "LOCAL") == 0)
    return LOCAL_DRIVER;
  else if (strcmp( driver_name , "RSH") == 0)
    return RSH_DRIVER;
  else if (strcmp( driver_name , "LSF") == 0)
    return LSF_DRIVER;
  else {
    util_abort("%s: driver:%s not recognized \n",__func__ , driver_name);
    return NULL_DRIVER;
  }
}

/*****************************************************************/



void job_queue_set_max_submit( job_queue_type * job_queue , int max_submit ) {
  job_queue->max_submit = max_submit;
}


int job_queue_get_max_submit(const job_queue_type * job_queue ) {
  return job_queue->max_submit;
}



static void job_queue_grow( job_queue_type * queue ) {
  int alloc_size                  = util_int_max( 2 * queue->alloc_size , JOB_QUEUE_START_SIZE );
  job_queue_node_type ** new_jobs = util_calloc(alloc_size , sizeof * queue->jobs );
  job_queue_node_type ** old_jobs = queue->jobs;
  if (old_jobs != NULL)
    memcpy( new_jobs , queue->jobs , queue->alloc_size * sizeof * queue->jobs );
  {
    int i;
    /* Creating the new nodes. */
    for (i = queue->alloc_size; i < alloc_size; i++) 
      new_jobs[i] = job_queue_node_alloc();
    
    /* Assigning the job pointer to the new array. */
    queue->jobs       = new_jobs;

    /* Free the old array - only the pointers, not the actual nodes! */
    util_safe_free( old_jobs );
    
    /* Update the status with the new nodes. */
    for (i=queue->alloc_size; i < alloc_size; i++) 
      queue->status_list[ STATUS_INDEX(job_queue_node_get_status(queue->jobs[i])) ]++;

    queue->alloc_size = alloc_size;
  }
  queue->grow = false;
}


/**
   Observe that the job_queue returned by this function is NOT ready
   for use; a driver must be set explicitly with a call to
   job_queue_set_driver() first.  
*/

job_queue_type * job_queue_alloc(int  max_submit               ,            
                                 const char * ok_file , 
                                 const char * exit_file ) {

                                 

  job_queue_type * queue  = util_malloc(sizeof * queue );
  queue->jobs             = NULL;
  queue->usleep_time      = 250000; /* 1000000 : 1 second */
  queue->max_ok_wait_time = 60;     
  queue->max_submit       = max_submit;
  queue->driver           = NULL;
  queue->ok_file          = util_alloc_string_copy( ok_file );
  queue->exit_file        = util_alloc_string_copy( exit_file );
  queue->user_exit        = false;
  queue->pause_on         = false;
  queue->running          = false;
  queue->grow             = false;
  queue->submit_complete  = false;
  queue->active_size      = 0;
  queue->alloc_size       = 0;
  queue->jobs             = NULL;
  queue->work_pool        = NULL;
  job_queue_grow( queue );

  job_queue_clear_status( queue );
  pthread_mutex_init( &queue->status_mutex , NULL);
  pthread_mutex_init( &queue->queue_mutex  , NULL);
  pthread_mutex_init( &queue->run_mutex    , NULL );

  
  
  return queue;
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


void * job_queue_iget_job_data( job_queue_type * job_queue , int job_nr ) {
  job_queue_node_type * job = job_queue->jobs[ job_nr ];
  return job->job_data;
}


job_queue_node_type * job_queue_iget_job( job_queue_type * job_queue , int job_nr ) {
  job_queue_node_type * job = job_queue->jobs[ job_nr ];
  return job;
}



void job_queue_free(job_queue_type * queue) {
  util_safe_free( queue->ok_file );
  util_safe_free( queue->exit_file );
  {
    int i;
    for (i=0; i < queue->active_size; i++) 
      job_queue_node_free(queue->jobs[i]);
    free(queue->jobs);
  }
  free(queue);
  queue = NULL;
}


/*****************************************************************/

const char * job_queue_status_name( job_status_type status ) {
  return status_name[ STATUS_INDEX(status) ];
}


/*****************************************************************/

job_status_type job_queue_get_status( queue_driver_type * driver , job_queue_node_type * job) {
  return queue_driver_get_status( driver , job->job_data );
}


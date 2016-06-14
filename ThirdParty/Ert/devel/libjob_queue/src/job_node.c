/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'job_node.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/type_macros.h>

#include <ert/job_queue/job_node.h>

#define JOB_QUEUE_NODE_TYPE_ID 3315299
#define INVALID_QUEUE_INDEX    -999

struct job_queue_node_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                    num_cpu;         /* How many cpu's will this job need - the driver is free to ignore if not relevant. */
  char                  *run_cmd;         /* The path to the actual executable. */
  char                  *exit_file;       /* The queue will look for the occurence of this file to detect a failure. */
  char                  *ok_file;         /* The queue will look for this file to verify that the job was OK - can be NULL - in which case it is ignored. */
  char                  *status_file;     /* The queue will look for this file to verify that the job is running or has run. */
  char                  *job_name;        /* The name of the job. */
  char                  *run_path;        /* Where the job is run - absolute path. */
  job_callback_ftype    *done_callback;
  job_callback_ftype    *retry_callback;  /* To determine if job can be retried */
  job_callback_ftype    *exit_callback;   /* Callback to perform any cleanup */
  void                  *callback_arg;
  int                    argc;            /* The number of commandline arguments to pass when starting the job. */
  char                 **argv;            /* The commandline arguments. */
  int                    queue_index;

  /*-----------------------------------------------------------------*/
  char                  *failed_job;      /* Name of the job (in the chain) which has failed. */
  char                  *error_reason;    /* The error message from the failed job. */
  char                  *stderr_capture;
  char                  *stderr_file;     /* Name of the file containing stderr information. */
  /*-----------------------------------------------------------------*/

  int                    submit_attempt;  /* Which attempt is this ... */
  job_status_type        job_status;      /* The current status of the job. */
  bool                   confirmed_running;/* Set to true if file status_file has been detected written. */
  pthread_mutex_t        data_mutex;      /* Protecting the access to the job_data pointer. */
  void                  *job_data;        /* Driver specific data about this job - fully handled by the driver. */
  time_t                 submit_time;     /* When was the job added to job_queue - the FIRST TIME. */
  time_t                 sim_start;       /* When did the job change status -> RUNNING - the LAST TIME. */
  time_t                 sim_end ;        /* When did the job finish successfully */
  time_t                 max_confirm_wait;/* Max waiting between sim_start and confirmed_running is 2 minutes */
};



void job_queue_node_free_error_info( job_queue_node_type * node ) {
  util_safe_free(node->error_reason);
  util_safe_free(node->stderr_capture);
  util_safe_free(node->stderr_file);
  util_safe_free(node->failed_job);
}



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




/**
   This code is meant to capture which of the jobs has failed; why it
   has failed and the stderr stream of the failing job. Depending on
   the failure circumstances the EXIT file might not be around.
*/

void job_queue_node_fscanf_EXIT( job_queue_node_type * node ) {
  job_queue_node_free_error_info( node );
  if (node->exit_file) {
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
}






UTIL_IS_INSTANCE_FUNCTION( job_queue_node , JOB_QUEUE_NODE_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION( job_queue_node , JOB_QUEUE_NODE_TYPE_ID )



int job_queue_node_get_queue_index( const job_queue_node_type * node ) {
  if (node->queue_index == INVALID_QUEUE_INDEX)
    util_abort("%s: internal error: asked for not-yet-initialized node->queue_index\n",__func__);
  return node->queue_index;
}

void job_queue_node_set_queue_index( job_queue_node_type * node , int queue_index) {
  if (node->queue_index == INVALID_QUEUE_INDEX)
    node->queue_index = queue_index;
  else
    util_abort("%s: internal error: atteeempt to reset queue_index \n",__func__);
}


/*
 The error information is retained even after the job has completed
 completely, so that calling scope can ask for it - that is the
 reason there are separate free() and clear functions for the error related fields.
*/

void job_queue_node_free_data(job_queue_node_type * node) {
  util_safe_free( node->job_name );
  util_safe_free( node->exit_file );
  util_safe_free( node->ok_file );
  util_safe_free( node->run_cmd );
  util_free_stringlist( node->argv , node->argc );

  if (node->job_data != NULL)
    util_abort("%s: internal error - driver spesific job data has not been freed - will leak.\n",__func__);
}


void job_queue_node_free(job_queue_node_type * node) {
  job_queue_node_free_data(node);
  job_queue_node_free_error_info(node);
  util_safe_free(node->run_path);

  // Since the type of the callback_arg is void* it should maybe be
  // registered with a private destructor - or the type should be
  // changed to arg_pack?
  if (arg_pack_is_instance( node->callback_arg ))
      arg_pack_free( node->callback_arg );

  free(node);
}


job_status_type job_queue_node_get_status(const job_queue_node_type * node) {
  return node->job_status;
}






/******************************************************************/
/*
   These four functions all require that the caller has aquired the
   data lock before entering.
*/





void job_queue_node_reset_submit_attempt( job_queue_node_type * node) {
  node->submit_attempt = 0;
}

int job_queue_node_get_submit_attempt( const job_queue_node_type * node) {
  return node->submit_attempt;
}









job_queue_node_type * job_queue_node_alloc_simple( const char * job_name ,
                                                   const char * run_path ,
                                                   const char * run_cmd ,
                                                   int argc ,
                                                   const char ** argv) {
  return job_queue_node_alloc( job_name , run_path , run_cmd , argc , argv , 1, NULL , NULL, NULL, NULL, NULL, NULL, NULL);
}


job_queue_node_type * job_queue_node_alloc( const char * job_name ,
                                            const char * run_path ,
                                            const char * run_cmd ,
                                            int argc ,
                                            const char ** argv,
                                            int num_cpu,
                                            const char * ok_file,
                                            const char * status_file,
                                            const char * exit_file,
                                            job_callback_ftype * done_callback,
                                            job_callback_ftype * retry_callback,
                                            job_callback_ftype * exit_callback,
                                            void * callback_arg) {

  if (util_is_directory( run_path )) {
    job_queue_node_type * node = util_malloc(sizeof * node );

    UTIL_TYPE_ID_INIT( node , JOB_QUEUE_NODE_TYPE_ID );
    {
      /* The data initialized in this block should *NEVER* change. */
      node->job_name       = util_alloc_string_copy( job_name );

      if (util_is_abs_path(run_path))
        node->run_path = util_alloc_string_copy( run_path );
      else
        node->run_path = util_alloc_realpath( run_path );

      node->run_cmd        = util_alloc_string_copy( run_cmd );
      node->argc           = argc;
      node->argv           = util_alloc_stringlist_copy( argv , argc );
      node->num_cpu        = num_cpu;

      if (ok_file)
        node->ok_file = util_alloc_filename(node->run_path , ok_file , NULL);
      else
        node->ok_file = NULL;

      if (status_file)
        node->status_file = util_alloc_filename(node->run_path , status_file , NULL);
      else
        node->status_file = NULL;
      node->confirmed_running = false;

      if (exit_file)
        node->exit_file = util_alloc_filename(node->run_path , exit_file , NULL);
      else
        node->exit_file = NULL;

      node->exit_callback  = exit_callback;
      node->retry_callback = retry_callback;
      node->done_callback  = done_callback;
      node->callback_arg   = callback_arg;
    }
    {
      node->error_reason   = NULL;
      node->stderr_capture = NULL;
      node->stderr_file    = NULL;
      node->failed_job     = NULL;
    }
    {
      node->job_status     = JOB_QUEUE_NOT_ACTIVE;
      node->queue_index    = INVALID_QUEUE_INDEX;
      node->submit_attempt = 0;
      node->job_data       = NULL;                                    /* The allocation is run in single thread mode - we assume. */
      node->sim_start      = 0;
      node->sim_end        = 0;
      node->submit_time    = time( NULL );
      node->max_confirm_wait= 60*2; /* 2 minutes before we consider job dead. */
    }

    pthread_mutex_init( &node->data_mutex , NULL );
    return node;
  } else
    return NULL;
}


const char * job_queue_node_get_error_reason( const job_queue_node_type * node) {
  return node->error_reason;
}

const char * job_queue_node_get_stderr_capture( const job_queue_node_type * node) {
  return node->stderr_capture;
}


const char * job_queue_node_get_stderr_file( const job_queue_node_type * node) {
  return node->stderr_file;
}


const char * job_queue_node_get_exit_file( const job_queue_node_type * node) {
  return node->exit_file;
}


const char * job_queue_node_get_ok_file( const job_queue_node_type * node) {
  return node->ok_file;
}

const char * job_queue_node_get_status_file( const job_queue_node_type * node) {
  return node->status_file;
}

const char * job_queue_node_get_run_path( const job_queue_node_type * node) {
  return node->run_path;
}

const char * job_queue_node_get_name( const job_queue_node_type * node) {
  return node->job_name;
}

const char * job_queue_node_get_failed_job( const job_queue_node_type * node) {
  return node->failed_job;
}


time_t job_queue_node_get_sim_start( const job_queue_node_type * node ) {
  return node->sim_start;
}


time_t job_queue_node_get_sim_end( const job_queue_node_type * node ) {
  return node->sim_end;
}

time_t job_queue_node_get_submit_time( const job_queue_node_type * node ) {
  return node->submit_time;
}

double job_queue_node_time_since_sim_start (const job_queue_node_type * node ) {
  return util_difftime_seconds( node->sim_start , time(NULL));
}

bool job_queue_node_run_DONE_callback( job_queue_node_type * node ) {
  bool OK = true;
  if (node->done_callback)
    OK = node->done_callback( node->callback_arg );

  return OK;
}


bool job_queue_node_run_RETRY_callback( job_queue_node_type * node ) {
  bool retry = false;
  if (node->retry_callback)
    retry = node->retry_callback( node->callback_arg );

  return retry;
}


void job_queue_node_run_EXIT_callback( job_queue_node_type * node ) {
  if (node->exit_callback)
    node->exit_callback( node->callback_arg );
}

static void job_queue_node_set_status(job_queue_node_type * node , job_status_type new_status) {
  if (new_status != node->job_status) {
    node->job_status = new_status;

    /*
       We record sim start when the node is in state JOB_QUEUE_WAITING
       to be sure that we do not miss the start time completely for
       very fast jobs which are registered in the state
       JOB_QUEUE_RUNNING.
    */
    if (new_status == JOB_QUEUE_WAITING)
      node->sim_start = time( NULL );

    if (new_status == JOB_QUEUE_RUNNING)
      node->sim_start = time( NULL );

    if (new_status == JOB_QUEUE_SUCCESS)
      node->sim_end = time( NULL );

    if (new_status == JOB_QUEUE_FAILED)
      job_queue_node_fscanf_EXIT( node );

  }
}


submit_status_type job_queue_node_submit( job_queue_node_type * node , job_queue_status_type * status , queue_driver_type * driver) {
  submit_status_type submit_status;
  pthread_mutex_lock( &node->data_mutex );
  {
    void * job_data = queue_driver_submit_job( driver,
                                               node->run_cmd,
                                               node->num_cpu,
                                               node->run_path,
                                               node->job_name,
                                               node->argc,
                                               (const char **) node->argv);
    if (job_data != NULL) {
      job_status_type old_status = node->job_status;
      job_status_type new_status = JOB_QUEUE_SUBMITTED;

      node->job_data = job_data;
      node->submit_attempt++;
      /*
        The status JOB_QUEUE_SUBMITTED is internal, and not
        exported anywhere. The job_queue_update_status() will
        update this to PENDING or RUNNING at the next call. The
        important difference between SUBMITTED and WAITING is
        that SUBMITTED have job_data != NULL and the
        job_queue_node free function must be called on it.
      */
      submit_status = SUBMIT_OK;
      job_queue_node_set_status( node , new_status);
      job_queue_status_transition(status, old_status, new_status);
    } else
      /*
        In this case the status of the job itself will be
        unmodified; i.e. it will still be WAITING, and a new attempt
        to submit it will be performed in the next round.
      */
      submit_status = SUBMIT_DRIVER_FAIL;
  }
  pthread_mutex_unlock( &node->data_mutex );
  return submit_status;
}

static bool job_queue_node_status_update_confirmed_running__(job_queue_node_type * node) {
  if (node->confirmed_running)
      return true;

  if (!node->status_file) {
    node->confirmed_running = true;
    return true;
  }

  if (util_file_exists(node->status_file))
    node->confirmed_running = true;
  return node->confirmed_running;
}

// if status = running, and current_time > sim_start + max_confirm_wait
// (usually 2 min), check if job is confirmed running (status_file exists).
// If not confirmed, set job to JOB_QUEUE_FAILED.
bool job_queue_node_update_status( job_queue_node_type * node , job_queue_status_type * status , queue_driver_type * driver ) {
  bool status_change = false;
  pthread_mutex_lock(&node->data_mutex);
  {
    if (node->job_data) {
      job_status_type current_status = job_queue_node_get_status(node);

      bool confirmed = job_queue_node_status_update_confirmed_running__(node);

      if ((current_status & JOB_QUEUE_RUNNING) && !confirmed) {
        // it's running, but not confirmed running.
        double runtime = job_queue_node_time_since_sim_start(node);
        if (runtime >= node->max_confirm_wait) {
          // max_confirm_wait has passed since sim_start without success; the job is dead
          job_status_type new_status = JOB_QUEUE_EXIT;
          status_change = job_queue_status_transition(status, current_status, new_status);
          job_queue_node_set_status(node, new_status);
        }
      }
      current_status = job_queue_node_get_status(node);
      if (current_status & JOB_QUEUE_CAN_UPDATE_STATUS) {
        job_status_type new_status = queue_driver_get_status( driver , node->job_data);
        status_change = job_queue_status_transition(status , current_status , new_status);
        job_queue_node_set_status(node,new_status);
      }
    }
  }
  pthread_mutex_unlock( &node->data_mutex );
  return status_change;
}

bool job_queue_node_status_transition( job_queue_node_type * node , job_queue_status_type * status , job_status_type new_status) {
  bool status_change;
  pthread_mutex_lock( &node->data_mutex );
  {
    job_status_type old_status = job_queue_node_get_status( node );
    status_change = job_queue_status_transition(status , old_status, new_status);

    if (status_change)
      job_queue_node_set_status( node , new_status );
  }
  pthread_mutex_unlock( &node->data_mutex );
  return status_change;
}

void job_queue_node_set_max_confirmation_wait_time(job_queue_node_type * node, time_t time) {
  node->max_confirm_wait = time;
}



bool job_queue_node_status_confirmed_running(job_queue_node_type * node) {
  return node->confirmed_running;
}


bool job_queue_node_kill( job_queue_node_type * node , job_queue_status_type * status , queue_driver_type * driver) {
  bool result = false;
  pthread_mutex_lock( &node->data_mutex );
  {
    job_status_type current_status = job_queue_node_get_status( node );
    if (current_status & JOB_QUEUE_CAN_KILL) {
      /*
        If the job is killed before it is even started no driver
        specific job data has been assigned; we therefor must check
        the node->job_data pointer before entering.
      */
      if (node->job_data) {
        queue_driver_kill_job( driver , node->job_data );
        queue_driver_free_job( driver , node->job_data );
        node->job_data = NULL;
      }
      job_queue_status_transition(status, current_status, JOB_QUEUE_USER_KILLED);
      job_queue_node_set_status( node , JOB_QUEUE_USER_KILLED);
      result = true;
    }
  }
  pthread_mutex_unlock( &node->data_mutex );
  return result;
}


/*
   This frees the storage allocated by the driver - the storage
   allocated by the queue layer is retained.

   In the case of jobs which are first marked as successfull by the
   queue layer, and then subsequently set to status EXIT by the
   DONE_callback this function will be called twice; i.e. we must
   protect against a double free.
*/

void job_queue_node_free_driver_data( job_queue_node_type * node , queue_driver_type * driver) {
  pthread_mutex_lock( &node->data_mutex );
  {
    if (node->job_data != NULL)
      queue_driver_free_job( driver , node->job_data );
    node->job_data = NULL;
  }
  pthread_mutex_unlock( &node->data_mutex );
}


/*
  This returns a pointer to a very internal datastructure; used by the
  Job class in Python which interacts directly with the driver
  implementation. This is too low level, and the whole Driver / Job
  implementation in Python should be changed to only expose the higher
  level queue class.
*/

void * job_queue_node_get_driver_data( job_queue_node_type * node ) {
  return node->job_data;
}


void job_queue_node_restart( job_queue_node_type * node , job_queue_status_type * status) {
  pthread_mutex_lock( &node->data_mutex );
  {
    job_status_type current_status = job_queue_node_get_status( node );
    job_queue_status_transition(status, current_status, JOB_QUEUE_WAITING);
    job_queue_node_set_status( node , JOB_QUEUE_WAITING);
    job_queue_node_reset_submit_attempt(node);
  }
  pthread_mutex_unlock( &node->data_mutex );
}

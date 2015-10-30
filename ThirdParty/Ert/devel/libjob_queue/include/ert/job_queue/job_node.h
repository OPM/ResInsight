/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'job_node.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef __JOB_NODE_H__
#define __JOB_NODE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/job_queue_status.h>

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

typedef bool (job_callback_ftype)   (void *);
typedef struct job_queue_node_struct job_queue_node_type;


  bool job_queue_node_status_transition( job_queue_node_type * node , job_queue_status_type * status , job_status_type new_status);
  submit_status_type job_queue_node_submit( job_queue_node_type * node , job_queue_status_type * status , queue_driver_type * driver);
  void job_queue_node_free_error_info( job_queue_node_type * node );
  void job_queue_node_fscanf_EXIT( job_queue_node_type * node );
  void job_queue_node_clear_error_info(job_queue_node_type * node);
  void job_queue_node_clear(job_queue_node_type * node);
  void job_queue_node_free_data(job_queue_node_type * node);
  job_queue_node_type * job_queue_node_alloc( const char * job_name ,
                                              const char * run_path ,
                                              const char * run_cmd ,
                                              int argc ,
                                              const char ** argv ,
                                              int num_cpu ,
                                              const char * ok_file,
                                              const char * exit_file,
                                              job_callback_ftype * done_callback,
                                              job_callback_ftype * retry_callback,
                                              job_callback_ftype * exit_callback,
                                              void * callback_arg);


  job_queue_node_type * job_queue_node_alloc_simple( const char * job_name ,
                                                     const char * run_path ,
                                                     const char * run_cmd ,
                                                     int argc ,
                                                     const char ** argv );

  bool job_queue_node_kill( job_queue_node_type * node , job_queue_status_type * status , queue_driver_type * driver);
  void job_queue_node_free(job_queue_node_type * node);
  job_status_type job_queue_node_get_status(const job_queue_node_type * node);
  void job_queue_node_free_driver_data( job_queue_node_type * node , queue_driver_type * driver);
  void job_queue_node_restart( job_queue_node_type * node , job_queue_status_type * status);
  bool job_queue_node_update_status( job_queue_node_type * node , job_queue_status_type * status , queue_driver_type * driver);

  const char * job_queue_node_get_run_path( const job_queue_node_type * node);
  const char * job_queue_node_get_name( const job_queue_node_type * node);
  int  job_queue_node_get_submit_attempt( const job_queue_node_type * node);
  void job_queue_node_reset_submit_attempt( job_queue_node_type * node);
  const char * job_queue_node_get_failed_job( const job_queue_node_type * node);
  const char * job_queue_node_get_error_reason( const job_queue_node_type * node);
  const char * job_queue_node_get_stderr_capture( const job_queue_node_type * node);
  const char * job_queue_node_get_stderr_file( const job_queue_node_type * node);

  time_t job_queue_node_get_sim_start( const job_queue_node_type * node );
  time_t job_queue_node_get_sim_end( const job_queue_node_type * node );
  time_t job_queue_node_get_submit_time( const job_queue_node_type * node );

  const char * job_queue_node_get_ok_file( const job_queue_node_type * node);
  const char * job_queue_node_get_exit_file( const job_queue_node_type * node);

  bool job_queue_node_run_DONE_callback( job_queue_node_type * node );
  bool job_queue_node_run_RETRY_callback( job_queue_node_type * node );
  void job_queue_node_run_EXIT_callback( job_queue_node_type * node );
  int job_queue_node_get_queue_index( const job_queue_node_type * node );
  void job_queue_node_set_queue_index( job_queue_node_type * node , int queue_index);

  UTIL_IS_INSTANCE_HEADER( job_queue_node );
  UTIL_SAFE_CAST_HEADER( job_queue_node );

#ifdef __cplusplus
}
#endif
#endif

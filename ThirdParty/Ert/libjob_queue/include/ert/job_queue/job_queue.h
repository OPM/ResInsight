/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'job_queue.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_JOB_QUEUE_H
#define ERT_JOB_QUEUE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include <stdbool.h>

#include <ert/util/path_fmt.h>

#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/job_node.h>


  typedef struct job_queue_struct      job_queue_type;


  void                job_queue_submit_complete( job_queue_type * queue );
  job_driver_type     job_queue_get_driver_type( const job_queue_type * queue );
  void                job_queue_set_driver(job_queue_type * queue , queue_driver_type * driver);
  bool                job_queue_has_driver(const job_queue_type * queue );
  //void                job_queue_set_size( job_queue_type * job_queue , int size );
  void                job_queue_set_runpath_fmt(job_queue_type *  , const path_fmt_type * );
  job_queue_type   *  job_queue_alloc( int  , const char * ok_file , const char * status_file, const char * exit_file);
  void                job_queue_free(job_queue_type *);

  int                 job_queue_add_job(job_queue_type * ,
                                        const char * run_cmd ,
                                        job_callback_ftype * done_callback,
                                        job_callback_ftype * retry_callback,
                                        job_callback_ftype * exit_callback,
                                        void * callback_arg ,
                                        int num_cpu ,
                                        const char * ,
                                        const char * ,
                                        int argc ,
                                        const char ** argv );

  bool                job_queue_accept_jobs(const job_queue_type * queue);
  void                job_queue_reset(job_queue_type * queue);
  void                job_queue_run_jobs(job_queue_type * queue, int num_total_run, bool verbose);
  void                job_queue_run_jobs_threaded(job_queue_type * queue , int num_total_run, bool verbose);
  void *              job_queue_run_jobs__(void * );
  void                job_queue_start_manager_thread( job_queue_type * job_queue , pthread_t * queue_thread , int job_size , bool verbose);

  job_status_type     job_queue_iget_job_status(job_queue_type * , int );

  int                 job_queue_iget_status_summary( const job_queue_type * queue , job_status_type status);
  time_t              job_queue_iget_sim_start( job_queue_type * queue, int job_index);
  time_t              job_queue_iget_sim_end( job_queue_type * queue, int job_index);
  time_t              job_queue_iget_submit_time( job_queue_type * queue, int job_index);
  void                job_queue_iset_max_confirm_wait_time( job_queue_type * queue, int job_index, time_t time );

  void                job_queue_set_max_job_duration(job_queue_type * queue, int max_duration_seconds);
  int                 job_queue_get_max_job_duration(const job_queue_type * queue);
  void                job_queue_set_job_stop_time(job_queue_type * queue, time_t time);
  time_t              job_queue_get_job_stop_time(const job_queue_type * queue);
  void                job_queue_set_auto_job_stop_time(job_queue_type * queue);
  bool                job_queue_kill_job( job_queue_type * queue , int job_index);
  bool                job_queue_is_running( const job_queue_type * queue );
  void                job_queue_set_max_submit( job_queue_type * job_queue , int max_submit );
  int                 job_queue_get_max_submit(const job_queue_type * job_queue );
  bool                job_queue_get_open(const job_queue_type * job_queue);
  bool                job_queue_get_pause( const job_queue_type * job_queue );
  void                job_queue_set_pause_on( job_queue_type * job_queue);
  void                job_queue_set_pause_off( job_queue_type * job_queue);
  bool                job_queue_start_user_exit( job_queue_type * queue);
  bool                job_queue_get_user_exit( const job_queue_type * queue);
  void              * job_queue_iget_job_data( job_queue_type * job_queue , int job_nr );

  int                 job_queue_get_active_size( const job_queue_type * queue );
  int                 job_queue_get_num_callback( const job_queue_type * queue);
  int                 job_queue_get_num_running( const job_queue_type * queue);
  int                 job_queue_get_num_pending( const job_queue_type * queue);
  int                 job_queue_get_num_waiting( const job_queue_type * queue);
  int                 job_queue_get_num_complete( const job_queue_type * queue);
  int                 job_queue_get_num_failed( const job_queue_type * queue);
  int                 job_queue_get_num_killed( const job_queue_type * queue);
  void              * job_queue_iget_driver_data( job_queue_type * queue , int job_index);
  const char        * job_queue_iget_failed_job(  job_queue_type * queue , int job_index);
  const char        * job_queue_iget_error_reason(  job_queue_type * queue , int job_index);
  const char        * job_queue_iget_stderr_capture(  job_queue_type * queue , int job_index);
  const char        * job_queue_iget_stderr_file(  job_queue_type * queue , int job_index);
  const char        * job_queue_iget_run_path(  job_queue_type * queue , int job_index);
  void                job_queue_iset_external_restart(job_queue_type * queue , int job_index);
  job_queue_node_type * job_queue_iget_job( job_queue_type * job_queue , int job_nr );
  bool                job_queue_has_driver(const job_queue_type * queue );
  job_queue_node_type * job_queue_iget_node(job_queue_type * queue , int job_index);
  int job_queue_get_max_running( const job_queue_type * queue );

  UTIL_SAFE_CAST_HEADER( job_queue );

#ifdef __cplusplus
}
#endif
#endif


/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'lsf_driver.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_LSF_DRIVER_H
#define ERT_LSF_DRIVER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <ert/util/stringlist.h>

#include <ert/job_queue/queue_driver.h>


/*
  The options supported by the LSF driver.
*/
#define LSF_QUEUE        "LSF_QUEUE"
#define LSF_RESOURCE     "LSF_RESOURCE"
#define LSF_SERVER       "LSF_SERVER"
#define LSF_RSH_CMD      "LSF_RSH_CMD"      // This option is set to DEFAULT_RSH_CMD at driver creation.
#define LSF_LOGIN_SHELL   "LSF_LOGIN_SHELL"  // Not fully implemented yet
#define LSF_BSUB_CMD      "BSUB_CMD"
#define LSF_BJOBS_CMD     "BJOBS_CMD"
#define LSF_BKILL_CMD     "BKILL_CMD"
#define LSF_BHIST_CMD     "BHIST_CMD"
#define LSF_BJOBS_TIMEOUT "BJOBS_TIMEOUT"
#define LSF_DEBUG_OUTPUT  "DEBUG_OUTPUT"
#define LSF_SUBMIT_SLEEP  "SUBMIT_SLEEP"
#define LSF_EXCLUDE_HOST  "EXCLUDE_HOST"

#define LOCAL_LSF_SERVER "LOCAL"
#define NULL_LSF_SERVER  "NULL"
#define DEFAULT_SUBMIT_SLEEP "0"

  typedef enum {
    LSF_SUBMIT_INVALID = 0,
    LSF_SUBMIT_INTERNAL = 1,
    LSF_SUBMIT_LOCAL_SHELL = 2,
    LSF_SUBMIT_REMOTE_SHELL = 3
  } lsf_submit_method_enum;


typedef struct lsf_driver_struct lsf_driver_type;
typedef struct lsf_job_struct    lsf_job_type;

  void            lsf_job_export_hostnames( const lsf_job_type * job , stringlist_type * hostlist);
  void            lsf_job_free(lsf_job_type * job);
  long            lsf_job_get_jobnr( const lsf_job_type * job );

  void      * lsf_driver_alloc( );
  stringlist_type * lsf_driver_alloc_cmd(lsf_driver_type * driver ,
                                              const char *  run_path   ,
                                              const char *  job_name   ,
                                              const char *  submit_cmd ,
                                              int           num_cpu    ,
                                              int           job_argc,
                                              const char ** job_argv);

  void * lsf_driver_submit_job(void * __driver ,
                               const char  * submit_cmd     ,
                               int           num_cpu ,
                               const char  * run_path       ,
                               const char  * job_name ,
                               int           argc,
                               const char ** argv );
  job_status_type lsf_driver_convert_status( int lsf_status );
  void            lsf_driver_blacklist_node(void * __driver , void * __job );
  void            lsf_driver_kill_job(void * __driver , void * __job );
  void            lsf_driver_free__(void * __driver );
  void            lsf_driver_free( lsf_driver_type * driver );
  job_status_type lsf_driver_get_job_status(void * __driver , void * __job);
  int             lsf_driver_get_job_status_lsf(void * __driver , void * __job);
  void            lsf_driver_free_job(void * __job);
  void            lsf_driver_display_info( void * __driver , void * __job);
  void            lsf_driver_set_bjobs_refresh_interval( lsf_driver_type * driver , int refresh_interval);

  void lsf_driver_add_exclude_hosts( lsf_driver_type * driver , const char * excluded);
  lsf_submit_method_enum lsf_driver_get_submit_method( const lsf_driver_type * driver );

  bool            lsf_driver_has_option( const void * __driver , const char * option_key);
  const  void   * lsf_driver_get_option( const void * __driver , const char * option_key);
  bool            lsf_driver_set_option( void * __driver , const char * option_key , const void * value);
  void            lsf_driver_init_option_list(stringlist_type * option_list);
  int             lsf_job_parse_bsub_stdout(const char * bsub_cmd, const char * stdout_file);
  char          * lsf_job_write_bjobs_to_file(const char * bjobs_cmd, lsf_driver_type * driver, const long jobid);

  stringlist_type * lsf_job_alloc_parse_hostnames(const char* fname);
  UTIL_SAFE_CAST_HEADER( lsf_driver );


#ifdef __cplusplus
}
#endif
#endif

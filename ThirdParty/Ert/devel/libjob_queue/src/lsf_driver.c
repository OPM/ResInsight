/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lsf_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>

#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/lsf_driver.h>

#include <lsf/lsbatch.h>
#ifdef HAVE_LSF_LIBRARY
#include <ert/job_queue/lsb.h>
#endif





/**
   Documentation/examples of programming towards the lsf libraries can
   be found in /prog/LSF/7.0/misc/examples
*/


/*
  How to call the lsf commands bsub/bjobs/bkill:
  ----------------------------------------------
  
  The commands to submit, monitor and modify LSF jobs are available
  through library calls through the lsf library. This is a good
  solution which works well.

  Unfortunately only quite few of the workstations in Statoil are
  "designated LSF machines", meaning that they are allowed to talk to
  the LIM servers, to be able to use the low-level lsb_xxx() function
  calls the host making the calls must configured (by an LSF
  administrator) to be a LSF client.
  
  The lsf_driver can either make use of the proper lsf library calls
  (lsb_submit(), lsb_openjobinfo(), ...) or alternatively it can issue
  ssh calls to an external LSF_SERVER and call up the bsub/bkill/bjob
  executables on the remote server. 

  All the functions with 'library' in the name are based on library
  calls, and the functions with 'shell' in the name are based on
  external functions (the actual calls are through the
  util_fork_exec() function).

  By default the driver will use the library, but if a value is
  provided with the LSF_SERVER option, the shell based functions will
  be used. Internally this is goverened by the boolean flag
  'use_library_calls'.

  Even though you only intend to submit through the shell commands
  bsub / bjobs / bkill the build process still requires access to the
  lsf headers and the lsf library; that is probably not optimal.


  Remote login shell 
  ------------------ 

  When submitting with LSF the job will inherit the current
  environment on the submitting host, and not read the users login
  files on the remote host where the job is actually executed. E.g. in
  situations where submitting host and executing host are
  e.g. different operating system versions this might be
  unfortunate. The '-L @shell' switch can used with bsub to force lsf
  to source schell specific input files prior to executing your
  job. This can be achieved with the LSF_LOGIN_SHELL option:

     lsf_driver_set_option( driver , LSF_LOGIN_SHELL , "/bin/csh" );

*/





#define LSF_DRIVER_TYPE_ID 10078365
#define LSF_JOB_TYPE_ID    9963900
#define BJOBS_REFRESH_TIME 10
#define DEFAULT_RSH_CMD    "/usr/bin/ssh"
#define DEFAULT_BSUB_CMD   "bsub"
#define DEFAULT_BJOBS_CMD  "bjobs"
#define DEFAULT_BKILL_CMD  "bkill"



struct lsf_job_struct {
  UTIL_TYPE_ID_DECLARATION;
  long int    lsf_jobnr;
  int         num_exec_host;
  char      **exec_host;
  char       * lsf_jobnr_char;  /* Used to look up the job status in the bjobs_cache hash table */
};



struct lsf_driver_struct {
  UTIL_TYPE_ID_DECLARATION;
  char              * queue_name;
  char              * resource_request;
  char              * login_shell;
  pthread_mutex_t     submit_lock;

  lsf_submit_method_enum submit_method;
  
  /*-----------------------------------------------------------------*/
  /* Fields used by the lsf library functions */
#ifdef HAVE_LSF_LIBRARY
  struct submit      lsf_request;
  struct submitReply lsf_reply; 
  lsb_type         * lsb;
#endif

  /*-----------------------------------------------------------------*/
  /* Fields used by the shell based functions */
  
  int                 bjobs_refresh_interval; 
  time_t              last_bjobs_update;
  hash_type         * my_jobs;            /* A hash table of all jobs submitted by this ERT instance - 
                                             to ensure that we do not check status of old jobs in e.g. ZOMBIE status. */
  hash_type         * status_map;
  hash_type         * bjobs_cache;        /* The output of calling bjobs is cached in this table. */
  pthread_mutex_t     bjobs_mutex;        /* Only one thread should update the bjobs_chache table. */
  char              * remote_lsf_server;
  char              * rsh_cmd;
  char              * bsub_cmd;
  char              * bjobs_cmd;
  char              * bkill_cmd;
};




/*****************************************************************/

UTIL_SAFE_CAST_FUNCTION( lsf_driver , LSF_DRIVER_TYPE_ID)
static UTIL_SAFE_CAST_FUNCTION_CONST( lsf_driver , LSF_DRIVER_TYPE_ID)
static UTIL_SAFE_CAST_FUNCTION( lsf_job , LSF_JOB_TYPE_ID)

lsf_job_type * lsf_job_alloc() {
  lsf_job_type * job;
  job                = util_malloc(sizeof * job);
  job->num_exec_host = 0;
  job->exec_host     = NULL;

  job->lsf_jobnr_char = NULL;
  UTIL_TYPE_ID_INIT( job , LSF_JOB_TYPE_ID);
  return job;
}



void lsf_job_free(lsf_job_type * job) {
  util_safe_free(job->lsf_jobnr_char);
  util_free_stringlist(job->exec_host , job->num_exec_host);
  free(job);
}


static int lsf_job_parse_bsub_stdout(const lsf_driver_type * driver , const char * stdout_file) {
  int     jobid = -1;
  FILE * stream = util_fopen(stdout_file , "r");
  if (util_fseek_string(stream , "<" , true , true)) {
    char * jobid_string = util_fscanf_alloc_upto(stream , ">" , false);
    if (jobid_string != NULL) {
      jobid = atoi( jobid_string );
      free( jobid_string );
    } 
  } 
  fclose( stream );

  if (jobid == -1) {
    char * file_content = util_fread_alloc_file_content( stdout_file , NULL );
    fprintf(stderr,"Failed to get lsf job id from file: %s \n",stdout_file );
    fprintf(stderr,"bsub command                      : %s \n",driver->bsub_cmd );
    fprintf(stderr,"%s\n", file_content);
    free( file_content );
    util_exit("%s: \n",__func__);
  }
  return jobid;
}



static void lsf_driver_internal_error( const lsf_driver_type * driver ) {
  fprintf(stderr , "\n\n");
  fprintf(stderr , "*****************************************************************\n");
  fprintf(stderr , "** The LSF driver can be configured and used in many different **\n");
  fprintf(stderr , "** ways. The important point is how we choose to submit:       **\n");
  fprintf(stderr , "**                                                             **\n");
  fprintf(stderr , "**    1. Using the lsf library calls                           **\n");
  fprintf(stderr , "**    2. Using the bsub/bjobs/bkill commands locally           **\n");
  fprintf(stderr , "**    3. Using the bsub/bjobs/bkill commands through ssh       **\n");
  fprintf(stderr , "**                                                             **\n");
  fprintf(stderr , "** To chose between these three alternatives you set the remote**\n");
  fprintf(stderr , "** server with the lsf_driver_set_option() function. Passing   **\n");
  fprintf(stderr , "** the value NULL will give alternative 1, passing the special **\n");
  fprintf(stderr , "** string \'%s\' will give alternative 2, and any other       **\n",LOCAL_LSF_SERVER);
  fprintf(stderr , "** value will submit through that host using ssh.              **\n");
  fprintf(stderr , "**                                                             **\n");
  fprintf(stderr , "** The ability to submit thorugh lsf library calls must be     **\n");
  fprintf(stderr , "** compiled in by defining the symbol \'HAVE_LSF_LIBRARY\' when  **\n");
  fprintf(stderr , "** compiling.                                                  **\n");
  fprintf(stderr , "**                                                             **\n");
#ifdef HAVE_LSF_LIBRARY
  fprintf(stderr , "** This lsf driver has support for using lsf library calls.    **\n");
#else
  fprintf(stderr , "** This lsf driver does NOT have support for using lsf         **\n");
  fprintf(stderr , "** library calls; but you have tried to submit without setting **\n");
  fprintf(stderr , "** a value for LSF_SERVER. Set this and try again.             **\n");
#endif     
  fprintf(stderr , "*****************************************************************\n\n");
  exit(1);
}



static void lsf_driver_assert_submit_method( const lsf_driver_type * driver ) {
  if (driver->submit_method == LSF_SUBMIT_INVALID) {
    lsf_driver_internal_error(driver);
  }
}





stringlist_type * lsf_driver_alloc_cmd(lsf_driver_type * driver , 
                                       const char *  lsf_stdout   , 
                                       const char *  job_name   , 
                                       const char *  submit_cmd ,
                                       int           num_cpu    , 
                                       int           job_argc,
                                       const char ** job_argv) {
  
  stringlist_type * argv  = stringlist_alloc_new();
  char *  num_cpu_string  = util_alloc_sprintf("%d" , num_cpu);
  char * quoted_resource_request = NULL;
  
  /*
    The resource request string contains spaces, and when passed
    through the shell it must be protected with \"..\"; this applies
    when submitting to a remote lsf server with ssh. However when
    submitting to the local workstation using a bsub command the
    command will be invoked with the util_fork_exec() command - and no
    shell is involved. In this latter case we must avoid the \"...\"
    quoting.
  */

  if  (driver->resource_request != NULL) { 
    if (driver->submit_method == LSF_SUBMIT_REMOTE_SHELL) 
      quoted_resource_request =util_alloc_sprintf("\"%s\"" , driver->resource_request); 
    else
      quoted_resource_request = util_alloc_string_copy( driver->resource_request ); 
  }

  if (driver->submit_method == LSF_SUBMIT_REMOTE_SHELL)
    stringlist_append_ref( argv , driver->bsub_cmd);
  
  stringlist_append_ref( argv , "-o" );
  stringlist_append_copy( argv , lsf_stdout );
  if (driver->queue_name != NULL) {
    stringlist_append_ref( argv , "-q" );
    stringlist_append_ref( argv , driver->queue_name );
  }
  stringlist_append_ref( argv , "-J" );
  stringlist_append_ref( argv , job_name );
  stringlist_append_ref( argv , "-n" );
  stringlist_append_copy( argv , num_cpu_string );

  if (quoted_resource_request != NULL) {
    stringlist_append_ref( argv , "-R");
    stringlist_append_copy( argv , quoted_resource_request );
  }
  
  if (driver->login_shell != NULL) {
    stringlist_append_ref( argv , "-L");
    stringlist_append_ref( argv , driver->login_shell );
  }

  stringlist_append_ref( argv , submit_cmd);
  {
    int iarg;
    for (iarg = 0; iarg < job_argc; iarg++) 
      stringlist_append_ref( argv , job_argv[ iarg ]);
  }
  free( num_cpu_string );
  util_safe_free( quoted_resource_request );
  return argv;
}


static int lsf_driver_submit_internal_job( lsf_driver_type * driver , 
                                           const char *  lsf_stdout , 
                                           const char *  job_name   , 
                                           const char *  submit_cmd ,
                                           int           num_cpu    , 
                                           int           argc,
                                           const char ** argv) {

#ifdef HAVE_LSF_LIBRARY
  char * command;
  {
    buffer_type * command_buffer = buffer_alloc( 256 );
    buffer_fwrite_char_ptr( command_buffer , submit_cmd );
    for (int iarg = 0; iarg < argc; iarg++) {
      buffer_fwrite_char( command_buffer , ' ');
      buffer_fwrite_char_ptr( command_buffer , argv[ iarg ]);
    }
    buffer_terminate_char_ptr( command_buffer );
    command = buffer_get_data( command_buffer );
    buffer_free_container( command_buffer );
  }
  
  {
    int options = SUB_JOB_NAME + SUB_OUT_FILE;
    
    if (driver->queue_name != NULL) 
      options += SUB_QUEUE;
    
    if (driver->resource_request != NULL) 
      options += SUB_RES_REQ;
    
    if (driver->login_shell != NULL) 
      options += SUB_LOGIN_SHELL;
    
    driver->lsf_request.options = options;
  }
  
  driver->lsf_request.resReq        = driver->resource_request;
  driver->lsf_request.loginShell    = driver->login_shell;
  driver->lsf_request.queue         = driver->queue_name;
  driver->lsf_request.jobName       = (char *) job_name;
  driver->lsf_request.outFile       = (char *) lsf_stdout;
  driver->lsf_request.command       = command;
  driver->lsf_request.numProcessors = num_cpu;

  {
    int lsf_jobnr = lsb_submitjob( driver->lsb , &driver->lsf_request , &driver->lsf_reply );
    free( command );  /* I trust the lsf layer is finished with the command? */
    if (lsf_jobnr <= 0)
      fprintf(stderr,"%s: ** Warning: lsb_submit() failed: %s \n",__func__ , lsb_sys_msg( driver->lsb ));
    
    return lsf_jobnr;
  }
#else
  lsf_driver_internal_error( driver );
#endif
}



static int lsf_driver_submit_shell_job(lsf_driver_type * driver , 
                                       const char *  lsf_stdout , 
                                       const char *  job_name   , 
                                       const char *  submit_cmd ,
                                       int           num_cpu    , 
                                       int           job_argc,
                                       const char ** job_argv) {
  int job_id;
  char * tmp_file         = util_alloc_tmp_file("/tmp" , "enkf-submit" , true);

  if (driver->remote_lsf_server != NULL) {
    stringlist_type * remote_argv = lsf_driver_alloc_cmd( driver , lsf_stdout , job_name , submit_cmd , num_cpu , job_argc , job_argv);

    if (driver->submit_method == LSF_SUBMIT_REMOTE_SHELL) {
      char ** argv = util_calloc( 2 , sizeof * argv );
      argv[0] = driver->remote_lsf_server;
      argv[1] = stringlist_alloc_joined_string( remote_argv , " ");
      util_fork_exec(driver->rsh_cmd , 2 , (const char **) argv , true , NULL , NULL , NULL , tmp_file , NULL);
      free( argv[1] );
      free( argv );
    } else if (driver->submit_method == LSF_SUBMIT_LOCAL_SHELL) {
      char ** argv = stringlist_alloc_char_ref( remote_argv );
      util_fork_exec(driver->bsub_cmd , stringlist_get_size( remote_argv) , (const char **) argv , true , NULL , NULL , NULL , tmp_file , tmp_file);
      free( argv );
    }
    
    stringlist_free( remote_argv );
  }
  
  job_id = lsf_job_parse_bsub_stdout(driver , tmp_file);
  util_unlink_existing( tmp_file );
  free(tmp_file);
  return job_id;
}



static int lsf_driver_get_status__(lsf_driver_type * driver , const char * status, const char * job_id) {
  if (hash_has_key( driver->status_map , status))
    return hash_get_int( driver->status_map , status);
  else {
    util_exit("The lsf_status:%s  for job:%s is not recognized; call your LSF administrator - sorry :-( \n", status , job_id);
    return -1;
  }
}



static void lsf_driver_update_bjobs_table(lsf_driver_type * driver) {
  char * tmp_file   = util_alloc_tmp_file("/tmp" , "enkf-bjobs" , true);

  if (driver->submit_method == LSF_SUBMIT_REMOTE_SHELL) {
    char ** argv = util_calloc( 2 , sizeof * argv);
    argv[0] = driver->remote_lsf_server;
    argv[1] = util_alloc_sprintf("%s -a" , driver->bjobs_cmd);
    util_fork_exec(driver->rsh_cmd , 2 , (const char **) argv , true , NULL , NULL , NULL , tmp_file , NULL);
    free( argv[1] );
    free( argv );
  } else if (driver->submit_method == LSF_SUBMIT_LOCAL_SHELL) {
    char ** argv = util_calloc( 1 , sizeof * argv);
    argv[0] = "-a";
    util_fork_exec(driver->bjobs_cmd , 1 , (const char **) argv , true , NULL , NULL , NULL , tmp_file , NULL);
    free( argv );
  }
  
  {
    char user[32];
    char status[16];
    FILE *stream = util_fopen(tmp_file , "r");;
    bool at_eof = false;
    hash_clear(driver->bjobs_cache);
    util_fskip_lines(stream , 1);
    while (!at_eof) {
      char * line = util_fscanf_alloc_line(stream , &at_eof);
      if (line != NULL) {
        int  job_id_int;

        if (sscanf(line , "%d %s %s", &job_id_int , user , status) == 3) {
          char * job_id = util_alloc_sprintf("%d" , job_id_int);

          if (hash_has_key( driver->my_jobs , job_id ))   /* Consider only jobs submitted by this ERT instance - not old jobs lying around from the same user. */
            hash_insert_int(driver->bjobs_cache , job_id , lsf_driver_get_status__( driver , status , job_id));
          
          free(job_id);
        }
        free(line);
      }
    }
    fclose(stream);
  }
  util_unlink_existing(tmp_file); 
  free(tmp_file);
}



static int lsf_driver_get_job_status_libary(void * __driver , void * __job) {
  if (__job == NULL) 
    /* the job has not been registered at all ... */
    return JOB_QUEUE_NOT_ACTIVE;
  else {
    int status;
    lsf_driver_type * driver = lsf_driver_safe_cast( __driver );
#ifdef HAVE_LSF_LIBRARY
    lsf_job_type    * job    = lsf_job_safe_cast( __job );
    if (lsb_openjob( driver->lsb , job->lsf_jobnr) != 1) {
      /* 
         Failed to get information about the job - we boldly assume
         the following situation has occured:
         
         1. The job is running happily along.
         2. The lsf deamon is not responding for a long time.
         3. The job finishes, and is eventually expired from the LSF job database.
         4. The lsf deamon answers again - but can not find the job...
         
      */
      fprintf(stderr,"Warning: failed to get status information for job:%ld - assuming it is finished. \n", job->lsf_jobnr);
      status = JOB_QUEUE_DONE;
    } else {
      struct jobInfoEnt *job_info = lsb_readjob( driver->lsb );
      if (job->num_exec_host == 0) {
        job->num_exec_host = job_info->numExHosts;
        job->exec_host     = util_alloc_stringlist_copy( (const char **) job_info->exHosts , job->num_exec_host);
      }
      status = job_info->status;
      lsb_closejob(driver->lsb);
    }
#else
    lsf_driver_internal_error( driver );
#endif
    
    return status;
  }
}




static int lsf_driver_get_job_status_shell(void * __driver , void * __job) {
  int status = JOB_STAT_NULL;
  
  if (__job != NULL) {
    lsf_job_type    * job    = lsf_job_safe_cast( __job );
    lsf_driver_type * driver = lsf_driver_safe_cast( __driver );
    
    {
      /**
         Updating the bjobs_table of the driver involves a significant change in
         the internal state of the driver; that is semantically a bit
         unfortunate because this is clearly a get() function; to protect
         against concurrent updates of this table we use a mutex.
      */
      pthread_mutex_lock( &driver->bjobs_mutex );
      {
        if (difftime(time(NULL) , driver->last_bjobs_update) > driver->bjobs_refresh_interval) {
          lsf_driver_update_bjobs_table(driver);
          driver->last_bjobs_update = time( NULL );
        }
      }
      pthread_mutex_unlock( &driver->bjobs_mutex );
      
      
      if (hash_has_key( driver->bjobs_cache , job->lsf_jobnr_char) ) 
        status = hash_get_int(driver->bjobs_cache , job->lsf_jobnr_char);
      else
        /* 
           It might be running - but since job != NULL it is at least in the queue system.
        */
        status = JOB_STAT_PEND;

    }
  }
  
  return status;
}


job_status_type lsf_driver_convert_status( int lsf_status ) {
  job_status_type job_status;
  switch (lsf_status) {
  case JOB_STAT_NULL:
    job_status = JOB_QUEUE_NOT_ACTIVE;
    break;
  case JOB_STAT_PEND:
    job_status = JOB_QUEUE_PENDING;
    break;
  case JOB_STAT_SSUSP:
    job_status = JOB_QUEUE_RUNNING;
    break;
  case JOB_STAT_USUSP:
    job_status = JOB_QUEUE_RUNNING;
    break;
  case JOB_STAT_PSUSP:
    job_status = JOB_QUEUE_RUNNING;
    break;
  case JOB_STAT_RUN:
    job_status = JOB_QUEUE_RUNNING;
    break;   
  case JOB_STAT_DONE:
    job_status = JOB_QUEUE_DONE;
    break;
  case JOB_STAT_EXIT:
    job_status = JOB_QUEUE_EXIT;
    break;
  case JOB_STAT_UNKWN:  // Have lost contact with one of the daemons.
    job_status = JOB_QUEUE_EXIT;
    break;
  case 192:     /* this 192 seems to pop up - where the fuck does it come frome ??  _pdone + _ususp ??? */
    job_status = JOB_QUEUE_DONE;
    break;
  default:
    job_status = JOB_QUEUE_NOT_ACTIVE;
    util_abort("%s: unrecognized lsf status code:%d \n",__func__ , lsf_status );
  }
  return job_status;
}


int lsf_driver_get_job_status_lsf(void * __driver , void * __job) {
  int lsf_status;
  lsf_driver_type * driver = lsf_driver_safe_cast( __driver );

  if (driver->submit_method == LSF_SUBMIT_INTERNAL) 
    lsf_status = lsf_driver_get_job_status_libary(__driver , __job);
  else
    lsf_status = lsf_driver_get_job_status_shell(__driver , __job);

  return lsf_status;
}



job_status_type lsf_driver_get_job_status(void * __driver , void * __job) {
  int lsf_status = lsf_driver_get_job_status_lsf( __driver , __job );
  return lsf_driver_convert_status( lsf_status );
}




void lsf_driver_free_job(void * __job) {
  lsf_job_type    * job    = lsf_job_safe_cast( __job );
  lsf_job_free(job);
}



void lsf_driver_kill_job(void * __driver , void * __job) {
  lsf_driver_type * driver = lsf_driver_safe_cast( __driver );
  lsf_job_type    * job    = lsf_job_safe_cast( __job );
  {
    if (driver->submit_method == LSF_SUBMIT_INTERNAL)
#ifdef HAVE_LSF_LIBRARY
      lsb_killjob( driver->lsb , job->lsf_jobnr);
#else
    lsf_driver_internal_error( driver );
#endif
    else {
      if (driver->submit_method == LSF_SUBMIT_REMOTE_SHELL) {
        char ** argv = util_calloc( 2, sizeof * argv );
        argv[0] = driver->remote_lsf_server;
        argv[1] = util_alloc_sprintf("%s %s" , driver->bkill_cmd , job->lsf_jobnr_char);

        util_fork_exec(driver->rsh_cmd , 2 , (const char **)  argv , true , NULL , NULL , NULL , NULL , NULL);

        free( argv[1] );
        free( argv );
      } else if (driver->submit_method == LSF_SUBMIT_LOCAL_SHELL) 
        util_fork_exec(driver->bkill_cmd , 1 , (const char **)  &job->lsf_jobnr_char , true , NULL , NULL , NULL , NULL , NULL);
    }
  }
}





void * lsf_driver_submit_job(void * __driver , 
                             const char  * submit_cmd     , 
                             int           num_cpu        , 
                             const char  * run_path       , 
                             const char  * job_name       ,
                             int           argc,     
                             const char ** argv ) {
  lsf_driver_type * driver = lsf_driver_safe_cast( __driver );
  lsf_driver_assert_submit_method( driver );
  {
    lsf_job_type * job      = lsf_job_alloc();
    {
      char * lsf_stdout                    = util_alloc_filename(run_path , job_name , "LSF-stdout");
      lsf_submit_method_enum submit_method = driver->submit_method;
      pthread_mutex_lock( &driver->submit_lock );
      
      if (submit_method == LSF_SUBMIT_INTERNAL) {
        job->lsf_jobnr = lsf_driver_submit_internal_job( driver , lsf_stdout , job_name , submit_cmd , num_cpu , argc, argv);
      } else {
        job->lsf_jobnr      = lsf_driver_submit_shell_job( driver , lsf_stdout , job_name , submit_cmd , num_cpu , argc, argv);
        job->lsf_jobnr_char = util_alloc_sprintf("%ld" , job->lsf_jobnr);
        hash_insert_ref( driver->my_jobs , job->lsf_jobnr_char , NULL );   
      }
      
      pthread_mutex_unlock( &driver->submit_lock );
      free( lsf_stdout );
    }
    
    if (job->lsf_jobnr > 0) 
      return job;
    else {
      /*
        The submit failed - the queue system shall handle
        NULL return values.
      */
      lsf_job_free(job);
      return NULL;
    }
  }
}



void lsf_driver_free(lsf_driver_type * driver ) {
  util_safe_free(driver->login_shell);
  util_safe_free(driver->queue_name);
  util_safe_free(driver->resource_request );
  util_safe_free(driver->remote_lsf_server );
  util_safe_free(driver->rsh_cmd );
  free( driver->bkill_cmd );
  free( driver->bjobs_cmd );
  free( driver->bsub_cmd );
  
  hash_free(driver->status_map);
  hash_free(driver->bjobs_cache);
  hash_free(driver->my_jobs);
  
#ifdef HAVE_LSF_LIBRARY
  if (driver->lsb != NULL)
    lsb_free( driver->lsb );
#endif
  
  free(driver);
  driver = NULL;
}

void lsf_driver_free__(void * __driver ) {
  lsf_driver_type * driver = lsf_driver_safe_cast( __driver );
  lsf_driver_free( driver );
}


static void lsf_driver_set_queue( lsf_driver_type * driver,  const char * queue ) {
  driver->queue_name         = util_realloc_string_copy( driver->queue_name , queue);
}


static void lsf_driver_set_login_shell( lsf_driver_type * driver,  const char * login_shell ) {
  driver->login_shell        = util_realloc_string_copy( driver->login_shell , login_shell);
}

static void lsf_driver_set_rsh_cmd( lsf_driver_type * driver , const char * rsh_cmd) {
  driver->rsh_cmd = util_realloc_string_copy( driver->rsh_cmd , rsh_cmd );    
}

static void lsf_driver_set_bsub_cmd( lsf_driver_type * driver , const char * bsub_cmd) {
  driver->bsub_cmd = util_realloc_string_copy( driver->bsub_cmd , bsub_cmd );    
}

static void lsf_driver_set_bjobs_cmd( lsf_driver_type * driver , const char * bjobs_cmd) {
  driver->bjobs_cmd = util_realloc_string_copy( driver->bjobs_cmd , bjobs_cmd );    
}

static void lsf_driver_set_bkill_cmd( lsf_driver_type * driver , const char * bkill_cmd) {
  driver->bkill_cmd = util_realloc_string_copy( driver->bkill_cmd , bkill_cmd );    
}

static void lsf_driver_set_internal_submit( lsf_driver_type * driver) {
  /* No remote server has been set - assuming we can issue proper library calls. */
  /* The BSUB_QUEUE variable must NOT be set when using the shell
     function, because then stdout is redirected and read. */
  
  util_setenv("BSUB_QUIET" , "yes");            
  driver->submit_method = LSF_SUBMIT_INTERNAL;
  util_safe_free( driver->remote_lsf_server );
  driver->remote_lsf_server = NULL;
}


static void lsf_driver_set_remote_server( lsf_driver_type * driver , const char * remote_server) {
  if (remote_server == NULL) {
#ifdef HAVE_LSF_LIBRARY
    if (driver->lsb)
      lsf_driver_set_internal_submit( driver );
    else
      lsf_driver_set_remote_server( driver , LOCAL_LSF_SERVER );  // If initializing the lsb layer failed we try the local shell commands.
#endif
  } else {
    driver->remote_lsf_server = util_realloc_string_copy( driver->remote_lsf_server , remote_server );
    util_unsetenv( "BSUB_QUIET" );
    {
      char * tmp_server = util_alloc_strupr_copy( remote_server );
      
      if (strcmp(tmp_server , LOCAL_LSF_SERVER) == 0)
        driver->submit_method = LSF_SUBMIT_LOCAL_SHELL;
      else if (strcmp(tmp_server , NULL_LSF_SERVER) == 0)  // We trap the special string 'NULL' and call again with a true NULL pointer.
        lsf_driver_set_remote_server( driver , NULL);
      else
        driver->submit_method = LSF_SUBMIT_REMOTE_SHELL;
      
      free( tmp_server );
    }
  }
}



lsf_submit_method_enum lsf_driver_get_submit_method( const lsf_driver_type * driver ) {
  return driver->submit_method;
}

/*****************************************************************/
/* Generic functions for runtime manipulation of options.        

   LSF_SERVER
   LSF_QUEUE
   LSF_RESOURCE
*/

bool lsf_driver_set_option( void * __driver , const char * option_key , const void * value) {
  lsf_driver_type * driver  = lsf_driver_safe_cast( __driver );
  bool has_option = true;
  {
    if (strcmp( LSF_RESOURCE , option_key ) == 0)
      driver->resource_request = util_realloc_string_copy( driver->resource_request , value );
    else if (strcmp( LSF_SERVER , option_key) == 0)
      lsf_driver_set_remote_server( driver , value );
    else if (strcmp( LSF_QUEUE , option_key) == 0)
      lsf_driver_set_queue( driver , value );
    else if (strcmp( LSF_LOGIN_SHELL , option_key) == 0)
      lsf_driver_set_login_shell( driver , value );
    else if (strcmp( LSF_RSH_CMD , option_key) == 0)
      lsf_driver_set_rsh_cmd( driver , value );
    else if (strcmp( LSF_BSUB_CMD , option_key) == 0)
      lsf_driver_set_bsub_cmd( driver , value );
    else if (strcmp( LSF_BJOBS_CMD , option_key) == 0)
      lsf_driver_set_bjobs_cmd( driver , value );
    else if (strcmp( LSF_BKILL_CMD , option_key) == 0)
      lsf_driver_set_bkill_cmd( driver , value );
    else 
      has_option = false;
  }
  return has_option;
}


const void * lsf_driver_get_option( const void * __driver , const char * option_key) {
  const lsf_driver_type * driver = lsf_driver_safe_cast_const( __driver );
  {
    if (strcmp( LSF_RESOURCE , option_key ) == 0)
      return driver->resource_request;
    else if (strcmp( LSF_SERVER , option_key ) == 0)
      return driver->remote_lsf_server;
    else if (strcmp( LSF_QUEUE , option_key ) == 0)
      return driver->queue_name;
    else if (strcmp( LSF_LOGIN_SHELL , option_key ) == 0)
      return driver->login_shell;
    else if (strcmp( LSF_RSH_CMD , option_key ) == 0)
      return driver->rsh_cmd;
    else if (strcmp( LSF_BJOBS_CMD , option_key ) == 0)
      return driver->bjobs_cmd;
    else if (strcmp( LSF_BSUB_CMD , option_key ) == 0)
      return driver->bsub_cmd;
    else if (strcmp( LSF_BKILL_CMD , option_key ) == 0)
      return driver->bkill_cmd;
    else {
      util_abort("%s: option_id:%s not recognized for LSF driver \n",__func__ , option_key);
      return NULL;
    }
  }
}



bool lsf_driver_has_option( const void * __driver , const char * option_key) {
  return false;
}




/*****************************************************************/

/* 
   Observe that this driver IS not properly initialized when returning
   from this function, the option interface must be used to set the
   keys:
*/

void lsf_driver_set_bjobs_refresh_interval( lsf_driver_type * driver , int refresh_interval) {
  driver->bjobs_refresh_interval = refresh_interval;
}


static void lsf_driver_lib_init( lsf_driver_type * lsf_driver ) {
#ifdef HAVE_LSF_LIBRARY
  memset(&lsf_driver->lsf_request , 0 , sizeof (lsf_driver->lsf_request));
  lsf_driver->lsf_request.beginTime        = 0;
  lsf_driver->lsf_request.termTime         = 0;   
  lsf_driver->lsf_request.numProcessors    = 1;
  lsf_driver->lsf_request.maxNumProcessors = 1;
  {
    int i;
    for (i=0; i < LSF_RLIM_NLIMITS; i++) 
      lsf_driver->lsf_request.rLimits[i] = DEFAULT_RLIMIT;
  }
  lsf_driver->lsf_request.options2 = 0;
  
  lsf_driver->lsb = lsb_alloc();
  if (lsb_ready(lsf_driver->lsb)) 
    lsb_initialize(lsf_driver->lsb);
  else {
    lsb_free( lsf_driver->lsb );
    lsf_driver->lsb = NULL;
  }
#endif
}



static void lsf_driver_shell_init( lsf_driver_type * lsf_driver ) {
  lsf_driver->last_bjobs_update   = time( NULL );
  lsf_driver->bjobs_cache         = hash_alloc(); 
  lsf_driver->my_jobs             = hash_alloc(); 
  lsf_driver->status_map          = hash_alloc();
  lsf_driver->bsub_cmd            = NULL;
  lsf_driver->bjobs_cmd           = NULL;
  lsf_driver->bkill_cmd           = NULL;


  hash_insert_int(lsf_driver->status_map , "PEND"   , JOB_STAT_PEND);
  hash_insert_int(lsf_driver->status_map , "SSUSP"  , JOB_STAT_SSUSP);
  hash_insert_int(lsf_driver->status_map , "PSUSP"  , JOB_STAT_PSUSP);
  hash_insert_int(lsf_driver->status_map , "USUSP"  , JOB_STAT_USUSP);
  hash_insert_int(lsf_driver->status_map , "RUN"    , JOB_STAT_RUN);
  hash_insert_int(lsf_driver->status_map , "EXIT"   , JOB_STAT_EXIT);
  hash_insert_int(lsf_driver->status_map , "DONE"   , JOB_STAT_DONE);
  hash_insert_int(lsf_driver->status_map , "UNKWN"  , JOB_STAT_UNKWN);    /* Uncertain about this one */
  pthread_mutex_init( &lsf_driver->bjobs_mutex , NULL );
}



void * lsf_driver_alloc( ) {
  lsf_driver_type * lsf_driver     = util_malloc(sizeof * lsf_driver );
  UTIL_TYPE_ID_INIT( lsf_driver , LSF_DRIVER_TYPE_ID);
  lsf_driver->submit_method        = LSF_SUBMIT_INVALID;
  lsf_driver->login_shell          = NULL;
  lsf_driver->queue_name           = NULL;
  lsf_driver->remote_lsf_server    = NULL; 
  lsf_driver->rsh_cmd              = NULL; 
  lsf_driver->resource_request     = NULL;
  lsf_driver_set_bjobs_refresh_interval( lsf_driver , BJOBS_REFRESH_TIME );
  pthread_mutex_init( &lsf_driver->submit_lock , NULL );

  lsf_driver_lib_init( lsf_driver );
  lsf_driver_shell_init( lsf_driver );
  
  lsf_driver_set_option( lsf_driver , LSF_SERVER    , NULL );
  lsf_driver_set_option( lsf_driver , LSF_RSH_CMD   , DEFAULT_RSH_CMD );
  lsf_driver_set_option( lsf_driver , LSF_BSUB_CMD  , DEFAULT_BSUB_CMD );
  lsf_driver_set_option( lsf_driver , LSF_BJOBS_CMD , DEFAULT_BJOBS_CMD );
  lsf_driver_set_option( lsf_driver , LSF_BKILL_CMD , DEFAULT_BKILL_CMD );
  return lsf_driver;
}


/*****************************************************************/


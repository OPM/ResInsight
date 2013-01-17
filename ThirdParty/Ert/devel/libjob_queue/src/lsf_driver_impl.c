/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lsf_driver_impl.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <queue_driver.h>
#include <lsf_driver.h>
#include <util.h>
#include <hash.h>
#include <pthread.h>
#include <stringlist.h>
#include <lsf/lsbatch.h>
#include <dlfcn.h>



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
  
  bool                use_library_calls; 
  
  /*-----------------------------------------------------------------*/
  /* Fields used by the lsf library functions */
  struct submit      lsf_request;
  struct submitReply lsf_reply; 

  /*-----------------------------------------------------------------*/
  /* Fields used by the shell based functions */
  
  time_t              last_bjobs_update;
  hash_type         * my_jobs;            /* A hash table of all jobs submitted by this ERT instance - 
                                             to ensure that we do not check status of old jobs in e.g. ZOMBIE status. */
  hash_type         * status_map;
  hash_type         * bjobs_cache;        /* The output of calling bjobs is cached in this table. */
  pthread_mutex_t     bjobs_mutex;        /* Only one thread should update the bjobs_chache table. */
  char              * remote_lsf_server;
  char              * rsh_cmd;
};


void init_lsf_driver() {
  
}




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


static int lsf_job_parse_bsub_stdout(const char * stdout_file) {
  int     jobid = -1;
  FILE * stream = util_fopen(stdout_file , "r");
  if (util_fseek_string(stream , "<" , true , true)) {
    char * jobid_string = util_fscanf_alloc_upto(stream , ">" , false);
    if (jobid_string != NULL) {
      jobid = atoi( jobid_string );
      free( jobid_string );
    } else
      util_abort("%s: Could not extract job id from bsub submit_file:%s \n",__func__ , stdout_file );
  } else
    util_abort("%s: Could not extract job id from bsub submit_file:%s \n",__func__ , stdout_file );
  
  fclose( stream );
  return jobid;
}





static int lsf_driver_submit_shell_job(lsf_driver_type * driver , 
                                       const char *  run_path   , 
                                       const char *  job_name   , 
                                       const char *  submit_cmd ,
                                       int           num_cpu    , 
                                       int           job_argc,
                                       const char ** job_argv) {
  int job_id;
  char * tmp_file         = util_alloc_tmp_file("/tmp" , "enkf-submit" , true);
  char * lsf_stdout       = util_alloc_filename(run_path , job_name , "LSF-stdout");
  
  char num_cpu_string[4];

  
  sprintf(num_cpu_string , "%d" , num_cpu);
  if (driver->remote_lsf_server != NULL) {
    char * quoted_resource_request = NULL;
    if (driver->resource_request != NULL)
      quoted_resource_request = util_alloc_sprintf("\"%s\"" , driver->resource_request);

    /**
       Command is:
       
          ssh lsf_server remote_bsub_cmd

       where remote_bsub_cmd is one long unquoted string: 

          bsub -o <outfile> -q <queue_name> -J <job_name> -n <num_cpu>  -R "<resource_request>" -L <login_shell> cmd arg1 arg2 arg3
  
       The options -R and -L are optional.    
    */
    buffer_type * remote_cmd = buffer_alloc(256);
    buffer_fwrite_char_ptr( remote_cmd , "bsub -o " );
    buffer_fwrite_char_ptr( remote_cmd , lsf_stdout );
    buffer_fwrite_char_ptr( remote_cmd , " -q " );
    buffer_fwrite_char_ptr( remote_cmd , driver->queue_name );
    buffer_fwrite_char_ptr( remote_cmd , " -J " );
    buffer_fwrite_char_ptr( remote_cmd , job_name );
    buffer_fwrite_char_ptr( remote_cmd , " -n " );
    buffer_fwrite_char_ptr( remote_cmd , num_cpu_string );
    if (quoted_resource_request != NULL) {
      buffer_fwrite_char_ptr( remote_cmd , " -R ");
      buffer_fwrite_char_ptr( remote_cmd , quoted_resource_request );
    }
    if (driver->login_shell != NULL) {
      buffer_fwrite_char_ptr( remote_cmd , " -L ");
      buffer_fwrite_char_ptr( remote_cmd , driver->login_shell );
    }
    buffer_fwrite_char( remote_cmd , ' ');
    buffer_fwrite_char_ptr( remote_cmd , submit_cmd);
    {
      int iarg;
      for (iarg = 0; iarg < job_argc; iarg++) {
        buffer_fwrite_char( remote_cmd , ' ');
        buffer_fwrite_char_ptr( remote_cmd , job_argv[ iarg ]);
      }
    }
    buffer_terminate_char_ptr( remote_cmd );
    {
      char ** argv = util_calloc( 2 , sizeof * argv );
      argv[0] = driver->remote_lsf_server;
      argv[1] = buffer_get_data( remote_cmd );
      util_fork_exec(driver->rsh_cmd , 2 , (const char **) argv , true , NULL , NULL , NULL , tmp_file , NULL);
      free( argv );
    }
    buffer_free( remote_cmd );
    util_safe_free(quoted_resource_request);
  }
  
  job_id = lsf_job_parse_bsub_stdout(tmp_file);
  util_unlink_existing( tmp_file );
  free(lsf_stdout);
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
  {
    char ** argv = util_calloc( 2 , sizeof * argv);
    argv[0] = driver->remote_lsf_server;
    argv[1] = "bjobs -a";
    util_fork_exec(driver->rsh_cmd , 2 , (const char **) argv , true , NULL , NULL , NULL , tmp_file , NULL);
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



#define CASE_SET(s1,s2) case(s1):  status = s2; break;
static job_status_type lsf_driver_get_job_status_libary(void * __driver , void * __job) {
  if (__job == NULL) 
    /* the job has not been registered at all ... */
    return JOB_QUEUE_NOT_ACTIVE;
  else {
    lsf_job_type    * job    = lsf_job_safe_cast( __job );
    {
      job_status_type status;
      struct jobInfoEnt *job_info;
      if (lsb_openjobinfo(job->lsf_jobnr , NULL , NULL , NULL , NULL , ALL_JOB) != 1) {
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
        job_info = lsb_readjobinfo( NULL );
        lsb_closejobinfo();
        if (job->num_exec_host == 0) {
          job->num_exec_host = job_info->numExHosts;
          job->exec_host     = util_alloc_stringlist_copy( (const char **) job_info->exHosts , job->num_exec_host);
        }
        
        switch (job_info->status) {
          CASE_SET(JOB_STAT_PEND  , JOB_QUEUE_PENDING);
          CASE_SET(JOB_STAT_SSUSP , JOB_QUEUE_RUNNING);
          CASE_SET(JOB_STAT_RUN   , JOB_QUEUE_RUNNING);
          CASE_SET(JOB_STAT_EXIT  , JOB_QUEUE_EXIT);
          CASE_SET(JOB_STAT_DONE  , JOB_QUEUE_DONE);
          CASE_SET(JOB_STAT_PDONE , JOB_QUEUE_DONE);
          CASE_SET(JOB_STAT_PERR  , JOB_QUEUE_EXIT);
          CASE_SET(192            , JOB_QUEUE_DONE); /* this 192 seems to pop up - where the fuck 
                                                        does it come frome ??  _pdone + _ususp ??? */
        default:
          util_abort("%s: job:%ld lsf_status:%d not recognized - internal LSF fuck up - aborting \n",__func__ , job->lsf_jobnr , job_info->status);
          status = JOB_QUEUE_DONE; /* ????  */
        }
      }
      
      return status;
    }
  }
}


static job_status_type lsf_driver_get_job_status_shell(void * __driver , void * __job) {
  job_status_type status = JOB_QUEUE_NOT_ACTIVE;
  
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
        if (difftime(time(NULL) , driver->last_bjobs_update) > BJOBS_REFRESH_TIME) {
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
        status = JOB_QUEUE_PENDING;

    }
  }
  
  return status;
}




job_status_type lsf_driver_get_job_status(void * __driver , void * __job) {
  job_status_type status;
  lsf_driver_type * driver = lsf_driver_safe_cast( __driver );
  if (driver->use_library_calls) 
    status = lsf_driver_get_job_status_libary(__driver , __job);
  else
    status = lsf_driver_get_job_status_shell(__driver , __job);
  return status;
}



void lsf_driver_free_job(void * __job) {
  lsf_job_type    * job    = lsf_job_safe_cast( __job );
  lsf_job_free(job);
}



void lsf_driver_kill_job(void * __driver , void * __job) {
  lsf_driver_type * driver = lsf_driver_safe_cast( __driver );
  lsf_job_type    * job    = lsf_job_safe_cast( __job );
  {
    if (driver->use_library_calls)
      lsb_forcekilljob(job->lsf_jobnr);
    else {
      char ** argv = util_calloc( 2, sizeof * argv );
      argv[0] = driver->remote_lsf_server;
      argv[1] = util_alloc_sprintf("bkill %s" , job->lsf_jobnr_char);
      
      util_fork_exec(driver->rsh_cmd , 1 , (const char **)  &job->lsf_jobnr_char , true , NULL , NULL , NULL , NULL , NULL);

      free( argv[1] );
      free( argv );
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
  {
    lsf_job_type * job            = lsf_job_alloc();
    char * lsf_stdout             = util_alloc_joined_string( (const char *[2]) {run_path   , "/LSF.stdout"}  , 2 , "");
    pthread_mutex_lock( &driver->submit_lock );
    
    if (driver->use_library_calls) {
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
        int options = SUB_QUEUE + SUB_JOB_NAME + SUB_OUT_FILE;
        if (driver->resource_request != NULL) {
          options += SUB_RES_REQ;
          driver->lsf_request.resReq = driver->resource_request;
        }
        if (driver->login_shell != NULL) {
          driver->lsf_request.loginShell = driver->login_shell;
          options += SUB_LOGIN_SHELL;
        } 
        driver->lsf_request.options = options;
      }
      driver->lsf_request.jobName       = (char *) job_name;
      driver->lsf_request.outFile       = lsf_stdout;
      driver->lsf_request.command       = command;
      driver->lsf_request.numProcessors = num_cpu;
      job->lsf_jobnr = lsb_submit( &driver->lsf_request , &driver->lsf_reply );
      free( command );  /* I trust the lsf layer is finished with the command? */
    } else {
      job->lsf_jobnr      = lsf_driver_submit_shell_job( driver , run_path , job_name , submit_cmd , num_cpu , argc, argv);
      job->lsf_jobnr_char = util_alloc_sprintf("%ld" , job->lsf_jobnr);
      hash_insert_ref( driver->my_jobs , job->lsf_jobnr_char , NULL );   
    }

    pthread_mutex_unlock( &driver->submit_lock );
    free(lsf_stdout);
    
    if (job->lsf_jobnr > 0) 
      return job;
    else {
      /*
        The submit failed - the queue system shall handle
        NULL return values.
      */
      if (driver->use_library_calls) 
        fprintf(stderr,"%s: ** Warning: lsb_submit() failed: %s/%d \n",__func__ , lsb_sysmsg() , lsberrno);
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
  
  hash_free(driver->status_map);
  hash_free(driver->bjobs_cache);
  hash_free(driver->my_jobs);
  
  free(driver);
  driver = NULL;
}

void lsf_driver_free__(void * __driver ) {
  lsf_driver_type * driver = lsf_driver_safe_cast( __driver );
  lsf_driver_free( driver );
}


static void lsf_driver_set_queue( lsf_driver_type * driver,  const char * queue ) {
  driver->queue_name         = util_realloc_string_copy( driver->queue_name , queue);
  driver->lsf_request.queue  = driver->queue_name;
}


static void lsf_driver_set_login_shell( lsf_driver_type * driver,  const char * login_shell ) {
  driver->login_shell        = util_realloc_string_copy( driver->login_shell , login_shell);
}

static void lsf_driver_set_rsh_cmd( lsf_driver_type * driver , const char * rsh_cmd) {
  driver->rsh_cmd = util_realloc_string_copy( driver->rsh_cmd , rsh_cmd );    
}

static void lsf_driver_set_remote_server( lsf_driver_type * driver , const char * remote_server) {
  driver->remote_lsf_server = util_realloc_string_copy( driver->remote_lsf_server , remote_server );
  if (driver->remote_lsf_server != NULL) {
    driver->use_library_calls = false;
    util_unsetenv( "BSUB_QUIET" );
  } else {
    /* No remote server has been set - assuming we can issue proper library calls. */
    util_setenv("BSUB_QUIET" , "yes");            /* This must NOT be set when using the shell function, because then stdout is redirected and read. */
    driver->use_library_calls = true;
  }
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

/* Observe that this driver IS not properly initialized when returning
   from this function, the option interface must be used to set the
   keys:
   
*/

void * lsf_driver_alloc( ) {
  lsf_driver_type * lsf_driver     = util_malloc(sizeof * lsf_driver );
  lsf_driver->login_shell          = NULL;
  lsf_driver->queue_name           = NULL;
  lsf_driver->remote_lsf_server    = NULL; 
  lsf_driver->rsh_cmd              = NULL; 
  lsf_driver->resource_request     = NULL;
  UTIL_TYPE_ID_INIT( lsf_driver , LSF_DRIVER_TYPE_ID);
  pthread_mutex_init( &lsf_driver->submit_lock , NULL );

  /* Library initialisation */
  /*****************************************************************/
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
  

  /*
    The environment variable LSF_ENVDIR must be set to point the
    directory containing LSF configuration information, the whole
    thing will crash and burn if this is not properly set.
  */
  if ( lsb_init(NULL) != 0 ) {
    fprintf(stderr,"LSF_ENVDIR: ");
    if (getenv("LSF_ENVDIR") != NULL)
      fprintf(stderr,"%s\n", getenv("LSF_ENVDIR"));
    else
      fprintf(stderr, "not set\n");
    
    util_abort("%s failed to initialize LSF environment : %s/%d  \n",__func__ , lsb_sysmsg() , lsberrno);
  }
  
  /*****************************************************************/
  /* Shell initialization */
  lsf_driver->last_bjobs_update   = time( NULL );
  lsf_driver->bjobs_cache         = hash_alloc(); 
  lsf_driver->my_jobs             = hash_alloc(); 
  lsf_driver->status_map          = hash_alloc();
  

  hash_insert_int(lsf_driver->status_map , "PEND"   , JOB_QUEUE_PENDING);
  hash_insert_int(lsf_driver->status_map , "SSUSP"  , JOB_QUEUE_RUNNING);
  hash_insert_int(lsf_driver->status_map , "PSUSP"  , JOB_QUEUE_PENDING);
  hash_insert_int(lsf_driver->status_map , "RUN"    , JOB_QUEUE_RUNNING);
  hash_insert_int(lsf_driver->status_map , "EXIT"   , JOB_QUEUE_EXIT);
  hash_insert_int(lsf_driver->status_map , "USUSP"  , JOB_QUEUE_RUNNING);
  hash_insert_int(lsf_driver->status_map , "DONE"   , JOB_QUEUE_DONE);
  hash_insert_int(lsf_driver->status_map , "UNKWN"  , JOB_QUEUE_EXIT);    /* Uncertain about this one */
  pthread_mutex_init( &lsf_driver->bjobs_mutex , NULL );
  lsf_driver_set_remote_server( lsf_driver , NULL );
  lsf_driver_set_rsh_cmd( lsf_driver , DEFAULT_RSH_CMD );
  return lsf_driver;
}


/*****************************************************************/


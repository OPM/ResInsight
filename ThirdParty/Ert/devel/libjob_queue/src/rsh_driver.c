/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rsh_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>

#include <ert/util/util.h>
#include <ert/util/arg_pack.h>

#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/rsh_driver.h>






struct rsh_job_struct {
  UTIL_TYPE_ID_DECLARATION;
  bool         active;       /* Means that it allocated - not really in use */ 
  job_status_type status;        
  pthread_t    run_thread;
  const char * host_name;    /* Currently not set */
  char       * run_path;
};



typedef struct {
  char            * host_name;
  int               max_running;  /* How many can the host handle. */
  int               running;      /* How many are currently running on the host (goverened by this driver instance that is). */
  pthread_mutex_t   host_mutex;
} rsh_host_type;



#define RSH_DRIVER_TYPE_ID 44963256
#define RSH_JOB_TYPE_ID    63256701


struct rsh_driver_struct {
  UTIL_TYPE_ID_DECLARATION;
  pthread_mutex_t     submit_lock;
  pthread_attr_t      thread_attr;
  char              * rsh_command;
  int                 num_hosts;
  int                 last_host_index;
  rsh_host_type     **host_list;
  hash_type          *__host_hash;  /* Stupid redundancy ... */
};



/******************************************************************/
static UTIL_SAFE_CAST_FUNCTION_CONST( rsh_driver , RSH_DRIVER_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( rsh_driver , RSH_DRIVER_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( rsh_job , RSH_JOB_TYPE_ID )



/**
   If the host is for some reason not available, NULL should be
   returned. Will also return NULL if some funny guy tries to allocate
   with max_running <= 0.  
*/

static rsh_host_type * rsh_host_alloc(const char * host_name , int max_running) {
  if (max_running > 0) {
    struct addrinfo * result;
    if (getaddrinfo(host_name , NULL , NULL , &result) == 0) {
      rsh_host_type * host = util_malloc(sizeof * host );
      
      host->host_name   = util_alloc_string_copy(host_name);
      host->max_running = max_running;
      host->running     = 0;
      pthread_mutex_init( &host->host_mutex , NULL );
      
      freeaddrinfo( result );
      return host;
    } else {
      fprintf(stderr,"** Warning: could not locate server: %s \n",host_name);
      return NULL;
    }
  } else
    return NULL;
}



static void rsh_host_free(rsh_host_type * rsh_host) {
  free(rsh_host->host_name);
  free(rsh_host);
}


static bool rsh_host_available(rsh_host_type * rsh_host) {
  bool available;

  pthread_mutex_lock( &rsh_host->host_mutex );
  {
    available = false;
    if ((rsh_host->max_running - rsh_host->running) > 0) {  // The host has free slots()
      if (util_ping( rsh_host->host_name )) {                // The host answers to ping()
        available = true;
        rsh_host->running++;
      }
    } 
  }
  pthread_mutex_unlock( &rsh_host->host_mutex );

  return available;
}





static void rsh_host_submit_job(rsh_host_type * rsh_host , rsh_job_type * job, const char * rsh_cmd , const char * submit_cmd , int num_cpu , int job_argc , const char ** job_argv) {
  /* 
     Observe that this job has already been added to the running jobs
     in the rsh_host_available function.
  */
  int argc           = job_argc + 2;
  const char ** argv = util_malloc( argc * sizeof * argv );
  
  argv[0] = rsh_host->host_name;
  argv[1] = submit_cmd;
  {
    int iarg;
    for (iarg = 0; iarg < job_argc; iarg++)
      argv[iarg + 2] = job_argv[iarg];
  }
  
  util_fork_exec(rsh_cmd , argc , argv , true , NULL , NULL , NULL , NULL , NULL);   /* This call is blocking. */
  job->status = JOB_QUEUE_DONE;

  pthread_mutex_lock( &rsh_host->host_mutex );
  rsh_host->running--;
  pthread_mutex_unlock( &rsh_host->host_mutex );  
  free( argv );
}


/*
  static const char * rsh_host_get_hostname(const rsh_host_type * host) { return host->host_name; }
*/



static void * rsh_host_submit_job__(void * __arg_pack) {
  arg_pack_type * arg_pack = arg_pack_safe_cast(__arg_pack);
  char * rsh_cmd           = arg_pack_iget_ptr(arg_pack , 0); 
  rsh_host_type * rsh_host = arg_pack_iget_ptr(arg_pack , 1);
  char * submit_cmd        = arg_pack_iget_ptr(arg_pack , 2); 
  int num_cpu              = arg_pack_iget_int(arg_pack , 3);    
  int argc                 = arg_pack_iget_int(arg_pack , 4); 
  const char ** argv       = arg_pack_iget_ptr(arg_pack , 5); 
  rsh_job_type * job       = arg_pack_iget_ptr(arg_pack , 6);
  
  rsh_host_submit_job(rsh_host , job , rsh_cmd , submit_cmd , num_cpu , argc , argv);
  pthread_exit( NULL );
  arg_pack_free( arg_pack );
}



  

/*****************************************************************/


/*****************************************************************/




rsh_job_type * rsh_job_alloc(const char * run_path) {
  rsh_job_type * job;
  job = util_malloc(sizeof * job );
  job->active     = false;
  job->status     = JOB_QUEUE_WAITING;
  job->run_path   = util_alloc_string_copy(run_path);
  UTIL_TYPE_ID_INIT( job , RSH_JOB_TYPE_ID );
  return job;
}



void rsh_job_free(rsh_job_type * job) {
  free(job->run_path);
  free(job);
}




job_status_type rsh_driver_get_job_status(void * __driver , void * __job) {
  if (__job == NULL) 
    /* The job has not been registered at all ... */
    return JOB_QUEUE_NOT_ACTIVE;
  else {
    rsh_job_type    * job    = rsh_job_safe_cast( __job );
    {
      if (job->active == false) {
        util_abort("%s: internal error - should not query status on inactive jobs \n" , __func__);
        return JOB_QUEUE_NOT_ACTIVE;   /* Dummy to shut up compiler */
      } else 
        return job->status;
    }
  }
}



void rsh_driver_free_job( void * __job ) {
  rsh_job_type    * job    = rsh_job_safe_cast( __job );
  rsh_job_free(job);
}



void rsh_driver_kill_job(void * __driver ,void  * __job) {
  rsh_job_type    * job    = rsh_job_safe_cast( __job );
  if (job->active)
    pthread_cancel( job->run_thread );
  rsh_job_free( job );
}



void * rsh_driver_submit_job(void  * __driver, 
                             const char  * submit_cmd      , 
                             int           num_cpu         ,     /* Ignored */
                             const char  * run_path        ,
                             const char  * job_name        ,
                             int           argc, 
                             const char ** argv ) {
  
  rsh_driver_type * driver = rsh_driver_safe_cast( __driver );
  rsh_job_type  * job      = NULL; 
  {
    /* 
       command is freed in the start_routine() function
    */
    pthread_mutex_lock( &driver->submit_lock );
    {
      rsh_host_type * host = NULL;
      int ihost;
      int host_index = 0;
      
      if (driver->num_hosts == 0)
        util_abort("%s: fatal error - no hosts added to the rsh driver.\n",__func__);
      
      for (ihost = 0; ihost < driver->num_hosts; ihost++) {
        host_index = (ihost + driver->last_host_index) % driver->num_hosts;
        if (rsh_host_available(driver->host_list[host_index])) {
          host = driver->host_list[host_index];
          break;
        } 
      }
      driver->last_host_index = (host_index + 1) % driver->num_hosts;
      
      if (host != NULL) {
        /* A host is available */
        arg_pack_type * arg_pack = arg_pack_alloc();   /* The arg_pack is freed() in the rsh_host_submit_job__() function.
                                                          freeing it here is dangerous, because we might free it before the 
                                                          thread-called function is finished with it. */
        
        job = rsh_job_alloc(run_path);
        
        arg_pack_append_ptr(arg_pack ,  driver->rsh_command);
        arg_pack_append_ptr(arg_pack ,  host);
        arg_pack_append_ptr(arg_pack , (char *) submit_cmd);
        arg_pack_append_int(arg_pack , num_cpu );
        arg_pack_append_int(arg_pack , argc );
        arg_pack_append_ptr(arg_pack , argv );
        arg_pack_append_ptr(arg_pack , job);  
        
        {
          int pthread_return_value = pthread_create( &job->run_thread , &driver->thread_attr , rsh_host_submit_job__ , arg_pack);
          if (pthread_return_value != 0) 
            util_abort("%s failed to create thread ERROR:%d  \n", __func__ , pthread_return_value);
        }
        job->status = JOB_QUEUE_RUNNING; 
        job->active = true;
      }
    }
    pthread_mutex_unlock( &driver->submit_lock );
  }
  return job;
}


void rsh_driver_clear_host_list( rsh_driver_type * driver ) {
  int ihost;
  for (ihost =0; ihost < driver->num_hosts; ihost++) 
    rsh_host_free(driver->host_list[ihost]);
  util_safe_free(driver->host_list);
  
  driver->num_hosts            = 0;
  driver->host_list            = NULL;
  driver->last_host_index      = 0;  
}


void rsh_driver_free(rsh_driver_type * driver) {
  rsh_driver_clear_host_list( driver ); 
  pthread_attr_destroy ( &driver->thread_attr );
  util_safe_free(driver->rsh_command );
  hash_free( driver->__host_hash );
  free(driver);
  driver = NULL;
}


void rsh_driver_free__(void * __driver) {
  rsh_driver_type * driver = rsh_driver_safe_cast( __driver );
  rsh_driver_free( driver );
}


void rsh_driver_set_host_list( rsh_driver_type * rsh_driver , const hash_type * rsh_host_list) {
  rsh_driver_clear_host_list( rsh_driver ); 
  if (rsh_host_list != NULL) {
    hash_iter_type * hash_iter = hash_iter_alloc( rsh_host_list );
    while (!hash_iter_is_complete( hash_iter )) {
      const char * host = hash_iter_get_next_key( hash_iter );
      int max_running   = hash_get_int( rsh_host_list , host );
      rsh_driver_add_host(rsh_driver , host , max_running);
    }
    if (rsh_driver->num_hosts == 0) 
      util_abort("%s: failed to add any valid RSH hosts - aborting.\n",__func__);
  }
}




/**
   
*/

void * rsh_driver_alloc( ) {
  rsh_driver_type * rsh_driver = util_malloc( sizeof * rsh_driver );
  UTIL_TYPE_ID_INIT( rsh_driver , RSH_DRIVER_TYPE_ID );
  pthread_mutex_init( &rsh_driver->submit_lock , NULL );
  pthread_attr_init( &rsh_driver->thread_attr );
  pthread_attr_setdetachstate( &rsh_driver->thread_attr , PTHREAD_CREATE_DETACHED );

  /** 
      To simplify the Python wrapper it is possible to pass in NULL as
      rsh_host_list pointer, and then subsequently add hosts with
      rsh_driver_add_host().
  */
  rsh_driver->num_hosts       = 0;
  rsh_driver->host_list       = NULL;
  rsh_driver->last_host_index = 0;  
  rsh_driver->rsh_command = NULL;
  rsh_driver->__host_hash = hash_alloc();
  return rsh_driver;
}



void rsh_driver_add_host(rsh_driver_type * rsh_driver , const char * hostname , int host_max_running) {
  rsh_host_type * new_host = rsh_host_alloc(hostname , host_max_running);  /* Could in principle update an existing node if the host name is old. */
  if (new_host != NULL) {
    rsh_driver->num_hosts++;
    rsh_driver->host_list = util_realloc(rsh_driver->host_list , rsh_driver->num_hosts * sizeof * rsh_driver->host_list );
    rsh_driver->host_list[(rsh_driver->num_hosts - 1)] = new_host;
  }
}


/**
   Hostname should be a string as host:max_running, the ":max_running"
   part is optional, and will default to 1.  
*/
   
void rsh_driver_add_host_from_string(rsh_driver_type * rsh_driver , const char * hostname) {
  int     host_max_running;
  char ** tmp;
  char  * host;
  int     tokens;
  
  util_split_string( hostname , ":" , &tokens , &tmp);
  if (tokens > 1) {
    if (!util_sscanf_int( tmp[tokens - 1] , &host_max_running))
      util_abort("%s: failed to parse out integer from: %s \n",__func__ , hostname);
    host = util_alloc_joined_string((const char **) tmp , tokens - 1 , ":");
  } else
    host = util_alloc_string_copy( tmp[0] );
  rsh_driver_add_host( rsh_driver , host , host_max_running );
  
  util_free_stringlist( tmp , tokens );
  free( host );
}




bool rsh_driver_set_option( void * __driver , const char * option_key , const void * value ) {
  rsh_driver_type * driver = rsh_driver_safe_cast( __driver );
  bool has_option = true;
  {
    if (strcmp(RSH_HOST , option_key) == 0)                 /* Add one host - value should be hostname:max */  
      rsh_driver_add_host_from_string( driver , value );
    else if (strcmp(RSH_HOSTLIST , option_key) == 0) {      /* Set full host list - value should be hash of integers. */
      if (value != NULL) {
        hash_safe_cast( value );
        rsh_driver_set_host_list( driver , value );
      }
    } else if (strcmp( RSH_CLEAR_HOSTLIST , option_key) == 0)
      /* Value is not considered - this is an action, and not a _set operation. */
      rsh_driver_set_host_list( driver , NULL );
    else if (strcmp( RSH_CMD , option_key) == 0)
      driver->rsh_command = util_realloc_string_copy( driver->rsh_command , value );
    else
      has_option = false;
  }
  return has_option;
}


const void * rsh_driver_get_option( const void * __driver , const char * option_key ) {
  const rsh_driver_type * driver = rsh_driver_safe_cast_const( __driver );
  {
    if (strcmp( RSH_CMD , option_key ) == 0)
      return driver->rsh_command;
    else if (strcmp( RSH_HOSTLIST , option_key) == 0) {
      int ihost;
      hash_clear( driver->__host_hash );
      for (ihost = 0; ihost < driver->num_hosts; ihost++) {
        rsh_host_type * host = driver->host_list[ ihost ];
        hash_insert_int( driver->__host_hash , host->host_name , host->max_running);
      }
      return driver->__host_hash;
    } else {
      util_abort("%s: get not implemented fro option_id:%s for rsh \n",__func__ , option_key );
      return NULL;
    }
  }
}



#undef RSH_JOB_ID    

/*****************************************************************/


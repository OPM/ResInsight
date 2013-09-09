/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'block_node.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <signal.h>
#include <lsf/lsbatch.h>

#include <ert/util/stringlist.h>
#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/vector.h>

#include <ert/job_queue/lsf_driver.h>

#define BLOCK_COMMAND        "/project/res/bin/block-job"
#define STATOIL_LSF_REQUEST  "select[cs && x86_64Linux]"



static lsf_driver_type * lsf_driver;
static vector_type * job_pool;
static hash_type * nodes;


typedef struct {
  lsf_job_type    * lsf_job;
  stringlist_type * hostlist;
  bool              running;
  bool              block_job;
} block_job_type;


typedef struct {
  int target;
  int current;
} count_pair_type;


count_pair_type * count_pair_alloc() {
  count_pair_type * pair = util_malloc( sizeof * pair );
  pair->target = 0;
  pair->current = 0;
  return pair;
}




block_job_type * block_job_alloc() {
  block_job_type * job = util_malloc( sizeof * job );

  job->lsf_job   = NULL;
  job->running   = false;
  job->block_job = false;
  job->hostlist  = stringlist_alloc_new();

  return job;
}


void block_job_free( block_job_type * block_job ) {
  stringlist_free( block_job->hostlist );
  if (block_job->lsf_job)
    lsf_job_free( block_job->lsf_job );
  
  free( block_job );
}


const char * block_job_get_hostname( const block_job_type * block_job ) {
  return stringlist_iget( block_job->hostlist , 0 );
}

  

void update_job_status( block_job_type * job ) {
  if (!job->running) {
    int lsf_status = lsf_driver_get_job_status_lsf( lsf_driver , job->lsf_job );
    if (lsf_status == JOB_STAT_RUN) {
      lsf_job_export_hostnames( job->lsf_job , job->hostlist ); 
      {
        int ihost;
        for (ihost = 0; ihost < stringlist_get_size( job->hostlist ); ihost++) {
          const char * host = stringlist_iget( job->hostlist , ihost );
          if (hash_has_key( nodes, host))  { /* This is one of the instances which should be left running. */
            count_pair_type * pair = hash_get( nodes , host);
            if (pair->current < pair->target) {
              pair->current += 1;
              job->block_job = true;
            }
          }
        }
      }
      job->running = true;
    }
  }
}



/*****************************************************************/



void add_jobs( int chunk_size) {
  int i;
  char * cwd = util_alloc_cwd();
  for (i=0; i < chunk_size; i++) {
    block_job_type * job = block_job_alloc();
    job->lsf_job = lsf_driver_submit_job(lsf_driver , BLOCK_COMMAND , 1 , cwd , "BLOCK" , 0 , NULL );
    vector_append_ref( job_pool , job );
  }
  free( cwd );
}


void update_pool_status( bool *all_blocked , int * pending) {
  int i;
  int pend_count   = 0;
  *all_blocked = true;

  for (i=0; i < vector_get_size( job_pool ); i++) {
    block_job_type * job = vector_iget( job_pool , i );
    update_job_status( job );

    if (!job->running)
      pend_count++;
  }

  {
    hash_iter_type * iter = hash_iter_alloc( nodes );
    while (!hash_iter_is_complete( iter )) {
      const char * hostname = hash_iter_get_next_key( iter );
      const count_pair_type * count = hash_get( nodes , hostname );
      if (count->current < count->target)
        *all_blocked = false;
    }
  }
  *pending = pend_count;
}


void print_status() {
  int total_running = 0;
  int total_pending = 0;
  for (int i=0; i < vector_get_size( job_pool ); i++) {
    block_job_type * job = vector_iget( job_pool , i );
    if (job->running)
      total_running += 1;
    else
      total_pending += 1;
  }
  printf("Running:%3d  Pending: %3d   Blocks active: ",total_running , total_pending);
  {
    hash_iter_type * iter = hash_iter_alloc( nodes );
    while (!hash_iter_is_complete( iter )) {
      const char * hostname = hash_iter_get_next_key( iter );
      const count_pair_type * count = hash_get( nodes , hostname );
      printf("%s %d/%d   ",hostname , count->current , count->target);
    }
    printf("\n");
    hash_iter_free( iter );
  }
}


void block_node_exit( int signal ) {
  int job_nr;

  print_status();
  for (job_nr = 0; job_nr < vector_get_size( job_pool ); job_nr++) {
    block_job_type * job = vector_iget( job_pool , job_nr );
    
    if (job->block_job) {
      printf("Job:%ld is running on host: ", lsf_job_get_jobnr( job->lsf_job ));
      stringlist_fprintf( job->hostlist , " " , stdout );
      printf("\n");
    } else
      lsf_driver_kill_job( lsf_driver , job->lsf_job );
    
    block_job_free( job );
  }
  printf("Remember to kill these jobs when the BLOCK is no longer needed\n");
  if (signal != 0)
    exit(0);
}


int main( int argc, char ** argv) {
  if (argc == 1)
    util_exit("block_node  node1  node2  node3:2  \n");
  
  /* Initialize lsf environment */
  util_setenv( "LSF_BINDIR"    , "/prog/LSF/8.0/linux2.6-glibc2.3-x86_64/bin" );
  util_setenv( "LSF_LINDIR"    , "/prog/LSF/8.0/linux2.6-glibc2.3-x86_64/lib" );
  util_setenv( "XLSF_UIDDIR"   , "/prog/LSF/8.0/linux2.6-glibc2.3-x86_64/lib/uid" );
  util_setenv( "LSF_SERVERDIR" , "/prog/LSF/8.0/linux2.6-glibc2.3-x86_64/etc");
  util_setenv( "LSF_ENVDIR"    , "/prog/LSF/conf");
  
  util_update_path_var( "PATH"               , "/prog/LSF/8.0/linux2.6-glibc2.3-x86_64/bin" , false);
  util_update_path_var( "LD_LIBRARY_PATH"    , "/prog/LSF/8.0/linux2.6-glibc2.3-x86_64/lib" , false);

  
  lsf_driver = lsf_driver_alloc();
  if (lsf_driver_get_submit_method( lsf_driver ) != LSF_SUBMIT_INTERNAL)
    util_exit("Sorry - the block_node program must be invoked on a proper LSF node \n");

  {
    
    int iarg;
    int total_blocked_target = 0;
    nodes       = hash_alloc();
    for (iarg = 1; iarg < argc; iarg++) {
      char   *node_name;
      int    num_slots;
      
      {
        char * num_slots_string;
        util_binary_split_string( argv[iarg] , ":" , true , &node_name , &num_slots_string);
        if (num_slots_string)
          util_sscanf_int( num_slots_string , &num_slots);
        else
          num_slots = 1;
      }
      
      if (!hash_has_key( nodes , node_name))
        hash_insert_hash_owned_ref( nodes , node_name , count_pair_alloc() , free);

      {
        count_pair_type * pair = hash_get( nodes , node_name);
        pair->target += num_slots;
      }
      total_blocked_target += num_slots;
    }

    signal(SIGINT , block_node_exit );
    {
      const int sleep_time    = 5;
      const int chunk_size    = 10;    /* We submit this many at a time. */
      const int max_pool_size = 1000;  /* The absolute total maximum of jobs we will submit. */  

      bool           cont        = true;
      int            pending     = 0;   
      bool           all_blocked;
      job_pool                   = vector_alloc_new();

      while (cont) {
        printf("[Ctrl-C to give up] "); fflush( stdout );
        if (cont) sleep( sleep_time );
        if (pending == 0) {
          if (vector_get_size( job_pool ) < max_pool_size)
            add_jobs( chunk_size );
        }
        
        update_pool_status( &all_blocked , &pending);
        print_status();

        if (all_blocked)
          cont = false;
      }
      if (!all_blocked)
        printf("Sorry - failed to block all the nodes \n");
      
      block_node_exit( 0 );
      hash_free( nodes );
    }
  }
}

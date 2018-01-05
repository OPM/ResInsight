/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'thread_pool_posix.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "ert/util/build_config.h"

#include <ert/util/thread_pool.h>
#include <ert/util/util.h>
#include <ert/util/type_macros.h>


/**
   This file implements a small thread_pool object based on
   pthread_create() function calls. The characetristics of this
   implementation is as follows:

    1. The jobs are mangaged by a separate thread - dispatch_thread.
    2. The new jobs are just appended to the queue, the
       dispatch_thread sees them in the queue and dispatches them.
    3. The dispatch thread manages a list of thread_pool_job_slot_type
       instances - one slot for each actually running job.

   Example
   -------

   1. Start with creating a thread pool object. The arguments to the
      allocater are the (maximum) number of concurrently running
      threads and a boolean flag of whether the queue should start
      immediately (that is in general the case).

        thread_pool_type * tp = thread_pool_alloc( NUM_THREADS , immediate_start);


   2. Add the jobs you want to run:

        thread_pool_add_job( tp , some_function , argument_to_some_function );

      Here the prototype for the function which is being run is

        void * (some_func) (void *);

      I.e. it expects a (void *) input pointer, and also returns a
      (void *) pointer as output. The thread pool implementation does
      not touch the input and output of some_function.


  3.  When all the jobs have been added you inform the thread pool of
      that by calling:

         thread_pool_join( tp );

      This function will not return before all the added jobs have run
      to completion.


  4. Optional: If you want to get the return value from the function
     you supplied, you can use:

         thread_pool_iget_return_value( tp , index );

     To get the return value from function nr index.


  5. Optional: The thread pool will probably mainly be used only once,
     but after a join it is possible to reuse a thread pool, but then
     you MUST call thread_pool_restart() before adding jobs again.


  6. When you are really finished: thread_pool_free( tp );

*/


typedef void * (start_func_ftype) (void *) ;


/**
   Internal struct which is used as queue node.
*/
typedef struct {
  thread_pool_type * pool;                /* A back-reference to the thread_pool holding the queue. */
  int                slot_index;          /* The index in the space [0,max_running) of the job slot where this job is running. */
  int                queue_index;         /* The index of the current tp_arg in the queue. */
  void             * func_arg;            /* The arguments to this job - supplied by the calling scope. */
  start_func_ftype * func;                /* The function to call - supplied by the calling scope. */
  void             * return_value;
} thread_pool_arg_type;



/**
   Internal struct used to keep track of the job slots.
*/
typedef struct {
  pthread_t  thread;                /* The thread variable currently (or more correct:last) running. */
  int        run_count;             /* The number of times this slot has been used - just to check whether the slot has been used AT ALL when/if joining. */
  bool       running;               /* Is the job_slot running now?? */
} thread_pool_job_slot_type;




#define THREAD_POOL_TYPE_ID 71443207
struct thread_pool_struct {
  UTIL_TYPE_ID_DECLARATION;
  thread_pool_arg_type      * queue;              /* The jobs to be executed are appended in this vector. */
  int                         queue_index;        /* The index of the next job to run. */
  int                         queue_size;         /* The number of jobs in the queue - including those which are complete. [Should be protected / atomic / ... ] */
  int                         queue_alloc_size;   /* The allocated size of the queue. */

  int                         max_running;        /* The max number of concurrently running jobs. */
  bool                        join;               /* Flag set by the main thread to inform the dispatch thread that joining should start. */
  bool                        accepting_jobs;     /* True|False whether the dispatch thread is running. */

  thread_pool_job_slot_type * job_slots;          /* A vector to @max_running job slots, each slot can be reused several times.*/
  pthread_t                   dispatch_thread;
  pthread_rwlock_t            queue_lock;
};


static UTIL_SAFE_CAST_FUNCTION( thread_pool , THREAD_POOL_TYPE_ID )


/**
   This function will grow the queue. It is called by the main thread
   (i.e. the context of the calling scope), and the queue is read by
   the dispatch_thread - i.e. access to the queue must be protected by
   rwlock.
*/

static void thread_pool_resize_queue( thread_pool_type * pool, int queue_length ) {
  pthread_rwlock_wrlock( &pool->queue_lock );
  {
    pool->queue            = (thread_pool_arg_type*)util_realloc( pool->queue , queue_length * sizeof * pool->queue );
    pool->queue_alloc_size = queue_length;
  }
  pthread_rwlock_unlock( &pool->queue_lock );
}


/**
   This function updates an element in the queue, the function is
   called by the executing threads, on the same time the main thread
   might be resizing the thread, we therefor take a read lock during
   execution of this function. (Write lock is not necessary because we
   will not change the queue pointer itself, only something it points
   to.)
*/

static void thread_pool_iset_return_value( thread_pool_type * pool , int index , void * return_value) {
  pthread_rwlock_rdlock( &pool->queue_lock );
  {
    pool->queue[ index ].return_value = return_value;
  }
  pthread_rwlock_unlock( &pool->queue_lock );
}


void * thread_pool_iget_return_value( const thread_pool_type * pool , int queue_index ) {
  return pool->queue[ queue_index ].return_value;
}


/**
   The pthread_create() call which this is all about, does not start
   the user supplied function. Instead it will start an instance of
   this function, which will do some housekeeping before calling the
   user supplied function.
*/

static void * thread_pool_start_job( void * arg ) {
  thread_pool_arg_type * tp_arg = (thread_pool_arg_type * ) arg;
  thread_pool_type * tp         =  tp_arg->pool;
  int slot_index                =  tp_arg->slot_index;
  void * func_arg               =  tp_arg->func_arg;
  start_func_ftype * func       =  tp_arg->func;
  void * return_value;


  return_value = func( func_arg );                  /* Starting the real external function */
  tp->job_slots[ slot_index ].running = false;      /* We mark the job as completed. */
  free( arg );

  if (return_value != NULL)
    thread_pool_iset_return_value( tp , tp_arg->queue_index , return_value);

  return NULL;
}



/**
   This function is run by the dispatch_thread. The thread will keep
   an eye on the queue, and dispatch new jobs when there are free
   slots available.
*/

static void * thread_pool_main_loop( void * arg ) {
  thread_pool_type * tp = thread_pool_safe_cast( arg );
  {
    const int usleep_init = 1000;  /* The sleep time when there are free slots available - but no jobs wanting to run. */
    int internal_offset   = 0;     /* Keep track of the (index of) the last job slot fired off - minor time saving. */
    while (true) {
      if (tp->queue_size > tp->queue_index) {
        /*
           There are jobs in the queue which would like to run -
           let us see if we can find a slot for them.
        */
        int counter     = 0;
        bool slot_found = false;
        do {
          int slot_index = (counter + internal_offset) % tp->max_running;
          thread_pool_job_slot_type * job_slot = &tp->job_slots[ slot_index ];
          if (!job_slot->running) {
            /* OK thread[slot_index] is ready to take this job.*/
            thread_pool_arg_type * tp_arg;

            /*
               The queue might be updated by the main thread - we must
               take a copy of the node we are interested in.
            */
            pthread_rwlock_rdlock( &tp->queue_lock );
            tp_arg = (thread_pool_arg_type*)util_alloc_copy( &tp->queue[ tp->queue_index ] , sizeof * tp_arg );
            pthread_rwlock_unlock( &tp->queue_lock );

            tp_arg->slot_index = slot_index;
            job_slot->running = true;
            /*
               Here is the actual pthread_create() call creating an
               additional running thread.
            */

            /*Cleanup of previous run threads. Needed to avoid memory leak*/
            if (job_slot->run_count > 0)
              pthread_join(job_slot->thread, NULL);

            pthread_create( &job_slot->thread , NULL , thread_pool_start_job , tp_arg );
            job_slot->run_count += 1;
            tp->queue_index++;
            internal_offset += (counter + 1);
            slot_found = true;
          } else
            counter++;
        } while (!slot_found && (counter < tp->max_running));

        if (!slot_found) {
            util_yield();
        }
      } else
        util_usleep(usleep_init);    /* There are no jobs wanting to run. */

      /*****************************************************************/
      /*
        We exit explicitly from this loop when both conditions apply:

         1. tp->join       == true              :  The calling scope has signaled that it will not submit more jobs.
         2. tp->queue_size == tp->queue_index   :  This function has submitted all the jobs in the queue.
      */
      if ((tp->join) && (tp->queue_size == tp->queue_index))
        break;
    } /* End of while() loop */
  }

  /*
     There are no more jobs in the queue, and the main scope has
     signaled that join should start. Observe that we join only the
     jobs corresponding to explicitly running job_slots; when a job
     slot is used multiple times the first jobs run in the job_slot
     will not be explicitly joined.
  */
  {
    int i;
    for (i=0; i < tp->max_running; i++) {
      thread_pool_job_slot_type job_slot = tp->job_slots[i];
      if (job_slot.run_count > 0)
        pthread_join( job_slot.thread , NULL );
    }
  }
  /* When we are here all the jobs have completed. */
  return NULL;
}




/**
   This function initializes a couple of counters, and starts up the
   dispatch thread. If the thread_pool should be reused after a join,
   this function must be called before adding new jobs.

   The functions thread_pool_restart() and thread_pool_join() should
   be joined up like open/close and malloc/free combinations.
*/

void thread_pool_restart( thread_pool_type * tp ) {
  if (tp->accepting_jobs)
    util_abort("%s: fatal error - tried restart already running thread pool\n",__func__);
  {
    tp->join           = false;
    tp->queue_index    = 0;
    tp->queue_size     = 0;
    {
      int i;
      for (i=0; i < tp->max_running; i++) {
        tp->job_slots[i].run_count = 0;
        tp->job_slots[i].running   = false;
      }
    }

    /* Starting the dispatch thread. */
    pthread_create( &tp->dispatch_thread , NULL , thread_pool_main_loop , tp );
    tp->accepting_jobs = true;
  }
}



/**
   This function is called by the calling scope when all the jobs have
   been submitted, and we just wait for them to complete.

   This function just sets the join switch to true - this again tells
   the dispatch_thread to start the join process on the worker
   threads.
*/

void thread_pool_join(thread_pool_type * pool) {
  pool->join = true;                               /* Signals to the main thread that joining can start. */
  if (pool->max_running > 0) {
    pthread_join( pool->dispatch_thread , NULL );  /* Wait for the main thread to complete. */
    pool->accepting_jobs = false;
  }
}

/*
  This will try to join the thread; if the manager thread has not
  completed within @timeout_seconds the function will return false. If
  the join fails the queue will be reset in a non-joining state and it
  will be open for more jobs. Probably not in a 100% sane state.
*/

bool thread_pool_try_join(thread_pool_type * pool, int timeout_seconds) {
  bool join_ok = true;

  pool->join = true;                               /* Signals to the main thread that joining can start. */
  if (pool->max_running > 0) {
    time_t timeout_time = time( NULL );
    util_inplace_forward_seconds_utc(&timeout_time , timeout_seconds );

#ifdef HAVE_TIMEDJOIN

    struct timespec ts;
    ts.tv_sec = timeout_time;
    ts.tv_nsec = 0;

    {
      int join_return = pthread_timedjoin_np( pool->dispatch_thread , NULL , &ts);  /* Wait for the main thread to complete. */
      if (join_return == 0)
        pool->accepting_jobs = false;
      else {
        pool->join = false;
        join_ok = false;
      }
    }

#else

    while(true) {
        if (pthread_kill(pool->dispatch_thread, 0) == 0){
            util_yield();
        } else {
            pthread_join(pool->dispatch_thread, NULL);
            pool->accepting_jobs = false;
            break;
        }

        time_t now = time(NULL);

        if(util_difftime_seconds(now, timeout_time) <= 0) {
            join_ok = false;
            break;
        }
    }

#endif



  }
  return join_ok;
}




/**
   max_running is the maximum number of concurrent threads. If
   @start_queue is true the dispatch thread will start immediately. If
   the function is called with @start_queue == false you must first
   call thread_pool_restart() BEFORE you can start adding jobs.
*/

thread_pool_type * thread_pool_alloc(int max_running , bool start_queue) {
  thread_pool_type * pool = (thread_pool_type*)util_malloc( sizeof *pool );
  UTIL_TYPE_ID_INIT( pool , THREAD_POOL_TYPE_ID );
  pool->job_slots         = (thread_pool_job_slot_type*)util_calloc( max_running , sizeof * pool->job_slots );
  pool->max_running       = max_running;
  pool->queue             = NULL;
  pool->accepting_jobs    = false;
  pthread_rwlock_init( &pool->queue_lock , NULL);
  thread_pool_resize_queue( pool  , 32 );
  if (start_queue)
    thread_pool_restart( pool );
  return pool;
}



void thread_pool_add_job(thread_pool_type * pool , start_func_ftype * start_func , void * func_arg ) {
  if (pool->max_running == 0) /* Blocking non-threaded mode: */
    start_func( func_arg );
  else {
    if (pool->accepting_jobs) {
      if (pool->queue_size == pool->queue_alloc_size)
        thread_pool_resize_queue( pool , pool->queue_alloc_size * 2);

      /*
         The new job is added to the queue - the main thread is watching
         the queue and will pick up the new job.
      */
      {
        int queue_index = pool->queue_size;

        pool->queue[ queue_index ].pool         = pool;
        pool->queue[ queue_index ].func_arg     = func_arg;
        pool->queue[ queue_index ].func         = start_func;
        pool->queue[ queue_index ].return_value = NULL;
        pool->queue[ queue_index ].queue_index = queue_index;
      }
      pool->queue_size++;  /* <- This is shared between this thread and the dispatch thread */
    } else
      util_abort("%s: thread_pool is not running - restart with thread_pool_restart()?? \n",__func__);
  }
}



/*
  Observe that this function does not join the worker threads,
  i.e. you should call thread_pool_join() first (otherwise the thing
  will go up in flames).
*/


void thread_pool_free(thread_pool_type * pool) {
  util_safe_free( pool->job_slots );
  util_safe_free( pool->queue );
  free(pool);
}

int thread_pool_get_max_running( const thread_pool_type * pool ) {
  return pool->max_running;
}

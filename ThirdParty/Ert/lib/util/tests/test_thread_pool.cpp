/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'test_thread_pool.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <pthread.h>

#include <ert/util/test_util.hpp>
#include <ert/util/thread_pool.hpp>


pthread_mutex_t lock;


void create_and_destroy() {
  int run_size = 10;
  thread_pool_type * tp = thread_pool_alloc( run_size , true );

  thread_pool_join( tp );
  thread_pool_free( tp );
}


void * inc(void * arg) {
  int * int_arg = (int *) arg;
  pthread_mutex_lock( &lock );
  int_arg[0]++;
  pthread_mutex_unlock( &lock );
  return NULL;
}


void run() {
  int run_size = 10;
  int job_size = 1000;
  int value = 0;
  thread_pool_type * tp = thread_pool_alloc( run_size , true );

  pthread_mutex_init(&lock , NULL);
  for (int i=0; i < job_size; i++)
    thread_pool_add_job( tp , inc , &value );

  thread_pool_join( tp );
  thread_pool_free( tp );
  test_assert_int_equal( job_size , value );
  pthread_mutex_destroy( &lock );
}






int main( int argc , char ** argv) {
  create_and_destroy();
  run();
}

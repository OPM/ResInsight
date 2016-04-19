/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_fs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "ert/util/build_config.h"

#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>
#include <ert/util/test_work_area.h>
#include <ert/enkf/enkf_fs.h>


typedef struct
{
    pthread_mutex_t mutex1;
    pthread_mutex_t mutex2;
} shared_data;

static shared_data* data = NULL;

void test_mount() {
  test_work_area_type * work_area = test_work_area_alloc("enkf_fs/mount");

  test_assert_false( enkf_fs_exists( "mnt" ));
  test_assert_NULL( enkf_fs_create_fs("mnt" , BLOCK_FS_DRIVER_ID , NULL , false));
  test_assert_true( enkf_fs_exists( "mnt" ));
  {
    enkf_fs_type * fs = enkf_fs_mount( "mnt"  );
    test_assert_true( util_file_exists("mnt/mnt.lock"));
    test_assert_true( enkf_fs_is_instance( fs ));
    enkf_fs_decref( fs );
    test_assert_false( util_file_exists("mnt/mnt.lock"));
  }
  {
    enkf_fs_type * fs = enkf_fs_create_fs( "mnt2" , BLOCK_FS_DRIVER_ID , NULL , true);
    test_assert_true( enkf_fs_is_instance( fs ));
    enkf_fs_decref( fs );
  }

  
  test_work_area_free( work_area );
}

void test_refcount() {
  test_work_area_type * work_area = test_work_area_alloc("enkf_fs/refcount");
  
  enkf_fs_create_fs("mnt" , BLOCK_FS_DRIVER_ID , NULL , false);
  {
    enkf_fs_type * fs = enkf_fs_mount( "mnt" );
    test_assert_int_equal( 1 , enkf_fs_get_refcount( fs ));
    enkf_fs_decref( fs );
  }
  test_work_area_free( work_area );
}

void createFS() {

 pthread_mutex_lock(&data->mutex1);
 pid_t pid = fork();

  if (pid == 0) {
    enkf_fs_type * fs_false = enkf_fs_mount( "mnt" );
    test_assert_false(enkf_fs_is_read_only(fs_false));
    test_assert_true( util_file_exists("mnt/mnt.lock"));
    pthread_mutex_unlock(&data->mutex1);
    pthread_mutex_lock(&data->mutex2);
    enkf_fs_decref( fs_false );
    pthread_mutex_unlock(&data->mutex2);
    exit(0);
  }
}

void test_fwrite_readonly( void * arg ) {
  enkf_fs_type * fs = enkf_fs_safe_cast( arg );
  /* 
     The arguments here are completely bogus; the important thing is
     that this fwrite call should be intercepted by a util_abort()
     call (which is again intercepted by the testing function) before
     the argument are actually accessed. 
  */
  enkf_fs_fwrite_node( fs , NULL , "KEY" , PARAMETER , 100 , 1 , FORECAST );
}

void initialise_shared()
{
    // place our shared data in shared memory
    int prot = PROT_READ | PROT_WRITE;
#ifdef __linux
    int flags = MAP_SHARED | MAP_ANONYMOUS;
#elif __APPLE__
    int flags = MAP_SHARED | MAP_ANON;
#endif

    data = mmap(NULL, sizeof(shared_data), prot, flags, -1, 0);
    assert(data);

    // initialise mutex so it works properly in shared memory
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->mutex1, &attr);
    pthread_mutex_init(&data->mutex2, &attr);
}

/*
  This test needs to fork off a seperate process to test the cross-process file locking.
*/
void test_read_only2() {
  initialise_shared();
  test_work_area_type * work_area = test_work_area_alloc("enkf_fs/read_only2");
  enkf_fs_create_fs("mnt" , BLOCK_FS_DRIVER_ID , NULL , false);
  pthread_mutex_lock(&data->mutex2);
  createFS();
  pthread_mutex_lock(&data->mutex1);
  {
    enkf_fs_type * fs_false = enkf_fs_mount( "mnt" );
    test_assert_true(enkf_fs_is_read_only(fs_false));
    test_assert_util_abort( "enkf_fs_fwrite_node" , test_fwrite_readonly , fs_false );
    enkf_fs_decref( fs_false );
  }
  pthread_mutex_unlock(&data->mutex2);
  pthread_mutex_unlock(&data->mutex1);
  pthread_mutex_lock(&data->mutex2);
  test_work_area_free( work_area );
  pthread_mutex_unlock(&data->mutex2);
  munmap(data, sizeof(data));
}

int main(int argc, char ** argv) {
  test_mount();
  test_refcount();
  test_read_only2();
  exit(0);
}

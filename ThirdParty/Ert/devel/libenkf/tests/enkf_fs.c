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

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/enkf/enkf_fs.h>







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
  pid_t pid = fork();

  if (pid == 0) {
    enkf_fs_type * fs_false = enkf_fs_mount( "mnt" );
    test_assert_false(enkf_fs_is_read_only(fs_false));
    test_assert_true( util_file_exists("mnt/mnt.lock"));
    {
      int total_sleep = 0;
      while (true) {
        if (util_file_exists( "stop")) {
          unlink("stop");
          break;
        }
        
        usleep(1000);
        total_sleep += 1000;
        if (total_sleep > 1000000 * 5) {
          fprintf(stderr,"Test failure - never receieved \"stop\" file from parent process \n");
          break;
        }
      }
    }
    enkf_fs_decref( fs_false );
    exit(0);
  } 
  usleep(10000);
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


/*
  This test needs to fork off a seperate process to test the cross-process file locking. 
*/
void test_read_only2() {
  test_work_area_type * work_area = test_work_area_alloc("enkf_fs/read_only2");
  enkf_fs_create_fs("mnt" , BLOCK_FS_DRIVER_ID , NULL , false);
  createFS();

  while (true) {
    if (util_file_exists("mnt/mnt.lock"))
      break;
  }
  
  {
    enkf_fs_type * fs_false = enkf_fs_mount( "mnt" );
    test_assert_true(enkf_fs_is_read_only(fs_false));
    test_assert_util_abort( "enkf_fs_fwrite_node" , test_fwrite_readonly , fs_false );
    enkf_fs_decref( fs_false );
  }
  {
    FILE * stream = util_fopen("stop" , "w");
    fclose( stream );
  }
  
  while (util_file_exists( "stop")) {
    usleep( 1000 );
  }      
  test_work_area_free( work_area );
}








int main(int argc, char ** argv) {
  test_mount();
  test_refcount();
  test_read_only2();
  exit(0);
}

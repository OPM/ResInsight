/*                
   Copyright (C) 2014  Statoil ASA, Norway. 
   
   The file 'ert_util_block_fs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>


#include <ert/util/block_fs.h>
#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>

void test_assert_util_abort(const char * function_name , void call_func (void *) , void * arg);


void violating_fwrite( void * arg ) {
  block_fs_type * bfs = block_fs_safe_cast( arg );
  block_fs_fwrite_file( bfs , "name" , NULL , 100 );
}


void test_readonly( ) {
  test_work_area_type * work_area = test_work_area_alloc("block_fs/read_only");
  block_fs_type * bfs = block_fs_mount( "test.mnt" , 1000 , 10000 , 0.67 , 10 , true , true , false );
  test_assert_true( block_fs_is_readonly( bfs ));
  test_assert_util_abort("block_fs_aquire_wlock" , violating_fwrite , bfs );
  block_fs_close(bfs , true);
  test_work_area_free( work_area );
}


void createFS1() {
  pid_t pid = fork();

  if (pid == 0) {
    block_fs_type * bfs = block_fs_mount( "test.mnt" , 1000 , 10000 , 0.67 , 10 , true , false , true );
    test_assert_false( block_fs_is_readonly( bfs ) );
    test_assert_true( util_file_exists("test.lock_0"));
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
    block_fs_close( bfs , false );
    exit(0);
  } 
  usleep(10000);
}



void test_lock_conflict() {
  test_work_area_type * work_area = test_work_area_alloc("block_fs/lock_conflict");
  createFS1();
  while (true) {
    if (util_file_exists("test.lock_0"))
      break;
  }
  
  {
    block_fs_type * bfs = block_fs_mount( "test.mnt" , 1000 , 10000 , 0.67 , 10 , true , false , true );
    test_assert_true( block_fs_is_readonly( bfs ) );
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




int main(int argc , char ** argv) {
  test_readonly();
  test_lock_conflict();
  exit(0);
}

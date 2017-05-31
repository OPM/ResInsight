/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'ert_util_spawn.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <sys/wait.h>
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/rng.h>
#include <ert/util/test_work_area.h>
#include <ert/util/thread_pool.h>

void make_script(const char * name , const char * stdout_msg, const char * stderr_msg) {
  FILE * stream = util_fopen(name , "w");
  fprintf(stream,"#!/usr/bin/env python\n");
  fprintf(stream,"import sys\n");
  fprintf(stream,"sys.stdout.write('%s')\n" , stdout_msg);
  fprintf(stream,"sys.stdout.flush()\n");
  fprintf(stream,"sys.stderr.write('%s')\n" , stderr_msg);
  fprintf(stream,"sys.stderr.flush()\n");
  fclose( stream );

  util_addmode_if_owner( name , S_IRUSR + S_IWUSR + S_IXUSR + S_IRGRP + S_IWGRP + S_IXGRP + S_IROTH + S_IXOTH);  /* u:rwx  g:rwx  o:rx */

  usleep(100000); // Seems to be required to ensure that the script is actually found on disk. NFS?
}


void test_spawn_no_redirect() {
  test_work_area_type * test_area = test_work_area_alloc("spawn1");
  {
    int status;
    make_script("script" , "stdout" , "stderr");

    /* Blocking */
    status = util_spawn_blocking("script" , 0 , NULL , NULL , NULL);
    test_assert_int_equal( status , 0 );

    /* Not blocking */
    {
      pid_t pid = util_spawn( "script" , 0 , NULL , NULL , NULL);
      waitpid(pid , &status , 0 );
      test_assert_int_equal( status , 0 );
    }
  }
  test_work_area_free( test_area );
}


void * test_spawn_redirect__( void * path ) {
  const char * stdout_msg = "stdout_xxx";
  const char * stderr_msg = "stderr_123";

  char * stdout_file;
  char * stderr_file;
  char * script;

  if (path) {
    stdout_file = util_alloc_filename( path , "stdout.txt" , NULL );
    stderr_file = util_alloc_filename( path , "stderr.txt" , NULL );
    script = util_alloc_filename( path , "script" , NULL);
  } else {
    stdout_file = util_alloc_string_copy("stdout.txt");
    stderr_file = util_alloc_string_copy("stderr.txt");
    script = util_alloc_string_copy("script");
  }

  make_script(script , stdout_msg , stderr_msg);

  /* Blocking */
  while (true) {
    int status;
    status = util_spawn_blocking(script , 0 , NULL , stdout_file , stderr_file);
    if (status == 0)
      break;

    if (WIFEXITED( status )) {
      int script_status = WEXITSTATUS( status );
      if (script_status == 0)
        break;
      else if (script_status == 127)
        test_error_exit("Spawn failed, cannot find executable: %s\n", script);
      else
        test_assert_int_equal( status , 0 );
    } else
      test_assert_int_equal( status , 0 );
  }
  test_assert_file_content( stdout_file , stdout_msg );
  test_assert_file_content( stderr_file , stderr_msg );


  /* Not blocking - now we know the script is on disk - skip the while() { ... } */
  {
    pid_t pid = util_spawn( script , 0 , NULL , stdout_file , stderr_file );
    int status;
    waitpid(pid , &status , 0 );
    test_assert_int_equal( status , 0 );
  }
  test_assert_file_content( stdout_file , stdout_msg );
  test_assert_file_content( stderr_file , stderr_msg );

  free( stdout_file );
  free( stderr_file );
  free( script );
  return NULL;
}


void test_spawn_redirect() {
  test_work_area_type * test_area = test_work_area_alloc("spawn1");
  {
    test_spawn_redirect__( NULL );
  }
  test_work_area_free( test_area );
}

 void test_spawn_redirect_threaded() {
   rng_type * rng = rng_alloc( MZRAN , INIT_DEFAULT );
   const int num = 128;
   test_work_area_type * test_area = test_work_area_alloc("spawn1_threaded");
   thread_pool_type * tp = thread_pool_alloc( 8 , true );
   for (int i=0; i < num; i++) {
     char * path = util_alloc_sprintf("%06d" , rng_get_int( rng , 1000000));
     util_make_path( path );
     thread_pool_add_job( tp , test_spawn_redirect__ , path );
   }
   thread_pool_join( tp );
   thread_pool_free( tp );
   test_work_area_free( test_area );
   rng_free( rng );
 }



int main(int argc , char ** argv) {
  util_install_signals();
  test_spawn_no_redirect( );
  test_spawn_redirect( );
  test_spawn_redirect_threaded( );
  exit(0);
}

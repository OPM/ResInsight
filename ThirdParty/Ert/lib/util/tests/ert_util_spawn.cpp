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

#include <ert/util/stringlist.hpp>
#include <ert/util/test_util.hpp>
#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>


static const char * stdout_msg = "stdout_xxx";
static const char * stderr_msg = "stderr_123";



void make_script(const char * name , const char * stdout_msg, const char * stderr_msg) {
  FILE * stream = util_fopen(name , "w");
  fprintf(stream,"#!/usr/bin/env python\n");
  fprintf(stream,"import sys\n");
  fprintf(stream,"sys.stdout.write('%s')\n" , stdout_msg);
  fprintf(stream,"sys.stdout.flush()\n");
  fprintf(stream,"sys.stderr.write('%s')\n" , stderr_msg);
  fprintf(stream,"sys.stderr.flush()\n");
  fclose( stream );

  // Check that the file is actually there (may take a while before it appears on some file systems)
  for(int attempt = 0; attempt < 10; ++attempt) {
    if(util_file_exists(name))
      break;
    usleep(1 * 1e6);
  }
}

/// Check that the given script exists and is executable
bool check_script(const char* script) {
   if( ! util_file_exists(script) )
      return false;
   mode_t mode = util_getmode( script );
   return (mode & S_IXUSR) && (mode & S_IXGRP) && (mode & S_IXOTH);
}

void test_spawn_no_redirect() {
  test_work_area_type * test_area = test_work_area_alloc("spawn1");
  {
    int status;
    make_script("script" , stdout_msg , stderr_msg);
    util_addmode_if_owner( "script" , S_IRUSR + S_IWUSR + S_IXUSR + S_IRGRP + S_IWGRP + S_IXGRP + S_IROTH + S_IXOTH);  /* u:rwx  g:rwx  o:rx */
    test_assert_true(check_script("script"));

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


void * test_spawn_redirect__( const char * path ) {
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
    make_script("script" , stdout_msg , stderr_msg);
    util_addmode_if_owner( "script" , S_IRUSR + S_IWUSR + S_IXUSR + S_IRGRP + S_IWGRP + S_IXGRP + S_IROTH + S_IXOTH);  /* u:rwx  g:rwx  o:rx */
    test_assert_true(check_script("script"));

    test_spawn_redirect__( NULL );
  }
  test_work_area_free( test_area );
}

void test_spawn_redirect_threaded() {
   const int num = 128;

   // Generate the scripts on disk first
   test_work_area_type * test_area = test_work_area_alloc("spawn1_threaded");
   int * path_codes = (int *)util_calloc(num, sizeof *path_codes);
   stringlist_type * script_fullpaths = stringlist_alloc_new();
   for (int i=0; i < num; i++) {
      path_codes[i] = rand() % 1000000;

      char * path = util_alloc_sprintf("%06d" , path_codes[i]);
      util_make_path( path );
      char * script = util_alloc_filename( path , "script" , NULL);
      make_script(script, stdout_msg, stderr_msg);
      stringlist_append_copy(script_fullpaths, script);
      free(script);
      free(path);
   }

   // Set file access permissions
   for(int i = 0; i < num; i++) {
      char const * script = stringlist_iget(script_fullpaths, i);
      util_addmode_if_owner( script , S_IRUSR + S_IWUSR + S_IXUSR + S_IRGRP + S_IWGRP + S_IXGRP + S_IROTH + S_IXOTH);  /* u:rwx  g:rwx  o:rx */
   }

   // Double check that the scripts are present and executable
   for(int i = 0; i < num; i++) {
      char const * script = stringlist_iget(script_fullpaths, i);
      test_assert_true(check_script(script));
   }

   stringlist_free(script_fullpaths);
   free(path_codes);
   test_work_area_free( test_area );
}



int main(int argc , char ** argv) {
  util_install_signals();
  test_spawn_no_redirect( );
  test_spawn_redirect( );
  test_spawn_redirect_threaded( );
  exit(0);
}

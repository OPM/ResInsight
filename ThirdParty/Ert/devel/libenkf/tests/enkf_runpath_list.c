/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'enkf_runpath_list.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdio.h>
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>
#include <ert/util/test_work_area.h>

#include <ert/enkf/runpath_list.h>
#include <ert/enkf/ert_test_context.h>

void * add_pathlist( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  runpath_list_type * list = arg_pack_iget_ptr( arg_pack , 0 );
  int offset = arg_pack_iget_int( arg_pack , 1 );
  int bs = arg_pack_iget_int( arg_pack , 2 );

  int i;
  for (i=0; i < bs; i++)
    runpath_list_add( list , i + offset , 0,  "Path" , "Basename");

  return NULL;
}

void test_runpath_list() {
  runpath_list_type * list = runpath_list_alloc("DefaultFile");

  test_assert_int_equal( runpath_list_size( list ) , 0 );

  runpath_list_add( list , 3 , 0, "path" , "base");
  runpath_list_add( list , 2 , 0, "path" , "base");
  runpath_list_add( list , 1 , 0, "path" , "base");

  runpath_list_add( list , 3 , 1, "path" , "base");
  runpath_list_add( list , 2 , 1, "path" , "base");
  runpath_list_add( list , 1 , 1, "path" , "base");

  test_assert_int_equal( runpath_list_size( list ) , 6 );
  test_assert_int_equal( runpath_list_iget_iens( list , 0 ) , 3 );
  test_assert_int_equal( runpath_list_iget_iens( list , 2 ) , 1 );
  test_assert_int_equal( runpath_list_iget_iter( list , 3 ) , 1 );

  runpath_list_clear( list );
  test_assert_int_equal( runpath_list_size( list ) , 0 );

  test_assert_string_equal( runpath_list_get_line_fmt( list ) , RUNPATH_LIST_DEFAULT_LINE_FMT );
  {
    const char * other_line = "%d %s %s";
    runpath_list_set_line_fmt( list , other_line );
    test_assert_string_equal( runpath_list_get_line_fmt( list ) , other_line );
  }
  runpath_list_set_line_fmt( list , NULL );
  test_assert_string_equal( runpath_list_get_line_fmt( list ) , RUNPATH_LIST_DEFAULT_LINE_FMT );

  {
    const int block_size = 100;
    const int threads = 100;
    thread_pool_type * tp = thread_pool_alloc( threads , true );
    int it;

    for (it = 0; it < threads; it++) {
      int iens_offset = it * block_size;
      arg_pack_type * arg_pack = arg_pack_alloc();

      arg_pack_append_ptr( arg_pack , list );
      arg_pack_append_int( arg_pack , iens_offset );
      arg_pack_append_int( arg_pack , block_size );

      thread_pool_add_job( tp , add_pathlist , arg_pack );
    }
    thread_pool_join( tp );
    test_assert_int_equal( runpath_list_size( list ) , block_size * threads );

    {
      test_work_area_type * work_area = test_work_area_alloc("enkf_runpath_list" );
      runpath_list_fprintf( list );
      {
        int file_iens;
        int file_iter;
        char file_path[256];
        char file_base[256];
        int iens;
        FILE * stream = util_fopen( runpath_list_get_export_file(list) , "r");
        for (iens = 0; iens < threads * block_size; iens++) {
          int fscanf_return = fscanf( stream , "%d %s %s %d" , &file_iens , file_path , file_base, &file_iter);
          test_assert_int_equal(fscanf_return, 4 );
          test_assert_int_equal( file_iens , iens );
          test_assert_int_equal( file_iter , 0 );
        }
        fclose( stream );
      }
      test_work_area_free( work_area );
    }
  }
  runpath_list_free( list );
}



void test_config( const char * config_file ) {
  ert_test_context_type * test_context = ert_test_context_alloc( "RUNPATH_FILE" , config_file );
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  hook_manager_type * hook_manager = enkf_main_get_hook_manager( enkf_main );

  ert_test_context_run_worklow( test_context , "ARGECHO_WF");
  {
    FILE * stream = util_fopen("runpath_list.txt" , "r");
    char runpath_file[256];
    fscanf(stream , "%s" , runpath_file );
    fclose( stream );
    test_assert_string_equal( runpath_file , hook_manager_get_runpath_list_file( hook_manager ));
  }

  ert_test_context_free( test_context );
}


void test_filename() {
  runpath_list_type * list = runpath_list_alloc("DefaultFile");
  test_assert_string_equal( "DefaultFile" , runpath_list_get_export_file(list));
  runpath_list_set_export_file( list , "/tmp/file.txt");
  test_assert_string_equal( "/tmp/file.txt" , runpath_list_get_export_file(list));
  runpath_list_free( list );
}

int main(int argc , char ** argv) {
  util_install_signals();
  {
    test_runpath_list();
    test_config( argv[1] );
    test_filename();
    exit(0);
  }
}


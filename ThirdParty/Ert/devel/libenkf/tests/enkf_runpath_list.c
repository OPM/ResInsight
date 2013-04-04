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

#include <ert/enkf/runpath_list.h>

void * add_pathlist( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  runpath_list_type * list = arg_pack_iget_ptr( arg_pack , 0 );
  int offset = arg_pack_iget_int( arg_pack , 1 );
  int bs = arg_pack_iget_int( arg_pack , 2 );
  
  int i;
  for (i=0; i < bs; i++) 
    runpath_list_add( list , i + offset , "Path" , "Basename");

  return NULL;
}




int main(int argc , char ** argv) {

  runpath_list_type * list = runpath_list_alloc();

  test_assert_int_equal( runpath_list_size( list ) , 0 );

  runpath_list_add( list , 3 , "path" , "base");
  runpath_list_add( list , 2 , "path" , "base");
  runpath_list_add( list , 1 , "path" , "base");
  
  test_assert_int_equal( runpath_list_size( list ) , 3 );
  test_assert_int_equal( runpath_list_iget_iens( list , 0 ) , 3 );
  test_assert_int_equal( runpath_list_iget_iens( list , 2 ) , 1 );
  runpath_list_sort( list );

  test_assert_int_equal( runpath_list_iget_iens( list , 0 ) , 1 );
  test_assert_int_equal( runpath_list_iget_iens( list , 2 ) , 3 );
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
    runpath_list_sort( list );
    {
      int iens;
      for (iens = 0; iens < block_size * threads; iens++)
        test_assert_int_equal( runpath_list_iget_iens( list , iens ) , iens );
    }
    
    {
      const char *filename = "/tmp/runpath_list.txt";
      {
        FILE * stream = util_fopen( filename, "w");
        runpath_list_fprintf( list , stream );
        fclose( stream );
      }

      {
        int file_iens;
        char file_path[256];
        char file_base[256];
        int iens;
        FILE * stream = util_fopen( filename, "r");
        for (iens = 0; iens < threads * block_size; iens++) {
          int fscanf_return = fscanf( stream , "%d %s %s" , &file_iens , file_path , file_base);
          test_assert_int_equal(fscanf_return, 3 );
          test_assert_int_equal( file_iens , iens );
        }
        fclose( stream );
      }
    }
  }
  runpath_list_free( list );
  exit(0);
}


/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_time_map.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/time_map.h>



void ecl_test( const char * ecl_case ) {
  ecl_sum_type * ecl_sum  = ecl_sum_fread_alloc_case( ecl_case , ":");
  time_t start_time = ecl_sum_get_start_time( ecl_sum );
  time_t end_time   = ecl_sum_get_end_time( ecl_sum );
  time_map_type * ecl_map = time_map_alloc( );
  
  test_assert_true( time_map_summary_update( ecl_map , ecl_sum ) );
  test_assert_true( time_map_summary_update( ecl_map , ecl_sum ) );
  
  test_assert_time_t_equal( time_map_get_start_time( ecl_map ) , start_time );
  test_assert_time_t_equal( time_map_get_end_time( ecl_map ) , end_time );
  test_assert_double_equal( time_map_get_end_days( ecl_map ) , ecl_sum_get_sim_length( ecl_sum ));

  time_map_clear( ecl_map );
  time_map_update( ecl_map , 1 , 256 );
  test_assert_false( time_map_summary_update( ecl_map , ecl_sum ));
  
  time_map_free( ecl_map );
  ecl_sum_free( ecl_sum );
}



void simple_test() {
time_map_type * time_map = time_map_alloc( );
  const char * mapfile = "/tmp/map";
  
  test_assert_true( time_map_update( time_map , 0 , 100 )   );
  test_assert_true( time_map_update( time_map , 1 , 200 )   );
  test_assert_true( time_map_update( time_map , 1 , 200 )   );
  test_assert_false( time_map_update( time_map , 1 , 250 )  );

  test_assert_true( time_map_equal( time_map , time_map ) );
  time_map_fwrite( time_map , mapfile);
  {
    time_map_type * time_map2 = time_map_alloc( );

    test_assert_false( time_map_equal( time_map , time_map2 ) );
    time_map_fread( time_map2 , mapfile );
    test_assert_true( time_map_equal( time_map , time_map2 )  );
    time_map_free( time_map2 );
  }
  {
    time_t mtime1 = util_file_mtime( mapfile );
    sleep(2);
    time_map_fwrite( time_map , mapfile);
    
    test_assert_time_t_equal( mtime1 , util_file_mtime( mapfile ) );
    time_map_update( time_map , 2 , 300 );
    time_map_fwrite( time_map , mapfile);
    test_assert_time_t_not_equal( mtime1 , util_file_mtime( mapfile ) );
  }
}


#define MAP_SIZE 10000

void * update_time_map( void * arg ) {
  time_map_type * time_map = time_map_safe_cast( arg );
  int i;
  for (i=0; i < MAP_SIZE; i++)
    time_map_update( time_map , i , i );
  return NULL;
}


void thread_test() {
  time_map_type * time_map = time_map_alloc( );
  
  {
    int pool_size = 1000;
    thread_pool_type * tp = thread_pool_alloc( pool_size/2 , true );
    
    thread_pool_add_job( tp , update_time_map , time_map );
  
    thread_pool_join(tp);
    thread_pool_free(tp);
  }
  {
    int i;
    for (i=0; i < MAP_SIZE; i++) 
      test_assert_true( time_map_iget( time_map , i ) == i );
  }
  time_map_free( time_map );
}


int main(int argc , char ** argv) {
  
  if (argc == 1) {
    simple_test();
    thread_test();
  } else
    ecl_test( argv[1] );

  
  
  exit(0);
}


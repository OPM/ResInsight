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

#include "ert/util/build_config.h"

#include <ert/util/test_work_area.h>
#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>
#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>
#include <ert/util/vector.h>
#include <ert/util/test_work_area.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/time_map.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/ert_log.h>
#include <ert/enkf/enkf_main.h>

void ecl_test( const char * ecl_case ) {
  ecl_sum_type * ecl_sum  = ecl_sum_fread_alloc_case( ecl_case , ":");
  time_t start_time = ecl_sum_get_start_time( ecl_sum );
  time_t end_time   = ecl_sum_get_end_time( ecl_sum );
  time_map_type * ecl_map = time_map_alloc(  );

  test_assert_true( time_map_summary_update( ecl_map , ecl_sum ) );
  test_assert_true( time_map_summary_update( ecl_map , ecl_sum ) );

  test_assert_time_t_equal( time_map_get_start_time( ecl_map ) , start_time );
  test_assert_time_t_equal( time_map_get_end_time( ecl_map ) , end_time );
  test_assert_double_equal( time_map_get_end_days( ecl_map ) , ecl_sum_get_sim_length( ecl_sum ));

  time_map_clear( ecl_map );
  time_map_update( ecl_map , 1 , 256 );
  time_map_set_strict( ecl_map , false );
  test_assert_false( time_map_summary_update( ecl_map , ecl_sum ));

  time_map_free( ecl_map );
  ecl_sum_free( ecl_sum );
}


static void map_update( void * arg ) {
  vector_type * arg_vector = vector_safe_cast( arg );
  time_map_type * tmap = vector_iget( arg_vector , 0 );
  ecl_sum_type * sum = vector_iget( arg_vector , 1 );

  time_map_summary_update( tmap , sum );
}



void test_inconsistent_summary( const char * case1, const char * case2) {
  ecl_sum_type * ecl_sum1  = ecl_sum_fread_alloc_case( case1 , ":");
  ecl_sum_type * ecl_sum2  = ecl_sum_fread_alloc_case( case2 , ":");

  time_map_type * ecl_map = time_map_alloc(  );

  test_assert_true( time_map_summary_update( ecl_map , ecl_sum1 ) );
  {
    vector_type * arg = vector_alloc_new();
    vector_append_ref( arg , ecl_map );
    vector_append_ref( arg , ecl_sum2 );
    test_assert_util_abort("time_map_summary_update_abort" , map_update , arg);
    vector_free( arg );
  }

  time_map_free( ecl_map );
  ecl_sum_free( ecl_sum1 );
  ecl_sum_free( ecl_sum2 );
}

static void alloc_index_map( void * arg) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  time_map_type * map = arg_pack_iget_ptr( arg_pack , 0 );
  ecl_sum_type * sum  = arg_pack_iget_ptr( arg_pack , 1 );

  time_map_alloc_index_map( map , sum );
}



void test_refcase( const char * refcase_name , const char * case1, const char * case2 , const char * case3 , const char * case4) {
  ecl_sum_type * refcase   = ecl_sum_fread_alloc_case( refcase_name , ":");
  ecl_sum_type * ecl_sum1  = ecl_sum_fread_alloc_case( case1 , ":");
  ecl_sum_type * ecl_sum2  = ecl_sum_fread_alloc_case( case2 , ":");
  ecl_sum_type * ecl_sum3  = ecl_sum_fread_alloc_case( case3 , ":");
  ecl_sum_type * ecl_sum4  = ecl_sum_fread_alloc_case( case4 , ":");

  {
    time_map_type * ecl_map = time_map_alloc(  );
    test_assert_false( time_map_has_refcase( ecl_map ));
    test_assert_true( time_map_attach_refcase( ecl_map , refcase ) );
    test_assert_true( time_map_has_refcase( ecl_map ));
    time_map_free( ecl_map );
  }


  {
    time_map_type * ecl_map = time_map_alloc(  );
    time_map_attach_refcase( ecl_map , refcase );
    test_assert_true( time_map_summary_update( ecl_map , ecl_sum1 ) );
  }

  {
    time_map_type * ecl_map = time_map_alloc(  );

    time_map_set_strict( ecl_map , false );
    time_map_attach_refcase( ecl_map , refcase );

    test_assert_false( time_map_summary_update( ecl_map , ecl_sum2 ) );
    test_assert_int_equal( 25 , time_map_get_size( ecl_map ));
    test_assert_true( time_map_summary_update( ecl_map , ecl_sum1 ) );
    test_assert_int_equal( 63 , time_map_get_size( ecl_map ));
  }

  {
    time_map_type * ecl_map = time_map_alloc(  );
    test_assert_true( time_map_summary_update( ecl_map , ecl_sum2 ) );
    test_assert_false( time_map_attach_refcase( ecl_map , refcase ));
  }


  {
    test_work_area_type * work_area = test_work_area_alloc( "time_map/attach_short_refcase");
    {
      time_map_type * ecl_map = time_map_alloc(  );
      test_assert_true( time_map_summary_update( ecl_map , refcase ) );
      test_assert_true( time_map_update( ecl_map ,  ecl_sum_get_last_report_step( refcase ) + 1 , ecl_sum_get_end_time( refcase ) + 100 ));
      test_assert_true( time_map_update( ecl_map ,  ecl_sum_get_last_report_step( refcase ) + 2 , ecl_sum_get_end_time( refcase ) + 200 ));
      test_assert_true( time_map_update( ecl_map ,  ecl_sum_get_last_report_step( refcase ) + 3 , ecl_sum_get_end_time( refcase ) + 300 ));
      time_map_fwrite( ecl_map , "time_map");
      time_map_free( ecl_map );
    }
    {
      time_map_type * ecl_map = time_map_alloc(  );
      time_map_fread(ecl_map , "time_map");
      test_assert_true( time_map_attach_refcase( ecl_map , refcase ) );
      time_map_free( ecl_map );
    }
    test_work_area_free( work_area );
  }



  ecl_sum_free( refcase );
  ecl_sum_free( ecl_sum1 );
  ecl_sum_free( ecl_sum2 );
  ecl_sum_free( ecl_sum3 );
  ecl_sum_free( ecl_sum4 );
}


void test_index_map( const char * case1, const char * case2 , const char * case3 , const char * case4) {
  ecl_sum_type * ecl_sum1  = ecl_sum_fread_alloc_case( case1 , ":");
  ecl_sum_type * ecl_sum2  = ecl_sum_fread_alloc_case( case2 , ":");
  ecl_sum_type * ecl_sum3  = ecl_sum_fread_alloc_case( case3 , ":");
  ecl_sum_type * ecl_sum4  = ecl_sum_fread_alloc_case( case4 , ":");

  time_map_type * ecl_map = time_map_alloc(  );

  {
    int_vector_type * index_map = time_map_alloc_index_map( ecl_map , ecl_sum1 );
    test_assert_int_equal( int_vector_size( index_map ) , 0);
    int_vector_free( index_map );
  }

  test_assert_true( time_map_summary_update( ecl_map , ecl_sum1 ) );
  {
    int_vector_type * index_map = time_map_alloc_index_map( ecl_map , ecl_sum1 );
    int i;
    for (i=0; i < int_vector_size( index_map ); i++)
      test_assert_int_equal( i , int_vector_iget( index_map , i ));

    test_assert_int_equal( int_vector_size( index_map ) , ecl_sum_get_last_report_step( ecl_sum1) + 1);
    int_vector_free( index_map );
  }

  /* case2 has an extra tstep in the middle of the case. */
  time_map_set_strict( ecl_map , false );
  test_assert_false( time_map_summary_update( ecl_map , ecl_sum2 ) );
  {
    int_vector_type * index_map = time_map_alloc_index_map( ecl_map , ecl_sum2 );
    test_assert_int_equal( int_vector_size( index_map ) , ecl_sum_get_last_report_step( ecl_sum2));
    test_assert_int_equal( int_vector_iget( index_map , 24) , 24);
    test_assert_int_equal( int_vector_iget( index_map , 25) , 26);
    int_vector_free( index_map );
  }


  /* case3 has an extra tstep in the middle, and ends prematurely */
  test_assert_false( time_map_summary_update( ecl_map , ecl_sum3 ) );
  {
    int_vector_type * index_map = time_map_alloc_index_map( ecl_map , ecl_sum3 );
    test_assert_int_equal( int_vector_size( index_map ) , ecl_sum_get_last_report_step( ecl_sum3));
    int_vector_free( index_map );
  }


  /* case4 has a missing tstep in the middle - that is not handled; and we abort */
  test_assert_false( time_map_summary_update( ecl_map , ecl_sum4 ) );
  {
    arg_pack_type * arg = arg_pack_alloc();
    arg_pack_append_ptr( arg , ecl_map );
    arg_pack_append_ptr( arg , ecl_sum4 );

    test_assert_util_abort( "time_map_alloc_index_map" , alloc_index_map , arg);
    arg_pack_free( arg );
  }



  time_map_free( ecl_map );
  ecl_sum_free( ecl_sum1 );
  ecl_sum_free( ecl_sum2 );
  ecl_sum_free( ecl_sum3 );
  ecl_sum_free( ecl_sum4 );
}


void simple_test() {
  time_map_type * time_map = time_map_alloc(  );
  test_work_area_type * work_area = test_work_area_alloc("enkf_time_map" );
  const char * mapfile = "map";

  time_map_set_strict( time_map , false );
  test_assert_true( time_map_update( time_map , 0 , 100 )   );
  test_assert_true( time_map_update( time_map , 1 , 200 )   );
  test_assert_true( time_map_update( time_map , 1 , 200 )   );
  test_assert_false( time_map_update( time_map , 1 , 250 )  );

  test_assert_true( time_map_equal( time_map , time_map ) );
  time_map_fwrite( time_map , mapfile);
  {
    time_map_type * time_map2 = time_map_alloc(  );

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
  test_work_area_free( work_area );
}


static void simple_update(void * arg) {
  time_map_type * tmap = time_map_safe_cast( arg );

  time_map_update( tmap , 0 , 101 );
}



void simple_test_inconsistent() {
  time_map_type * time_map = time_map_alloc(  );

  test_assert_true( time_map_update( time_map , 0 , 100 )   );
  time_map_set_strict( time_map , false );
  test_assert_false( time_map_update( time_map , 0 , 101 )   );

  time_map_set_strict( time_map , true );
  test_assert_util_abort( "time_map_update_abort" , simple_update , time_map );

  time_map_free( time_map );
}



#define MAP_SIZE 10000

void * update_time_map( void * arg ) {
  time_map_type * time_map = time_map_safe_cast( arg );
  int i;
  for (i=0; i < MAP_SIZE; i++)
    time_map_update( time_map , i , i );

  test_assert_int_equal( MAP_SIZE , time_map_get_size( time_map ));
  return NULL;
}


void thread_test() {
  time_map_type * time_map = time_map_alloc(  );
  test_assert_false( time_map_is_readonly( time_map ));
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



void test_read_only() {
  test_work_area_type * work_area = test_work_area_alloc("time-map");
  {
    time_map_type * tm = time_map_alloc(  );

    test_assert_true( time_map_is_instance( tm ));
    test_assert_true( time_map_is_strict( tm ));
    test_assert_false( time_map_is_readonly( tm ));

    time_map_update( tm , 0 , 0 );
    time_map_update( tm , 1 , 10 );
    time_map_update( tm , 2 , 20 );

    time_map_fwrite( tm , "case/files/time-map" );
    time_map_free( tm );
  }
  {
    time_map_type * tm = time_map_fread_alloc_readonly( "case/files/time-map");
    test_assert_time_t_equal(  0 , time_map_iget( tm , 0 ));
    test_assert_time_t_equal( 10 , time_map_iget( tm , 1 ));
    test_assert_time_t_equal( 20 , time_map_iget( tm , 2 ));
    test_assert_int_equal( 3 , time_map_get_size( tm ));
    time_map_free( tm );
  }
  {
    time_map_type * tm = enkf_fs_alloc_readonly_time_map( "case" );
    test_assert_time_t_equal(  0 , time_map_iget( tm , 0 ));
    test_assert_time_t_equal( 10 , time_map_iget( tm , 1 ));
    test_assert_time_t_equal( 20 , time_map_iget( tm , 2 ));
    test_assert_int_equal( 3 , time_map_get_size( tm ));
    time_map_free( tm );
  }
  {
    time_map_type * tm = time_map_fread_alloc_readonly( "DoesNotExist");
    test_assert_true( time_map_is_instance( tm ));
    test_assert_true( time_map_is_readonly( tm ));
    test_assert_int_equal(0 , time_map_get_size( tm ));
    time_map_free( tm );
  }
  test_work_area_free( work_area );
}


int main(int argc , char ** argv) {

  enkf_main_install_SIGNALS();
  ert_log_init_log(0 , NULL , false ); // Make sure there will be no logging.

  if (argc == 1) {
    simple_test();
    simple_test_inconsistent();
    thread_test();
  } else {
    ecl_test( argv[1] );
    test_inconsistent_summary( argv[1] , argv[2]);
    test_index_map(argv[1] , argv[2] , argv[3] , argv[4]);
    test_refcase( argv[1] , argv[1] , argv[2] , argv[3] , argv[4]);
  }

  test_read_only();

  exit(0);
}


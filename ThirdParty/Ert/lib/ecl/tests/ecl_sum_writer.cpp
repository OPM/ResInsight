/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'ecl_sum_writer.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>
#include <ert/util/time_t_vector.hpp>
#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>

#include <ert/ecl/ecl_sum.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>


double write_summary( const char * name , time_t start_time , int nx , int ny , int nz , int num_dates, int num_ministep, double ministep_length) {
  ecl_sum_type * ecl_sum = ecl_sum_alloc_writer( name , false , true , ":" , start_time , true , nx , ny , nz );
  double sim_seconds = 0;

  smspec_node_type * node1 = ecl_sum_add_var( ecl_sum , "FOPT" , NULL   , 0   , "Barrels" , 99.0 );
  smspec_node_type * node2 = ecl_sum_add_var( ecl_sum , "BPR"  , NULL   , 567 , "BARS"    , 0.0  );
  smspec_node_type * node3 = ecl_sum_add_var( ecl_sum , "WWCT" , "OP-1" , 0   , "(1)"     , 0.0  );

  for (int report_step = 0; report_step < num_dates; report_step++) {
    for (int step = 0; step < num_ministep; step++) {
      /* Simulate .... */

      {
        ecl_sum_tstep_type * tstep = ecl_sum_add_tstep( ecl_sum , report_step + 1 , sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , node1 , sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , node2 , 10*sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , node3 , 100*sim_seconds );

        test_assert_double_equal( ecl_sum_tstep_get_from_node( tstep , node1 ), sim_seconds );
        test_assert_double_equal( ecl_sum_tstep_get_from_node( tstep , node2 ), sim_seconds*10 );
        test_assert_double_equal( ecl_sum_tstep_get_from_node( tstep , node3 ), sim_seconds*100 );
      }
      sim_seconds += ministep_length;
    }
  }
  ecl_sum_fwrite( ecl_sum );
  ecl_sum_free( ecl_sum );
  return sim_seconds;
}

int write_restart_summary(const char * name, const char * restart_name , int start_report_step, double sim_seconds, time_t start_time , int nx , int ny , int nz , int num_dates, int num_ministep, double ministep_length) {
  ecl_sum_type * ecl_sum = ecl_sum_alloc_restart_writer( name , restart_name, false , true , ":" , start_time , true , nx , ny , nz );


  smspec_node_type * node1 = ecl_sum_add_var( ecl_sum , "FOPT" , NULL   , 0   , "Barrels" , 99.0 );
  smspec_node_type * node2 = ecl_sum_add_var( ecl_sum , "BPR"  , NULL   , 567 , "BARS"    , 0.0  );
  smspec_node_type * node3 = ecl_sum_add_var( ecl_sum , "WWCT" , "OP-1" , 0   , "(1)"     , 0.0  );

  int num_report_steps = start_report_step + num_dates;
  for (int report_step = start_report_step; report_step < num_report_steps; report_step++) {
    for (int step = 0; step < num_ministep; step++) {
      {
        ecl_sum_tstep_type * tstep = ecl_sum_add_tstep( ecl_sum , report_step + 1 , sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , node1 , sim_seconds);
        ecl_sum_tstep_set_from_node( tstep , node2 , 10*sim_seconds );
        ecl_sum_tstep_set_from_node( tstep , node3 , 100*sim_seconds );

      }
      sim_seconds += ministep_length;
    }
  }
  ecl_sum_fwrite( ecl_sum );
  ecl_sum_free( ecl_sum );
  return sim_seconds;
}


void test_write_read( ) {
  const char * name = "CASE";
  time_t start_time = util_make_date_utc( 1,1,2010 );
  time_t end_time = start_time;
  int nx = 10;
  int ny = 11;
  int nz = 12;
  int num_dates = 5;
  int num_ministep = 10;
  double ministep_length = 86400; // Seconds - numerical value chosen to avoid rounding problems when converting between seconds and days.
  {
    test_work_area_type * work_area = test_work_area_alloc("sum/write");
    ecl_sum_type * ecl_sum;

    write_summary( name , start_time , nx , ny , nz , num_dates , num_ministep , ministep_length);
    ecl_sum = ecl_sum_fread_alloc_case( name , ":" );
    test_assert_true( ecl_sum_is_instance( ecl_sum ));

    /* Time direction */
    test_assert_time_t_equal( start_time , ecl_sum_get_start_time(ecl_sum));
    test_assert_time_t_equal( start_time , ecl_sum_get_data_start(ecl_sum));
    util_inplace_forward_seconds_utc(&end_time, (num_dates * num_ministep - 1) * ministep_length );
    test_assert_time_t_equal( end_time , ecl_sum_get_end_time(ecl_sum));

    /* Keys */
    test_assert_true( ecl_sum_has_key( ecl_sum , "FOPT" ));
    test_assert_true( ecl_sum_has_key( ecl_sum , "WWCT:OP-1" ));
    test_assert_true( ecl_sum_has_key( ecl_sum , "BPR:567" ));
    {
      ecl_grid_type *grid = ecl_grid_alloc_rectangular(nx,ny,nz,1,1,1,NULL);
      int i,j,k;
      char * ijk_key;
      ecl_grid_get_ijk1( grid , 567 - 1 , &i,&j,&k);
      ijk_key = util_alloc_sprintf( "BPR:%d,%d,%d" , i+1 ,j+1 ,k+1);

      free( ijk_key );
      ecl_grid_free( grid );
    }

    ecl_sum_free( ecl_sum );
    test_work_area_free( work_area );
  }
}


void test_ecl_sum_alloc_restart_writer() {

   test_work_area_type * work_area = test_work_area_alloc("sum_write_restart");
   {
      const char * name1 = "CASE1";
      const char * name2 = "CASE2";
      time_t start_time = util_make_date_utc( 1,1,2010 );
      int nx = 10;
      int ny = 11;
      int nz = 12;
      int num_dates = 5;
      int num_ministep = 10;
      double ministep_length = 36000; // Seconds

      int sim_seconds = write_summary( name1 , start_time , nx , ny , nz , num_dates , num_ministep , ministep_length);
      sim_seconds = write_restart_summary( name2 , name1 , num_dates, sim_seconds, start_time , nx , ny , nz , num_dates , num_ministep , ministep_length);

      ecl_sum_type * case1 = ecl_sum_fread_alloc_case( name1 , ":" );
      ecl_sum_type * case2 = ecl_sum_fread_alloc_case( name2 , ":" );
      test_assert_true( ecl_sum_is_instance(case2) );

      test_assert_true( ecl_sum_has_key( case2 , "FOPT" ));

      ecl_file_type * restart_file = ecl_file_open( "CASE2.SMSPEC" , 0 );
      ecl_file_view_type * view_file = ecl_file_get_global_view( restart_file );
      test_assert_true( ecl_file_view_has_kw(view_file, RESTART_KW));
      ecl_kw_type * restart_kw = ecl_file_view_iget_named_kw(view_file, "RESTART", 0);
      test_assert_int_equal(8, ecl_kw_get_size(restart_kw));
      test_assert_string_equal( "CASE1   ", (const char *) ecl_kw_iget_ptr( restart_kw , 0 ) );
      test_assert_string_equal( "        ", (const char *) ecl_kw_iget_ptr( restart_kw , 1 ) );

      for (int time_index=0; time_index < ecl_sum_get_data_length( case1 ); time_index++)
         test_assert_double_equal(  ecl_sum_get_general_var( case1 , time_index , "FOPT"), ecl_sum_get_general_var( case2 , time_index , "FOPT"));

      ecl_sum_free(case2);
      ecl_sum_free(case1);
      ecl_file_close(restart_file);

   }
   test_work_area_free( work_area );
}


void test_long_restart_names() {
   char restart_case[65] = { 0 };
   for (int n = 0; n < 8; n++) {
      char s[9];
      sprintf(s, "WWWWGGG%d", n);
      strcat(restart_case, s);
   }
   const char * name = "THE_CASE";
   test_work_area_type * work_area = test_work_area_alloc("sum_write_restart_long_name");
   {
       int restart_step = 77;
       time_t start_time = util_make_date_utc( 1,1,2010 );
       ecl_sum_type * ecl_sum = ecl_sum_alloc_restart_writer2( name , restart_case , restart_step, false , true , ":" , start_time , true , 3, 3, 3);
       ecl_sum_fwrite( ecl_sum );
       ecl_sum_free(ecl_sum);

       ecl_file_type * smspec_file = ecl_file_open( "THE_CASE.SMSPEC" , 0 );
       ecl_file_view_type * view_file = ecl_file_get_global_view( smspec_file );
       test_assert_true( ecl_file_view_has_kw(view_file, RESTART_KW));
       ecl_kw_type * restart_kw = ecl_file_view_iget_named_kw(view_file, "RESTART", 0);
       test_assert_int_equal(8, ecl_kw_get_size(restart_kw));

       for (int n = 0; n < 8; n++) {
         char s[9]; sprintf(s, "WWWWGGG%d", n);
         test_assert_string_equal(s, ecl_kw_iget_char_ptr(restart_kw, n) );
       }
       ecl_file_close( smspec_file);
       {
         ecl_smspec_type * smspec = ecl_smspec_alloc_restart_writer( ":" , "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 10, start_time, true, 3, 3 ,3);
         /*
           Restart case is too long - it is ignored.
         */
         test_assert_NULL( ecl_smspec_get_restart_case( smspec));
         ecl_smspec_free( smspec);
       }
   }

   test_work_area_free( work_area );

}

int main( int argc , char ** argv) {
  test_write_read();
  test_ecl_sum_alloc_restart_writer();
  test_long_restart_names();
  exit(0);
}

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

#include <ert/util/test_util.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/util.h>
#include <ert/util/test_work_area.h>

#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_grid.h>


void write_summary( const char * name , time_t start_time , int nx , int ny , int nz , int num_dates, int num_ministep, double ministep_length) {
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
  double ministep_length = 36000; // Seconds
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



int main( int argc , char ** argv) {
  test_write_read();
  exit(0);
}

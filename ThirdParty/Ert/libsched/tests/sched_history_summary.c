/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'sched_history_summary.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>

#include <ert/util/test_util.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/sched/history.h>



int main(int argc, char **argv) {
  char * sum_case = argv[1];
  ecl_sum_type * refcase = ecl_sum_fread_alloc_case( sum_case , ":" );
  history_type * hist_h = history_alloc_from_refcase( refcase , true );
  history_type * hist_sim = history_alloc_from_refcase( refcase , false );
  
  test_assert_true( history_is_instance( hist_h ) );
  test_assert_true( history_is_instance( hist_sim ) );
  test_assert_int_equal( history_get_last_restart( hist_sim ) , ecl_sum_get_last_report_step( refcase ) );
  test_assert_int_equal( history_get_last_restart( hist_h ) , ecl_sum_get_last_report_step( refcase ) );

  {
    double_vector_type * value_sim = double_vector_alloc(0 , 0);
    double_vector_type * value_h   = double_vector_alloc(0 , 0);
    bool_vector_type * valid_sim = bool_vector_alloc( 0 , false );
    bool_vector_type * valid_h = bool_vector_alloc( 0 , false );
    
    test_assert_true( history_init_ts( hist_sim , "FOPT" , value_sim , valid_sim ));
    test_assert_true( history_init_ts( hist_h , "FOPT" , value_h , valid_h ));
    {
      int step;
      for (step = 1; step < ecl_sum_get_last_report_step( refcase ); step++) {
        test_assert_true( bool_vector_iget( valid_sim , step ));
        test_assert_true( bool_vector_iget( valid_h , step ));
        {
          int time_index = ecl_sum_iget_report_end( refcase , step );
          test_assert_double_equal( ecl_sum_get_general_var( refcase , time_index , "FOPT" )  , double_vector_iget( value_sim , step ));
          test_assert_double_equal( ecl_sum_get_general_var( refcase , time_index , "FOPTH" ) , double_vector_iget( value_h , step ));
        }
      }
    }
    bool_vector_free( valid_sim );
    bool_vector_free( valid_h );
    
    double_vector_free( value_sim );
    double_vector_free( value_h );
  }


  history_free( hist_h );
  history_free( hist_sim );
  ecl_sum_free( refcase );
  exit(0);
}

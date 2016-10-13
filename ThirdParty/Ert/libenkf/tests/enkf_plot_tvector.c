/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_plot_tvector.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_work_area.h>
#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/bool_vector.h>
#include <ert/util/arg_pack.h>

#include <ert/enkf/enkf_plot_tvector.h>
#include <ert/enkf/summary_config.h>



void create_test() {
  enkf_config_node_type * config_node = enkf_config_node_alloc_summary("KEY" , LOAD_FAIL_SILENT);
  enkf_plot_tvector_type * tvector = enkf_plot_tvector_alloc( config_node , 0 );
  test_assert_true( enkf_plot_tvector_is_instance( tvector ));
  enkf_plot_tvector_free( tvector );
}



void test_iset() {
  enkf_config_node_type * config_node = enkf_config_node_alloc_summary("KEY" , LOAD_FAIL_SILENT);
  enkf_plot_tvector_type * tvector = enkf_plot_tvector_alloc( config_node , 0 );
  enkf_plot_tvector_iset( tvector , 10 , 0 , 100 );

  test_assert_int_equal( 11 , enkf_plot_tvector_size( tvector  ));
  test_assert_time_t_equal( 0   , enkf_plot_tvector_iget_time( tvector , 10 ));
  test_assert_double_equal( 100 , enkf_plot_tvector_iget_value( tvector , 10 ));
  {
    for (int i=0; i < (enkf_plot_tvector_size( tvector  ) - 1); i++) 
      test_assert_false( enkf_plot_tvector_iget_active( tvector , i ));
    
    test_assert_true( enkf_plot_tvector_iget_active( tvector , 10 ));
  }
  
  enkf_plot_tvector_free( tvector );
}


void test_all_active() {
  enkf_config_node_type * config_node = enkf_config_node_alloc_summary("KEY" , LOAD_FAIL_SILENT);
  enkf_plot_tvector_type * tvector = enkf_plot_tvector_alloc( config_node , 0);
  test_assert_true( enkf_plot_tvector_all_active( tvector ));

  enkf_plot_tvector_iset( tvector , 00 , 0 , 100 );
  test_assert_true( enkf_plot_tvector_all_active( tvector ));
  
  enkf_plot_tvector_iset( tvector , 1 , 0 , 100 );
  test_assert_true( enkf_plot_tvector_all_active( tvector ));

  enkf_plot_tvector_iset( tvector , 10 , 0 , 100 );
  test_assert_false( enkf_plot_tvector_all_active( tvector ));
}



void test_iget() {
  enkf_config_node_type * config_node = enkf_config_node_alloc_summary("KEY" , LOAD_FAIL_SILENT);
  enkf_plot_tvector_type * tvector = enkf_plot_tvector_alloc( config_node , 0);
  enkf_plot_tvector_iset( tvector , 0 , 0 , 0 );
  enkf_plot_tvector_iset( tvector , 1 , 100 , 10 );
  enkf_plot_tvector_iset( tvector , 2 , 200 , 20 );
  enkf_plot_tvector_iset( tvector , 3 , 300 , 30 );
  enkf_plot_tvector_iset( tvector , 4 , 400 , 40 );

  enkf_plot_tvector_iset( tvector , 6 , 600 , 60 );


  test_assert_int_equal( 7 , enkf_plot_tvector_size( tvector ));
  for (int i=0; i < 7; i++) {
    if (i == 5)
      test_assert_false( enkf_plot_tvector_iget_active( tvector , i ));
    else {
      test_assert_true( enkf_plot_tvector_iget_active( tvector , i ));
      test_assert_time_t_equal( i * 100 , enkf_plot_tvector_iget_time( tvector , i ));
      test_assert_double_equal( i * 10  , enkf_plot_tvector_iget_value( tvector , i ));
    }
  }
}



int main(int argc , char ** argv) {
  create_test();
  test_iset();
  test_all_active();
  test_iget();
  
  exit(0);
}


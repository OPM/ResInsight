/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_plot_data.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/enkf/enkf_plot_data.h>



void test_create() {
  enkf_plot_data_type * plot_data = enkf_plot_data_alloc( NULL );
  test_assert_true( enkf_plot_data_is_instance( plot_data ));
  test_assert_int_equal( 0 , enkf_plot_data_get_size( plot_data ));
  enkf_plot_data_free( plot_data );
}



int main(int argc , char ** argv) {
  test_create();
}

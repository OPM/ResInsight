/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'enkf_plot_genvector.c' is part of ERT - Ensemble based Reservoir Tool.
    
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

#include <ert/util/test_util.h>

#include <ert/ecl/ecl_grid.h>

#include <ert/enkf/block_obs.h>
#include <ert/enkf/enkf_plot_genvector.h>
#include <ert/enkf/gen_data.h>
#include <ert/enkf/enkf_config_node.h>


void test_create() {
  enkf_config_node_type * config_node = enkf_config_node_alloc_GEN_DATA_result( "Key" , ASCII , "Result%d");
  enkf_plot_genvector_type * gen_vector = enkf_plot_genvector_alloc( config_node , 0 );
  test_assert_true( enkf_plot_genvector_is_instance( gen_vector ));
  test_assert_int_equal( 0 , enkf_plot_genvector_get_size( gen_vector ));
  enkf_config_node_free( config_node );
  enkf_plot_genvector_free(gen_vector);
}




int main( int argc , char ** argv) {
  test_create();
  exit(0);
}

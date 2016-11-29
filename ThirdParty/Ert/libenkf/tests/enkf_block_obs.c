/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'enkf_plot_blockdata.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/ecl/ecl_grid.h>

#include <ert/enkf/field_config.h>
#include <ert/enkf/block_obs.h>
#include <ert/enkf/container_config.h>


void test_create_invalid_data(ecl_grid_type * grid) {
  void * data_config = NULL;
  test_assert_NULL(block_obs_alloc( "ObsKey" , data_config , grid ));
}



void test_create_from_field(ecl_grid_type * grid) {
  field_config_type * field_config = field_config_alloc_empty( "PRESSURE" , grid , NULL, false );
  block_obs_type * block_obs = block_obs_alloc( "ObsKey" , field_config , grid );
  
  test_assert_true( block_obs_is_instance( block_obs ));
  test_assert_int_equal(0 , block_obs_get_size( block_obs ));
  block_obs_append_field_obs( block_obs , 10 , 12 , 8  , 100 , 25);
  test_assert_int_equal(1 , block_obs_get_size( block_obs ));
  block_obs_append_field_obs( block_obs , 10 , 12 , 9  , 100 , 25);
  test_assert_int_equal(2 , block_obs_get_size( block_obs ));
  block_obs_free( block_obs );
  field_config_free( field_config );
}


void test_create_from_summary(ecl_grid_type * grid) {
  container_config_type * container_config = container_config_alloc( "Container");
  block_obs_type * block_obs = block_obs_alloc( "ObsKey" , container_config , grid );
  
  test_assert_true( block_obs_is_instance( block_obs ));
  test_assert_int_equal(0 , block_obs_get_size( block_obs ));

  
  block_obs_append_summary_obs( block_obs , 10 , 12 , 8  , "BPR:111,13,9" , 100 , 25);
  test_assert_int_equal(1 , block_obs_get_size( block_obs ));
  block_obs_append_summary_obs( block_obs , 10 , 12 , 9  , "BPR:11,13,10" , 100 , 25);
  test_assert_int_equal(2 , block_obs_get_size( block_obs ));
  block_obs_free( block_obs );

  container_config_free( container_config );
}



int main (int argc , char ** argv) {
  ecl_grid_type * grid = ecl_grid_alloc( argv[1] );
  {
    test_create_invalid_data( grid );
    test_create_from_field(grid);
    test_create_from_summary( grid);
  }
  ecl_grid_free( grid );
  exit(0);
}

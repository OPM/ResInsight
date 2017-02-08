/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'well_state.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/stringlist.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>

#include <ert/ecl_well/well_state.h>


int main(int argc , char ** argv) {
  test_install_SIGNALS();

  test_assert_int_equal( well_state_translate_ecl_type_int( IWEL_UNDOCUMENTED_ZERO) , ECL_WELL_ZERO);
  test_assert_int_equal( well_state_translate_ecl_type_int( IWEL_PRODUCER) , ECL_WELL_PRODUCER);
  test_assert_int_equal( well_state_translate_ecl_type_int( IWEL_WATER_INJECTOR) , ECL_WELL_WATER_INJECTOR);
  test_assert_int_equal( well_state_translate_ecl_type_int( IWEL_GAS_INJECTOR)   , ECL_WELL_GAS_INJECTOR);
  test_assert_int_equal( well_state_translate_ecl_type_int( IWEL_OIL_INJECTOR)   , ECL_WELL_OIL_INJECTOR);

  {
    const char * well_name = "WELL";
    int report_nr = 100;
    int global_well_nr = 67;
    time_t valid_from = -1;
    bool open = false;
    well_type_enum type = ECL_WELL_GAS_INJECTOR;

    well_state_type * well_state = well_state_alloc(well_name , global_well_nr , open , type , report_nr , valid_from);
    test_assert_true( well_state_is_instance( well_state) );

    test_assert_false( well_state_is_MSW( well_state ));
    test_assert_string_equal( well_name , well_state_get_name( well_state ));
    test_assert_int_equal( global_well_nr , well_state_get_well_nr( well_state ));
    test_assert_bool_equal( open , well_state_is_open( well_state ));
    test_assert_int_equal( type , well_state_get_type( well_state ));
    test_assert_int_equal( report_nr , well_state_get_report_nr( well_state ));
    test_assert_time_t_equal( valid_from , well_state_get_sim_time( well_state ));

    test_assert_NULL( well_state_get_global_connections( well_state ));
    test_assert_false( well_state_has_global_connections( well_state ));
    test_assert_NULL( well_state_get_grid_connections( well_state , "GRID"));
    test_assert_false( well_state_has_grid_connections( well_state , "GRID"));

    test_assert_double_equal( 0.0, well_state_get_oil_rate( well_state ));
    test_assert_double_equal( 0.0, well_state_get_gas_rate( well_state ));
    test_assert_double_equal( 0.0, well_state_get_water_rate( well_state ));
    test_assert_double_equal( 0.0, well_state_get_volume_rate( well_state ));

    well_state_free( well_state );
  }
  exit(0);
}

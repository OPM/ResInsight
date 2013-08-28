/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'well_state_load_missing_RSEG.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw_magic.h>

#include <ert/ecl_well/well_state.h>


int main(int argc , char ** argv) {
  test_install_SIGNALS();
  {
    const char * grid_file = argv[1];
    const char * rst_file_name = argv[2];

    ecl_grid_type * grid = ecl_grid_alloc( grid_file );
    ecl_file_type * rst_file = ecl_file_open( rst_file_name , 0);
    ecl_rsthead_type * header = ecl_rsthead_alloc( rst_file );
    const char * well_name = "WELL";
    int report_nr = 100;
    time_t valid_from = -1;
    bool open = false;
    well_type_enum type = GAS_INJECTOR;
    int global_well_nr = 0;
    
    for (global_well_nr = 0; global_well_nr < header->nwells; global_well_nr++) {
      well_state_type * well_state = well_state_alloc(well_name , global_well_nr , open , type , report_nr , valid_from);
      test_assert_true( well_state_is_instance( well_state) );
      well_state_add_connections( well_state , grid , rst_file , 0 );
      well_state_add_MSW( well_state , rst_file , global_well_nr );

      {
        const well_segment_collection_type * segments = well_state_get_segments( well_state );

        if (!ecl_file_has_kw( rst_file , RSEG_KW ))
          test_assert_int_equal( 0 , well_segment_collection_get_size( segments ));

      }
      well_state_free( well_state );
    }
  }

  exit(0);
}

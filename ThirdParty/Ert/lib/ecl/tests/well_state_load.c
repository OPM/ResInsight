/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'well_state_load.c' is part of ERT - Ensemble based Reservoir Tool.

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
    ecl_rsthead_type * header = ecl_rsthead_alloc( ecl_file_get_global_view( rst_file ) , ecl_util_filename_report_nr(rst_file_name) );
    const char * well_name = "WELL";
    int report_nr = 100;
    time_t valid_from = -1;
    bool open = false;
    well_type_enum type = ECL_WELL_GAS_INJECTOR;
    int global_well_nr = 0;
    bool load_segment_information = true;
    ecl_file_view_type * rst_view = ecl_file_get_global_view( rst_file );

    for (global_well_nr = 0; global_well_nr < header->nwells; global_well_nr++) {
      well_state_type * well_state = well_state_alloc(well_name , global_well_nr , open , type , report_nr , valid_from);
      test_assert_true( well_state_is_instance( well_state) );
      well_state_add_connections2( well_state , grid , rst_view , 0 );

      test_assert_true( well_state_has_grid_connections( well_state , ECL_GRID_GLOBAL_GRID ));
      test_assert_false( well_state_has_grid_connections( well_state , "???" ));
      test_assert_true( well_state_has_global_connections( well_state ));

      well_state_add_MSW2( well_state , rst_view , global_well_nr , load_segment_information );
      {
        const well_segment_collection_type * segments = well_state_get_segments( well_state );
        const well_branch_collection_type * branches = well_state_get_branches( well_state );

        if (well_state_is_MSW( well_state )) {
          test_assert_true( ecl_file_has_kw( rst_file , ISEG_KW ));
          test_assert_int_not_equal( well_segment_collection_get_size( segments ) , 0);
          test_assert_int_not_equal( well_branch_collection_get_size( branches ) , 0);
        } else {
          test_assert_int_equal( well_segment_collection_get_size( segments ) , 0);
          test_assert_int_equal( well_branch_collection_get_size( branches ) , 0);
          test_assert_false( well_state_is_MSW( well_state ));
        }
      }
      well_state_free( well_state );
    }
  }

  exit(0);
}

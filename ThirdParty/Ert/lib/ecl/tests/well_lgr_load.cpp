/*
   Copyright (C) 2013  Equinor ASA, Norway.

   The file 'well_lgr_load.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <time.h>
#include <stdbool.h>
#include <signal.h>

#include <ert/util/util.h>
#include <ert/util/int_vector.hpp>
#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_util.hpp>

#include <ert/ecl_well/well_state.hpp>
#include <ert/ecl_well/well_info.hpp>
#include <ert/ecl_well/well_conn.hpp>
#include <ert/ecl_well/well_ts.hpp>



int main( int argc , char ** argv) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation,
                                             i.e. overwriting memory ... */
  signal(SIGTERM , util_abort_signal);    /* If killing the enkf program with
                                             SIGTERM (the default kill signal)
                                             you will get a backtrace.  Killing
                                             with SIGKILL (-9) will not give a
                                             backtrace.*/
  signal(SIGABRT , util_abort_signal);    /* Signal abort. */
  {
    ecl_grid_type * grid = ecl_grid_alloc( argv[1] );
    well_info_type * well_info = well_info_alloc( grid );

    well_info_load_rstfile( well_info , argv[2] , true);

    // List all wells:
    for (int iwell = 0; iwell < well_info_get_num_wells( well_info ); iwell++) {
      well_ts_type * well_ts = well_info_get_ts( well_info , well_info_iget_well_name( well_info , iwell));
      well_state_type * well_state = well_ts_get_last_state( well_ts );
      test_assert_not_NULL(well_state);
    }
    well_info_free( well_info );
  }

  exit(0);
}

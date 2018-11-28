/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_rst_header.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_rsthead.hpp>


void test_file( const char * filename , int occurence , bool exists , const ecl_rsthead_type * true_header) {
  int report_step = ecl_util_filename_report_nr( filename );
  ecl_file_type * rst_file = ecl_file_open( filename , 0);
  ecl_file_enum file_type = ecl_util_get_file_type( filename , NULL , NULL );
  ecl_file_view_type * rst_view;
  ecl_rsthead_type * rst_head;

  if (file_type == ECL_RESTART_FILE)
    rst_view = ecl_file_get_global_view( rst_file );
  else
    rst_view = ecl_file_get_restart_view( rst_file , occurence , -1 , -1 , -1 );

  if (exists) {
    test_assert_not_NULL( rst_view );
    rst_head = ecl_rsthead_alloc( rst_view , report_step);
    test_assert_not_NULL( rst_head );

    if (occurence == 0) {
      ecl_rsthead_type * rst_head0 = ecl_rsthead_alloc( rst_view , report_step );

      test_assert_true( ecl_rsthead_equal( rst_head , rst_head0 ));
      ecl_rsthead_free( rst_head0 );
    }
    test_assert_true( ecl_rsthead_equal( rst_head , true_header ));

    ecl_rsthead_free( rst_head );
  } else
    test_assert_NULL( rst_view );

}


int main(int argc , char ** argv) {
  ecl_rsthead_type true1;
  true1.report_step = 1;
  true1.day = 1;
  true1.year = 2000;
  true1.month = 1;
  true1.sim_time = (time_t) 946684800;
  true1.version = 100;
  true1.phase_sum = 7;
  true1.nx = 40;
  true1.ny = 64;
  true1.nz = 14;
  true1.nactive = 34770;
  true1.nwells = 3;
  true1.niwelz = 145;
  true1.nzwelz = 3;
  true1.niconz = 20;
  true1.ncwmax = 120;
  true1.nisegz = 18;
  true1.nsegmx = 1;
  true1.nswlmx = 1;
  true1.nlbrmx = -1;
  true1.nilbrz = -1;
  true1.dualp  = 0;
  true1.sim_days  = 0;

  ecl_rsthead_type true2;
  true2.report_step = 5;
  true2.day = 22;
  true2.year = 1990;
  true2.month = 1;
  true2.sim_time = (time_t) 632966400;
  true2.version = 100;
  true2.phase_sum = 7;
  true2.nx = 4;
  true2.ny = 4;
  true2.nz = 4;
  true2.nactive = 64;
  true2.nwells = 3;
  true2.niwelz = 147;
  true2.nzwelz = 3;
  true2.niconz = 20;
  true2.ncwmax = 13;
  true2.nisegz = 18;
  true2.nsegmx = 1;
  true2.nswlmx = 1;
  true2.nlbrmx = -1;
  true2.nilbrz = -1;
  true2.dualp  = 1;
  true2.sim_days  = 21;

  const char * unified_file = argv[1];
  const char * Xfile        = argv[2];

  test_file( unified_file , 0 , true , &true1 );
  test_file( unified_file , 100 , false , NULL );
  test_file( Xfile , 0 , true , &true2 );

  exit(0);
}

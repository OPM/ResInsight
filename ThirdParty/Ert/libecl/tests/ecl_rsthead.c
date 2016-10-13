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

#include <ert/util/test_util.h>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_rsthead.h>


void test_file( const char * filename , int occurence , bool exists , const ecl_rsthead_type * true_header) {
  ecl_file_type * rst_file = ecl_file_open( filename , 0);
  ecl_rsthead_type * rst_head = ecl_rsthead_ialloc( rst_file , occurence);

  if (exists) {
    test_assert_not_NULL( rst_head );

    if (occurence == 0) {
      ecl_rsthead_type * rst_head0 = ecl_rsthead_alloc( rst_file );

      test_assert_true( ecl_rsthead_equal( rst_head , rst_head0 ));
      ecl_rsthead_free( rst_head0 );
    }
    test_assert_true( ecl_rsthead_equal( rst_head , true_header ));

    ecl_rsthead_free( rst_head );
  } else
    test_assert_NULL( rst_head );

}


int main(int argc , char ** argv) {
  ecl_rsthead_type true1 = {.report_step = 1,
                            .day = 1,
                            .year = 2000,
                            .month = 1,
                            .sim_time = (time_t) 946684800,
                            .version = 100,
                            .phase_sum = 7,
                            .nx = 40,
                            .ny = 64,
                            .nz = 14,
                            .nactive = 34770,
                            .nwells = 3,
                            .niwelz = 145,
                            .nzwelz = 3,
                            .niconz = 20,
                            .ncwmax = 120,
                            .nisegz = 18,
                            .nsegmx = 1,
                            .nswlmx = 1,
                            .nlbrmx = -1,
                            .nilbrz = -1,
                            .dualp  = 0,
                            .sim_days  = 0};

  ecl_rsthead_type true2 = {.report_step = 5,
                            .day = 22,
                            .year = 1990,
                            .month = 1,
                            .sim_time = (time_t) 632966400,
                            .version = 100,
                            .phase_sum = 7,
                            .nx = 4,
                            .ny = 4,
                            .nz = 4,
                            .nactive = 64,
                            .nwells = 3,
                            .niwelz = 147,
                            .nzwelz = 3,
                            .niconz = 20,
                            .ncwmax = 13,
                            .nisegz = 18,
                            .nsegmx = 1,
                            .nswlmx = 1,
                            .nlbrmx = -1,
                            .nilbrz = -1,
                            .dualp  = 1,
                            .sim_days  = 21};

  const char * unified_file = argv[1];
  const char * Xfile        = argv[2];

  //  test_file( unified_file , 0 , true , &true1 );
  //test_file( unified_file , 100 , false , NULL );
  test_file( Xfile , 0 , true , &true2 );

  exit(0);
}

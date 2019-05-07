/*
   Copyright (C) 2014  Equinor ASA, Norway.

   The file 'ecl_kw_fwrite.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>

#include <ert/ecl/ecl_grid.hpp>


void test_fwrite_EGRID(ecl_grid_type * grid ) {
  test_work_area_type * work_area = test_work_area_alloc("grid-has-mapaxes");

  ecl_grid_fwrite_EGRID2( grid , "TEST.EGRID", ECL_METRIC_UNITS);
  {
    ecl_grid_type * copy = ecl_grid_alloc( "TEST.EGRID" );
    test_assert_true( ecl_grid_compare( grid , copy , false , false , true ));
    ecl_grid_free( copy );
  }
  test_work_area_free( work_area );
}

namespace {
  void test_fwrite_fmt_vs_unfmt( ) {
    ecl::util::TestArea ta( "fmt_file" );
    ecl_grid_type * ecl_grid = ecl_grid_alloc_rectangular( 5 , 5 , 5 , 1 , 1 , 1 , nullptr);

    /* .FEGRID -> formatted */
    {
      ecl_grid_fwrite_EGRID2( ecl_grid , "CASE.FEGRID" , ECL_METRIC_UNITS );
      test_assert_true( util_fmt_bit8( "CASE.FEGRID" ) );
    }

    /* .EGRID -> unformatted */
    {
      ecl_grid_fwrite_EGRID2( ecl_grid , "CASE.EGRID" , ECL_METRIC_UNITS );
      test_assert_false( util_fmt_bit8( "CASE.EGRID" ) );
    }

    /* Unknown -> unformatted */
    {
      ecl_grid_fwrite_EGRID2( ecl_grid , "CASE.UNKNOWN" , ECL_METRIC_UNITS );
      test_assert_false( util_fmt_bit8( "CASE.UNKNOWN" ) );
    }

    /* Abuse: .FUNRST -> formatted */
    {
      ecl_grid_fwrite_EGRID2( ecl_grid , "CASE.FUNRST" , ECL_METRIC_UNITS );
      test_assert_true( util_fmt_bit8( "CASE.FUNRST" ) );
    }

    /* Abuse: .FSMSPEC -> formatted */
    {
      ecl_grid_fwrite_EGRID2( ecl_grid , "CASE.FSMSPEC" , ECL_METRIC_UNITS );
      test_assert_true( util_fmt_bit8( "CASE.FSMSPEC" ) );
    }

    /* Abuse: .F0001 -> formatted */
    {
      ecl_grid_fwrite_EGRID2( ecl_grid , "CASE.F0001" , ECL_METRIC_UNITS );
      test_assert_true( util_fmt_bit8( "CASE.F0001" ) );
    }

    /* Abuse: .X1234 -> unformatted */
    {
      ecl_grid_fwrite_EGRID2( ecl_grid , "CASE.X1234" , ECL_METRIC_UNITS );
      test_assert_false( util_fmt_bit8( "CASE.X1234" ) );
    }

    /* Abuse: .UNSMRY -> unformatted */
    {
      ecl_grid_fwrite_EGRID2( ecl_grid , "CASE.UNSMRY" , ECL_METRIC_UNITS );
      test_assert_false( util_fmt_bit8( "CASE.UNSMRY" ) );
    }

    ecl_grid_free( ecl_grid );
  }
}

int main( int argc , char **argv) {
  if (argc > 1) {
    const char * src_file = argv[1];
    ecl_grid_type * grid = ecl_grid_alloc( src_file );

    test_fwrite_EGRID( grid );

    ecl_grid_free( grid );
  }

  test_fwrite_fmt_vs_unfmt( );
}

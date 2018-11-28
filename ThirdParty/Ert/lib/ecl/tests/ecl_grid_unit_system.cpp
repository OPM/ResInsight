/*
  Copyright (C) 2018  Statoil ASA, Norway.

  The file 'ecl_grid_unit_system.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/test_work_area.hpp>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_file.hpp>


void test_EGRID(const char * filename, ert_ecl_unit_enum unit_system) {
  ecl_grid_type * grid = ecl_grid_alloc_rectangular(4,4,2,1,1,1,NULL);
  ecl_grid_fwrite_EGRID2( grid , filename, unit_system);
  ecl_grid_free( grid );


  ecl_grid_type * grid2 = ecl_grid_alloc(filename);
  test_assert_int_equal( ecl_grid_get_unit_system(grid2), unit_system);
  ecl_grid_free(grid2);
}



void test_GRID(const char * filename, ert_ecl_unit_enum unit_system) {
  ecl_grid_type * grid = ecl_grid_alloc_rectangular(4,4,2,1,1,1,NULL);
  ecl_grid_fwrite_GRID2( grid , filename, unit_system);
  ecl_grid_free( grid );


  ecl_grid_type * grid2 = ecl_grid_alloc(filename);
  test_assert_int_equal( ecl_grid_get_unit_system(grid2), unit_system);
  ecl_grid_free(grid2);
}


int main(int argc, char **argv) {
  test_work_area_type * work_area = test_work_area_alloc("grid_export");

  test_EGRID("METRIC.EGRID", ECL_METRIC_UNITS);
  test_EGRID("FIELD.EGRID", ECL_FIELD_UNITS);
  test_GRID("METRIC.GRID", ECL_METRIC_UNITS);
  test_GRID("FIELD.GRID", ECL_FIELD_UNITS);

  test_work_area_free(work_area);
}

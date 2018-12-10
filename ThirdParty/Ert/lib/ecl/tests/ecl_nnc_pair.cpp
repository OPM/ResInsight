/*
   Copyright (C) 2017  Statoil ASA, Norway.

   The file 'ecl_nnc_pair.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/ecl_nnc_geometry.hpp>

void test_pair( int grid1_1 , int grid1_2 , int grid2_1, int grid2_2, bool expected) {
  ecl_nnc_pair_type pair1 = {grid1_1, grid1_2, 0, 0};
  ecl_nnc_pair_type pair2 = {grid2_1, grid2_2, 0, 0};

  test_assert_bool_equal( ecl_nnc_geometry_same_kw( &pair1 , &pair2 ), expected);
}



int main(int argc , char ** argv) {
  test_pair(1,1,  1,1, true);
  test_pair(1,3,  1,3, true);
  test_pair(1,1,  3,3, false);
  test_pair(1,3,  3,1, false);
}

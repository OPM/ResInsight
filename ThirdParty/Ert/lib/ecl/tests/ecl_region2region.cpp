/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ecl_region2region.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/ecl/ecl_sum.hpp>


void has_r2r_key(const ecl_sum_type* ecl_sum) {
  test_assert_true(ecl_sum_has_key( ecl_sum  , "RGF:393217" ));
  test_assert_true(ecl_sum_has_key( ecl_sum  , "RGF:1-2" ));
  test_assert_true(ecl_sum_has_key( ecl_sum  , "ROF:524291"  ));
  test_assert_true(ecl_sum_has_key( ecl_sum , "RWF:393222"  ));
  test_assert_true(ecl_sum_has_key( ecl_sum , "RWF:6-2"  ));
  test_assert_true(ecl_sum_has_key( ecl_sum  , "ROF:3-6"  ));
  test_assert_true(ecl_sum_has_key( ecl_sum  , "RNLF:458753"  ));
  test_assert_true(ecl_sum_has_key( ecl_sum  , "RNLF:1-4"  ));
}

void get_unit(const ecl_sum_type* ecl_sum) {
  test_assert_string_equal(ecl_sum_get_unit( ecl_sum , "RGF:1-2" ), "GFLOW");
  test_assert_string_equal(ecl_sum_get_unit( ecl_sum , "ROF:3-6" ), "OFLOW");
  test_assert_string_equal(ecl_sum_get_unit( ecl_sum , "RWF:6-2" ), "WFLOW");
  test_assert_string_equal(ecl_sum_get_unit( ecl_sum , "RNLF:1-4" ), "NLFLOW");
}

void get_var_params_index(const ecl_sum_type* ecl_sum) {
  test_assert_int_equal(ecl_sum_get_general_var_params_index( ecl_sum , "RGF:1-2"), 21917);
  test_assert_int_equal(ecl_sum_get_general_var_params_index( ecl_sum , "ROF:3-6"), 21918);
  test_assert_int_equal(ecl_sum_get_general_var_params_index( ecl_sum,  "RWF:6-2"), 21919);
  test_assert_int_equal(ecl_sum_get_general_var_params_index( ecl_sum, "RNLF:1-4"), 21920);
}

int main(int argc , char ** argv) {

  const char * headerfile = argv[1];

  ecl_sum_type * ecl_sum = ecl_sum_fread_alloc_case( headerfile , ":");

  if (ecl_sum) {
     has_r2r_key(ecl_sum);
     get_unit(ecl_sum);
     get_var_params_index(ecl_sum);
  }
  else
    test_error_exit("ecl_region2region_test: Test file not read correctly");


  if (ecl_sum)
    ecl_sum_free( ecl_sum );

  exit(0);
}


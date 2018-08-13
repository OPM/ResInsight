/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_kw_cmp_string.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_type.hpp>



void test_cmp_string() {
  ecl_kw_type * ecl_kw = ecl_kw_alloc( "HEADER" , 1 , ECL_CHAR);

  ecl_kw_iset_string8( ecl_kw , 0 , "ABCD");

  test_assert_int_equal( 0 , strcmp( ecl_kw_iget_char_ptr( ecl_kw , 0 ) , "ABCD    "));
  test_assert_true(ecl_kw_icmp_string( ecl_kw , 0 , "ABCD"));
  test_assert_true(ecl_kw_icmp_string( ecl_kw , 0 , "ABCD    "));
  test_assert_true(ecl_kw_icmp_string( ecl_kw , 0 , "ABCD "));

  test_assert_false( ecl_kw_icmp_string( ecl_kw , 0 , "Different"));
  test_assert_false( ecl_kw_icmp_string( ecl_kw , 0 , ""));
  test_assert_false( ecl_kw_icmp_string( ecl_kw , 0 , ""));

}


int main(int argc , char ** argv) {
  test_cmp_string();

  exit(0);
}

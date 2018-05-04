/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_kw_init.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/bool_vector.h>
#include <ert/util/test_util.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_kw.h>


void test_int() {
  size_t N = 1000;
  int i;
  ecl_kw_type * kw = ecl_kw_alloc("KW" , N , ECL_INT);
  for (i=0; i < N; i++)
    test_assert_int_equal( 0 , ecl_kw_iget_int( kw , i ));

  ecl_kw_free( kw );
}


void test_double() {
  size_t N = 1000;
  double i;
  ecl_kw_type * kw = ecl_kw_alloc("KW" , N , ECL_DOUBLE);
  for (i=0; i < N; i++)
    test_assert_double_equal( 0 , ecl_kw_iget_double( kw , i ));

  ecl_kw_free( kw );
}


void test_float() {
  size_t N = 1000;
  int i;
  ecl_kw_type * kw = ecl_kw_alloc("KW" , N , ECL_FLOAT);
  for (i=0; i < N; i++)
    test_assert_int_equal( 0 , ecl_kw_iget_float( kw , i ));

  ecl_kw_free( kw );
}


void test_bool() {
  size_t N = 100;
  bool * data = util_malloc(N * sizeof * data);
  ecl_kw_type * kw = ecl_kw_alloc("BOOL", N , ECL_BOOL);
  for (int i=0; i < N/2; i++) {
    ecl_kw_iset_bool(kw, 2*i, true);
    ecl_kw_iset_bool(kw, 2*i + 1, false);

    data[2*i] = true;
    data[2*i + 1] = false;
  }

  const bool * internal_data = ecl_kw_get_bool_ptr(kw);

  test_assert_int_equal( memcmp(internal_data, data, N * sizeof * data), 0);
  ecl_kw_free(kw);
  free(data);
}

int main( int argc , char ** argv) {
  test_int();
  test_double();
  test_float();
  test_bool();
  exit(0);
}

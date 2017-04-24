/*
   Copyright (C) 2017  Statoil ASA, Norway.

   The file 'ecl_kw_ix_types.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_kw.h>


/*
  The behaviour of the ECL_C010_TYPE is quite unclear; we therefor do
  not alloaw instanstiation of keywords with this type.
*/


void test_create_ECL_C010_TYPE() {
  ecl_kw_type * ecl_kw = ecl_kw_alloc("TEST" , 1000 , ECL_C010 );
  test_assert_NULL( ecl_kw );
}


int main( int argc , char ** argv) {
  test_create_ECL_C010_TYPE();
}

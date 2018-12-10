/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_kw_equal.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_type.hpp>


int main(int argc , char ** argv) {
  ecl_kw_type * ecl_kw1 = ecl_kw_alloc( "KW" , 10 , ECL_INT );
  int data[10];
  int i;
  for (i=0; i < 10; i++) {
    ecl_kw_iset_int(ecl_kw1 , i , i );
    data[i] = i;
  }

  {
    ecl_kw_type * ecl_kw2 = ecl_kw_alloc_copy( ecl_kw1 );

    test_assert_true( ecl_kw_equal( ecl_kw1 , ecl_kw2 ));

    ecl_kw_iset_int( ecl_kw2 , 1 , 77 );
    test_assert_false( ecl_kw_equal( ecl_kw1 , ecl_kw2 ));
    ecl_kw_iset_int( ecl_kw2 , 1 , 1 );
    test_assert_true( ecl_kw_equal( ecl_kw1 , ecl_kw2 ));

    ecl_kw_set_header_name( ecl_kw2 , "TEST" );
    test_assert_false( ecl_kw_equal( ecl_kw1 , ecl_kw2 ));
    test_assert_true( ecl_kw_content_equal( ecl_kw1 , ecl_kw2 ));
    ecl_kw_free( ecl_kw2 );
  }

  {
    ecl_kw_type * ecl_ikw = ecl_kw_alloc_new_shared( "KW" , 10 , ECL_INT , data);
    ecl_kw_type * ecl_fkw = ecl_kw_alloc_new_shared( "KW" , 10 , ECL_FLOAT , data);

    test_assert_true( ecl_kw_content_equal( ecl_kw1 , ecl_ikw ));
    test_assert_false( ecl_kw_content_equal( ecl_kw1 , ecl_fkw ));
  }

  test_assert_true( ecl_kw_data_equal( ecl_kw1 , data ));
  data[0] = 99;
  test_assert_false( ecl_kw_data_equal( ecl_kw1 , data ));



}

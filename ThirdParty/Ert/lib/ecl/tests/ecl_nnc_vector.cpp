/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_nnc_vector.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/int_vector.hpp>

#include <ert/ecl/nnc_vector.hpp>

void test_basic() {
  int lgr_nr = 100;
  nnc_vector_type * vector = nnc_vector_alloc( lgr_nr );

  test_assert_true( nnc_vector_is_instance( vector ));
  test_assert_int_equal( lgr_nr , nnc_vector_get_lgr_nr( vector ));

  nnc_vector_add_nnc( vector , 100 , 1);
  nnc_vector_add_nnc( vector , 200 , 2);
  nnc_vector_add_nnc( vector , 300 , 3);

  nnc_vector_add_nnc( vector , 100 , 4);
  nnc_vector_add_nnc( vector , 200 , 5);
  nnc_vector_add_nnc( vector , 300 , 6);

  test_assert_int_equal( 6 , nnc_vector_get_size( vector ));

  {
    const int_vector_type * grid_index_list = nnc_vector_get_grid_index_list( vector );
    const int_vector_type * nnc_index_list = nnc_vector_get_nnc_index_list( vector );

    test_assert_int_equal( 6   , int_vector_size( nnc_index_list ));
    test_assert_int_equal( 1 , int_vector_iget( nnc_index_list , 0 ));
    test_assert_int_equal( 6 , int_vector_iget( nnc_index_list , 5 ));

    test_assert_int_equal( 6   , int_vector_size( grid_index_list ));
    test_assert_int_equal( 100 , int_vector_iget( grid_index_list , 0 ));
    test_assert_int_equal( 200 , int_vector_iget( grid_index_list , 1 ));
    test_assert_int_equal( 300 , int_vector_iget( grid_index_list , 2 ));
  }

  nnc_vector_free( vector );
}

void test_copy() {
  int lgr_nr = 100;
  nnc_vector_type * vector1 = nnc_vector_alloc( lgr_nr );
  nnc_vector_type * vector2 = nnc_vector_alloc( lgr_nr );
  nnc_vector_type * vector3 = NULL;

  test_assert_true( nnc_vector_equal( vector1 , vector2 ));
  test_assert_false( nnc_vector_equal( vector1 , vector3 ));
  test_assert_false( nnc_vector_equal( vector3 , vector1 ));
  test_assert_true( nnc_vector_equal( vector3 , vector3 ));


  nnc_vector_add_nnc( vector1 , 100 , 1);
  nnc_vector_add_nnc( vector1 , 200 , 2);
  test_assert_false( nnc_vector_equal( vector1 , vector2 ));

  nnc_vector_add_nnc( vector2 , 100 , 1);
  nnc_vector_add_nnc( vector2 , 200 , 2);
  test_assert_true( nnc_vector_equal( vector1 , vector2 ));

  nnc_vector_add_nnc( vector1 , 300 , 3);
  nnc_vector_add_nnc( vector2 , 300 , 30);
  test_assert_false( nnc_vector_equal( vector1 , vector2 ));

  vector3 = nnc_vector_alloc_copy( vector1 );
  test_assert_true( nnc_vector_is_instance( vector3 ));
  test_assert_true( nnc_vector_equal( vector1 , vector3 ));

  nnc_vector_free( vector1 );
  nnc_vector_free( vector2 );
  nnc_vector_free( vector3 );
}


int main(int argc , char ** argv) {
  test_basic();
  test_copy();
  exit(0);
}

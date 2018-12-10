/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_nnc_info_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/int_vector.hpp>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/nnc_info.hpp>
#include <ert/ecl/nnc_vector.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>


void test_equal( ) {

  int lgr_nr = 1;
  nnc_info_type * nnc_info1 = nnc_info_alloc(lgr_nr);
  nnc_info_type * nnc_info2 = nnc_info_alloc(lgr_nr);

  test_assert_false( nnc_info_equal( NULL , nnc_info1 ));
  test_assert_false( nnc_info_equal( nnc_info1, NULL ));

  test_assert_true( nnc_info_equal( nnc_info1 , nnc_info2 ));


  nnc_info_add_nnc(nnc_info1, lgr_nr, 3 , 0);
  test_assert_false( nnc_info_equal( nnc_info1 , nnc_info2 ));

  nnc_info_add_nnc(nnc_info2, lgr_nr, 3 , 0);
  test_assert_true( nnc_info_equal( nnc_info1 , nnc_info2 ));

  nnc_info_add_nnc( nnc_info1 , lgr_nr + 1 , 10 , 10 );
  nnc_info_add_nnc( nnc_info2 , lgr_nr + 2 , 11 , 11 );
  test_assert_false( nnc_info_equal( nnc_info1 , nnc_info2 ));

  nnc_info_add_nnc( nnc_info1 , lgr_nr + 2 , 11 , 11 );
  nnc_info_add_nnc( nnc_info2 , lgr_nr + 1 , 10 , 10 );
  test_assert_true( nnc_info_equal( nnc_info1 , nnc_info2 ));
}



void test_copy( ) {
  int lgr_nr = 1;
  nnc_info_type * nnc_info1 = nnc_info_alloc(lgr_nr);
  nnc_info_add_nnc( nnc_info1 , lgr_nr + 1 , 11 , 11 );
  nnc_info_add_nnc( nnc_info1 , lgr_nr + 2 , 11 , 11 );
  nnc_info_add_nnc( nnc_info1 , lgr_nr + 1 , 111 , 111 );

  {
    nnc_info_type * nnc_copy = nnc_info_alloc_copy( nnc_info1 );
    test_assert_true( nnc_info_equal( nnc_info1 , nnc_copy ));
    nnc_info_free( nnc_copy );
  }
  nnc_info_free( nnc_info1 );
}


void basic_test() {
  int lgr_nr = 77;
  nnc_info_type * nnc_info = nnc_info_alloc(lgr_nr);

  test_assert_int_equal( 0 , nnc_info_get_total_size( nnc_info ));
  test_assert_int_equal( lgr_nr , nnc_info_get_lgr_nr(  nnc_info ));
  test_assert_true(nnc_info_is_instance(nnc_info));
  test_assert_not_NULL(nnc_info);

  nnc_info_add_nnc(nnc_info, lgr_nr, 110 , 0);
  test_assert_int_equal( 1, nnc_info_get_total_size( nnc_info ));

  nnc_info_add_nnc(nnc_info, 1, 110 , 1);
  nnc_info_add_nnc(nnc_info, 1, 111 , 2);
  test_assert_int_equal( 3, nnc_info_get_total_size( nnc_info ));


  nnc_vector_type * nnc_vector = nnc_info_get_vector( nnc_info , 1);
  const int_vector_type * nnc_cells = nnc_info_get_grid_index_list(nnc_info, 1);
  test_assert_int_equal(int_vector_size(nnc_cells), 2);
  test_assert_ptr_equal( nnc_cells , nnc_vector_get_grid_index_list( nnc_vector ));


  nnc_vector_type * nnc_vector_null  = nnc_info_get_vector( nnc_info , 2);
  const int_vector_type * nnc_cells_null = nnc_info_get_grid_index_list(nnc_info, 2);
  test_assert_NULL(nnc_cells_null);
  test_assert_NULL(nnc_vector_null);

  nnc_vector_type * nnc_vector_self  = nnc_info_get_self_vector( nnc_info );
  const nnc_vector_type * nnc_vector_77  = nnc_info_get_vector( nnc_info , lgr_nr );
  test_assert_ptr_equal( nnc_vector_77 , nnc_vector_self );

  const int_vector_type * nnc_cells_77 = nnc_info_get_grid_index_list(nnc_info, lgr_nr);
  const int_vector_type * nnc_cells_self = nnc_info_get_self_grid_index_list(nnc_info);
  test_assert_ptr_equal( nnc_cells_77 , nnc_cells_self );


  test_assert_int_equal( 2 , nnc_info_get_size( nnc_info ));
  test_assert_ptr_equal( nnc_info_get_vector( nnc_info , 1 ) , nnc_info_iget_vector( nnc_info , 1 ));
  nnc_info_free(nnc_info);
}


int main(int argc , char ** argv) {
  basic_test();
  test_equal();
  test_copy();
  exit(0);
}

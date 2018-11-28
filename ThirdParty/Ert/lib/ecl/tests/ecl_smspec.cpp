/*
   Copyright (C) 2016  Statoil ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/ecl/ecl_smspec.hpp>

void test_sort( ecl_smspec_type * smspec )
{
  int num_nodes = ecl_smspec_num_nodes( smspec );
  ecl_smspec_sort( smspec );
  test_assert_int_equal( num_nodes, ecl_smspec_num_nodes( smspec ));

  for (int i=1; i < ecl_smspec_num_nodes( smspec ); i++) {
    const smspec_node_type * node1 = ecl_smspec_iget_node( smspec, i - 1 );
    const smspec_node_type * node2 = ecl_smspec_iget_node( smspec, i );
    test_assert_true( smspec_node_cmp( node1 , node2 ) <= 0 );

    test_assert_int_equal( smspec_node_get_params_index( node1 ) , i - 1 );
  }
}


void test_copy(const ecl_smspec_type * smspec1) {
  ecl_sum_type * ecl_sum2 = ecl_sum_alloc_writer("CASE", false, true, ":", 0, true, 100, 100, 100);
  const ecl_smspec_type * smspec2 = ecl_sum_get_smspec(ecl_sum2);
  for (int i=0; i < ecl_smspec_num_nodes(smspec1); i++) {
    const smspec_node_type * node = ecl_smspec_iget_node(smspec1, i);
    ecl_sum_add_smspec_node(ecl_sum2, node);
  }
  test_assert_true( ecl_smspec_equal(smspec1, smspec2));
  ecl_sum_free(ecl_sum2);
}


int main(int argc, char ** argv) {
  const char * case1 = argv[1];
  const char * case2 = argv[2];
  ecl_smspec_type * smspec1 = ecl_smspec_fread_alloc( case1 , ":" , false );
  ecl_smspec_type * smspec2 = ecl_smspec_fread_alloc( case2 , ":" , false );

  test_assert_true( ecl_smspec_equal( smspec2 , smspec2 ));
  test_assert_true( ecl_smspec_equal( smspec1 , smspec1 ));

  test_assert_false( ecl_smspec_equal( smspec1 , smspec2 ));
  test_assert_false( ecl_smspec_equal( smspec2 , smspec1 ));

  test_sort( smspec1 );
  test_sort( smspec2 );
  ecl_smspec_free( smspec1 );
  ecl_smspec_free( smspec2 );
}

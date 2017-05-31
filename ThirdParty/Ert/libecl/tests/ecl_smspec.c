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

#include <ert/util/test_util.h>

#include <ert/ecl/ecl_smspec.h>



int main(int argc, char ** argv) {
  const char * case1 = argv[1];
  const char * case2 = argv[2];
  ecl_smspec_type * smspec1 = ecl_smspec_fread_alloc( case1 , ":" , false );
  ecl_smspec_type * smspec2 = ecl_smspec_fread_alloc( case2 , ":" , false );

  test_assert_true( ecl_smspec_equal( smspec2 , smspec2 ));
  test_assert_true( ecl_smspec_equal( smspec1 , smspec1 ));

  test_assert_false( ecl_smspec_equal( smspec1 , smspec2 ));
  test_assert_false( ecl_smspec_equal( smspec2 , smspec1 ));

  ecl_smspec_free( smspec1 );
  ecl_smspec_free( smspec2 );
}

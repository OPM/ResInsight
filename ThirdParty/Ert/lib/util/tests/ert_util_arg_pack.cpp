/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ert_util_arg_pack.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <execinfo.h>
#include <unistd.h>

#include <ert/util/test_util.hpp>
#include <ert/util/arg_pack.hpp>



/*
  This test must be compiled with -Werror; an important part of the test
  is that we should get no warnings related to the const pointers.
*/

int main( int argc , char ** argv) {
  const char * ptr1 = "Pointer1";
  const char * ptr2 = "Pointer2";

  arg_pack_type * arg_pack = arg_pack_alloc();
  test_assert_int_equal( 0 , arg_pack_size( arg_pack ));
  arg_pack_append_const_ptr( arg_pack , ptr1 );
  arg_pack_append_const_ptr( arg_pack , ptr2 );

  test_assert_int_equal( 2 , arg_pack_size( arg_pack ));
  test_assert_ptr_equal( ptr1 , arg_pack_iget_const_ptr( arg_pack , 0 ));
  test_assert_ptr_equal( ptr2 , arg_pack_iget_const_ptr( arg_pack , 1 ));



  exit(0);
}

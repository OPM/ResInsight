/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ert_util_before_after.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <time.h>

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>



int main( int argc , char ** argv) {
  time_t t1 = util_make_date_utc(1,1,2000);
  time_t t2 = util_make_date_utc(1,1,2001);

  test_assert_true( util_before( t1 , t2 ));
  test_assert_true( util_after( t2 , t1 ));

  test_assert_false( util_before( t2 , t1 ));
  test_assert_false( util_after( t1 , t2 ));

  test_assert_false( util_before( t1 , t1 ));
  test_assert_false( util_after( t1 , t1 ));

  exit(0);
}

 /*  Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_util_make_date_shift.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/time_t_vector.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>


void test_date(int mday, int month , int year, int * year_offset) {
  time_t t0 = ecl_util_make_date__( mday , month , year , year_offset);
  time_t t1 = util_make_date_utc( mday , month , year + *year_offset);
  test_assert_time_t_equal( t0 , t1 );
}


void test_offset(int mday, int month , int year , int current_offset) {
  int year_offset;
  time_t t0 = ecl_util_make_date__( mday , month , year , &year_offset);
  time_t t1 = util_make_date_utc( mday , month , year + current_offset);

  test_assert_time_t_equal(t0,t1);
  test_assert_int_equal( current_offset , year_offset );
}




int main(int argc , char ** argv) {
  int year_offset;
  test_date(1,1,0 , &year_offset);
  test_offset(1,1,1000 , year_offset);
  exit(0);
}

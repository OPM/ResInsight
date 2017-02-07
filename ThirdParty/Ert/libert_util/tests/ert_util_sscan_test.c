/*
   Copyright (C) 2014  Statoil ASA, Norway.
    
   The file 'ert_util_sscan_test.c' is part of ERT - Ensemble based Reservoir Tool.
    
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
#include <unistd.h>
#include <sys/types.h>

#include <ert/util/util.h>
#include <ert/util/test_util.h>




void test_sscan_percent() {
  {
    const char * MIN_REALIZATIONS = "10%";
    double value = 0.0;
    test_assert_true(util_sscanf_percent(MIN_REALIZATIONS, &value));
    test_assert_double_equal(10.0, value);
  }

  {
    const char * MIN_REALIZATIONS_no_percent = "10";
    double value = 0.0;
    test_assert_false(util_sscanf_percent(MIN_REALIZATIONS_no_percent, &value));
    test_assert_double_equal(0.0, value);
  }

  {
    const char * MIN_REALIZATIONS_float = "10.2%";
    double value = 0.0;
    test_assert_true(util_sscanf_percent(MIN_REALIZATIONS_float, &value));
    test_assert_double_equal(10.2, value);
  }

  {
    const char * MIN_REALIZATIONS_float_no_percentage = "10.2";
    double value = 0.0;
    test_assert_false(util_sscanf_percent(MIN_REALIZATIONS_float_no_percentage, &value));
    test_assert_double_equal(0.0, value);
  }

  {
    const char * MIN_REALIZATIONS= "9 %";
    double value = 0.0;
    test_assert_false(util_sscanf_percent(MIN_REALIZATIONS, &value));
    test_assert_double_equal(0.0, value);
  }

}


void test_date(time_t expected , const char * date_string, bool expected_return) {
  time_t t;
  bool valid = util_sscanf_isodate(date_string, &t);

  test_assert_bool_equal( valid , expected_return );
  if (valid)
    test_assert_time_t_equal( t , expected );
  else
    test_assert_time_t_equal( t , -1 );
}


void test_scan_iso_date() {
  time_t expected = util_make_date_utc(10,  11 , 2011);
  test_date( expected , "2011-11-10", true);

  test_assert_false( util_sscanf_isodate( "2017.10.07" , NULL ));
  test_assert_false( util_sscanf_isodate( "2017-10.7" , NULL ));
  test_assert_false( util_sscanf_isodate( "2017/10/07" , NULL ));

  /* Invalid numeric values */
  test_assert_false( util_sscanf_isodate( "2017-15-07" , NULL ));
  test_assert_false( util_sscanf_isodate( "2017-10-47" , NULL ));
}



int main(int argc , char ** argv) {
  test_sscan_percent();
  test_scan_iso_date();
  exit(0);
}

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

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <ert/util/util.h>
#include <ert/util/test_util.hpp>



void test_sscanf_bool() {
  char const* expected_true = "1\0___""T\0___""True\0""tRuE";
  int const expected_true_step = 5;
  char const* const expected_true_end = expected_true + 4 * expected_true_step;

  char const* expected_false = "0\0____""F\0____""False\0""fALse";
  int const expected_false_step = 6;
  char const* const expected_false_end = expected_false + 4 * expected_false_step;

  char const* expected_fail = "\0___""t\0___""f\0___""Tru\0""asd";
  int const expected_fail_step = 4;
  char const* const expected_fail_end = expected_fail + 5 * expected_fail_step;

  bool value = false;

  for(; expected_true < expected_true_end; expected_true += expected_true_step) {
    value = false;
    test_assert_true( util_sscanf_bool( expected_true, &value) );
    test_assert_bool_equal(value, true);
  }

  for(; expected_false < expected_false_end; expected_false += expected_false_step) {
    value = true;
    test_assert_true( util_sscanf_bool( expected_false, &value) );
    test_assert_bool_equal(value, false);
  }

  for(; expected_fail < expected_fail_end; expected_fail += expected_fail_step) {
    value = true;
    test_assert_false( util_sscanf_bool( expected_fail, &value) );
    test_assert_bool_equal(value, false);
  }

  // Test null buffer
  value = true;
  test_assert_false( util_sscanf_bool(NULL, &value) );
  test_assert_bool_equal(value, false);

  test_assert_false( util_sscanf_bool(NULL, NULL) );
}

void test_sscanf_bytesize() {
  size_t value = 0u;
  test_assert_true(util_sscanf_bytesize("1KB", &value));
  test_assert_size_t_equal(value, 1024);

  test_assert_true(util_sscanf_bytesize("12   mB", &value));
  test_assert_size_t_equal(value, 12 * 1024 * 1024);

  test_assert_true(util_sscanf_bytesize("-47", &value));
  test_assert_size_t_equal(value, (size_t )(-47)); // documentation says overflows are not checked for

  value = 0u;
  test_assert_false(util_sscanf_bytesize("3.7 MB", &value)); // no decimals allowed
  test_assert_size_t_equal(value, 0u);
  test_assert_false(util_sscanf_bytesize("14 TB", &value));  // TB not supported yet
  test_assert_size_t_equal(value, 0u);

  // Test NULL buffer
  test_assert_false(util_sscanf_bytesize(NULL, &value));
  test_assert_size_t_equal(value, 0); // documentation says the value is set to 0 on parsing error

  test_assert_false(util_sscanf_bytesize(NULL, NULL));
}



void test_sscanf_double() {
  double value = 1.0;
  test_assert_true( util_sscanf_double("0.0", &value) );
  test_assert_double_equal(value, 0.0);

  test_assert_true(util_sscanf_double("47.35", &value));
  test_assert_double_equal(value, 47.35);

  test_assert_true( util_sscanf_double("-0.0", &value) );
  test_assert_double_equal(value, 0.0);

  test_assert_true(util_sscanf_double("-54.1341", &value));
  test_assert_double_equal(value, -54.1341);

  test_assert_true(util_sscanf_double("-.1341", &value));
  test_assert_double_equal(value, -0.1341);

  test_assert_true(util_sscanf_double("+.284", &value));
  test_assert_double_equal(value, 0.284);

  test_assert_true(util_sscanf_double("-.45e-2", &value));
  test_assert_double_equal(value, -0.0045);

  test_assert_true(util_sscanf_double("0xFF", &value));
  test_assert_double_equal(value, 255);

  test_assert_true(util_sscanf_double("INF", &value));
  test_assert_double_equal(value, INFINITY);

  test_assert_true(util_sscanf_double("NaN", &value));
  test_assert_true(isnan(value));

  // double max and min
  char buffer[30];
  snprintf(buffer, 30, "-%.20g", DBL_MAX);
  test_assert_true(util_sscanf_double(buffer, &value));
  test_assert_double_equal(value, -DBL_MAX);

  snprintf(buffer, 30, "%.20g", DBL_MIN);
  test_assert_true(util_sscanf_double(buffer, &value));
  test_assert_double_equal(value, DBL_MIN);

  // Garbage characters
  value = 15.3;
  test_assert_false(util_sscanf_double("0x12GWS", &value));
  test_assert_double_equal(value, 15.3);

  test_assert_false(util_sscanf_double("--.+", &value));
  test_assert_double_equal(value, 15.3);

  // NULL buffer
  value = 15.3;
  test_assert_false( util_sscanf_double(NULL, &value) );
  test_assert_double_equal(value, 15.3);

  test_assert_false( util_sscanf_double(NULL, NULL) );

}

void test_sscanf_int() {
  int value = 1;
  test_assert_true( util_sscanf_int("0", &value) );
  test_assert_int_equal(value, 0);

  test_assert_true( util_sscanf_int("241", &value) );
  test_assert_int_equal(value, 241);

  test_assert_true( util_sscanf_int("-0", &value) );
  test_assert_int_equal(value, 0);

  test_assert_true( util_sscanf_int("-852", &value) );
  test_assert_int_equal(value, -852);

  value = 1;
  test_assert_false( util_sscanf_int("+-+-+-", &value) );
  test_assert_int_equal(value, 1);

  test_assert_false( util_sscanf_int("7.5", &value) );
  test_assert_int_equal(value, 1);

  // max and min
  char buffer[30];
  snprintf(buffer, 30, "-%d", INT_MAX);
  test_assert_true(util_sscanf_int(buffer, &value));
  test_assert_int_equal(value, -INT_MAX);

  snprintf(buffer, 30, "%d", INT_MIN);
  test_assert_true(util_sscanf_int(buffer, &value));
  test_assert_int_equal(value, INT_MIN);


  // NULL buffer
  value = 9;
  test_assert_false( util_sscanf_int(NULL, &value) );
  test_assert_int_equal(value, 9);

  test_assert_false( util_sscanf_int(NULL, NULL) );
}


void test_sscanf_octal_int() {
  int value = 1;

  test_assert_true(util_sscanf_octal_int("0", &value));
  test_assert_int_equal(value, 0);

  test_assert_true(util_sscanf_octal_int("241", &value));
  test_assert_int_equal(value, 2 * 64 + 4 * 8 + 1);

  test_assert_true(util_sscanf_octal_int("-0", &value));
  test_assert_int_equal(value, 0);

  test_assert_true(util_sscanf_octal_int("-1742", &value));
  test_assert_int_equal(value, -(512 + 7 * 64 + 4 * 8 + 2));

  value = 1;
  test_assert_false(util_sscanf_octal_int("--5++", &value));
  test_assert_int_equal(value, 1);

  test_assert_false(util_sscanf_octal_int("89", &value));
  test_assert_int_equal(value, 1);

  test_assert_false(util_sscanf_octal_int("7.5", &value));
  test_assert_int_equal(value, 1);

  // NULL buffer
  value = 3;
  test_assert_false(util_sscanf_octal_int(NULL, &value));
  test_assert_int_equal(value, 3);

  test_assert_false(util_sscanf_octal_int(NULL, NULL));
}


void test_sscanf_percent() {
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

  {
    double value = 12.5;
    test_assert_false(util_sscanf_percent(NULL, &value));
    test_assert_double_equal(12.5, value);
  }

  {
    test_assert_false(util_sscanf_percent(NULL, NULL));
  }

}


void check_iso_date(time_t expected , const char * date_string, bool expected_return) {
  time_t t;
  bool valid = util_sscanf_isodate(date_string, &t);

  test_assert_bool_equal( valid , expected_return );
  if (valid)
    test_assert_time_t_equal( t , expected );
  else
    test_assert_time_t_equal( t , -1 );
}


void test_sscanf_isodate() {
  time_t expected = util_make_date_utc(10,  11 , 2011);
  check_iso_date( expected , "2011-11-10", true);

  test_assert_false( util_sscanf_isodate( "2017.10.07" , NULL ));
  test_assert_false( util_sscanf_isodate( "2017-10.7" , NULL ));
  test_assert_false( util_sscanf_isodate( "2017/10/07" , NULL ));

  /* Invalid numeric values */
  test_assert_false( util_sscanf_isodate( "2017-15-07" , NULL ));
  test_assert_false( util_sscanf_isodate( "2017-10-47" , NULL ));

  // Test NULL buffer
  check_iso_date( expected , NULL, false);
  test_assert_false(util_sscanf_isodate(NULL, NULL));
}

void test_sscanf_date_utc() {
  time_t value = 0;
  test_assert_true(util_sscanf_date_utc("16.07^1997", &value));
  test_assert_time_t_equal(value, util_make_date_utc(16, 7, 1997));

  value = util_make_date_utc(10, 11, 2011);
  test_assert_false(util_sscanf_date_utc(NULL, &value));
  test_assert_time_t_equal(value, -1);

  test_assert_false(util_sscanf_date_utc(NULL, NULL));
}




int main(int argc , char ** argv) {
  test_sscanf_bool();
  test_sscanf_bytesize();
  test_sscanf_date_utc();
  test_sscanf_double();
  test_sscanf_int();
  test_sscanf_isodate();
  test_sscanf_octal_int();
  test_sscanf_percent();

  exit(0);
}

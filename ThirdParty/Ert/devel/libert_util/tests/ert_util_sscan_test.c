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


int main(int argc , char ** argv) {
  test_sscan_percent();
  exit(0);
}

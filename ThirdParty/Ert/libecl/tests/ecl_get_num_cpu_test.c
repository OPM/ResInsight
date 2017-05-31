/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ecl_get_num_cpu_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/ecl/ecl_util.h>
   

int main(int argc , char ** argv) {
  const char * filename1 = argv[1];
  const char * filename2 = argv[2];
  const char * filename3 = argv[3];
  const char * filename4 = argv[4];


  test_assert_int_equal(ecl_util_get_num_cpu(filename1), 4);
  test_assert_int_equal(ecl_util_get_num_cpu(filename2), 4);
  test_assert_int_equal(ecl_util_get_num_cpu(filename3), 15);
  test_assert_int_equal(ecl_util_get_num_cpu(filename4), 4);
  exit(0);

}


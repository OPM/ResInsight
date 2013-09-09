/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_cases_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/rng.h>

#include <ert/enkf/cases_config.h>


void test_create_get_set_and_get() {
  cases_config_type * cases_config = cases_config_alloc( );
  test_assert_int_equal( 0, cases_config_get_iteration_number( cases_config ) );
  cases_config_set_int( cases_config , "iteration_number" , 12);
  test_assert_int_equal( 12, cases_config_get_iteration_number( cases_config ) );
  cases_config_fwrite( cases_config , "TEST_CASES_CONFIG" );
  cases_config_fread( cases_config , "TEST_CASES_CONFIG" );
  cases_config_free( cases_config );
}

int main(int argc , char ** argv) {  
  test_create_get_set_and_get();
  exit(0);
}


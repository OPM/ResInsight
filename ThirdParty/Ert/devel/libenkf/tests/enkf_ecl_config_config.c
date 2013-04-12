/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_ecl_config_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/path_stack.h>
#include <ert/util/util.h>

#include <ert/config/config.h>

#include <ert/enkf/ecl_config.h>
#include <ert/enkf/ecl_refcase_list.h>

int main(int argc , char ** argv) {
  const char * config_file = argv[1];

  ecl_config_type * ecl_config = ecl_config_alloc_empty();
  ecl_refcase_list_type * refcase_list = ecl_config_get_refcase_list( ecl_config );
  {
    config_type * config = config_alloc();

    ecl_config_add_config_items( config );
    test_assert_true( config_parse( config , config_file , "--" , NULL , NULL , CONFIG_UNRECOGNIZED_WARN , true));
    ecl_config_init( ecl_config , config );

    config_free( config );
  }

  test_assert_true( ecl_config_has_refcase( ecl_config ));
  test_assert_int_equal( ecl_refcase_list_get_size( refcase_list) , 17);

  ecl_config_free( ecl_config );
  

  exit(0);
}


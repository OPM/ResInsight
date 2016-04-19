/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_gen_data_config_parse.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#include "ert/util/build_config.h"

#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>

#include <ert/enkf/ensemble_config.h>

void add_NULL_node( void * arg) {
  ensemble_config_type * ens_config = ensemble_config_safe_cast( arg );
  ensemble_config_add_node( ens_config , NULL );
}



void test_abort_on_add_NULL() {
  ensemble_config_type * ensemble_config = ensemble_config_alloc();

  test_assert_true( ensemble_config_is_instance( ensemble_config ));
  test_assert_util_abort("ensemble_config_add_node" , add_NULL_node , ensemble_config );

  ensemble_config_free( ensemble_config );
}


int main( int argc , char ** argv) {
  test_abort_on_add_NULL();
  exit(0);
}

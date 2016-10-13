/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_analysis_config_ext_module.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/enkf/analysis_config.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/rng.h>

#include <ert/enkf/analysis_config.h>


void test_load_external_module( analysis_config_type * ac , const char * user_name , const char * lib_name) {
  test_assert_true( analysis_config_load_external_module(ac , lib_name , user_name ));
}



int main(int argc , char ** argv) {
  rng_type * rng = rng_alloc(MZRAN , INIT_DEFAULT);
  analysis_config_type * analysis_config = analysis_config_alloc(rng);

  for (int i = 1; i < argc; i+= 2) {
    const char * user_name = argv[i];
    const char * lib_name = argv[i + 1];
    test_load_external_module( analysis_config , user_name , lib_name );
  }

  analysis_config_free(analysis_config);
  rng_free( rng );
  exit(0);
}




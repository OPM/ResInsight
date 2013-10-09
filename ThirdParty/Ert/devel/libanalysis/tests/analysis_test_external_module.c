/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'analysis_test_external_module.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>
#include <ert/util/test_util.h>
#include <ert/util/rng.h>

#include <ert/analysis/analysis_module.h>



void load_module( rng_type * rng , const char * user_name , const char * lib_name, const char * options_str , int nvar , const char ** var_list) {
  long flags = strtol(options_str , NULL , 10);
  analysis_module_type * analysis_module = analysis_module_alloc_external(rng , user_name , lib_name);

  printf("Loading:%s \n" , lib_name);
  test_assert_string_equal( EXTERNAL_MODULE_NAME , analysis_module_get_table_name(analysis_module));
  if (util_is_abs_path(lib_name))
    test_assert_string_equal( lib_name , analysis_module_get_lib_name(analysis_module));

  test_assert_true( analysis_module_is_instance( analysis_module));
  for (int i=0; i < nvar; i++) {
    printf("has_var(%s) \n" , var_list[i]);
    test_assert_true( analysis_module_has_var(analysis_module , var_list[i]));
  }
  test_assert_false( analysis_module_has_var(analysis_module , "DoesNotHaveThisVariable"));
  test_assert_true( analysis_module_check_option( analysis_module , flags));
  flags += 1;
  test_assert_false( analysis_module_check_option( analysis_module , flags));
  analysis_module_free( analysis_module);
}


int main(int argc , char ** argv) {
  const char * user_name = argv[1];
  const char * lib_name  = argv[2];
  const char * options_str = argv[3];
  int nvar = argc - 4;
  rng_type * rng = rng_alloc( MZRAN , INIT_DEFAULT);

  load_module(rng , user_name , lib_name , options_str , nvar , (const char **) &argv[4]);
  rng_free( rng );

  exit(0);
}

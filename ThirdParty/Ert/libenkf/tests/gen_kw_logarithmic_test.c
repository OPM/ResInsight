/*
   Copyright (C) 2014  Statoil ASA, Norway.
    
   The file 'gen_kw_test.c' is part of ERT - Ensemble based Reservoir Tool.
    
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
#include <ert/enkf/ert_test_context.h>
#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/fortio.h>
#include <ert/util/type_macros.h>
#include <ert/ecl/ecl_endian_flip.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_node.h>
#include <ert/enkf/gen_kw.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/run_arg.h>



void test_write_gen_kw_export_file(enkf_main_type * enkf_main)
{
  enkf_state_type * state = enkf_main_iget_state( enkf_main , 0 );
  test_assert_not_NULL(state);
  enkf_node_type * enkf_node  = enkf_state_get_node( state , "MULTFLT" );
  enkf_node_type * enkf_node2 = enkf_state_get_node( state , "MULTFLT2" );
  test_assert_not_NULL(enkf_node);
  test_assert_not_NULL(enkf_node2);
  test_assert_true(enkf_node_get_impl_type(enkf_node)  == GEN_KW);
  test_assert_true(enkf_node_get_impl_type(enkf_node2) == GEN_KW);

  gen_kw_type * gen_kw  = enkf_node_value_ptr(enkf_node);
  gen_kw_type * gen_kw2 = enkf_node_value_ptr(enkf_node2);


  {
    rng_type * rng                       = rng_alloc( MZRAN , INIT_DEFAULT );
    const enkf_config_node_type * config = enkf_node_get_config(enkf_node);
    const int    data_size               = enkf_config_node_get_data_size( config, 0 );
    const double                    mean = 0.0; /* Mean and std are hardcoded - the variability should be in the transformation. */
    const double                    std  = 1.0;

    for (int i=0; i < data_size; ++i) {
      double random_number = enkf_util_rand_normal(mean , std , rng);
      gen_kw_data_iset(gen_kw,  i, random_number);
      gen_kw_data_iset(gen_kw2, i, random_number);
    }

    rng_free(rng);
  }


  {
    enkf_fs_type * init_fs = enkf_main_get_fs( enkf_main );
    run_arg_type * run_arg = run_arg_alloc_INIT_ONLY( init_fs , 0 ,0 , "simulations/run0");
    enkf_state_ecl_write(state, run_arg , init_fs);
    test_assert_true(util_file_exists("simulations/run0/parameters.txt"));
    run_arg_free( run_arg );
  }


  {
    int buffer_size = 0;
    char * file_content = util_fread_alloc_file_content("simulations/run0/parameters.txt", &buffer_size);

    stringlist_type * token_list = stringlist_alloc_from_split(file_content, " \n");
    double value = stringlist_iget_as_double(token_list, 5, NULL);

    test_assert_true(value > 0.0); //Verify precision
    test_assert_true(NULL != strstr(file_content, "LOG10_")); //Verify log entry

    stringlist_free(token_list);
    free(file_content);
  }
}



int main(int argc , char ** argv) {
  const char * config_file             =  argv[1];
  ert_test_context_type * test_context = ert_test_context_alloc("gen_kw_logarithmic_test" , config_file );
  enkf_main_type * enkf_main           = ert_test_context_get_main(test_context);

  test_assert_not_NULL(enkf_main);

  test_write_gen_kw_export_file(enkf_main);

  ert_test_context_free( test_context );
  exit(0);
}


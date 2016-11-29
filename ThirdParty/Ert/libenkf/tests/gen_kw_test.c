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

#include "ert/util/build_config.h"

#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>
#include <ert/enkf/ert_test_context.h>
#include <ert/util/util.h>
#include <ert/util/vector.h>

#include <ert/ecl/fortio.h>
#include <ert/util/type_macros.h>
#include <ert/ecl/ecl_endian_flip.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_node.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/run_arg.h>
#include <ert/enkf/gen_kw_config.h>



void test_send_fortio_to_gen_kw_ecl_write(void * arg) {
  enkf_main_type * enkf_main = arg;
  test_assert_not_NULL(enkf_main);
  fortio_type * fortio  = fortio_open_writer("my_new_file", false, ECL_ENDIAN_FLIP);
  test_assert_not_NULL(fortio);

  enkf_state_type * state  = enkf_main_iget_state( enkf_main , 0 );
  test_assert_not_NULL(state);
  enkf_node_type * enkf_node = enkf_state_get_node( state , "MULTFLT" );
  test_assert_not_NULL(enkf_node);
  const enkf_config_node_type * config_node = enkf_node_get_config(enkf_node);
  test_assert_not_NULL(config_node);

  if (GEN_KW == enkf_config_node_get_impl_type(config_node)) {
    const char * dummy_path = "dummy_path";
    enkf_node_ecl_write(enkf_node, dummy_path, fortio, 0);
  }
}


void test_write_gen_kw_export_file(enkf_main_type * enkf_main)
{
  test_assert_not_NULL(enkf_main);
  enkf_fs_type * init_fs = enkf_main_get_fs( enkf_main );
  enkf_state_type * state = enkf_main_iget_state( enkf_main , 0 );
  run_arg_type * run_arg = run_arg_alloc_INIT_ONLY( init_fs , 0 ,0 , "simulations/run0");
  test_assert_not_NULL(state);
  enkf_node_type * enkf_node = enkf_state_get_node( state , "MULTFLT" );

  test_assert_not_NULL(enkf_node);
  const enkf_config_node_type * config_node = enkf_node_get_config(enkf_node);
  test_assert_not_NULL(config_node);

  if (GEN_KW == enkf_config_node_get_impl_type(config_node)) {
    enkf_state_ecl_write(state, run_arg , init_fs);
    test_assert_true(util_file_exists("simulations/run0/parameters.txt"));
  }
  run_arg_free( run_arg );
}



static void read_erroneous_gen_kw_file( void * arg) {
  vector_type * arg_vector = vector_safe_cast( arg );
  gen_kw_config_type * gen_kw_config = vector_iget( arg_vector, 0 );
  const char * filename = vector_iget( arg, 1 );
  gen_kw_config_set_parameter_file(gen_kw_config, filename);
}


void test_read_erroneous_gen_kw_file() {
  const char * parameter_filename = "MULTFLT_with_errors.txt";
  const char * tmpl_filename = "MULTFLT.tmpl";

  {
    FILE * stream = util_fopen(parameter_filename, "w");
    const char * data = util_alloc_sprintf("MULTFLT1 NORMAL 0\nMULTFLT2 RAW\nMULTFLT3 NORMAL 0");
    util_fprintf_string(data, 30, true, stream);
    util_fclose(stream);

    FILE * tmpl_stream = util_fopen(tmpl_filename, "w");
    const char * tmpl_data = util_alloc_sprintf("<MULTFLT1> <MULTFLT2> <MULTFLT3>\n");
    util_fprintf_string(tmpl_data, 30, true, tmpl_stream);
    util_fclose(tmpl_stream);
  }

  gen_kw_config_type * gen_kw_config = gen_kw_config_alloc_empty("MULTFLT", "<%s>");
  vector_type * arg = vector_alloc_new();
  vector_append_ref( arg , gen_kw_config );
  vector_append_ref(arg, parameter_filename);

  test_assert_util_abort("arg_pack_fscanf", read_erroneous_gen_kw_file,  arg);

  vector_free(arg);
  gen_kw_config_free(gen_kw_config);
}


int main(int argc , char ** argv) {
  const char * config_file             =  argv[1];
  ert_test_context_type * test_context = ert_test_context_alloc("gen_kw_test" , config_file );
  enkf_main_type * enkf_main           = ert_test_context_get_main(test_context);
  test_assert_not_NULL(enkf_main);

  test_write_gen_kw_export_file(enkf_main);
  test_assert_util_abort("gen_kw_ecl_write", test_send_fortio_to_gen_kw_ecl_write, enkf_main);
  test_read_erroneous_gen_kw_file();

  ert_test_context_free( test_context );
  exit(0);
}


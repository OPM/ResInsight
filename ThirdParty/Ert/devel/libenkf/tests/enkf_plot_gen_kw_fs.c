/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_plot_gen_kw_fs.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>

#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_plot_gen_kw.h>
#include <ert/enkf/ert_test_context.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/gen_kw_config.h>


void test_load(const char * config_file) {
  ert_test_context_type * test_context = ert_test_context_alloc( "GEN_KW" , config_file );
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  int ens_size = enkf_main_get_ensemble_size( enkf_main );
  stringlist_type * param_list = stringlist_alloc_new();
  enkf_fs_type * init_fs = enkf_fs_create_fs( "fs" , BLOCK_FS_DRIVER_ID , NULL , true );

  stringlist_append_ref( param_list , "GEN_KW");
  enkf_main_initialize_from_scratch( enkf_main , init_fs , param_list , 0 , ens_size - 1 , INIT_FORCE);
  {
    ensemble_config_type  * ensemble_config = enkf_main_get_ensemble_config( enkf_main );
    enkf_config_node_type * config_node = ensemble_config_get_node( ensemble_config , "GEN_KW");
    enkf_plot_gen_kw_type * plot_gen_kw = enkf_plot_gen_kw_alloc( config_node );
    bool_vector_type      * input_mask  = bool_vector_alloc( ens_size , true );
    gen_kw_config_type    * gen_kw_config = enkf_config_node_get_ref( config_node );

    enkf_plot_gen_kw_load( plot_gen_kw , init_fs , true , 0 , ANALYZED , input_mask );

    test_assert_int_equal( ens_size , enkf_plot_gen_kw_get_size( plot_gen_kw ));

    test_assert_int_equal(4, enkf_plot_gen_kw_get_keyword_count(plot_gen_kw));

    {
      enkf_plot_gen_kw_vector_type * vector = enkf_plot_gen_kw_iget( plot_gen_kw , 0 );
      for (int i=0; i < enkf_plot_gen_kw_vector_get_size( vector ); i++)
        test_assert_string_equal( enkf_plot_gen_kw_iget_key( plot_gen_kw , i ) , gen_kw_config_iget_name( gen_kw_config , i));
    }
    bool_vector_free( input_mask );
  }

  stringlist_free( param_list );
  enkf_fs_decref( init_fs );
  ert_test_context_free( test_context );
}





int main( int argc , char ** argv) {
  util_install_signals();
  {
    const char * config_file = argv[1];
    test_load( config_file );
    exit(0);
  }
}


/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_plot_gen_kw.c' is part of ERT - Ensemble based Reservoir Tool.

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

void test_create_invalid() {
  enkf_config_node_type * config_node = enkf_config_node_alloc_summary( "WWCT" , LOAD_FAIL_SILENT);
  enkf_plot_gen_kw_type * gen_kw = enkf_plot_gen_kw_alloc( config_node );

  test_assert_NULL( gen_kw );
  enkf_config_node_free( config_node );
}


void test_create() {
  enkf_config_node_type * config_node = enkf_config_node_new_gen_kw( "GEN_KW" , DEFAULT_GEN_KW_TAG_FORMAT, false);

  {
    enkf_plot_gen_kw_type * gen_kw = enkf_plot_gen_kw_alloc( config_node );
    test_assert_true( enkf_plot_gen_kw_is_instance( gen_kw ));
    test_assert_int_equal( 0 , enkf_plot_gen_kw_get_size( gen_kw ));
    enkf_plot_gen_kw_free( gen_kw );
  }

  enkf_config_node_free( config_node );
}




int main( int argc , char ** argv) {
  test_create();
  test_create_invalid();
  exit(0);
}

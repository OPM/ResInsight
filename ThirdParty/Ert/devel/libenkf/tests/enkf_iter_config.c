/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_iter_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/config/config.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>

#include <ert/enkf/analysis_iter_config.h>
#include <ert/enkf/config_keys.h>


#define TMP_PATH "/tmp"
char * create_config_file( const char * enspath_fmt , const char * runpath_fmt , int iter_count) {
  char * config_file = util_alloc_tmp_file(TMP_PATH , "iter-config" , false);
  FILE * stream = util_fopen( config_file , "w");
  fprintf(stream , "%s  %s\n" , ITER_CASE_KEY , enspath_fmt);
  fprintf(stream , "%s  %s\n" , ITER_RUNPATH_KEY , runpath_fmt);
  fprintf(stream , "%s  %d\n" , ITER_COUNT_KEY   , iter_count);
  fclose( stream );
  return config_file;
}


int main(int argc , char ** argv) {
  const char * enspath_fmt = "iter%d";
  const char * runpath_fmt = "run/iter%d/real%d";
  const int iter_count = 10;
  char * config_file = create_config_file( enspath_fmt , runpath_fmt , iter_count);
  

  config_type * config = config_alloc();
  analysis_iter_config_add_config_items( config );
  
  test_assert_true( config_parse( config , config_file , NULL , NULL , NULL , CONFIG_UNRECOGNIZED_ERROR , true));

  test_assert_true( config_item_set( config , ITER_CASE_KEY ));
  test_assert_true( config_item_set( config , ITER_RUNPATH_KEY ));
  test_assert_true( config_item_set( config , ITER_COUNT_KEY ));

  {
    analysis_iter_config_type * iter_config = analysis_iter_config_alloc();
    test_assert_string_equal( analysis_iter_config_iget_case( iter_config , 5) , NULL );
    test_assert_string_equal( analysis_iter_config_iget_runpath_fmt( iter_config , 5) , NULL );
    
    analysis_iter_config_init( iter_config , config );
    
    test_assert_int_equal( analysis_iter_config_get_num_iterations( iter_config ) , iter_count );
    test_assert_string_equal( analysis_iter_config_iget_case( iter_config , 5) , "iter5");
    test_assert_string_equal( analysis_iter_config_iget_runpath_fmt( iter_config , 5) , "run/iter5/real%d" );    
    
    analysis_iter_config_free( iter_config );
  }
  remove( config_file );
  free( config_file );
  config_free( config );
  exit(0);
}

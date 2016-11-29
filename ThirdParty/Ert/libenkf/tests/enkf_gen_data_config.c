/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'enkf_gen_data_config.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_work_area.h>
#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>
#include <ert/enkf/gen_data.h>
#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/forward_load_context.h>
#include <ert/enkf/run_arg.h>


void test_report_steps_param() {

  gen_data_config_type * config = gen_data_config_alloc_GEN_PARAM("KEY" , ASCII , ASCII);
  test_assert_false( gen_data_config_is_dynamic( config ));
  test_assert_int_equal( 0 , gen_data_config_num_report_step( config ));
  test_assert_false( gen_data_config_has_report_step( config , 0 ));

  /* Add to parameter should fail. */
  gen_data_config_add_report_step( config , 10 );
  test_assert_int_equal( 0 , gen_data_config_num_report_step( config ));
  test_assert_false( gen_data_config_has_report_step( config , 10 ));

  /* Add to parameter should fail. */
  gen_data_config_set_active_report_steps_from_string( config , "0-9,100");
  test_assert_int_equal( 0 , gen_data_config_num_report_step( config ));
  test_assert_false( gen_data_config_has_report_step( config , 10 ));


  gen_data_config_free( config );
}


void test_report_steps_dynamic() {
  gen_data_config_type * config = gen_data_config_alloc_GEN_DATA_result("KEY" , ASCII);
  test_assert_true( gen_data_config_is_dynamic( config ));
  test_assert_int_equal( 0 , gen_data_config_num_report_step( config ));
  test_assert_false( gen_data_config_has_report_step( config , 0 ));

  gen_data_config_add_report_step( config , 10 );
  test_assert_int_equal( 1 , gen_data_config_num_report_step( config ));
  test_assert_true( gen_data_config_has_report_step( config , 10 ));
  test_assert_int_equal( gen_data_config_iget_report_step( config , 0 ) , 10);

  gen_data_config_add_report_step( config , 10 );
  test_assert_int_equal( 1 , gen_data_config_num_report_step( config ));
  test_assert_true( gen_data_config_has_report_step( config , 10 ));


  gen_data_config_add_report_step( config , 5 );
  test_assert_int_equal( 2 , gen_data_config_num_report_step( config ));
  test_assert_true( gen_data_config_has_report_step( config , 10 ));
  test_assert_int_equal( gen_data_config_iget_report_step( config , 0 ) , 5);
  test_assert_int_equal( gen_data_config_iget_report_step( config , 1 ) , 10);

  {
    const int_vector_type * active_steps = gen_data_config_get_active_report_steps( config );

    test_assert_int_equal( int_vector_iget( active_steps  , 0 ) , 5);
    test_assert_int_equal( int_vector_iget( active_steps  , 1 ) , 10);
  }

  gen_data_config_set_active_report_steps_from_string( config , "0-3,7-10,100"); // 0,1,2,3,7,8,9,10,100
  test_assert_int_equal( 9 , gen_data_config_num_report_step( config ));
  test_assert_int_equal( 0 , gen_data_config_iget_report_step( config , 0 ));
  test_assert_int_equal( 3 , gen_data_config_iget_report_step( config , 3));
  test_assert_int_equal( 9 , gen_data_config_iget_report_step( config , 6));
  test_assert_int_equal( 100 , gen_data_config_iget_report_step( config , 8));

  gen_data_config_free( config );
}


void test_gendata_fload(const char * filename) {
  test_work_area_type * work_area = test_work_area_alloc( "test_gendata_fload");
  gen_data_config_type * config = gen_data_config_alloc_GEN_DATA_result("KEY" , ASCII);
  gen_data_type * gen_data = gen_data_alloc(config);

  const char * cwd = test_work_area_get_cwd(work_area);
  enkf_fs_type * write_fs = enkf_fs_create_fs(cwd, BLOCK_FS_DRIVER_ID, NULL , true);
  run_arg_type * run_arg = run_arg_alloc_ENSEMBLE_EXPERIMENT(write_fs , 0,0,"path");
  forward_load_context_type * load_context = forward_load_context_alloc( run_arg , false , NULL , NULL , NULL);
  forward_load_context_select_step(load_context , 0 );
  gen_data_fload_with_report_step(gen_data, filename , load_context);
  int data_size = gen_data_config_get_data_size(config, 0);
  test_assert_true(data_size > 0);
  enkf_fs_decref( write_fs );

  gen_data_free(gen_data);
  gen_data_config_free( config );
  test_work_area_free(work_area);
  run_arg_free( run_arg );
  forward_load_context_free( load_context );
}


void test_gendata_fload_empty_file(const char * filename) {
  test_work_area_type * work_area = test_work_area_alloc( "test_gendata_fload_empty_file" );
  gen_data_config_type * config = gen_data_config_alloc_GEN_DATA_result("KEY" , ASCII);
  gen_data_type * gen_data = gen_data_alloc(config);
  const char * cwd = test_work_area_get_cwd(work_area);
  enkf_fs_type * write_fs = enkf_fs_create_fs(cwd, BLOCK_FS_DRIVER_ID, NULL , true);
  run_arg_type * run_arg = run_arg_alloc_ENSEMBLE_EXPERIMENT(write_fs , 0,0,"path");
  forward_load_context_type * load_context = forward_load_context_alloc( run_arg , false , NULL , NULL , NULL);

  forward_load_context_select_step(load_context , 0 );
  gen_data_fload_with_report_step(gen_data, filename, load_context);
  int data_size = gen_data_config_get_data_size(config, 0);
  test_assert_true(data_size == 0);
  enkf_fs_decref( write_fs );

  gen_data_free(gen_data);
  gen_data_config_free( config );
  test_work_area_free(work_area);
  run_arg_free( run_arg );
  forward_load_context_free( load_context );
}


void test_result_format() {
  test_assert_true( gen_data_config_valid_result_format("path/file%d/extra"));
  test_assert_true( gen_data_config_valid_result_format("file%04d"));
  test_assert_false( gen_data_config_valid_result_format("/path/file%04d"));

  test_assert_false( gen_data_config_valid_result_format("/path/file%s"));
  test_assert_false( gen_data_config_valid_result_format("/path/file"));
  test_assert_false( gen_data_config_valid_result_format("/path/file%f"));

  test_assert_false( gen_data_config_valid_result_format(NULL));
}


void alloc_invalid_io_format1( void * arg) {
  gen_data_config_type * config = gen_data_config_alloc_GEN_DATA_result("KEY" , ASCII_TEMPLATE );
  gen_data_config_free( config );
}


void alloc_invalid_io_format2( void * arg) {
  gen_data_config_type * config = gen_data_config_alloc_GEN_DATA_state("KEY" , GEN_DATA_UNDEFINED , ASCII);
  gen_data_config_free( config );
}


void alloc_invalid_io_format3( void *arg) {
  gen_data_config_type * config = gen_data_config_alloc_GEN_PARAM("KEY" , ASCII , ASCII_TEMPLATE );
  gen_data_config_free( config );
}



void test_set_invalid_format() {
  test_assert_util_abort( "gen_data_config_alloc_GEN_DATA_result" , alloc_invalid_io_format1  , NULL);
  test_assert_util_abort( "gen_data_config_alloc_GEN_DATA_state"  , alloc_invalid_io_format2  , NULL);
  test_assert_util_abort( "gen_data_config_alloc_GEN_PARAM"       , alloc_invalid_io_format3  , NULL);
}


void test_format_check() {
  test_assert_int_equal( GEN_DATA_UNDEFINED , gen_data_config_check_format( NULL ));
  test_assert_int_equal( GEN_DATA_UNDEFINED , gen_data_config_check_format("Error?"));
  test_assert_int_equal( ASCII              , gen_data_config_check_format("ASCII"));
  test_assert_int_equal( ASCII_TEMPLATE     , gen_data_config_check_format("ASCII_TEMPLATE"));
  test_assert_int_equal( BINARY_DOUBLE      , gen_data_config_check_format("BINARY_DOUBLE"));
  test_assert_int_equal( BINARY_FLOAT       , gen_data_config_check_format("BINARY_FLOAT"));
}


void test_set_template_invalid() {
  test_work_area_type * work_area = test_work_area_alloc("GEN_DATA_SET_TEMPLATE_INVALID");
  gen_data_config_type * config = gen_data_config_alloc_GEN_PARAM("KEY" , ASCII , ASCII);

  test_assert_false( gen_data_config_set_template( config , "does/not/exist" , NULL ) );

  {
    FILE * stream = util_fopen("template.txt" , "w");
    fprintf(stream , "Header1\n<KEY>\nHeader2\n");
    fclose( stream );

    gen_data_config_set_template( config , "template.txt" , "<KEY>");
    test_assert_string_equal( "template.txt" , gen_data_config_get_template_file( config ));
    test_assert_string_equal( "<KEY>" , gen_data_config_get_template_key( config ));


    {
      char * buffer;
      int data_offset , buffer_size , data_skip;
      gen_data_config_get_template_data( config , &buffer , &data_offset , &buffer_size , &data_skip);

        test_assert_string_equal( buffer , "Header1\n<KEY>\nHeader2\n");
        test_assert_int_equal( data_offset , 8  );
        test_assert_int_equal( buffer_size , 22 );
        test_assert_int_equal( data_skip   , 5  );
    }
  }


  {
    FILE * stream = util_fopen("template2.txt" , "w");
    fprintf(stream , "Template XYZ - lots of shit .... \n");
    fclose( stream );

    test_assert_false( gen_data_config_set_template( config , "template2.txt" , "<KEY>"));

    test_assert_string_equal( "template.txt" , gen_data_config_get_template_file( config ));
    test_assert_string_equal( "<KEY>" , gen_data_config_get_template_key( config ));
    {
      char * buffer;
      int data_offset , buffer_size , data_skip;
      gen_data_config_get_template_data( config , &buffer , &data_offset , &buffer_size , &data_skip);

        test_assert_string_equal( buffer , "Header1\n<KEY>\nHeader2\n");
        test_assert_int_equal( data_offset , 8  );
        test_assert_int_equal( buffer_size , 22 );
        test_assert_int_equal( data_skip   , 5  );
    }
  }

  gen_data_config_free( config );
  test_work_area_free( work_area );
}



void test_set_template() {
  test_work_area_type * work_area = test_work_area_alloc("GEN_DATA_SET_TEMPLATE");
  {
    gen_data_config_type * config = gen_data_config_alloc_GEN_PARAM("KEY" , ASCII , ASCII);

    test_assert_true( gen_data_config_set_template( config , NULL , NULL ) );
    test_assert_NULL( gen_data_config_get_template_file( config ));
    test_assert_NULL( gen_data_config_get_template_key( config ));

    {
      char * buffer;
      int data_offset , buffer_size , data_skip;
      gen_data_config_get_template_data( config , &buffer , &data_offset , &buffer_size , &data_skip);

      test_assert_NULL( buffer );
      test_assert_int_equal( data_offset , 0 );
      test_assert_int_equal( buffer_size , 0 );
      test_assert_int_equal( data_skip   , 0 );
    }


    {
      FILE * stream = util_fopen("template.txt" , "w");
      fprintf(stream , "Header\n");
      fclose( stream );

      test_assert_true( gen_data_config_set_template( config , "template.txt" , NULL ));
      test_assert_string_equal( "template.txt" , gen_data_config_get_template_file( config ));
      test_assert_NULL( gen_data_config_get_template_key( config ));


      {
        char * buffer;
        int data_offset , buffer_size , data_skip;
        gen_data_config_get_template_data( config , &buffer , &data_offset , &buffer_size , &data_skip);

        test_assert_string_equal( buffer , "Header\n");
        test_assert_int_equal( data_offset , 7 );
        test_assert_int_equal( buffer_size , 7 );
        test_assert_int_equal( data_skip   , 0 );
      }
    }

    {
      FILE * stream = util_fopen("template.txt" , "w");
      fprintf(stream , "Header1\n<KEY>\nHeader2\n");
      fclose( stream );

      gen_data_config_set_template( config , "template.txt" , "<KEY>");
      test_assert_string_equal( "template.txt" , gen_data_config_get_template_file( config ));
      test_assert_string_equal( "<KEY>" , gen_data_config_get_template_key( config ));


      {
        char * buffer;
        int data_offset , buffer_size , data_skip;
        gen_data_config_get_template_data( config , &buffer , &data_offset , &buffer_size , &data_skip);

        test_assert_string_equal( buffer , "Header1\n<KEY>\nHeader2\n");
        test_assert_int_equal( data_offset , 8  );
        test_assert_int_equal( buffer_size , 22 );
        test_assert_int_equal( data_skip   , 5  );
      }
    }


    gen_data_config_set_template( config , NULL , NULL );
    test_assert_NULL( gen_data_config_get_template_file( config ));
    test_assert_NULL( gen_data_config_get_template_key( config ));

    {
      char * buffer;
      int data_offset , buffer_size , data_skip;
      gen_data_config_get_template_data( config , &buffer , &data_offset , &buffer_size , &data_skip);

      test_assert_NULL( buffer );
      test_assert_int_equal( data_offset , 0 );
      test_assert_int_equal( buffer_size , 0 );
      test_assert_int_equal( data_skip   , 0 );
    }


    test_assert_true( gen_data_config_set_template( config , NULL , "KEY"));
    test_assert_NULL( gen_data_config_get_template_file( config ));
    test_assert_NULL( gen_data_config_get_template_key( config ));

    {
      char * buffer;
      int data_offset , buffer_size , data_skip;
      gen_data_config_get_template_data( config , &buffer , &data_offset , &buffer_size , &data_skip);

      test_assert_NULL( buffer );
      test_assert_int_equal( data_offset , 0 );
      test_assert_int_equal( buffer_size , 0 );
      test_assert_int_equal( data_skip   , 0 );
    }


    gen_data_config_free( config );
  }
  test_work_area_free( work_area );
}


int main(int argc , char ** argv) {

  const char * gendata_file       = argv[1];
  const char * gendata_file_empty = argv[2];
  util_install_signals();

  test_report_steps_param();
  test_report_steps_dynamic();
  test_result_format();
  test_set_template();
  test_set_template_invalid();
  test_set_invalid_format();
  test_format_check();
  test_gendata_fload(gendata_file);
  test_gendata_fload_empty_file(gendata_file_empty);

  exit(0);
}


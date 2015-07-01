/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ert_test_context.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/util/test_work_area.h>
#include <ert/util/rng.h>

#include <ert/enkf/ert_test_context.h>
#include <ert/enkf/enkf_main.h>


#define ERT_TEST_CONTEXT_TYPE_ID 99671055
struct ert_test_context_struct {
  UTIL_TYPE_ID_DECLARATION;
  enkf_main_type * enkf_main;
  test_work_area_type * work_area;
  rng_type * rng;
};


UTIL_IS_INSTANCE_FUNCTION( ert_test_context , ERT_TEST_CONTEXT_TYPE_ID )


static ert_test_context_type * ert_test_context_alloc__( const char * test_name , const char * model_config , bool python_mode) {
  ert_test_context_type * test_context = util_malloc( sizeof * test_context );
  UTIL_TYPE_ID_INIT( test_context , ERT_TEST_CONTEXT_TYPE_ID );

  /*
    This environment variable is set to ensure that test context will
    parse the correct files when loading site config.
  */
  if (python_mode)
    setenv("ERT_UI_MODE" , "gui" , 1);
  else
    setenv("ERT_UI_MODE" , "tui" , 1);


  if (util_file_exists(model_config)) {
    test_context->work_area = test_work_area_alloc(test_name);
    test_work_area_set_store( test_context->work_area , false );
    test_work_area_copy_parent_content(test_context->work_area , model_config );
    {
      char * config_file = util_split_alloc_filename( model_config );
      test_context->enkf_main = enkf_main_bootstrap(config_file , true , false );
      free( config_file );
    }
    test_context->rng = rng_alloc( MZRAN , INIT_DEV_URANDOM );
  } else {
    test_context->enkf_main = NULL;
    test_context->work_area = NULL;
    test_context->rng = NULL;
  }
  return test_context;
}

ert_test_context_type * ert_test_context_alloc( const char * test_name , const char * model_config) {
  return ert_test_context_alloc__( test_name , model_config , false );
}


ert_test_context_type * ert_test_context_alloc_python( const char * test_name , const char * model_config) {
  return ert_test_context_alloc__( test_name , model_config , true );
}



enkf_main_type * ert_test_context_get_main( ert_test_context_type * test_context ) {
  return test_context->enkf_main;
}


const char * ert_test_context_get_cwd( const ert_test_context_type * test_context ) {
  return test_work_area_get_cwd( test_context->work_area );
}



void ert_test_context_free( ert_test_context_type * test_context ) {

  if (test_context->enkf_main)
    enkf_main_free( test_context->enkf_main );

  if (test_context->work_area)
    test_work_area_free( test_context->work_area );

  if (test_context->rng)
    rng_free( test_context->rng );

  free( test_context );
}


bool ert_test_context_install_workflow_job( ert_test_context_type * test_context , const char * job_name , const char * job_file) {
  if (util_file_exists( job_file )) {
    enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
    ert_workflow_list_type * workflow_list = enkf_main_get_workflow_list( enkf_main );
    ert_workflow_list_add_job( workflow_list , job_name , job_file );
    return ert_workflow_list_has_job( workflow_list , job_name );
  } else
    return false;
}


bool ert_test_context_install_workflow( ert_test_context_type * test_context , const char * workflow_name , const char * workflow_file) {
  if (util_file_exists( workflow_file )) {
    enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
    ert_workflow_list_type * workflow_list = enkf_main_get_workflow_list( enkf_main );
    ert_workflow_list_add_workflow( workflow_list , workflow_file , workflow_name );
    return ert_workflow_list_has_workflow( workflow_list , workflow_name);
  } else
    return false;
}


void ert_test_context_fwrite_workflow_job( FILE * stream , const char * job_name , const stringlist_type * args) {
  fprintf(stream , "%s  " , job_name);
  stringlist_fprintf(args , " ", stream);
  fprintf(stream , "\n");
}


bool ert_test_context_run_worklow( ert_test_context_type * test_context , const char * workflow_name) {
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  ert_workflow_list_type * workflow_list = enkf_main_get_workflow_list( enkf_main );

  if (ert_workflow_list_has_workflow( workflow_list , workflow_name )){
    bool result =  ert_workflow_list_run_workflow_blocking( workflow_list , workflow_name , enkf_main );
    return result;
  }
  else{
    return false;
  }
}



bool ert_test_context_run_worklow_job( ert_test_context_type * test_context , const char * job_name, const stringlist_type * args) {
  enkf_main_type * enkf_main = ert_test_context_get_main( test_context );
  ert_workflow_list_type * workflow_list = enkf_main_get_workflow_list( enkf_main );

  if (ert_workflow_list_has_job( workflow_list , job_name )) {
    bool status;
    {
      char * workflow = util_alloc_sprintf("WORKFLOW-%06d" , rng_get_int( test_context->rng , 1000000));
      {
        FILE * stream = util_fopen( workflow , "w");
        ert_test_context_fwrite_workflow_job( stream , job_name , args );
        fclose(stream);
      }
      ert_test_context_install_workflow( test_context , workflow , workflow);
      status = ert_test_context_run_worklow( test_context , workflow );
      free(workflow);
    }
    return status;
  } else
    return false;
}


void ert_test_context_set_store( ert_test_context_type * test_context , bool store) {
  test_work_area_set_store( test_context->work_area , store );
}

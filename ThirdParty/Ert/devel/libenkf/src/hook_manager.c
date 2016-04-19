/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'hook_manager.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/subst_list.h>
#include <ert/util/vector.h>

#include <ert/config/config_parser.h>

#include <ert/job_queue/workflow.h>

#include <ert/enkf/config_keys.h>
#include <ert/enkf/hook_manager.h>
#include <ert/enkf/ert_workflow_list.h>
#include <ert/enkf/runpath_list.h>

#define HOOK_MANAGER_NAME             "HOOK MANAGER"
#define RUNPATH_LIST_FILE             ".ert_runpath_list"
#define QC_WORKFLOW_NAME              "QC WORKFLOW"
#define RUN_MODE_PRE_SIMULATION_NAME  "PRE_SIMULATION"
#define RUN_MODE_POST_SIMULATION_NAME "POST_SIMULATION"

struct hook_manager_struct {
  vector_type            * hook_workflow_list;  /* vector of hook_workflow_type instances */
  runpath_list_type      * runpath_list;
  ert_workflow_list_type * workflow_list;
  hash_type              * input_context;


  /* Deprecated stuff */
  hook_workflow_type     * post_hook_workflow; /* This is the good old QC workflow, kept for backward compatibility, obsolete */
};

hook_manager_type * hook_manager_alloc( ert_workflow_list_type * workflow_list ) {
  hook_manager_type * hook_manager = util_malloc( sizeof * hook_manager );
  hook_manager->hook_workflow_list = vector_alloc_new();

  hook_manager->runpath_list = runpath_list_alloc( NULL );
  hook_manager->workflow_list = workflow_list;
  hook_manager_set_runpath_list_file( hook_manager, NULL, RUNPATH_LIST_FILE );
  hook_manager->input_context = hash_alloc();

  return hook_manager;
}


void hook_manager_free( hook_manager_type * hook_manager ) {
  runpath_list_free( hook_manager->runpath_list );
  vector_free( hook_manager->hook_workflow_list );
  hash_free( hook_manager->input_context );
  free( hook_manager );
}



void hook_manager_add_input_context( hook_manager_type * hook_manager, const char * key , const char * value) {
  hash_insert_hash_owned_ref(hook_manager->input_context, key, util_alloc_string_copy(value), free);
}



runpath_list_type * hook_manager_get_runpath_list( hook_manager_type * hook_manager ) {
  return hook_manager->runpath_list;
}


static void hook_manager_add_workflow( hook_manager_type * hook_manager , const char * workflow_name , hook_run_mode_enum run_mode) {
  if (ert_workflow_list_has_workflow( hook_manager->workflow_list , workflow_name) ){
    workflow_type * workflow = ert_workflow_list_get_workflow( hook_manager->workflow_list , workflow_name);
    hook_workflow_type * hook = hook_workflow_alloc( workflow , run_mode );
    vector_append_owned_ref(hook_manager->hook_workflow_list, hook , hook_workflow_free__);
  }
  else {
    fprintf(stderr, "** Warning: While hooking workflow: %s not recognized among the list of loaded workflows.", workflow_name);
  }
}



void hook_manager_init( hook_manager_type * hook_manager , const config_content_type * config_content) {

  /* Old stuff explicitly prefixed with QC  */
  {
    if (config_content_has_item( config_content , QC_WORKFLOW_KEY)) {
      char * workflow_name;
      const char * file_name = config_content_get_value_as_path(config_content , QC_WORKFLOW_KEY);
      util_alloc_file_components( file_name , NULL , &workflow_name , NULL );
      workflow_type * workflow = ert_workflow_list_add_workflow( hook_manager->workflow_list , file_name , workflow_name);
      if (workflow != NULL) {
       hook_workflow_type * hook = hook_workflow_alloc( workflow , POST_SIMULATION );
       vector_append_owned_ref(hook_manager->hook_workflow_list, hook , hook_workflow_free__);
     }
    }
  }



  if (config_content_has_item( config_content , HOOK_WORKFLOW_KEY)) {
    for (int ihook = 0; ihook < config_content_get_occurences(config_content , HOOK_WORKFLOW_KEY); ihook++) {
      const char * workflow_name = config_content_iget( config_content , HOOK_WORKFLOW_KEY, ihook , 0 );
      hook_run_mode_enum run_mode = hook_workflow_run_mode_from_name(config_content_iget(config_content , HOOK_WORKFLOW_KEY , ihook , 1));
      hook_manager_add_workflow( hook_manager , workflow_name , run_mode );
    }
  }
}



void hook_manager_add_config_items( config_parser_type * config ) {
  config_schema_item_type * item;

  /* Old stuff - explicitly prefixed with QC. */
  {
    item = config_add_schema_item( config , QC_PATH_KEY , false  );
    config_schema_item_set_argc_minmax(item , 1 , 1 );
    config_install_message( config , QC_PATH_KEY , "The \'QC_PATH\' keyword is ignored.");


    item = config_add_schema_item( config , QC_WORKFLOW_KEY , false );
    config_schema_item_set_argc_minmax(item , 1 , 1 );
    config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );

    config_install_message( config , QC_WORKFLOW_KEY , "The \'QC_WORKFLOW\' keyword is deprecated - use \'HOOK_WORKFLOW\' instead");
  }

  item = config_add_schema_item( config , HOOK_WORKFLOW_KEY , false );
  config_schema_item_set_argc_minmax(item , 2 , 2 );
  config_schema_item_iset_type( item , 0 , CONFIG_STRING );
  config_schema_item_iset_type( item , 1 , CONFIG_STRING );
  {
    char ** argv = util_malloc( 2 * sizeof * argv );

    argv[0] = RUN_MODE_PRE_SIMULATION_NAME;
    argv[1] = RUN_MODE_POST_SIMULATION_NAME;
    config_schema_item_set_indexed_selection_set(item, 1, 2, (const char **) argv);

    free( argv );
  }

  item = config_add_schema_item( config , RUNPATH_FILE_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
}


void hook_manager_export_runpath_list( const hook_manager_type * hook_manager ) {
  runpath_list_fprintf( hook_manager->runpath_list );
}

const char * hook_manager_get_runpath_list_file( const hook_manager_type * hook_manager) {
  return runpath_list_get_export_file( hook_manager->runpath_list );
}

static void hook_manager_set_runpath_list_file__( hook_manager_type * hook_manager , const char * runpath_list_file) {
  runpath_list_set_export_file( hook_manager->runpath_list , runpath_list_file );
}

void hook_manager_set_runpath_list_file( hook_manager_type * hook_manager , const char * basepath, const char * filename) {

  if (filename && util_is_abs_path( filename ))
    hook_manager_set_runpath_list_file__( hook_manager , filename );
  else {
    const char * file = RUNPATH_LIST_FILE;

    if (filename != NULL)
      file = filename;

    char * file_with_path_prefix = NULL;
    if (basepath != NULL) {
      file_with_path_prefix = util_alloc_filename(basepath, file, NULL);
    }
    else
      file_with_path_prefix = util_alloc_string_copy(file);

    {
      char * absolute_path = util_alloc_abs_path(file_with_path_prefix);
      hook_manager_set_runpath_list_file__( hook_manager , absolute_path );
      free( absolute_path );
    }

    free(file_with_path_prefix);
  }
}


void hook_manager_run_workflows( const hook_manager_type * hook_manager , hook_run_mode_enum run_mode , void * self )
{
  bool verbose = false;
  for (int i=0; i < vector_get_size( hook_manager->hook_workflow_list ); i++) {
    hook_workflow_type * hook_workflow = vector_iget( hook_manager->hook_workflow_list , i );
    if (hook_workflow_get_run_mode(hook_workflow) == run_mode) {
      workflow_type * workflow = hook_workflow_get_workflow( hook_workflow );
      workflow_run( workflow, self , verbose , ert_workflow_list_get_context( hook_manager->workflow_list ));
      /*
        The workflow_run function will return a bool to indicate
        success/failure, and in the case of error the function
        workflow_get_last_error() can be used to get a config_error
        object.
      */
    }
  }
}

const hook_workflow_type * hook_manager_iget_hook_workflow(const hook_manager_type * hook_manager, int index){
 return vector_iget(hook_manager->hook_workflow_list, index);
}

int hook_manager_get_size(const hook_manager_type * hook_manager){
 return vector_get_size(hook_manager->hook_workflow_list);
}


/*****************************************************************/
/* Deprecated stuff                                              */
/*****************************************************************/



bool hook_manager_run_post_hook_workflow( const hook_manager_type * hook_manager , void * self) {
  const char * export_file = runpath_list_get_export_file( hook_manager->runpath_list );
  if (!util_file_exists( export_file ))
      fprintf(stderr,"** Warning: the file:%s with a list of runpath directories was not found - workflow will probably fail.\n" , export_file);

  return hook_workflow_run_workflow(hook_manager->post_hook_workflow, hook_manager->workflow_list, self);
}




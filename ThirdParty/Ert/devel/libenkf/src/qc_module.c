/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'qc_module.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/config/config.h>

#include <ert/job_queue/workflow.h>

#include <ert/enkf/config_keys.h>
#include <ert/enkf/qc_module.h>
#include <ert/enkf/ert_workflow_list.h>
#include <ert/enkf/runpath_list.h>

#define QC_WORKFLOW_NAME    "QC"
#define RUNPATH_LIST_FILE   ".ert_runpath_list"


struct qc_module_struct {
  char                   * qc_path;
  workflow_type          * qc_workflow;
  ert_workflow_list_type * workflow_list;
  runpath_list_type      * runpath_list; 
  char                   * runpath_list_file; 
};




qc_module_type * qc_module_alloc( ert_workflow_list_type * workflow_list , const char * qc_path ) {
  qc_module_type * qc_module = util_malloc( sizeof * qc_module );
  qc_module->qc_workflow = NULL;
  qc_module->qc_path = NULL;
  qc_module->workflow_list = workflow_list;
  qc_module->runpath_list = runpath_list_alloc();
  qc_module->runpath_list_file = NULL;
  qc_module_set_path( qc_module , qc_path );
  {
    char * cwd = util_alloc_cwd();
    char * runpath_list_file = util_alloc_filename( cwd , RUNPATH_LIST_FILE , NULL);

    qc_module_set_runpath_list_file( qc_module , runpath_list_file );

    free( runpath_list_file );
    free( cwd );
  }
  return qc_module;
}


void qc_module_free( qc_module_type * qc_module ) {
  util_safe_free( qc_module->qc_path );
  runpath_list_free( qc_module->runpath_list );
  free( qc_module );
}

runpath_list_type * qc_module_get_runpath_list( qc_module_type * qc_module ) {
  return qc_module->runpath_list;
}


void qc_module_export_runpath_list( const qc_module_type * qc_module ) {
  FILE * stream = util_fopen( qc_module->runpath_list_file , "w");
  runpath_list_fprintf( qc_module->runpath_list , stream );
  fclose( stream );
}

void qc_module_set_runpath_list_file( qc_module_type * qc_module , const char * filename) {
  qc_module->runpath_list_file = util_realloc_string_copy( qc_module->runpath_list_file , filename );
}

const char * qc_module_get_runpath_list_file( qc_module_type * qc_module) {
  return qc_module->runpath_list_file;
}


void qc_module_set_path( qc_module_type * qc_module , const char * qc_path) {
  qc_module->qc_path = util_realloc_string_copy( qc_module->qc_path , qc_path );
}


const char * qc_module_get_path( const qc_module_type * qc_module ) {
  return qc_module->qc_path;
}


void qc_module_set_workflow( qc_module_type * qc_module , const char * qc_workflow ) {
  char * workflow_name;
  util_alloc_file_components( qc_workflow , NULL , &workflow_name , NULL );
  {
    workflow_type * workflow = ert_workflow_list_add_workflow( qc_module->workflow_list , qc_workflow , workflow_name);
    if (workflow != NULL) {
      ert_workflow_list_add_alias( qc_module->workflow_list , workflow_name , QC_WORKFLOW_NAME );
      qc_module->qc_workflow = workflow;
    }
  }
  free( workflow_name );
}


bool qc_module_run_workflow( qc_module_type * qc_module , void * self) {
  bool verbose = false;
  if (qc_module->qc_workflow != NULL ) {
    if (!util_file_exists( qc_module->runpath_list_file ))
      fprintf(stderr,"** Warning: the file:%s with a list of runpath directories was not found - QC workflow wil probably fail.\n" , qc_module->runpath_list_file);
    
    return ert_workflow_list_run_workflow__( qc_module->workflow_list , qc_module->qc_workflow , verbose , self);
  } else
    return false;
}


bool qc_module_has_workflow( const qc_module_type * qc_module ) {
  if (qc_module->qc_workflow == NULL)
    return false;
  else
    return true;
}



/*****************************************************************/


void qc_module_init( qc_module_type * qc_module , const config_type * config) {
  if (config_item_set( config , QC_PATH_KEY )) 
    qc_module_set_path( qc_module , config_get_value( config , QC_PATH_KEY ));
  
  if (config_item_set( config , QC_WORKFLOW_KEY)) {
    const char * qc_workflow = config_get_value_as_path(config , QC_WORKFLOW_KEY);
    qc_module_set_workflow( qc_module , qc_workflow );
  }
}



void qc_module_add_config_items( config_type * config ) {
  config_schema_item_type * item;
  
  item = config_add_schema_item( config , QC_PATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1 );

  item = config_add_schema_item( config , QC_WORKFLOW_KEY , false );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_EXISTING_PATH );
}




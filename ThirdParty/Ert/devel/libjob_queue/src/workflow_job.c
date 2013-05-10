/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'workflow_job.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <ert/util/int_vector.h>
#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/config/config.h>

#include <ert/job_queue/workflow_job.h>


/* The default values are interepreted as no limit. */
#define DEFAULT_INTERNAL false


#define MIN_ARG_KEY    "MIN_ARG"
#define MAX_ARG_KEY    "MAX_ARG"
#define ARG_TYPE_KEY   "ARG_TYPE"
#define INTERNAL_KEY   "INTERNAL"
#define MODULE_KEY     "MODULE" 
#define FUNCTION_KEY   "FUNCTION"
#define EXECUTABLE_KEY "EXECUTABLE"

#define NULL_STRING         "NULL"
#define WORKFLOW_JOB_STRING_TYPE "STRING"
#define WORKFLOW_JOB_INT_TYPE    "INT"
#define WORKFLOW_JOB_FLOAT_TYPE  "FLOAT"

#define WORKFLOW_JOB_TYPE_ID 614441


struct workflow_job_struct {
  UTIL_TYPE_ID_DECLARATION;
  bool                 internal;
  int                  min_arg;
  int                  max_arg;   
  int_vector_type    * arg_types;     // Should contain values from the config_item_types enum in config.h.
  char               * executable;
  char               * module;
  char               * function;
  char               * name;
  void               * lib_handle;
  workflow_job_ftype * dl_func;
  bool                 valid;
};


bool workflow_job_internal( const workflow_job_type * workflow_job ) {
  return workflow_job->internal;
}

const char * workflow_job_get_name( const workflow_job_type * workflow_job ) {
  return workflow_job->name;
}


config_type * workflow_job_alloc_config() {
  config_type * config = config_alloc();
  {
    config_schema_item_type * item;
  
    item = config_add_schema_item( config , MIN_ARG_KEY , false );
    config_schema_item_set_argc_minmax( item , 1 , 1 );
    config_schema_item_iset_type( item , 0 , CONFIG_INT );

    item = config_add_schema_item( config , MAX_ARG_KEY , false );
    config_schema_item_set_argc_minmax( item , 1 , 1 );
    config_schema_item_iset_type( item , 0 , CONFIG_INT );

    item = config_add_schema_item( config , ARG_TYPE_KEY , false );
    config_schema_item_set_argc_minmax( item , 2 , 2 );
    config_schema_item_iset_type( item , 0 , CONFIG_INT );
    config_schema_item_set_indexed_selection_set( item , 1 , 3 , (const char *[3]) {WORKFLOW_JOB_STRING_TYPE , WORKFLOW_JOB_INT_TYPE , WORKFLOW_JOB_FLOAT_TYPE});

    /*****************************************************************/
    item = config_add_schema_item( config , EXECUTABLE_KEY , false );
    config_schema_item_set_argc_minmax( item , 1 , 1 );
    config_schema_item_iset_type( item , 0 , CONFIG_PATH );

    /*---------------------------------------------------------------*/
    
    item = config_add_schema_item( config , FUNCTION_KEY , false );
    config_schema_item_set_argc_minmax( item , 1 , 1);

    item = config_add_schema_item( config , MODULE_KEY , false );
    config_schema_item_set_argc_minmax( item , 1 , 1);
    /*****************************************************************/

    item = config_add_schema_item( config , INTERNAL_KEY , false );
    config_schema_item_set_argc_minmax( item , 1 , 1);
    config_schema_item_iset_type( item , 0 , CONFIG_BOOL);
  }
  return config;
}



static UTIL_SAFE_CAST_FUNCTION(workflow_job , WORKFLOW_JOB_TYPE_ID );

void workflow_job_update_config_compiler( const workflow_job_type * workflow_job , config_type * config_compiler ) {
  config_schema_item_type * item = config_add_schema_item( config_compiler , workflow_job->name , false );
  /* 
     Ensure that the arg_types mapping is at least as large as the
     max_arg value. The arg_type vector will be left padded with
     CONFIG_STRING values.
  */
  {
    int iarg;
    config_schema_item_set_argc_minmax( item , workflow_job->min_arg , workflow_job->max_arg );
    for (iarg = 0; iarg < int_vector_size( workflow_job->arg_types ); iarg++)
      config_schema_item_iset_type( item , iarg , int_vector_iget( workflow_job->arg_types , iarg ));
  }
}


workflow_job_type * workflow_job_alloc( const char * name , bool internal ) {
  workflow_job_type * workflow_job = util_malloc( sizeof * workflow_job );
  UTIL_TYPE_ID_INIT( workflow_job , WORKFLOW_JOB_TYPE_ID );
  workflow_job->internal   = internal;      // this can not be changed run-time.
  workflow_job->min_arg    = CONFIG_DEFAULT_ARG_MIN;
  workflow_job->max_arg    = CONFIG_DEFAULT_ARG_MAX;
  workflow_job->arg_types  = int_vector_alloc( 0 , CONFIG_STRING );

  workflow_job->executable = NULL;
  workflow_job->module     = NULL;
  workflow_job->function   = NULL;

  if (name == NULL)
    util_abort("%s: trying to create workflow_job with name == NULL - illegal\n",__func__);
  else
    workflow_job->name       = util_alloc_string_copy( name );

  workflow_job->valid      = false;

  return workflow_job;
}


void workflow_job_set_executable( workflow_job_type * workflow_job , const char * executable ) {
  workflow_job->executable = util_realloc_string_copy( workflow_job->executable , executable );
}


void workflow_job_set_module( workflow_job_type * workflow_job , const char * module) {
  if (strcmp(module  ,NULL_STRING) == 0)
    module = NULL;

  workflow_job->module = util_realloc_string_copy( workflow_job->module , module );
}


void workflow_job_set_function( workflow_job_type * workflow_job , const char * function) {
  workflow_job->function = util_realloc_string_copy( workflow_job->function , function );
}


void workflow_job_iset_argtype( workflow_job_type * workflow_job , int iarg , config_item_types type) {
  if (type == CONFIG_STRING || type == CONFIG_INT || type == CONFIG_FLOAT)
    int_vector_iset( workflow_job->arg_types , iarg , type );
}

void workflow_job_set_min_arg( workflow_job_type * workflow_job , int min_arg) {
  workflow_job->min_arg = min_arg;
}

void workflow_job_set_max_arg( workflow_job_type * workflow_job , int max_arg) {
  workflow_job->max_arg = max_arg;
}

int workflow_job_get_min_arg( const workflow_job_type * workflow_job ) {
  return workflow_job->min_arg;
}
 
int workflow_job_get_max_arg( workflow_job_type * workflow_job ) {
  return workflow_job->max_arg;
}



static void workflow_job_iset_argtype_string( workflow_job_type * workflow_job , int iarg , const char * arg_type) {
  config_item_types type = CONFIG_INVALID;

  if (strcmp( arg_type , WORKFLOW_JOB_STRING_TYPE) == 0)
    type = CONFIG_STRING;
  else if (strcmp( arg_type , WORKFLOW_JOB_INT_TYPE) == 0)
    type = CONFIG_INT;
  else if (strcmp( arg_type , WORKFLOW_JOB_FLOAT_TYPE) == 0)
    type = CONFIG_FLOAT;

  if (type != CONFIG_INVALID)
    workflow_job_iset_argtype( workflow_job , iarg , type );
  
}


static void workflow_job_validate_internal( workflow_job_type * workflow_job ) {
  if ((workflow_job->executable == NULL) && (workflow_job->function != NULL)) {
    workflow_job->lib_handle = dlopen( workflow_job->module , RTLD_NOW );
    if (workflow_job->lib_handle != NULL) {
      workflow_job->dl_func = (workflow_job_ftype *) dlsym( workflow_job->lib_handle , workflow_job->function );
      if (workflow_job->dl_func != NULL)
        workflow_job->valid = true;
      else 
        fprintf(stderr,"Failed to load symbol:%s Error:%s \n",workflow_job->function , dlerror());
    } else {
      if (workflow_job->module != NULL)
        fprintf(stderr,"Failed to load module:%s Error:%s \n",workflow_job->module , dlerror());
    }
  } else
    fprintf(stderr,"Must have executable == NULL and function != NULL for internal jobs \n");
}


static void workflow_job_validate_external( workflow_job_type * workflow_job ) {
  if (workflow_job->executable != NULL) {
    if (util_is_executable( workflow_job->executable ) && 
        (workflow_job->module == workflow_job->function) && 
        (workflow_job->module == NULL))
      workflow_job->valid = true;
  }
}



static void workflow_job_validate( workflow_job_type * workflow_job ) {
  if (workflow_job->internal) 
    workflow_job_validate_internal( workflow_job );
  else 
    workflow_job_validate_external( workflow_job );
}




workflow_job_type * workflow_job_config_alloc( const char * name , config_type * config , const char * config_file) {
  config_clear( config );
  if (config_parse( config , config_file , "--", NULL , NULL , CONFIG_UNRECOGNIZED_WARN , true)) {
    bool internal = DEFAULT_INTERNAL;
    if (config_item_set( config , INTERNAL_KEY))
      internal = config_iget_as_bool( config , INTERNAL_KEY , 0 , 0 );
    
    {
      workflow_job_type * workflow_job = workflow_job_alloc( name , internal );
      
      if (config_item_set( config , MIN_ARG_KEY))
        workflow_job_set_min_arg( workflow_job , config_iget_as_int( config , MIN_ARG_KEY , 0 , 0 ));
      
      if (config_item_set( config , MAX_ARG_KEY))
        workflow_job_set_max_arg( workflow_job , config_iget_as_int( config , MAX_ARG_KEY , 0 , 0 ));
      
      {
        int i;
        for (i=0; i < config_get_occurences( config , ARG_TYPE_KEY); i++) {
          int iarg = config_iget_as_int( config , ARG_TYPE_KEY , i , 0 );
          const char * arg_type = config_iget( config , ARG_TYPE_KEY , i , 1 );
          
          workflow_job_iset_argtype_string( workflow_job , iarg , arg_type );
        }
      }
      
      if (config_item_set( config , MODULE_KEY))
        workflow_job_set_module( workflow_job , config_get_value( config , MODULE_KEY));  // Could be a pure so name; or a full path ..... Like executable
      
      if (config_item_set( config , FUNCTION_KEY))
        workflow_job_set_function( workflow_job , config_get_value( config , FUNCTION_KEY));
      
      if (config_item_set( config , EXECUTABLE_KEY)) 
        workflow_job_set_executable( workflow_job , config_get_value_as_abspath( config , EXECUTABLE_KEY));
      
      workflow_job_validate( workflow_job );
      
      if (!workflow_job->valid) {
        workflow_job_free( workflow_job );
        workflow_job = NULL;
      }
      
      return workflow_job;
    }
  } else 
    return NULL;
}




void workflow_job_free( workflow_job_type * workflow_job ) {
  util_safe_free( workflow_job->module );
  util_safe_free( workflow_job->function );
  util_safe_free( workflow_job->executable );
  int_vector_free( workflow_job->arg_types );
  free( workflow_job->name );
  free( workflow_job );
}


void workflow_job_free__( void * arg) {
  workflow_job_type * workflow_job = workflow_job_safe_cast( arg );
  workflow_job_free( workflow_job );
}

/*
  The workflow job can return an arbitrary (void *) pointer. It is the
  calling scopes responsability to interpret this object correctly. If
  the the workflow job allocates storage the calling scope must
  discard it.  
*/

static void * workflow_job_run_internal( const workflow_job_type * job , void * self , bool verbose , const stringlist_type * arg) {
  return job->dl_func( self , arg );
}


static void * workflow_job_run_external( const workflow_job_type * job  , bool verbose , const stringlist_type * arg) {
  char ** argv = stringlist_alloc_char_copy( arg );

  util_fork_exec( job->executable , 
                  stringlist_get_size( arg ),
                  (const char **) argv , 
                  true ,
                  NULL , 
                  NULL , 
                  NULL , 
                  NULL , 
                  NULL );
  
  
  if (argv != NULL) {
    int i;
    for (i=0; i < stringlist_get_size( arg ); i++)
      free( argv[i] );
    free( argv );
  }
  return NULL;
}


void * workflow_job_run( const workflow_job_type * job , void * self , bool verbose , const stringlist_type * arg) {
  if (job->internal)
    return workflow_job_run_internal( job , self , verbose , arg );
  else
    return workflow_job_run_external( job , verbose , arg );
}

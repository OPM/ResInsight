/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ext_cmd.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/job_queue/ext_cmd.h>


/* The default values are interepreted as no limit. */
#define ARG_MIN_DEFAULT     -1
#define ARG_MAX_DEFAULT     -1
#define DEFAULT_INTERNAL false


#define MIN_ARG_KEY    "MIN_ARG"
#define MAX_ARG_KEY    "MAX_ARG"
#define ARG_TYPE_KEY   "ARG_TYPE"
#define INTERNAL_KEY   "INTERNAL"
#define MODULE_KEY     "MODULE" 
#define FUNCTION_KEY   "FUNCTION"
#define EXECUTABLE_KEY "EXECUTABLE"

#define NULL_STRING         "NULL"
#define EXT_CMD_STRING_TYPE "STRING"
#define EXT_CMD_INT_TYPE    "INT"
#define EXT_CMD_FLOAT_TYPE  "FLOAT"

#define EXT_CMD_TYPE_ID 614441

struct ext_cmd_struct {
  UTIL_TYPE_ID_DECLARATION;
  bool              internal;
  int               min_arg;
  int               max_arg;   
  int_vector_type * arg_types;     // Should contain values from the config_item_types enum in config.h.
  char            * executable;
  char            * module;
  char            * function;
  void            * lib_handle;
  void            * dl_func;
  bool              valid;
};


bool ext_cmd_internal( const ext_cmd_type * ext_cmd ) {
  return ext_cmd->internal;
}


config_type * ext_cmd_alloc_config() {
  config_type * config = config_alloc();
  {
    config_schema_item_type * item;
  
    item = config_add_schema_item( config , MIN_ARG_KEY , false , false);
    config_schema_item_set_argc_minmax( item , 1 , 1 , 1 , (const config_item_types[1]) {CONFIG_INT});

    item = config_add_schema_item( config , MAX_ARG_KEY , false , false);
    config_schema_item_set_argc_minmax( item , 1 , 1 , 1 , (const config_item_types[1]) {CONFIG_INT});

    item = config_add_schema_item( config , ARG_TYPE_KEY , false , true );
    config_schema_item_set_argc_minmax( item , 2 , 2 , 2 , (const config_item_types[2]) {CONFIG_INT , CONFIG_STRING});
    config_schema_item_set_indexed_selection_set( item , 1 , 3 , (const char *[3]) {EXT_CMD_STRING_TYPE , EXT_CMD_INT_TYPE , EXT_CMD_FLOAT_TYPE});

    /*****************************************************************/
    item = config_add_schema_item( config , EXECUTABLE_KEY , false , false );
    config_schema_item_set_argc_minmax( item , 1 , 1 , 1 , (const config_item_types[1]) {CONFIG_EXECUTABLE});

    /*---------------------------------------------------------------*/
    
    item = config_add_schema_item( config , FUNCTION_KEY , false , false );
    config_schema_item_set_argc_minmax( item , 1 , 1 , 1 , (const config_item_types[1]) {CONFIG_STRING});

    item = config_add_schema_item( config , MODULE_KEY , false , false );
    config_schema_item_set_argc_minmax( item , 1 , 1 , 1 , (const config_item_types[1]) {CONFIG_STRING});
    /*****************************************************************/

    item = config_add_schema_item( config , INTERNAL_KEY , false , false);
    config_schema_item_set_argc_minmax( item , 1 , 1 , 1 , (const config_item_types[1]) {CONFIG_BOOLEAN});    
    
  }
  return config;
}



static UTIL_SAFE_CAST_FUNCTION(ext_cmd , EXT_CMD_TYPE_ID )

ext_cmd_type * ext_cmd_alloc( bool internal ) {
  ext_cmd_type * ext_cmd = util_malloc( sizeof * ext_cmd );
  UTIL_TYPE_ID_INIT( ext_cmd , EXT_CMD_TYPE_ID );
  ext_cmd->internal   = internal;      // This can NOT be changed run-time.
  ext_cmd->min_arg    = ARG_MIN_DEFAULT;
  ext_cmd->max_arg    = ARG_MAX_DEFAULT;
  ext_cmd->arg_types  = int_vector_alloc( 0 , CONFIG_STRING );

  ext_cmd->executable = NULL;
  ext_cmd->module     = NULL;
  ext_cmd->function   = NULL;
  ext_cmd->valid      = false;
  
  return ext_cmd;
}


void ext_cmd_set_executable( ext_cmd_type * ext_cmd , const char * executable ) {
  ext_cmd->executable = util_realloc_string_copy( ext_cmd->executable , executable );
}


void ext_cmd_set_module( ext_cmd_type * ext_cmd , const char * module) {
  if (strcmp(module  ,NULL_STRING) == 0)
    module = NULL;

  ext_cmd->module = util_realloc_string_copy( ext_cmd->module , module );
}


void ext_cmd_set_function( ext_cmd_type * ext_cmd , const char * function) {
  ext_cmd->function = util_realloc_string_copy( ext_cmd->function , function );
}


void ext_cmd_iset_argtype( ext_cmd_type * ext_cmd , int iarg , config_item_types type) {
  if (type == CONFIG_STRING || type == CONFIG_INT || type == CONFIG_FLOAT)
    int_vector_iset( ext_cmd->arg_types , iarg , type );
}

void ext_cmd_set_min_arg( ext_cmd_type * ext_cmd , int min_arg) {
  ext_cmd->min_arg = min_arg;
}

void ext_cmd_set_max_arg( ext_cmd_type * ext_cmd , int max_arg) {
  ext_cmd->max_arg = max_arg;
}


static void ext_cmd_iset_argtype_string( ext_cmd_type * ext_cmd , int iarg , const char * arg_type) {
  config_item_types type = CONFIG_INVALID;

  if (strcmp( arg_type , EXT_CMD_STRING_TYPE) == 0)
    type = CONFIG_STRING;
  else if (strcmp( arg_type , EXT_CMD_INT_TYPE) == 0)
    type = CONFIG_INT;
  else if (strcmp( arg_type , EXT_CMD_FLOAT_TYPE) == 0)
    type = CONFIG_FLOAT;

  if (type != CONFIG_INVALID)
    ext_cmd_iset_argtype( ext_cmd , iarg , type );
  
}


static void ext_cmd_validate( ext_cmd_type * ext_cmd ) {
  if (!ext_cmd->internal) {
    if (ext_cmd->executable != NULL) {
      if (util_is_executable( ext_cmd->executable ) && 
          (ext_cmd->module == ext_cmd->function) && 
          (ext_cmd == NULL))
        ext_cmd->valid = true;
    }
  } else {
    if ((ext_cmd->executable == NULL) && (ext_cmd->function != NULL)) {
      ext_cmd->lib_handle = dlopen( ext_cmd->module , RTLD_NOW );
      if (ext_cmd->lib_handle != NULL) {
        ext_cmd->dl_func = dlsym( ext_cmd->lib_handle , ext_cmd->function );
        if (ext_cmd->dl_func != NULL)
          ext_cmd->valid = true;
        else 
          fprintf(stderr,"Failed to load symbol:%s Error:%s \n",ext_cmd->function , dlerror());
      } else {
        if (ext_cmd->module != NULL)
          fprintf(stderr,"Failed to load module:%s Error:%s \n",ext_cmd->module , dlerror());
      }
    }
  }
}



ext_cmd_type * ext_cmd_config_alloc( config_type * config , const char * config_file) {
  config_clear( config );
  config_parse( config , config_file , "--", NULL , NULL , true , true);
  {
    
    bool internal = DEFAULT_INTERNAL;
    if (config_item_set( config , INTERNAL_KEY))
      internal = config_iget_as_bool( config , INTERNAL_KEY , 0 , 0 );
    
    {
      ext_cmd_type * ext_cmd = ext_cmd_alloc( internal );
      
      if (config_item_set( config , MIN_ARG_KEY))
        ext_cmd_set_min_arg( ext_cmd , config_iget_as_int( config , MIN_ARG_KEY , 0 , 0 ));
      
      if (config_item_set( config , MAX_ARG_KEY))
        ext_cmd_set_max_arg( ext_cmd , config_iget_as_int( config , MAX_ARG_KEY , 0 , 0 ));
      
      {
        int i;
        for (i=0; i < config_get_occurences( config , ARG_TYPE_KEY); i++) {
          int iarg = config_iget_as_int( config , ARG_TYPE_KEY , i , 0 );
          const char * arg_type = config_iget( config , ARG_TYPE_KEY , i , 1 );
          
          ext_cmd_iset_argtype_string( ext_cmd , iarg , arg_type );
        }
      }
      
      if (config_item_set( config , MODULE_KEY))
        ext_cmd_set_module( ext_cmd , config_iget( config , MODULE_KEY , 0 , 0 ));
      
      if (config_item_set( config , FUNCTION_KEY))
        ext_cmd_set_function( ext_cmd , config_iget( config , FUNCTION_KEY , 0 , 0 ));
      
      if (config_item_set( config , EXECUTABLE_KEY))
        ext_cmd_set_executable( ext_cmd , config_iget( config , EXECUTABLE_KEY , 0 , 0 ));
      
      ext_cmd_validate( ext_cmd );
      
      if (!ext_cmd->valid) {
        ext_cmd_free( ext_cmd );
        ext_cmd = NULL;
      }
      
      return ext_cmd;
    }
  }
}



void ext_cmd_free( ext_cmd_type * ext_cmd ) {
  util_safe_free( ext_cmd->module );
  util_safe_free( ext_cmd->function );
  util_safe_free( ext_cmd->executable );
  int_vector_free( ext_cmd->arg_types );
  free( ext_cmd );
}


void ext_cmd_free__( void * arg) {
  ext_cmd_type * ext_cmd = ext_cmd_safe_cast( arg );
  ext_cmd_free( ext_cmd );
}

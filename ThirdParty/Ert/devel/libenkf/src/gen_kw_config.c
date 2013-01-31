/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_kw_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/hash.h>
#include <ert/util/vector.h>

#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/gen_kw_common.h>
#include <ert/enkf/gen_kw_config.h>
#include <ert/enkf/trans_func.h>
#include <ert/enkf/config_keys.h>

#define GEN_KW_CONFIG_TYPE_ID     550761
#define GEN_KW_PARAMETER_TYPE_ID  886201


typedef struct {
  UTIL_TYPE_ID_DECLARATION;
  char             * name;
  char             * tagged_name;
  trans_func_type  * trans_func;  
} gen_kw_parameter_type;



struct gen_kw_config_struct {
  UTIL_TYPE_ID_DECLARATION;
  char                 * key;
  vector_type          * parameters;       /* Vector of gen_kw_parameter_type instances. */
  char                 * template_file;
  char                 * parameter_file;
  const char           * tag_fmt;          /* Pointer to the tag_format owned by the ensemble config object. */ 
};


/*****************************************************************/

UTIL_SAFE_CAST_FUNCTION( gen_kw_parameter , GEN_KW_PARAMETER_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION_CONST( gen_kw_parameter , GEN_KW_PARAMETER_TYPE_ID )


UTIL_SAFE_CAST_FUNCTION( gen_kw_config , GEN_KW_CONFIG_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION_CONST( gen_kw_config , GEN_KW_CONFIG_TYPE_ID )

static void gen_kw_parameter_update_tagged_name( gen_kw_parameter_type * parameter , const char * tag_fmt) {
  if (tag_fmt != NULL)
    parameter->tagged_name = util_realloc_sprintf( parameter->tagged_name , tag_fmt , parameter->name );
}


static gen_kw_parameter_type * gen_kw_parameter_alloc( const char * parameter_name , const char * tag_fmt ) {
  gen_kw_parameter_type * parameter = util_malloc( sizeof * parameter );
  UTIL_TYPE_ID_INIT( parameter , GEN_KW_PARAMETER_TYPE_ID); 
  parameter->name        = util_alloc_string_copy( parameter_name );
  parameter->tagged_name = NULL;
  parameter->trans_func  = NULL;
  gen_kw_parameter_update_tagged_name( parameter , tag_fmt );
  return parameter;
}


static void gen_kw_parameter_free( gen_kw_parameter_type * parameter ) {
  util_safe_free( parameter->name );
  util_safe_free( parameter->tagged_name );
  if (parameter->trans_func != NULL)
    trans_func_free( parameter->trans_func );
  free( parameter );
}


static void gen_kw_parameter_free__( void * __parameter ) {
  gen_kw_parameter_type * parameter = gen_kw_parameter_safe_cast( __parameter );
  gen_kw_parameter_free( parameter );
}


static void gen_kw_parameter_set_trans_func( gen_kw_parameter_type * parameter , trans_func_type * trans_func ) {
  if (parameter->trans_func != NULL)
    trans_func_free( parameter->trans_func );
  parameter->trans_func = trans_func;
}



/*****************************************************************/


const char * gen_kw_config_get_template_file(const gen_kw_config_type * config) {
  return config->template_file;
}


/*
  The input template file must point to an existing file. 
*/
void gen_kw_config_set_template_file( gen_kw_config_type * config , const char * template_file ) {
  if (template_file != NULL) {
    if (!util_file_exists(template_file))
      util_abort("%s: the template_file:%s does not exist - aborting.\n",__func__ , template_file);
  }

  config->template_file = util_realloc_string_copy( config->template_file , template_file );
}



void gen_kw_config_set_parameter_file( gen_kw_config_type * config , const char * parameter_file ) {
  config->parameter_file = util_realloc_string_copy( config->parameter_file , parameter_file );
  vector_clear( config->parameters );
  if (parameter_file != NULL) {
    FILE * stream = util_fopen(parameter_file , "r");
    
    while (true) {
      char parameter_name[256];
      int  fscanf_return;
      
      fscanf_return = fscanf(stream , "%s" , parameter_name);
      if (fscanf_return == 1) {
        gen_kw_parameter_type * parameter  = gen_kw_parameter_alloc( parameter_name , config->tag_fmt);
        trans_func_type       * trans_func = trans_func_fscanf_alloc( stream );
        gen_kw_parameter_set_trans_func( parameter , trans_func );

        vector_append_owned_ref( config->parameters , parameter , gen_kw_parameter_free__ );
      } else 
        break; /* OK - we are ate EOF. */
    } 
    
    fclose( stream );
  }
}




const char * gen_kw_config_get_parameter_file( const gen_kw_config_type * config ) {
  return config->parameter_file;
}


/**
   Unfortunately the GUI makes it necessary(??) to be able to create
   halfways initialized gen_kw_config objects; and we then have to be
   able to query the gen_kw_config object if it is valid. Observe that
   some of the required config information will be owned by the
   enkf_config_node itself, this function should therefor NOT be
   called directly, only through the enkf_config_node_is_valid()
   function.

   Requirements:
   -------------
    * template_file  != NULL
    * parameter_file != NULL  (this means that the special schedule_prediction_file keyword will be invalid).
    
*/

bool gen_kw_config_is_valid( const gen_kw_config_type * config ) {
  if (config->template_file != NULL && config->parameter_file != NULL)
    return true;
  else
    return false;
}


/**
   A call to gen_kw_config_update_tag_format() must be called
   afterwards, otherwise all tagged strings will just be NULL.
*/
gen_kw_config_type * gen_kw_config_alloc_empty( const char * key , const char * tag_fmt ) {
  gen_kw_config_type *gen_kw_config = util_malloc(sizeof *gen_kw_config);
  UTIL_TYPE_ID_INIT(gen_kw_config , GEN_KW_CONFIG_TYPE_ID);

  gen_kw_config->key                = NULL; 
  gen_kw_config->template_file      = NULL;
  gen_kw_config->parameter_file     = NULL;
  gen_kw_config->parameters         = vector_alloc_new();
  gen_kw_config->tag_fmt            = tag_fmt;
  gen_kw_config->key                = util_alloc_string_copy( key );  

  return gen_kw_config;
}



void gen_kw_config_update( gen_kw_config_type * config , const char * template_file , const char * parameter_file ) {
  gen_kw_config_set_template_file( config , template_file);
  gen_kw_config_set_parameter_file( config , parameter_file );
}



double gen_kw_config_transform(const gen_kw_config_type * config , int index, double x) {
  const gen_kw_parameter_type * parameter = vector_iget_const( config->parameters , index );
  return trans_func_eval( parameter->trans_func , x);
}



void gen_kw_config_free(gen_kw_config_type * gen_kw_config) {
  util_safe_free( gen_kw_config->key );
  util_safe_free( gen_kw_config->template_file );
  util_safe_free( gen_kw_config->parameter_file );

  vector_free( gen_kw_config->parameters );
  free(gen_kw_config);
}



int gen_kw_config_get_data_size(const gen_kw_config_type * gen_kw_config) {
  return vector_get_size(gen_kw_config->parameters);
}



const char * gen_kw_config_get_key(const gen_kw_config_type * config ) {
  return config->key;
}


char * gen_kw_config_alloc_user_key(const gen_kw_config_type * config , int kw_nr) {
  char * user_key = util_alloc_sprintf("%s:%s" , config->key ,gen_kw_config_iget_name( config , kw_nr ));
  return user_key;
}


const char * gen_kw_config_iget_name(const gen_kw_config_type * config, int kw_nr) {
  const gen_kw_parameter_type * parameter = vector_iget( config->parameters , kw_nr );
  return parameter->name;
}




const char * gen_kw_config_get_tagged_name(const gen_kw_config_type * config, int kw_nr) {
  const gen_kw_parameter_type * parameter = vector_iget( config->parameters , kw_nr );
  return parameter->tagged_name;
}


void gen_kw_config_update_tag_format(gen_kw_config_type * config , const char * tag_format) {
  int i;
  
  config->tag_fmt = tag_format;
  for (i=0; i < vector_get_size( config->parameters ); i++) 
    gen_kw_parameter_update_tagged_name( vector_iget( config->parameters , i ) , config->tag_fmt);
}


stringlist_type * gen_kw_config_alloc_name_list( const gen_kw_config_type * config ) {
  
  stringlist_type * name_list = stringlist_alloc_new();
  int i;
  for (i=0; i < vector_get_size( config->parameters ); i++) {
    const gen_kw_parameter_type * parameter = vector_iget_const( config->parameters , i );
    stringlist_append_ref( name_list , parameter->name );    /* If the underlying parameter goes out scope - whom bang .. */
  }

  return name_list;
}





/**
   Will return -1 if the index is invalid.
*/
int gen_kw_config_get_index(const gen_kw_config_type * config , const char * key) {
  const int size   = gen_kw_config_get_data_size(config);
  bool    have_key = false;
  int     index    = 0;
  
  while (index < size && !have_key) {
    const gen_kw_parameter_type * parameter = vector_iget_const( config->parameters , index );
    if (strcmp(parameter->name , key) == 0)
      have_key = true;
    else
      index++;
  }
  
  if (have_key)
    return index;
  else
    return -1;
}



void gen_kw_config_fprintf_config( const gen_kw_config_type * config , const char * outfile , const char * min_std_file , FILE * stream ) {
  fprintf(stream , CONFIG_VALUE_FORMAT , config->template_file );
  fprintf(stream , CONFIG_VALUE_FORMAT , outfile );
  fprintf(stream , CONFIG_VALUE_FORMAT , config->parameter_file );

  if (min_std_file != NULL) 
    fprintf( stream , CONFIG_OPTION_FORMAT , MIN_STD_KEY , min_std_file);
  
}


/*****************************************************************/

VOID_FREE(gen_kw_config)
VOID_GET_DATA_SIZE(gen_kw)

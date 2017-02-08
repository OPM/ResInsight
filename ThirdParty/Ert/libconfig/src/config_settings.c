/*
   Copyright (C) 2017  Statoil ASA, Norway.

   The file 'config_settings.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/hash.h>

#include <ert/config/config_schema_item.h>
#include <ert/config/config_settings.h>


#define CONFIG_SETTINGS_TYPE_ID 68621527
#define SETTING_NODE_TYPE_ID    76254096


struct config_settings_struct {
  UTIL_TYPE_ID_DECLARATION;
  char * root_key;
  hash_type * settings;
};

/*****************************************************************/

typedef struct setting_node_struct setting_node_type;

struct setting_node_struct {
  UTIL_TYPE_ID_DECLARATION;
  config_item_types value_type;
  char * key;
  char * string_value;
};

static void setting_node_assert_type( const setting_node_type * node , config_item_types expected_type) {
  if (node->value_type != expected_type)
    util_abort("%s: internal error. Asked for type:%d  is of type:%d \n",__func__ , expected_type , node->value_type);
}

UTIL_SAFE_CAST_FUNCTION( setting_node , SETTING_NODE_TYPE_ID )

static setting_node_type * setting_node_alloc( const char * key, config_item_types value_type, const char * initial_value) {
  if (!config_schema_item_valid_string( value_type , initial_value))
    return NULL;

  {
    setting_node_type * node = util_malloc( sizeof * node );
    UTIL_TYPE_ID_INIT( node , SETTING_NODE_TYPE_ID );
    node->value_type = value_type;
    node->string_value = util_alloc_string_copy( initial_value );
    node->key = util_alloc_string_copy( key );
    return node;
  }
}


static void setting_node_free( setting_node_type * node ) {
  free( node->key );
  free( node->string_value );
  free( node );
}


static void setting_node_free__( void * arg ) {
  setting_node_type * node = setting_node_safe_cast( arg );
  setting_node_free( node );
}


static bool setting_node_set_value( setting_node_type * node, const char * value) {
  if (config_schema_item_valid_string(node->value_type , value)) {
    node->string_value = util_realloc_string_copy( node->string_value , value );
    return true;
  } else
    return false;
}


static void setting_node_set_string_value( setting_node_type * node, const char * value) {
  setting_node_assert_type( node , CONFIG_STRING );
  setting_node_set_value( node , value );
}


static void setting_node_set_int_value( setting_node_type * node, int value) {
  setting_node_assert_type( node , CONFIG_INT );
  {
    char * string_value = util_alloc_sprintf("%d" , value);
    setting_node_set_value( node , string_value );
    free( string_value );
  }
}

static void setting_node_set_double_value( setting_node_type * node, double value) {
  setting_node_assert_type( node , CONFIG_FLOAT );
  {
    char * string_value = util_alloc_sprintf("%g" , value);
    setting_node_set_value( node , string_value );
    free( string_value );
  }
}


static void setting_node_set_bool_value( setting_node_type * node, bool value) {
  bool bool_value;
  setting_node_assert_type( node , CONFIG_BOOL );
  if (value)
    setting_node_set_value( node , "True");
  else
    setting_node_set_value( node , "False");
}


static const char * setting_node_get_value( const setting_node_type * node) {
  return node->string_value;
}


static const char * setting_node_get_string_value( const setting_node_type * node) {
  setting_node_assert_type( node , CONFIG_STRING );
  return node->string_value;
}

static int  setting_node_get_int_value( const setting_node_type * node) {
  setting_node_assert_type( node , CONFIG_INT );
  return strtol( node->string_value , NULL , 10 );
}

static double setting_node_get_double_value( const setting_node_type * node) {
  setting_node_assert_type( node , CONFIG_FLOAT );
  return strtod( node->string_value , NULL );
}


static bool setting_node_get_bool_value( const setting_node_type * node) {
  bool bool_value;
  setting_node_assert_type( node , CONFIG_BOOL );
  util_sscanf_bool( node->string_value , &bool_value );
  return bool_value;
}




/*****************************************************************/

config_settings_type * config_settings_alloc( const char * root_key ) {
  config_settings_type * settings = util_malloc( sizeof * settings );
  settings->root_key = util_alloc_string_copy( root_key );
  settings->settings = hash_alloc();
  return settings;
}


void config_settings_free( config_settings_type * settings) {
  free( settings->root_key );
  hash_free( settings->settings );
  free( settings );
}



bool config_settings_add_setting(config_settings_type * settings , const char* key, config_item_types value_type , const char* initial_value) {
  setting_node_type * node = setting_node_alloc( key , value_type , initial_value);
  if (node) {
    hash_insert_hash_owned_ref( settings->settings , key , node , setting_node_free__ );
    return true;
  } else
    return false;
}


void config_settings_add_bool_setting(config_settings_type * settings , const char* key, bool initial_value) {
  if (initial_value)
    config_settings_add_setting( settings , key , CONFIG_BOOL , "True");
  else
    config_settings_add_setting( settings , key , CONFIG_BOOL , "False");
}


void config_settings_add_int_setting(config_settings_type * settings , const char* key, int initial_value) {
  char * string_value = util_alloc_sprintf("%d" , initial_value);
  config_settings_add_setting( settings , key , CONFIG_INT , string_value);
  free( string_value );
}


void config_settings_add_double_setting(config_settings_type * settings , const char* key, double initial_value) {
  char * string_value = util_alloc_sprintf("%g" , initial_value);
  config_settings_add_setting( settings , key , CONFIG_FLOAT , string_value);
  free( string_value );
}


void config_settings_add_string_setting(config_settings_type * settings , const char* key, const char * initial_value) {
  config_settings_add_setting( settings , key , CONFIG_STRING , initial_value);
}




bool config_settings_has_key( const config_settings_type * settings , const char * key) {
  return hash_has_key( settings->settings , key );
}

static setting_node_type * config_settings_get_node( const config_settings_type * config_settings, const char * key){
  return hash_get( config_settings->settings , key );
}


const char * config_settings_get_value( const config_settings_type * config_settings , const char * key) {
  setting_node_type * node = config_settings_get_node( config_settings , key );
  return setting_node_get_value( node );
}

const char * config_settings_get_string_value( const config_settings_type * config_settings , const char * key) {
  setting_node_type * node = config_settings_get_node( config_settings , key );
  return setting_node_get_string_value( node );
}

double config_settings_get_double_value( const config_settings_type * config_settings , const char * key) {
  setting_node_type * node = config_settings_get_node( config_settings , key );
  return setting_node_get_double_value( node );
}

int config_settings_get_int_value( const config_settings_type * config_settings , const char * key) {
  setting_node_type * node = config_settings_get_node( config_settings , key );
  return setting_node_get_int_value( node );
}


bool config_settings_get_bool_value( const config_settings_type * config_settings , const char * key) {
  setting_node_type * node = config_settings_get_node( config_settings , key );
  return setting_node_get_bool_value( node );
}

config_item_types config_settings_get_value_type( const config_settings_type * config_settings , const char * key) {
  setting_node_type * node = config_settings_get_node( config_settings , key );
  return node->value_type;
}


bool config_settings_set_value( const config_settings_type * config_settings , const char * key, const char * value) {
  if (config_settings_has_key( config_settings , key )) {
    setting_node_type * node = config_settings_get_node( config_settings , key );
    return setting_node_set_value( node, value );
  }

  return false;
}


bool config_settings_set_int_value( const config_settings_type * config_settings , const char * key, int value) {
  if (config_settings_has_key( config_settings , key )) {
    setting_node_type * node = config_settings_get_node( config_settings , key );
    setting_node_set_int_value( node, value );
    return true;
  }

  return false;
}


bool config_settings_set_double_value( const config_settings_type * config_settings , const char * key, double value) {
  if (config_settings_has_key( config_settings , key )) {
    setting_node_type * node = config_settings_get_node( config_settings , key );
    setting_node_set_double_value( node, value );
    return true;
  }

  return false;
}


bool config_settings_set_bool_value( const config_settings_type * config_settings , const char * key, bool value) {
  if (config_settings_has_key( config_settings , key )) {
    setting_node_type * node = config_settings_get_node( config_settings , key );
    setting_node_set_bool_value( node, value );
    return true;
  }

  return false;
}


bool config_settings_set_string_value( const config_settings_type * config_settings , const char * key, const char * value) {
  if (config_settings_has_key( config_settings , key )) {
    setting_node_type * node = config_settings_get_node( config_settings , key );
    setting_node_set_string_value( node, value );
    return true;
  }

  return false;
}




void config_settings_init_parser__( const char * root_key , config_parser_type * config , bool required) {
  config_schema_item_type * item = config_add_schema_item(config, root_key , required);
  config_schema_item_set_argc_minmax(item, 2, 2);
}



void config_settings_init_parser( const config_settings_type * config_settings, config_parser_type * config , bool required) {
  config_settings_init_parser__( config_settings->root_key , config , required );
}


void config_settings_apply(config_settings_type * config_settings , const config_content_type * config ) {
  for (int i = 0; i < config_content_get_occurences(config, config_settings->root_key); i++) {
    const stringlist_type * tokens = config_content_iget_stringlist_ref(config, config_settings->root_key , i);
    const char * setting = stringlist_iget(tokens, 0);
    const char * value = stringlist_iget(tokens, 1);

    bool set_ok = config_settings_set_value( config_settings , setting , value );
    if (!set_ok)
      fprintf(stderr," ** Warning: failed to apply CONFIG_SETTING %s=%s \n",setting,value);
  }
}


stringlist_type * config_settings_alloc_keys( const config_settings_type * config_settings ) {
  return hash_alloc_stringlist(config_settings->settings);
}

/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'config_get.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


/*****************************************************************/
/* All the functions in this block will operate on the last item 
   which has been set with a particular key value. So assuming the 
   config file looks like:

   KEY   VALUE1
   KEY   VALUE2  OPTIONAL 
   KEY   100     VALUE3   OPTIONAL  ERROR

   these functions will all operate on the last line in the config file:

             KEY 100 VALUE3 OPTIONAL ERROR
*/


config_content_node_type * config_get_value_node( const config_type * config , const char * kw) {
  config_content_item_type * item = config_get_content_item(config , kw);
  if (item != NULL) {
    config_content_node_type * node = config_content_item_get_last_node( item );
    config_content_node_assert_key_value( node );
    return node;
  } else
    return NULL;  // Will return NULL on unset keywords - must check NULL return value?!
}

static config_content_node_type * config_get_value_node__( const config_type * config , const char * kw) {
  config_content_node_type * node = config_get_value_node( config , kw );
  if (node == NULL) 
    util_abort("Tried to get value node from unset kw:%s \n",__func__ , kw );
  
  return node;
}

bool config_get_value_as_bool(const config_type * config , const char * kw) {
  config_content_node_type * node = config_get_value_node__( config , kw );
  return config_content_node_iget_as_bool(node , 0);
}

int config_get_value_as_int(const config_type * config , const char * kw) {
  config_content_node_type * node = config_get_value_node__( config , kw );
  return config_content_node_iget_as_int(node , 0);
}

double config_get_value_as_double(const config_type * config , const char * kw) {
  config_content_node_type * node = config_get_value_node__( config , kw );
  return config_content_node_iget_as_double(node , 0);
}

const char * config_get_value_as_path( const config_type * config , const char * kw) {
  config_content_node_type * node = config_get_value_node__( config , kw );
  return config_content_node_iget_as_path(node , 0);
}

const char * config_get_value_as_abspath( const config_type * config , const char * kw) {
  config_content_node_type * node = config_get_value_node__( config , kw );
  return config_content_node_iget_as_abspath(node , 0);
}

const char * config_get_value_as_relpath( const config_type * config , const char * kw) {
  config_content_node_type * node = config_get_value_node__( config , kw );
  return config_content_node_iget_as_relpath(node , 0);
}


const char * config_get_value(const config_type * config , const char * kw) {
  config_content_node_type * node = config_get_value_node__( config , kw );
  return config_content_node_iget(node , 0);
}


/*****************************************************************/

int config_get_content_size( const config_type * config ) {
  return vector_get_size(config->content_list);
}


const config_content_node_type * config_iget_content_node( const config_type * config , int index) {
  return vector_iget_const( config->content_list , index );
}





int config_get_schema_size( const config_type * config ) {
  return hash_get_size( config->schema_items );
}




config_error_type * config_get_errors( const config_type * config ) {
  return config->parse_errors;
}

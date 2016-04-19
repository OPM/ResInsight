/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'config_content.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef __CONFIG_CONTENT_H__
#define __CONFIG_CONTENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/util/stringlist.h>
#include <ert/util/subst_list.h>

#include <ert/config/config_content_item.h>
#include <ert/config/config_schema_item.h>
#include <ert/config/config_error.h>
#include <ert/config/config_root_path.h>

typedef struct config_content_struct config_content_type;


  config_content_type * config_content_alloc();
  void config_content_free( config_content_type * content );
  void config_content_set_valid( config_content_type * content);
  bool config_content_is_valid( const config_content_type * content );
  bool config_content_has_item( const config_content_type * content , const char * key);
  void config_content_add_item( config_content_type * content , const config_schema_item_type * schema_item , const config_path_elm_type * path_elm);
  config_content_item_type * config_content_get_item( const config_content_type * content , const char * key);
  void config_content_add_node( config_content_type * content , config_content_node_type * content_node );
  config_error_type * config_content_get_errors( const config_content_type * content);

  const char * config_content_iget( const config_content_type * content , const char * key , int occurence , int index);
  int config_content_iget_as_int( const config_content_type * content , const char * key , int occurence , int index);
  bool   config_content_iget_as_bool( const config_content_type * content , const char * key , int occurence , int index);
  double config_content_iget_as_double( const config_content_type * content , const char * key , int occurence , int index);
  const char * config_content_iget_as_path( const config_content_type * content , const char * key , int occurence , int index);
  const char * config_content_safe_iget(const config_content_type * content , const char *kw, int occurence , int index);
  int config_content_get_occurences(const config_content_type * content, const char * kw);

  bool config_content_get_value_as_bool(const config_content_type * config , const char * kw);
  int config_content_get_value_as_int(const config_content_type * config , const char * kw);
  double config_content_get_value_as_double(const config_content_type * config , const char * kw);
  const char * config_content_get_value_as_path( const config_content_type * config , const char * kw);
  const char * config_content_get_value_as_abspath( const config_content_type * config , const char * kw);
  const char * config_content_get_value_as_relpath( const config_content_type * config , const char * kw);
  const char * config_content_get_value(const config_content_type * config , const char * kw);
  char * config_content_alloc_joined_string(const config_content_type * content , const char * kw, const char * sep);
  stringlist_type * config_content_alloc_complete_stringlist(const config_content_type * content , const char * kw);
  const stringlist_type * config_content_iget_stringlist_ref(const config_content_type * content , const char * kw, int occurence);
  config_content_node_type * config_content_get_value_node( const config_content_type * content , const char * kw);
  void config_content_add_define( config_content_type * content , const char * key , const char * value );
  subst_list_type * config_content_get_define_list( config_content_type * content );
  const char * config_content_get_config_file( const config_content_type * content , bool abs_path );
  void config_content_set_config_file( config_content_type * content , const char * config_file );
  int config_content_get_size(const config_content_type * content);
  const config_content_node_type * config_content_iget_node( const config_content_type * content , int index);
  bool config_content_add_file( config_content_type * content , const char * config_file);
  config_root_path_type * config_content_get_invoke_path( config_content_type * content );
  void config_content_set_invoke_path( config_content_type * content);
  config_path_elm_type * config_content_add_path_elm( config_content_type * content , const char * path );
  void config_content_pop_path_stack( config_content_type * content );

  UTIL_IS_INSTANCE_HEADER( config_content );

#ifdef __cplusplus
}
#endif
#endif

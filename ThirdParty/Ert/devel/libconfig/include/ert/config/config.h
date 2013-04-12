/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/stringlist.h>
#include <ert/util/hash.h>

#include <ert/config/config_schema_item.h>
#include <ert/config/config_content_item.h>
#include <ert/config/config_content_node.h>
  
#define ECL_COM_KW "--"
#define ENKF_COM_KW "--"





typedef struct config_struct              config_type;


  char       **     config_alloc_active_list(const config_type *, int *);
  void              config_free(config_type *);
  config_type *     config_alloc( );
  char       **     config_alloc_active_list(const config_type * , int * );
  bool              config_parse(config_type * , const char * , const char * , const char * , const char * , config_schema_unrecognized_enum unrecognized_behaviour , bool );
  bool              config_has_schema_item(const config_type * config , const char * kw);
  void              config_clear(config_type * config);

/*****************************************************************/
  
  config_schema_item_type * config_get_schema_item(const config_type *, const char *);
  bool               config_item_set(const config_type * , const char * );
  void               config_add_alias(config_type * , const char * , const char * );
  void               config_install_message(config_type * , const char * , const char * );
  const char       * config_safe_get(const config_type * , const char *);
  char             * config_alloc_joined_string(const config_type * , const char * , const char * );
  
  void               config_add_define( config_type * config , const char * key , const char * value );

  /*  
  bool               config_schema_item_is_set(const config_schema_item_type * );
  void               config_schema_item_set_argc_minmax(config_schema_item_type * , int  , int , int type_map_size , const config_item_types * );
  void               config_schema_item_set_common_selection_set(config_schema_item_type * , int argc , const char ** argv);
  void               config_schema_item_set_indexed_selection_set(config_schema_item_type * item , int  , int  , const char ** );
  void               config_schema_item_set_required_children(config_schema_item_type * , stringlist_type * );
  void               config_schema_item_set_required_children_on_value(config_schema_item_type * , const char * , stringlist_type * );
  void               config_schema_item_add_required_children(config_schema_item_type * item , const char * child_key);
  */

  config_schema_item_type * config_add_schema_item(config_type * config, 
                                     const char * kw,
                                     bool required);
  
  
  const char            * config_safe_iget(const config_type * config , const char *kw, int occurence , int index);
  const char            * config_iget(const config_type * , const char * , int occurence , int index);
  bool                    config_iget_as_bool(const config_type * , const char * , int occurence , int index);
  double                  config_iget_as_double(const config_type * , const char * , int occurence , int index);
  int                     config_iget_as_int(const config_type * , const char *, int occurence , int index);
  stringlist_type       * config_alloc_complete_stringlist(const config_type*  , const char * );
  stringlist_type       * config_alloc_stringlist(const config_type * config , const char * );
  hash_type             * config_alloc_hash(const config_type *  , const char * );
  const stringlist_type * config_iget_stringlist_ref(const config_type *  , const char * , int );
  
  int                     config_get_occurences(const config_type * , const char * );
  int                     config_get_occurence_size( const config_type * config , const char * kw , int occurence);
  
  bool                       config_has_content_item( const config_type * config , const char * input_kw);
  config_content_item_type * config_get_content_item( const config_type * config , const char * input_kw);
  config_schema_item_type      * config_add_key_value( config_type * config , const char * key , bool required , config_item_types item_type);  
  bool                    config_get_value_as_bool(const config_type * config , const char * kw);
  int                     config_get_value_as_int(const config_type * config , const char * kw);
  double                  config_get_value_as_double(const config_type * config , const char * kw);
  const char *            config_get_value_as_abspath( const config_type * config , const char * kw);
  const char *            config_get_value_as_relpath( const config_type * config , const char * kw);
  const char *            config_get_value_as_path( const config_type * config , const char * kw);
  const char *            config_get_value(const config_type * config , const char * kw);
  const char *            config_get_config_file( const config_type * config , bool abs_path);
  void                    config_fprintf_errors( const config_type * config , bool add_count , FILE * stream );
  
  int                     config_get_schema_size( const config_type * config );
  int                     config_get_content_size( const config_type * config );
  const config_content_node_type * config_iget_content_node( const config_type * config , int index );
  config_content_node_type       * config_get_value_node( const config_type * config , const char * kw);
  config_error_type * config_get_errors( const config_type * config );
  
#ifdef __cplusplus
}
#endif
#endif

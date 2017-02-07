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

#ifndef ERT_CONFIG_H
#define ERT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/stringlist.h>
#include <ert/util/subst_list.h>
#include <ert/util/hash.h>

#include <ert/config/config_schema_item.h>
#include <ert/config/config_content_item.h>
#include <ert/config/config_content_node.h>
#include <ert/config/config_content.h>
  
#define ECL_COM_KW "--"
#define ENKF_COM_KW "--"





typedef struct config_parser_struct              config_parser_type;


  void              config_free(config_parser_type *);
  config_parser_type *     config_alloc( );
  char       **     config_alloc_active_list(const config_parser_type * , int * );
  config_content_type * config_parse(config_parser_type * config, const char * filename, const char * comment_string, const char * include_kw, const char * define_kw, const hash_type * pre_defined_kw_map, config_schema_unrecognized_enum unrecognized_behaviour , bool validate);
  bool              config_has_schema_item(const config_parser_type * config , const char * kw);

/*****************************************************************/
  
  config_schema_item_type * config_get_schema_item(const config_parser_type *, const char *);
  bool               config_item_set(const config_parser_type * , const char * );
  void               config_add_alias(config_parser_type * , const char * , const char * );
  void               config_install_message(config_parser_type * , const char * , const char * );
  const char       * config_safe_get(const config_parser_type * , const char *);
  char             * config_alloc_joined_string(const config_parser_type * , const char * , const char * );
  
  void               config_add_define( config_parser_type * config , const char * key , const char * value );

  /*  
  bool               config_schema_item_is_set(const config_schema_item_type * );
  void               config_schema_item_set_argc_minmax(config_schema_item_type * , int  , int , int type_map_size , const config_item_types * );
  void               config_schema_item_set_common_selection_set(config_schema_item_type * , int argc , const char ** argv);
  void               config_schema_item_set_indexed_selection_set(config_schema_item_type * item , int  , int  , const char ** );
  void               config_schema_item_set_required_children(config_schema_item_type * , stringlist_type * );
  void               config_schema_item_set_required_children_on_value(config_schema_item_type * , const char * , stringlist_type * );
  void               config_schema_item_add_required_children(config_schema_item_type * item , const char * child_key);
  */

  config_schema_item_type * config_add_schema_item(config_parser_type * config, 
                                     const char * kw,
                                     bool required);
  
  
  stringlist_type       * config_alloc_complete_stringlist(const config_parser_type *  , const char * );
  stringlist_type       * config_alloc_stringlist(const config_parser_type * config , const char * );
  hash_type             * config_alloc_hash(const config_parser_type *  , const char * );
  const stringlist_type * config_iget_stringlist_ref(const config_parser_type *  , const char * , int );
  
  int                     config_get_occurences(const config_parser_type * , const char * );
  int                     config_get_occurence_size( const config_parser_type * config , const char * kw , int occurence);
  
  bool                       config_has_content_item( const config_parser_type * config , const char * input_kw);
  config_content_item_type * config_get_content_item( const config_parser_type * config , const char * input_kw);
  config_schema_item_type      * config_add_key_value( config_parser_type * config , const char * key , bool required , config_item_types item_type);
;
  const char *            config_get_value_as_relpath( const config_parser_type * config , const char * kw);
  const char *            config_get_value_as_path( const config_parser_type * config , const char * kw);
  const char *            config_get_value(const config_parser_type * config , const char * kw);

  const subst_list_type * config_get_define_list( const config_parser_type * config);
  int                     config_get_schema_size( const config_parser_type * config );
  config_content_node_type       * config_get_value_node( const config_parser_type * config , const char * kw);
  void                             config_parser_deprecate(config_parser_type * config , const char * kw, const char * msg);

#ifdef __cplusplus
}
#endif
#endif

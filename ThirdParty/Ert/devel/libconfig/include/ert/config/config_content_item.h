/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'config_content_item.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __CONFIG_CONTENT_ITEM_H__
#define __CONFIG_CONTENT_ITEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/hash.h>
#include <ert/util/stringlist.h>

#include <ert/config/config_error.h>
#include <ert/config/config_schema_item.h>
#include <ert/config/config_path_elm.h>
#include <ert/config/config_content_node.h>

typedef struct config_content_item_struct config_content_item_type;

  int                              config_content_item_get_size(const config_content_item_type * item);
  config_content_node_type       * config_content_item_get_last_node(const config_content_item_type * item);
  config_content_node_type       * config_content_item_iget_node(const config_content_item_type * item , int index);
  const config_content_node_type * config_content_item_get_last_node_const(const config_content_item_type * item);
  const config_content_node_type * config_content_item_iget_node_const(const config_content_item_type * item , int index);
  char                           * config_content_item_ialloc_joined_string(const config_content_item_type * item , const char * sep , int occurence);
  char                           * config_content_item_alloc_joined_string(const config_content_item_type * item , const char * sep);
  const stringlist_type          * config_content_item_iget_stringlist_ref(const config_content_item_type * item, int occurence);
  const stringlist_type          * config_content_item_get_stringlist_ref(const config_content_item_type * item);
  stringlist_type                * config_content_item_alloc_complete_stringlist(const config_content_item_type * item, bool copy);
  stringlist_type                * config_content_item_alloc_stringlist(const config_content_item_type * item, bool copy);
  hash_type                      * config_content_item_alloc_hash(const config_content_item_type * item , bool copy);
  const char                     * config_content_item_iget(const config_content_item_type * item , int occurence , int index);
  bool                             config_content_item_iget_as_bool(const config_content_item_type * item, int occurence , int index);
  int                              config_content_item_iget_as_int(const config_content_item_type * item, int occurence , int index);
  double                           config_content_item_iget_as_double(const config_content_item_type * item, int occurence , int index);
  void                             config_content_item_clear( config_content_item_type * item );
  void                             config_content_item_free( config_content_item_type * item );
  void                             config_content_item_free__( void * arg );
  config_content_item_type       * config_content_item_alloc( const config_schema_item_type * schema , const config_path_elm_type * path_elm);
  void                             config_content_item_validate(const config_content_item_type * item, config_error_type * error);
  config_content_node_type       * config_content_item_alloc_node( const config_content_item_type * item , const config_path_elm_type * path_elm);
  const config_schema_item_type  * config_content_item_get_schema( const config_content_item_type * item );
  const config_path_elm_type     * config_content_item_get_path_elm( const config_content_item_type * item );

#ifdef __cplusplus
}
#endif

#endif

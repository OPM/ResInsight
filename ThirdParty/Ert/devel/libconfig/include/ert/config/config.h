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
#include <stdbool.h>

#include <ert/util/stringlist.h>
#include <ert/util/hash.h>

#define ECL_COM_KW "--"
#define ENKF_COM_KW "--"


/** 
    Types used for validation of config items.
*/
typedef enum {CONFIG_STRING        = 0,
              CONFIG_INT           = 1,
              CONFIG_FLOAT         = 2,   
              CONFIG_FILE          = 9,  /* These file does not need to exist - but includes are handled. */
              CONFIG_EXISTING_FILE = 3,
              CONFIG_EXISTING_DIR  = 4,
              CONFIG_BOOLEAN       = 5,
              CONFIG_CONFIG        = 6,
              CONFIG_BYTESIZE      = 7,
              CONFIG_EXECUTABLE    = 8 ,
              CONFIG_INVALID       = 1000  } config_item_types;

typedef struct config_struct                  config_type;
typedef struct config_schema_item_struct      config_schema_item_type;

  char       **     config_alloc_active_list(const config_type *, int *);
  void              config_free(config_type *);
  config_type *     config_alloc( );
  char       **     config_alloc_active_list(const config_type * , int * );
  void              config_parse(config_type * , const char * , const char * , const char * , const char * , bool , bool );
  bool              config_has_schema_item(const config_type * config , const char * kw);
  void              config_clear(config_type * config);

/*****************************************************************/
  
  void               config_schema_item_set_envvar_expansion( config_schema_item_type * item , bool expand_envvar );
  bool               config_item_set(const config_type * , const char * );
  void               config_schema_item_free__ (void * );
  void               config_schema_item_free( config_schema_item_type * );
  config_schema_item_type * config_schema_item_alloc(const char * , bool , bool);
  config_schema_item_type * config_get_schema_item(const config_type *, const char *);
  void               config_add_alias(config_type * , const char * , const char * );
  void               config_install_message(config_type * , const char * , const char * );
  const char       * config_safe_get(const config_type * , const char *);
  char             * config_alloc_joined_string(const config_type * , const char * , const char * );
  
  void               config_add_define( config_type * config , const char * key , const char * value );
  
  bool               config_schema_item_is_set(const config_schema_item_type * );
  void               config_schema_item_set_argc_minmax(config_schema_item_type * , int  , int , int type_map_size , const config_item_types * );
  void               config_schema_item_set_common_selection_set(config_schema_item_type * , int argc , const char ** argv);
  void               config_schema_item_set_indexed_selection_set(config_schema_item_type * item , int  , int  , const char ** );
  void               config_schema_item_set_required_children(config_schema_item_type * , stringlist_type * );
  void               config_schema_item_set_required_children_on_value(config_schema_item_type * , const char * , stringlist_type * );
  void               config_schema_item_add_required_children(config_schema_item_type * item , const char * child_key);
  
  config_schema_item_type * config_add_schema_item(config_type *, 
                                     const char * ,
                                     bool         ,
                                     bool);
  
  
  bool config_has_keys(const config_type *,
                       const char       **,
                       int                ,
                       bool               );
  
  const char            * config_safe_iget(const config_type * config , const char *kw, int occurence , int index);
  const char            * config_iget(const config_type * , const char * , int occurence , int index);
  bool                    config_iget_as_bool(const config_type * , const char * , int occurence , int index);
  double                  config_iget_as_double(const config_type * , const char * , int occurence , int index);
  int                     config_iget_as_int(const config_type * , const char *, int occurence , int index);
  stringlist_type       * config_alloc_complete_stringlist(const config_type*  , const char * );
  stringlist_type       * config_alloc_stringlist(const config_type * config , const char * );
  hash_type             * config_alloc_hash(const config_type *  , const char * );
  const stringlist_type * config_get_stringlist_ref(const config_type *  , const char * );
  stringlist_type       * config_iget_stringlist_ref(const config_type *  , const char * , int );
  bool                    config_has_set_item(const config_type *  , const char * );
  
  int                     config_get_occurences(const config_type * , const char * );
  int                     config_get_occurence_size( const config_type * config , const char * kw , int occurence);
  
  
  config_schema_item_type      * config_add_key_value( config_type * config , const char * key , bool required , config_item_types item_type);  
  bool                    config_get_value_as_bool(const config_type * config , const char * kw);
  int                     config_get_value_as_int(const config_type * config , const char * kw);
  double                  config_get_value_as_double(const config_type * config , const char * kw);
  const char *            config_get_value(const config_type * config , const char * kw);
  void                    config_fprintf_item_list(const config_type * config , FILE * stream);
  const char *            config_get_config_file( const config_type * config , bool abs_path);
  
#ifdef __cplusplus
}
#endif
#endif

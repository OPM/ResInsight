/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'config_schema_item.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __CONFIG_SCHEMA_ITEM_H__
#define __CONFIG_SCHEMA_ITEM_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include <ert/util/stringlist.h>

#include <ert/config/config_error.h>
#include <ert/config/config_path_elm.h>

/** 
    Types used for validation of config items.
*/
typedef enum {CONFIG_STRING        = 1,
              CONFIG_INT           = 2,
              CONFIG_FLOAT         = 4,   
              CONFIG_PATH          = 8,
              CONFIG_EXISTING_PATH = 16,
              CONFIG_BOOL          = 32,
              CONFIG_CONFIG        = 64,
              CONFIG_BYTESIZE      = 128,
              CONFIG_EXECUTABLE    = 256 ,
              CONFIG_INVALID       = 512  } config_item_types;
  
#define CONFIG_ITEM_TYPE_ENUM_DEFS               \
{.value =    1 , .name="CONFIG_STRING"},         \
{.value =    2 , .name="CONFIG_INT"},            \
{.value =    4 , .name="CONFIG_FLOAT"},          \
{.value =    8 , .name="CONFIG_PATH"},           \
{.value =   16 , .name="CONFIG_EXISTING_PATH"},  \
{.value =   32 , .name="CONFIG_BOOL"},        \
{.value =   64 , .name="CONFIG_CONFIG"},         \
{.value =  128 , .name="CONFIG_BYTESIZE"},       \
{.value =  256 , .name="CONFIG_EXECUTABLE"},     \
{.value =  512 , .name="CONFIG_INVALID"}       
#define CONFIG_ITEM_TYPE_ENUM_SIZE 10

  
  typedef enum  {
    CONFIG_UNRECOGNIZED_IGNORE = 0,
    CONFIG_UNRECOGNIZED_WARN = 1,
    CONFIG_UNRECOGNIZED_ERROR = 2
  } config_schema_unrecognized_enum;


#define CONFIG_SCHEMA_UNRECOGNIZED_ENUM_DEFS          \
{.value =    0 , .name="CONFIG_UNRECOGNIZED_IGNORE"}, \
{.value =    1 , .name="CONFIG_UNRECOGNIZED_WARN"},   \
{.value =    2 , .name="CONFIG_UNRECOGNIZED_ERROR"}
#define CONFIG_SCHEMA_UNRECOGNIZED_ENUM_SIZE 3




#define CONFIG_DEFAULT_ARG_MIN -1
#define CONFIG_DEFAULT_ARG_MAX -1



  typedef struct config_schema_item_struct      config_schema_item_type;
  
  
  config_schema_item_type * config_schema_item_alloc(const char * kw , bool required);
  bool                      config_schema_item_validate_set(const config_schema_item_type * item , 
                                                            stringlist_type * token_list , 
                                                            const char * config_file, 
                                                            const config_path_elm_type * path_elm, 
                                                            config_error_type * error_list);

  void                      config_schema_item_free( config_schema_item_type * item);
  void                      config_schema_item_free__ (void * void_item);

  
  void                      config_schema_item_set_common_selection_set(config_schema_item_type * item , int argc , const char ** argv);
  void                      config_schema_item_set_indexed_selection_set(config_schema_item_type * item , int index , int argc , const char ** argv);
  void                      config_schema_item_set_required_children(config_schema_item_type * item , stringlist_type * stringlist);
  void                      config_schema_item_add_required_children(config_schema_item_type * item , const char * child_key);
  void                      config_schema_item_set_envvar_expansion( config_schema_item_type * item , bool expand_envvar );
  void                      config_schema_item_set_argc_minmax(config_schema_item_type * item , 
                                                               int argc_min , 
                                                               int argc_max);
  void                      config_schema_item_assure_type(const config_schema_item_type * item , int index , int type_mask);

  int                       config_schema_item_num_required_children(const config_schema_item_type * item);
  const char              * config_schema_item_iget_required_child( const config_schema_item_type * item , int index);     
  const char              * config_schema_item_get_kw( const config_schema_item_type * item );
  bool                      config_schema_item_required( const config_schema_item_type * item );
  bool                      config_schema_item_expand_envvar( const config_schema_item_type * item );
  void                      config_schema_item_get_argc( const config_schema_item_type * item , int *argc_min , int *argc_max);
  bool                      config_schema_item_has_required_children_value( const config_schema_item_type * item );
  stringlist_type         * config_schema_item_get_required_children_value(const config_schema_item_type * item , const char * value);

  void                      config_schema_item_iset_type( config_schema_item_type * item , int index , config_item_types type);
  config_item_types         config_schema_item_iget_type(const config_schema_item_type * item , int index );
  void                      config_schema_item_set_default_type( config_schema_item_type * item , config_item_types type);


#ifdef __cplusplus
}
#endif

#endif

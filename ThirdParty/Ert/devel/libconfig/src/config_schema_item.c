/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'config_schema_item.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/parser.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/set.h>
#include <ert/util/subst_list.h>
#include <ert/util/vector.h>

#include <ert/config/config_error.h>
#include <ert/config/config_schema_item.h>
#include <ert/config/config_path_elm.h>

typedef struct validate_struct validate_type;

/** 
   This is a 'support-struct' holding various pieces of information
   needed during the validation process. Observe the following about
   validation:

    1. It is atomic, in the sense that if you try to an item like this:

         KW  ARG1 ARG2 ARG3

       where ARG1 and ARG2 are valid, whereas there is something wrong
       with ARG3, NOTHING IS SET.

    2. Validation is a two-step process, the first step is run when an
       item is parsed. This includes checking:

        o The number of argument.  
        o That the arguments have the right type.
        o That the values match the selection set.
      
       The second validation step is done when the pasing is complete,
       in this pass we check dependencies - i.e. required_children and
       required_children_on_value.


   Observe that nothing has-to be set in this struct. There are some dependencies:

    1. Only _one_ of common_selection_set and indexed_selection_set
       can be set.

    2. If setting indexed_selection_set or type_map, you MUST set
       argc_max first.
*/

    
struct validate_struct {
  int                  argc_min;                /* The minimum number of arguments: -1 means no lower limit. */
  int                  argc_max;                /* The maximum number of arguments: -1 means no upper limit. */ 
  set_type          *  common_selection_set;    /* A selection set which will apply uniformly to all the arguments. */ 
  set_type          ** indexed_selection_set;   /* A selection set which will apply for specifi (indexed) arguments. */
  int_vector_type   *  type_map;                /* A list of types for the items. Set along with argc_minmax(); */
  stringlist_type   *  required_children;       /* A list of item's which must also be set (if this item is set). (can be NULL) */
  hash_type         *  required_children_value; /* A list of item's which must also be set - depending on the value of this item. (can be NULL) (This one is complex). */
};





#define CONFIG_SCHEMA_ITEM_ID 6751
struct config_schema_item_struct {
  UTIL_TYPE_ID_DECLARATION;
  char                        * kw;                      /* The kw which identifies this item· */
  

  bool                          required_set;            
  stringlist_type             * required_children;       /* A list of item's which must also be set (if this item is set). (can be NULL) */
  hash_type                   * required_children_value; /* A list of item's which must also be set - depending on the value of this item. (can be NULL) */
  validate_type               * validate;                /* Information need during validation. */ 
  bool                          expand_envvar;           /* Should environment variables like $HOME be expanded?*/ 
};


/*****************************************************************/


/*****************************************************************/
static void validate_set_default_type( validate_type * validate , config_item_types item_type) {
  int_vector_set_default(validate->type_map , item_type);
}

static validate_type * validate_alloc() {
  validate_type * validate = util_malloc(sizeof * validate );
  validate->argc_min                = CONFIG_DEFAULT_ARG_MIN;
  validate->argc_max                = CONFIG_DEFAULT_ARG_MAX;
  validate->common_selection_set    = NULL;
  validate->indexed_selection_set   = NULL;
  validate->required_children       = NULL;
  validate->required_children_value = NULL;
  validate->type_map                = int_vector_alloc(0 , 0);  
  validate_set_default_type( validate , CONFIG_STRING );
  return validate;
}


static void validate_free(validate_type * validate) {
  if (validate->common_selection_set != NULL) set_free(validate->common_selection_set);
  if (validate->indexed_selection_set != NULL) {
    for (int i = 0; i < validate->argc_max; i++)
      if (validate->indexed_selection_set[i] != NULL)
        set_free(validate->indexed_selection_set[i]);
    free(validate->indexed_selection_set);
  }
  
  int_vector_free( validate->type_map );
  if (validate->required_children != NULL) stringlist_free(validate->required_children);
  if (validate->required_children_value != NULL) hash_free(validate->required_children_value);
  free(validate);
}


static void validate_iset_type( validate_type * validate , int index , config_item_types type) {
  int_vector_iset( validate->type_map , index , type);
}


static config_item_types validate_iget_type( const validate_type * validate , int index) {
  return int_vector_safe_iget( validate->type_map , index ); 
}


static void validate_set_argc_minmax(validate_type * validate , int argc_min , int argc_max) {
  if (validate->argc_min != CONFIG_DEFAULT_ARG_MIN)
    util_abort("%s: sorry - current implementation does not allow repeated calls to: %s \n",__func__ , __func__);
  
  if (argc_min == CONFIG_DEFAULT_ARG_MIN)
    argc_min = 0;

  validate->argc_min = argc_min;
  validate->argc_max = argc_max;
  
  if ((argc_max != CONFIG_DEFAULT_ARG_MAX) && (argc_max < argc_min))
    util_abort("%s invalid arg min/max values. argc_min:%d  argc_max:%d \n",__func__ , argc_min , argc_max);
  
  {
    int internal_type_size = 0;  /* Should end up in the range [argc_min,argc_max] */

    if (argc_max > 0) 
      internal_type_size = argc_max;
    else
      internal_type_size = argc_min;

    if (internal_type_size > 0) {
      validate->indexed_selection_set = util_calloc( internal_type_size , sizeof * validate->indexed_selection_set );
      for (int iarg=0; iarg < internal_type_size; iarg++)
        validate->indexed_selection_set[iarg] = NULL;
    }
  }
}



static void validate_set_common_selection_set(validate_type * validate , int argc , const char ** argv) {
  if (validate->common_selection_set != NULL)
    set_free(validate->common_selection_set);
  validate->common_selection_set = set_alloc( argc , argv );
}


static void validate_set_indexed_selection_set(validate_type * validate , int index , int argc , const char ** argv) {
  
  if (validate->indexed_selection_set == NULL)
    util_abort("%s: must call xxx_set_argc_minmax() first - aborting \n",__func__);
  
  if (index >= validate->argc_min)
    util_abort("%s: When not not setting argc_max selection set can only be applied to indices up to argc_min\n",__func__);
  
  if (validate->indexed_selection_set[index] != NULL)
    set_free(validate->indexed_selection_set[index]);
  
  validate->indexed_selection_set[index] = set_alloc(argc , argv);
}


/*****************************************************************/


static UTIL_SAFE_CAST_FUNCTION( config_schema_item , CONFIG_SCHEMA_ITEM_ID)

void config_schema_item_assure_type(const config_schema_item_type * item , int index , int type_mask) {
  bool OK = false;
  
  if (int_vector_safe_iget( item->validate->type_map , index) & type_mask)
    OK = true;
  
  if (!OK)
    util_abort("%s: failed - wrong installed type \n" , __func__);
}


config_schema_item_type * config_schema_item_alloc(const char * kw , bool required) {
  config_schema_item_type * item = util_malloc(sizeof * item );
  UTIL_TYPE_ID_INIT( item , CONFIG_SCHEMA_ITEM_ID);
  item->kw         = util_alloc_string_copy(kw);

  item->required_set            = required;
  item->required_children       = NULL;
  item->required_children_value = NULL;
  item->expand_envvar           = true;  /* Default is to expand $VAR expressions; can be turned off with
                                            config_schema_item_set_envvar_expansion( item , false ); */
  item->validate                = validate_alloc();
  return item;
}



static char * __alloc_relocated__(const config_path_elm_type * path_elm , const char * value) {
  char * file;
  
  if (util_is_abs_path(value))
    file = util_alloc_string_copy( value );
  else
    file = util_alloc_filename(config_path_elm_get_relpath( path_elm ) , value , NULL);

  return file;
}


bool config_schema_item_validate_set(const config_schema_item_type * item , stringlist_type * token_list , const char * config_file, const config_path_elm_type * path_elm , config_error_type * error_list) {
  bool OK = true;
  int argc = stringlist_get_size( token_list ) - 1;
  if (item->validate->argc_min >= 0) {
    if (argc < item->validate->argc_min) {
      OK = false;
      {
        char * error_message;
        if (config_file != NULL) 
          error_message = util_alloc_sprintf("Error when parsing config_file:\"%s\" Keyword:%s must have at least %d arguments.",config_file , item->kw , item->validate->argc_min);
        else
          error_message = util_alloc_sprintf("Error:: Keyword:%s must have at least %d arguments.",item->kw , item->validate->argc_min);
        
        config_error_add( error_list , error_message );
      }
    }
  }

  if (item->validate->argc_max >= 0) {
    if (argc > item->validate->argc_max) {
      OK = false;
      {
        char * error_message;
        
        if (config_file != NULL)
          error_message = util_alloc_sprintf("Error when parsing config_file:\"%s\" Keyword:%s must have maximum %d arguments.",config_file , item->kw , item->validate->argc_max);
        else
          error_message = util_alloc_sprintf("Error:: Keyword:%s must have maximum %d arguments.",item->kw , item->validate->argc_max);
        
        config_error_add( error_list , error_message );
      }
    }
  }

  /* 
     OK - now we have verified that the number of arguments is correct. Then
     we start actually looking at the values. 
  */
  if (OK) { 
    /* Validating selection set - first common, then indexed */
    if (item->validate->common_selection_set) {
      for (int iarg = 0; iarg < argc; iarg++) {
        if (!set_has_key(item->validate->common_selection_set , stringlist_iget( token_list , iarg + 1))) {
          config_error_add( error_list , util_alloc_sprintf("%s: is not a valid value for: %s.",stringlist_iget( token_list , iarg + 1) , item->kw));
          OK = false;
        }
      }
    } else if (item->validate->indexed_selection_set != NULL) {
      for (int iarg = 0; iarg < argc; iarg++) {
        if ((item->validate->argc_max > 0) || (iarg < item->validate->argc_min)) {  /* Without this test we might go out of range on the indexed selection set. */
          if (item->validate->indexed_selection_set[iarg] != NULL) {
            if (!set_has_key(item->validate->indexed_selection_set[iarg] , stringlist_iget( token_list , iarg + 1))) {
              config_error_add( error_list , util_alloc_sprintf("%s: is not a valid value for item %d of \'%s\'.",stringlist_iget( token_list , iarg + 1) , iarg + 1 , item->kw));
              OK = false;
            }
          }
        }
      }
    }

    /*
      Observe that the following code might rewrite the content of
      argv for arguments referring to path locations.
    */


    /* Validate the TYPE of the various argumnents */
    {
      for (int iarg = 0; iarg < argc; iarg++) {
        const char * value = stringlist_iget(token_list , iarg + 1);
        switch (validate_iget_type( item->validate , iarg)) {
        case(CONFIG_STRING): /* This never fails ... */
          break;
        case(CONFIG_INT):
          if (!util_sscanf_int( value , NULL ))
            config_error_add( error_list , util_alloc_sprintf("Failed to parse:%s as an integer.",value));
          break;
        case(CONFIG_FLOAT):
          if (!util_sscanf_double( value , NULL )) {
            config_error_add( error_list , util_alloc_sprintf("Failed to parse:%s as a floating point number.", value));
            OK = false;
          }
          break;
        case(CONFIG_PATH):
          // As long as we do not reuqire the path to exist it is just a string.
          break;
        case(CONFIG_EXISTING_PATH):
          {
            char * path = config_path_elm_alloc_abspath( path_elm , value );
            if (!util_entry_exists(path)) {
              config_error_add( error_list , util_alloc_sprintf("Can not find entry %s in %s ",value , config_path_elm_get_relpath( path_elm) ));
              OK = false;
            }
            free( path );
          }
          break;
        case(CONFIG_EXECUTABLE):
          {
            /*
              1. If the supplied value is an abolute path - do nothing.
              2. If the supplied is _not_ an absolute path:
              
                 a. Try if the relocated exists - then use that.
                 b. Else - try if the util_alloc_PATH_executable() exists.
            */
            if (!util_is_abs_path( value )) {
              char * relocated  = __alloc_relocated__(path_elm , value);
              char * path_exe   = util_alloc_PATH_executable( value );
              
              if (util_file_exists(relocated)) {
                if (util_is_executable(relocated))
                  stringlist_iset_copy( token_list , iarg , relocated);
              } else if (path_exe != NULL)
                stringlist_iset_copy( token_list , iarg , path_exe);
              else
                config_error_add( error_list , util_alloc_sprintf("Could not locate executable:%s ", value));
              
              free(relocated);
              util_safe_free(path_exe);
            } else {
              if (!util_is_executable( value ))
                config_error_add( error_list , util_alloc_sprintf("Could not locate executable:%s ", value));
            }
          }
          break;
        case(CONFIG_BOOL):
          if (!util_sscanf_bool( value , NULL )) {
            config_error_add( error_list , util_alloc_sprintf("Failed to parse:%s as a boolean.", value));
            OK = false;
          }
          break;
        case(CONFIG_BYTESIZE):
          if (!util_sscanf_bytesize( value , NULL)) {
            config_error_add( error_list , util_alloc_sprintf("Failed to parse:\"%s\" as number of bytes." , value));
            OK = false;
          }
          break;
        default:
          util_abort("%s: config_item_type:%d not recognized \n",__func__ , validate_iget_type(item->validate , iarg));
        }
      }
    } 
  }
  return OK;
}


void config_schema_item_free( config_schema_item_type * item) {
  free(item->kw);
  if (item->required_children       != NULL) stringlist_free(item->required_children);
  if (item->required_children_value != NULL) hash_free(item->required_children_value); 
  validate_free(item->validate);
  free(item);
}


void config_schema_item_free__ (void * void_item) {
  config_schema_item_type * item = config_schema_item_safe_cast( void_item );
  config_schema_item_free( item );
}



void config_schema_item_set_required_children_on_value(config_schema_item_type * item , const char * value , stringlist_type * child_list) {
  if (item->required_children_value == NULL)
    item->required_children_value = hash_alloc();
  hash_insert_hash_owned_ref( item->required_children_value , value , stringlist_alloc_deep_copy(child_list) , stringlist_free__);
}



/**
   This function is used to set the minimum and maximum number of
   arguments for an item. In addition you can pass in a pointer to an
   array of config_schema_item_types values which will be used for validation
   of the input. This vector must be argc_max elements long; it can be
   NULL.
*/


void config_schema_item_set_argc_minmax(config_schema_item_type * item , 
                                        int argc_min , 
                                        int argc_max) {
  
  validate_set_argc_minmax(item->validate , argc_min , argc_max);

}

void config_schema_item_iset_type( config_schema_item_type * item , int index , config_item_types type) {
  validate_iset_type( item->validate , index , type );
}

void config_schema_item_set_default_type( config_schema_item_type * item , config_item_types type) {
  validate_set_default_type( item->validate , type );
}


config_item_types config_schema_item_iget_type(const config_schema_item_type * item , int index ) {
  return validate_iget_type( item->validate , index );
}

  


void config_schema_item_set_envvar_expansion( config_schema_item_type * item , bool expand_envvar ) {
  item->expand_envvar = expand_envvar;
}



void config_schema_item_set_common_selection_set(config_schema_item_type * item , int argc , const char ** argv) {
  validate_set_common_selection_set(item->validate , argc , argv);
}

void config_schema_item_set_indexed_selection_set(config_schema_item_type * item , int index , int argc , const char ** argv) {
  validate_set_indexed_selection_set(item->validate , index , argc , argv);
}


void config_schema_item_set_required_children(config_schema_item_type * item , stringlist_type * stringlist) {
  item->required_children = stringlist_alloc_deep_copy(stringlist);
}

void config_schema_item_add_required_children(config_schema_item_type * item , const char * child_key) {
  if (item->required_children == NULL)
    item->required_children = stringlist_alloc_new();
  
  stringlist_append_copy( item->required_children , child_key );
}


int config_schema_item_num_required_children(const config_schema_item_type * item) {
  if (item->required_children == NULL)
    return 0;
  else
    return stringlist_get_size( item->required_children );
}


const char * config_schema_item_iget_required_child( const config_schema_item_type * item , int index) {
  return stringlist_iget( item->required_children , index );
}


const char * config_schema_item_get_kw( const config_schema_item_type * item ) {
  return item->kw;
}


bool config_schema_item_required( const config_schema_item_type * item ) {
  return item->required_set;
}


bool config_schema_item_expand_envvar( const config_schema_item_type * item ) {
  return item->expand_envvar;
}


void config_schema_item_get_argc( const config_schema_item_type * item , int *argc_min , int *argc_max) {
  *argc_min = item->validate->argc_min;
  *argc_max = item->validate->argc_max;
}



bool config_schema_item_has_required_children_value( const config_schema_item_type * item ) {
  if (item->required_children_value == NULL)
    return false;
  else
    return true;
}



stringlist_type * config_schema_item_get_required_children_value(const config_schema_item_type * item , const char * value) {
  return hash_safe_get( item->required_children_value , value );
}


/*****************************************************************/
/* Small functions to support enum introspection. */


const char * config_schema_item_type_enum_iget( int index, int * value) {
  return util_enum_iget( index , CONFIG_ITEM_TYPE_ENUM_SIZE , (const util_enum_element_type []) { CONFIG_ITEM_TYPE_ENUM_DEFS }, value);
}


const char * config_schema_item_unrecognized_enum_iget( int index, int * value) {
  return util_enum_iget( index , CONFIG_SCHEMA_UNRECOGNIZED_ENUM_SIZE , (const util_enum_element_type []) { CONFIG_SCHEMA_UNRECOGNIZED_ENUM_DEFS }, value);
}

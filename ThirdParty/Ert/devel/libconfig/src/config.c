/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <unistd.h>

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/parser.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/set.h>
#include <ert/util/subst_list.h>
#include <ert/util/vector.h>
#include <ert/util/path_stack.h>

#include <ert/config/config.h>
#include <ert/config/config_error.h>
#include <ert/config/config_schema_item.h>
#include <ert/config/config_content_node.h>
#include <ert/config/config_content_item.h>
#include <ert/config/config_path_elm.h>
#include <ert/config/config_root_path.h>

#define  CLEAR_STRING "__RESET__"



/**
Structure to parse configuration files of this type:

KEYWORD1  ARG2   ARG2  ARG3
KEYWORD2  ARG1-2
....
KEYWORDN  ARG1  ARG2

A keyword can occure many times.

*/



/**

                                                              
                           =============================                                
                           | config_type object        |
                           |                           |                                
                           | Contains 'all' the        |                                
                           | configuration information.|                                
                           |                           |                                
                           =============================                                
                               |                   |
                               |                   \________________________                                             
                               |                                            \          
                              KEY1                                         KEY2   
                               |                                             |  
                              \|/                                           \|/ 
                   =========================                      =========================          
                   | config_item object    |                      | config_item object    |                     
                   |                       |                      |                       |                     
                   | Indexed by a keyword  |                      | Indexed by a keyword  |                     
                   | which is the first    |                      | which is the first    |                     
                   | string in the         |                      | string in the         |                     
                   | config file.          |                      | config file.          |                     
                   |                       |                      |                       |                                                                           
                   =========================                      =========================                     
                       |             |                                        |
                       |             |                                        |
                      \|/           \|/                                      \|/  
============================  ============================   ============================
| config_item_node object  |  | config_item_node object  |   | config_item_node object  |
|                          |  |                          |   |                          |
| Only containing the      |  | Only containing the      |   | Only containing the      |
| stringlist object        |  | stringlist object        |   | stringlist object        |
| directly parsed from the |  | directly parsed from the |   | directly parsed from the |
| file.                    |  | file.                    |   | file.                    |
|--------------------------|  |--------------------------|   |--------------------------|
| ARG1 ARG2 ARG3           |  | VERBOSE                  |   | DEBUG                    |
============================  ============================   ============================


The example illustrated above would correspond to the following config
file (invariant under line-permutations):

KEY1   ARG1 ARG2 ARG3
KEY1   VERBOSE
KEY2   DEBUG


Example config file(2):

OUTFILE   filename
INPUT     filename
OPTIONS   store
OPTIONS   verbose
OPTIONS   optimize cache=1

In this case the whole config object will contain three items,
corresponding to the keywords OUTFILE, INPUT and OPTIONS. The two
first will again only contain one node each, whereas the OPTIONS item
will contain three nodes, corresponding to the three times the keyword
"OPTIONS" appear in the config file. 
*/






 




struct config_struct {
  vector_type          * content_list;              /* Vector of content_content_node instances. */
  hash_type            * content_items;
  hash_type            * schema_items;              
  config_error_type    * parse_errors;
  set_type             * parsed_files;              /* A set of config files whcih have been parsed - to protect against circular includes. */
  hash_type            * messages;                  /* Can print a (warning) message when a keyword is encountered. */
  subst_list_type      * define_list;
  char                 * config_file;               /* The last parsed file - NULL if no file is parsed-. */
  char                 * abs_path;
  config_root_path_type * invoke_path;
  vector_type          * path_elm_storage;
  vector_type          * path_elm_stack;
};










/** 
    Adds a new node as side-effect ... 
*/
static config_content_node_type * config_get_new_content_node(config_type * config , config_content_item_type * item) {
  config_content_node_type * new_node = config_content_item_alloc_node( item , config_content_item_get_path_elm( item ));
  vector_append_ref( config->content_list , new_node );
  return new_node;
}








/*
  The last argument (config_file) is only used for printing
  informative error messages, and can be NULL. The config_cwd is
  essential if we are looking up a filename, otherwise it can be NULL.

  Returns a string with an error description, or NULL if the supplied
  arguments were OK. The string is allocated here, but is assumed that
  calling scope will free it.
*/

static void config_content_item_set_arg__(config_type * config , config_content_item_type * item , stringlist_type * token_list , 
                                          const config_path_elm_type * path_elm , 
                                          const char * config_file ) {

  int argc = stringlist_get_size( token_list ) - 1;

  if (argc == 1 && (strcmp(stringlist_iget(token_list , 1) , CLEAR_STRING) == 0)) {
    config_content_item_clear(item);
  } else {
    const config_schema_item_type * schema_item = config_content_item_get_schema( item );
    
    /* Filtering based on DEFINE statements */
    if (subst_list_get_size( config->define_list ) > 0) {
      int iarg;
      for (iarg = 0; iarg < argc; iarg++) {
        char * filtered_copy = subst_list_alloc_filtered_string( config->define_list , stringlist_iget(token_list , iarg + 1));
        stringlist_iset_owned_ref( token_list , iarg + 1 , filtered_copy);
      }
    }

    
    /* Filtering based on environment variables */
    if (config_schema_item_expand_envvar( schema_item )) {
      int iarg;
      for (iarg = 0; iarg < argc; iarg++) {
        int    env_offset = 0;
        char * env_var;
        do {
          env_var = util_isscanf_alloc_envvar(  stringlist_iget(token_list , iarg + 1) , env_offset );
          if (env_var != NULL) {
            const char * env_value = getenv( &env_var[1] );
            if (env_value != NULL) {
              char * new_value = util_string_replace_alloc( stringlist_iget( token_list , iarg + 1 ) , env_var , env_value );
              stringlist_iset_owned_ref( token_list , iarg + 1 , new_value );
            } else {
              env_offset += 1;
              fprintf(stderr,"** Warning: environment variable: %s is not defined \n", env_var);
            }
          }
        } while (env_var != NULL);
      }
    }

    {
      if (config_schema_item_validate_set(schema_item , token_list , config_file,  path_elm , config->parse_errors)) {
        config_content_node_type * node = config_get_new_content_node(config , item);
        config_content_node_set(node , token_list);
      } 
    }
  }
}
 




/*****************************************************************/



config_type * config_alloc() {
  config_type *config       = util_malloc(sizeof * config );
  config->content_list      = vector_alloc_new();
  config->path_elm_storage  = vector_alloc_new();
  config->path_elm_stack    = vector_alloc_new();
  
  config->schema_items    = hash_alloc();
  config->content_items   = hash_alloc(); 
  config->parse_errors    = config_error_alloc();
  config->parsed_files    = set_alloc_empty();
  config->messages        = hash_alloc();
  config->define_list     = subst_list_alloc( NULL );
  config->config_file     = NULL;
  config->abs_path        = NULL;
  config->invoke_path     = NULL;
  return config;
}




static void config_clear_content_items( config_type * config ) {
  hash_free( config->content_items );
  config->content_items = hash_alloc();
  vector_clear( config->content_list );
}


void config_clear(config_type * config) {
  config_error_clear(config->parse_errors);

  set_clear(config->parsed_files);
  subst_list_clear( config->define_list );
  config_clear_content_items( config );
  vector_clear( config->path_elm_storage );
  vector_clear( config->path_elm_stack );

  util_safe_free( config->config_file );
  util_safe_free( config->abs_path );    
  if (config->invoke_path != NULL) {
    config_root_path_free( config->invoke_path );
    config->invoke_path = NULL;
  }
  config->config_file = NULL;
  config->abs_path = NULL;
}



void config_free(config_type * config) {
  config_error_free( config->parse_errors );
  set_free(config->parsed_files);
  subst_list_free( config->define_list );
  
  vector_free( config->path_elm_storage );
  vector_free( config->path_elm_stack );
  vector_free( config->content_list );
  hash_free(config->schema_items);
  hash_free(config->content_items);
  hash_free(config->messages);

  free(config);
}



static void config_insert_schema_item(config_type * config , const char * kw , const config_schema_item_type * item , bool ref) {
  if (ref)
    hash_insert_ref(config->schema_items , kw , item);
  else
    hash_insert_hash_owned_ref(config->schema_items , kw , item , config_schema_item_free__);
}


/**
   This function allocates a simple item with all values
   defaulted. The item is added to the config object, and a pointer is
   returned to the calling scope. If you want to change the properties
   of the item you can do that with config_schema_item_set_xxxx() functions
   from the calling scope.
*/


config_schema_item_type * config_add_schema_item(config_type * config , 
                                                 const char  * kw, 
                                                 bool  required) { 
  
  config_schema_item_type * item = config_schema_item_alloc( kw , required );
  config_insert_schema_item(config , kw , item , false);
  return item;
}



/**
  This is a minor wrapper for adding an item with the properties. 

    1. It has argc_minmax = {1,1}
    
   The value can than be extracted with config_get_value() and
   config_get_value_as_xxxx functions. 
*/

config_schema_item_type * config_add_key_value( config_type * config , const char * key , bool required , config_item_types item_type) {
  config_schema_item_type * item = config_add_schema_item( config , key , required );
  config_schema_item_set_argc_minmax( item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , item_type );
  return item;
}



bool config_has_schema_item(const config_type * config , const char * kw) {
  return hash_has_key(config->schema_items , kw);
}


config_schema_item_type * config_get_schema_item(const config_type * config , const char * kw) {
  return hash_get(config->schema_items , kw);
}

/*
  Due to the possibility of aliases we must go through the canonical
  keyword which is internalized in the schema_item.  
*/


static config_content_item_type * config_add_content_item( const config_type * config , const char * input_kw, const config_path_elm_type * path_elm) {
  config_schema_item_type * schema_item = config_get_schema_item( config , input_kw );
  const char * kw = config_schema_item_get_kw( schema_item );
  config_content_item_type * content_item = config_content_item_alloc( schema_item , path_elm );
  hash_insert_hash_owned_ref( config->content_items , kw , content_item , config_content_item_free__ );

  return content_item;
}


bool config_has_content_item( const config_type * config , const char * input_kw) {
  if (config_has_schema_item( config , input_kw )) {
    config_schema_item_type * schema_item = config_get_schema_item( config , input_kw );
    const char * kw = config_schema_item_get_kw( schema_item );
    return hash_has_key( config->content_items , kw );
  } else
    return false;
}


config_content_item_type * config_get_content_item( const config_type * config , const char * input_kw) {
  if (config_has_schema_item( config , input_kw )) {
    config_schema_item_type * schema_item = config_get_schema_item( config , input_kw );
    const char * kw = config_schema_item_get_kw( schema_item );

    if (hash_has_key( config->content_items , kw ))
      return hash_get( config->content_items , kw );
    else
      return NULL;
    
  } else
    return NULL;   
}



bool config_item_set(const config_type * config , const char * kw) {
  if (config_has_content_item(config , kw)) {
    const config_content_item_type * item = config_get_content_item( config , kw );
    if (config_content_item_get_size( item ))
      return true;
    else
      return false;  // The item exists - but it has size zero.
  } else
    return false;
}





void config_add_define( config_type * config , const char * key , const char * value ) {
  subst_list_append_copy( config->define_list , key , value , NULL );
}




char ** config_alloc_active_list(const config_type * config, int * _active_size) {
  char ** complete_key_list = hash_alloc_keylist(config->schema_items);
  char ** active_key_list = NULL;
  int complete_size = hash_get_size(config->schema_items);
  int active_size   = 0;
  int i;

  for( i = 0; i < complete_size; i++) {
    if (config_has_content_item( config , complete_key_list[i])) {
      active_key_list = util_stringlist_append_copy(active_key_list , active_size , complete_key_list[i]);
      active_size++;
    }
  }
  *_active_size = active_size;
  util_free_stringlist(complete_key_list , complete_size);
  return active_key_list;
}




static void config_validate_content_item(const config_type * config , const config_content_item_type * item) {
  const config_schema_item_type *  schema_item = config_content_item_get_schema( item );
  const char * schema_kw = config_schema_item_get_kw( schema_item );
  
  {
    int i;
    for (i = 0; i < config_schema_item_num_required_children(schema_item); i++) {
      const char * required_child = config_schema_item_iget_required_child( schema_item , i );
      if (!config_item_set(config , required_child)) {
        char * error_message = util_alloc_sprintf("When:%s is set - you also must set:%s.",schema_kw , required_child);
        config_error_add( config->parse_errors , error_message );
      }
    }
    
    if (config_schema_item_has_required_children_value( schema_item )) {
      int inode;
      for (inode = 0; inode < config_content_item_get_size(item); inode++) {
        config_content_node_type * node = config_content_item_iget_node(item , inode);
        const stringlist_type * values  = config_content_node_get_stringlist( node );
        int is;

        for (is = 0; is < stringlist_get_size(values); is++) {
          const char * value = stringlist_iget(values , is);
          stringlist_type * required_children = config_schema_item_get_required_children_value( schema_item , value );
          
          if (required_children != NULL) {
            int ic;
            for (ic = 0; ic < stringlist_get_size( required_children ); ic++) {
              const char * req_child = stringlist_iget( required_children , ic );
              if (!config_item_set(config , req_child )) {
                char * error_message = util_alloc_sprintf("When:%s is set to:%s - you also must set:%s.",schema_kw , value , req_child );
                config_error_add( config->parse_errors , error_message );
              }
            }
          }
        }
      }
    }
  } 
}



static void config_validate(config_type * config, const char * filename) {
  int size = hash_get_size(config->schema_items);
  char ** key_list = hash_alloc_keylist(config->schema_items);
  int ikey;
  for (ikey = 0; ikey < size; ikey++) {
    if (config_has_content_item( config , key_list[ikey])) {
      const config_content_item_type * item = config_get_content_item(config , key_list[ikey]);
      config_validate_content_item(config , item );
    } else {
      const config_schema_item_type *  schema_item = config_get_schema_item( config , key_list[ikey]);
      if (config_schema_item_required( schema_item)) {  /* The item is not set ... */
        const char * schema_kw = config_schema_item_get_kw( schema_item );
        char * error_message = util_alloc_sprintf("Item:%s must be set - parsing:%s",schema_kw , config->abs_path);
        config_error_add( config->parse_errors , error_message );
      }
    }
  }
  util_free_stringlist(key_list , size);
}




static config_path_elm_type * config_add_path_elm( config_type * config , const char * path ) {
  const config_path_elm_type * current_path_elm;

  if (vector_get_size( config->path_elm_stack ) == 0)
    current_path_elm = NULL;
  else
    current_path_elm = vector_get_last_const(config->path_elm_stack);
  
  {
    config_path_elm_type * new_path_elm;

    {
      char * rel_path = NULL;
      if (path != NULL) {
        if (current_path_elm == NULL) 
          rel_path = util_alloc_rel_path( config_root_path_get_abs_path(config->invoke_path) , path);
        else
          rel_path = config_path_elm_alloc_relpath( current_path_elm , path );
      }
      new_path_elm = config_path_elm_alloc( config->invoke_path , rel_path );
      util_safe_free( rel_path );
    }
    vector_append_owned_ref( config->path_elm_storage , new_path_elm , config_path_elm_free__);
    vector_append_ref( config->path_elm_stack , new_path_elm );
    return new_path_elm;
  }
}



/**
   This function parses the config file 'filename', and updated the
   internal state of the config object as parsing proceeds. If
   comment_string != NULL everything following 'comment_string' on a
   line is discarded.

   include_kw is a string identifier for an include functionality, if
   an include is encountered, the included file is parsed immediately
   (through a recursive call to config_parse__). if include_kw == NULL,
   include files are not supported.

   Observe that use of include, relative paths and all that shit is
   quite tricky. The following is currently implemented:

    1. The front_end function will split the path to the config file
       in a path_name component and a file component.

    2. Recursive calls to config_parse__() will keep control of the
       parsers notion of cwd (note that the real OS'wise cwd never
       changes), and every item is tagged with the config_cwd
       currently active.

    3. When an item has been entered with type CONFIG_FILE /
       CONFIG_DIRECTORY / CONFIG_EXECUTABLE - the item is updated to
       reflect to be relative (iff it is relative in the first place)
       to the path of the root config file.

   These are not strict rules - it is possible to get other things to
   work as well, but the problem is that it very quickly becomes
   dependant on 'arbitrariness' in the parsing configuration.

   validate: whether we should validate when complete, that should
             typically only be done at the last parsing.

      
   define_kw: This a string which can serve as a "#define" for the
   parsing. The define_kw keyword should have two arguments - a key
   and a value. If the define_kw is present all __subsequent__
   occurences of 'key' are replaced with 'value'.  alloc_new_key
   is an optinal function (can be NULL) which is used to alloc a new
   key, i.e. add leading and trailing 'magic' characters.


   Example:  
   --------

   char * add_angular_brackets(const char * key) {
       char * new_key = util_alloc_sprintf("<%s>" , key);
   }



   config_parse(... , "myDEF" , add_angular_brackets , ...) 


   Config file:
   -------------
   myDEF   Name         BJARNE
   myDEF   sexual-pref  Dogs
   ...
   ... 
   PERSON  <Name> 28 <sexual-pref>      
   ...
   ------------

   After parsing we will have an entry: "NAME" , "Bjarne" , "28" , "Dogs".

   The         key-value pairs internalized during the config parsing are NOT
   returned to the calling scope in any way.
*/


static void config_parse__(config_type * config , 
                           path_stack_type * path_stack , 
                           const char * config_input , 
                           const char * comment_string , 
                           const char * include_kw ,
                           const char * define_kw , 
                           config_schema_unrecognized_enum unrecognized,
                           bool validate) {

  /* Guard against circular includes. */
  {
    char * abs_filename = util_alloc_realpath(config_input);
    if (!set_add_key(config->parsed_files , abs_filename)) 
      util_exit("%s: file:%s already parsed - circular include ? \n",__func__ , abs_filename);
    free( abs_filename );
  }
  config_path_elm_type * current_path_elm;

  char * config_file;
  {
    /* Extract the path component of the current input file and chdir() */
    char * config_path;
    {
      char * config_base;
      char * config_ext;
      util_alloc_file_components( config_input , &config_path , &config_base , &config_ext);
      config_file = util_alloc_filename( NULL , config_base , config_ext );
      free( config_base );
      util_safe_free( config_ext );
    }
    current_path_elm = config_add_path_elm( config , config_path );
    path_stack_push_cwd( path_stack );
    if (config_path != NULL) {
      chdir( config_path );
      free( config_path );
    }
  }
  

  {
    parser_type * parser = parser_alloc(" \t" , "\"", NULL , NULL , "--" , "\n");
    FILE * stream = util_fopen(config_file , "r");
    bool   at_eof = false;
    
    while (!at_eof) {
      int    active_tokens;
      stringlist_type * token_list;
      char  *line_buffer;
      
      
      line_buffer  = util_fscanf_alloc_line(stream , &at_eof);
      if (line_buffer != NULL) {
        token_list = parser_tokenize_buffer(parser , line_buffer , true);
        active_tokens = stringlist_get_size( token_list );
        
        /*
          util_split_string(line_buffer , " \t" , &tokens , &token_list);
          active_tokens = tokens;
          for (i = 0; i < tokens; i++) {
          char * comment_ptr = NULL;
          if(comment_string != NULL)
          comment_ptr = strstr(token_list[i] , comment_string);
          
          if (comment_ptr != NULL) {
          if (comment_ptr == token_list[i])
          active_tokens = i;
          else
          active_tokens = i + 1;
          break;
          }
          }
        */
        
        if (active_tokens > 0) {
          const char * kw = stringlist_iget( token_list , 0 );
          
          /*Treating the include keyword. */
          if (include_kw != NULL && (strcmp(include_kw , kw) == 0)) {
            if (active_tokens != 2) 
              util_abort("%s: keyword:%s must have exactly one argument. \n",__func__ ,include_kw);
            {
              const char *include_file  = stringlist_iget( token_list , 1);
              if (util_file_exists( include_file ))
                config_parse__(config , path_stack , include_file , comment_string , include_kw , define_kw , unrecognized, false); /* Recursive call */
              else 
                config_error_add(config->parse_errors , util_alloc_sprintf("%s file:%s not found" , include_kw , include_file));
            }
          } else if ((define_kw != NULL) && (strcmp(define_kw , kw) == 0)) {
            /* Treating the define keyword. */
            if (active_tokens < 3) 
              util_abort("%s: keyword:%s must have exactly one (or more) arguments. \n",__func__ , define_kw);
            {
              char * key   = util_alloc_string_copy( stringlist_iget(token_list ,1) );
              char * value = stringlist_alloc_joined_substring( token_list , 2 , active_tokens , " ");
                
              {
                char * filtered_value = subst_list_alloc_filtered_string( config->define_list , value);
                config_add_define( config , key , filtered_value );
                free( filtered_value );
              }
              free(key);
              free(value);
            }
          } else {
            if (hash_has_key(config->messages , kw))
              printf("%s \n", (const char *) hash_get(config->messages , kw));
              
            if (!config_has_schema_item(config , kw)) {
              if (unrecognized == CONFIG_UNRECOGNIZED_WARN)
                fprintf(stderr,"** Warning keyword:%s not recognized when parsing:%s --- \n" , kw , config_input);
              else if (unrecognized == CONFIG_UNRECOGNIZED_ERROR) 
                config_error_add(config->parse_errors , util_alloc_sprintf("Keyword:%s is not recognized" , kw));
                
            }

            if (config_has_schema_item(config , kw)) {
              char * config_cwd;
              util_alloc_file_components( config_file , &config_cwd , NULL , NULL );
                
              if (!config_has_content_item( config , kw ))
                config_add_content_item( config , kw , current_path_elm );

              {
                config_content_item_type * content_item = config_get_content_item( config , kw );
                config_content_item_set_arg__(config , content_item , token_list , current_path_elm , config_file );
              }
            } 
          }
        }
        stringlist_free(token_list);
        free(line_buffer);
      }
    }
    if (validate) 
      config_validate(config , config_file);
    fclose(stream);
    parser_free( parser );
  }
  free(config_file);
  path_stack_pop( path_stack );
  vector_pop_back( config->path_elm_stack );
}



static void config_set_config_file( config_type * config , const char * config_file ) {
  config->config_file = util_realloc_string_copy( config->config_file , config_file );
  util_safe_free(config->abs_path);
  config->abs_path = util_alloc_abs_path( config_file );
}


static void config_set_invoke_path( config_type * config ) {
  if (config->invoke_path != NULL) 
    config_root_path_free( config->invoke_path );
  config->invoke_path = config_root_path_alloc( NULL );
}


const char * config_get_config_file( const config_type * config , bool abs_path) {
  if (abs_path)
    return config->abs_path;
  else
    return config->config_file;
}


void config_fprintf_errors( const config_type * config , bool add_count , FILE * stream ) {
  config_error_fprintf( config->parse_errors , add_count , stderr );
}



bool config_parse(config_type * config , 
                  const char * filename, 
                  const char * comment_string , 
                  const char * include_kw ,
                  const char * define_kw , 
                  config_schema_unrecognized_enum unrecognized_behaviour,
                  bool validate) {
  
  if (util_file_readable( filename )) {
    path_stack_type * path_stack = path_stack_alloc();
    {
      config_set_config_file( config , filename );
      config_set_invoke_path( config );
      config_parse__(config , path_stack , filename , comment_string , include_kw , define_kw , unrecognized_behaviour , validate);
    }
    path_stack_free( path_stack );
  } else 
    config_error_add( config->parse_errors , util_alloc_sprintf("Could not open file:%s for parsing" , filename));
    
  if (config_error_count( config->parse_errors ) == 0)
    return true;   // No errors
  else
    return false;  // There were parse errors.
}




/*****************************************************************/
/* 
   Here comes some xxx_get() functions - many of them will fail if
   the item has not been added in the right way (this is to ensure that
   the xxx_get() request is unambigous. 
*/


/**
   This function can be used to get the value of a config
   parameter. But to ensure that the get is unambigous we set the
   following requirements to the item corresponding to 'kw':

    * argc_minmax has been set to 1,1

   If this is not the case - we die.
*/

/**
   Assume we installed a key 'KEY' which occurs three times in the final 
   config file:

   KEY  1    2     3
   KEY  11   22    33
   KEY  111  222   333


   Now when accessing these values the occurence variable will
   correspond to the linenumber, and the index will index along a line:

     config_iget_as_int( config , "KEY" , 0 , 2) => 3
     config_iget_as_int( config , "KEY" , 2 , 1) => 222
*/



bool config_iget_as_bool(const config_type * config , const char * kw, int occurence , int index) {
  config_content_item_type * item = config_get_content_item(config , kw);
  return config_content_item_iget_as_bool(item , occurence , index);
}


int config_iget_as_int(const config_type * config , const char * kw, int occurence , int index) {
  config_content_item_type * item = config_get_content_item(config , kw);
  return config_content_item_iget_as_int(item , occurence , index);
}


double config_iget_as_double(const config_type * config , const char * kw, int occurence , int index) {
  config_content_item_type * item = config_get_content_item(config , kw);
  return config_content_item_iget_as_double(item , occurence , index);
}


/** 
    As the config_get function, but the argc_minmax requiremnt has been removed.
*/
const char * config_iget(const config_type * config , const char * kw, int occurence , int index) {
  config_content_item_type * item = config_get_content_item(config , kw);
  return config_content_item_iget(item , occurence , index);
}



/**
   This function will return NULL is the item has not been set, 
   however it must be installed with config_add_schema_item().
*/
const char * config_safe_iget(const config_type * config , const char *kw, int occurence , int index) {
  const char * value = NULL;

  if (config_has_content_item( config , kw )) {
    config_content_item_type * item = config_get_content_item(config , kw);
    if (occurence < config_content_item_get_size( item )) {
      config_content_node_type * node = config_content_item_iget_node( item , occurence );
      value = config_content_node_safe_iget( node , index );
    }
  }
  return value;
}


const stringlist_type * config_iget_stringlist_ref(const config_type * config , const char * kw, int occurence) {
  config_content_item_type * item = config_get_content_item(config , kw);
  
  return config_content_item_iget_stringlist_ref(item , occurence);
}


/**
  This function allocates a new stringlist containing *ALL* the
  arguements for an item. With reference to the illustrated example at
  the top the function call:

     config_alloc_complete_strtinglist(config , "KEY1");

     would produce the list: ("ARG1" "ARG2" "ARG2" "VERBOSE"), i.e. the
  arguments for the various occurences of "KEY1" are collapsed to one
  stringlist.
*/

  
stringlist_type * config_alloc_complete_stringlist(const config_type* config , const char * kw) {
  bool copy = true;
  config_content_item_type * item = config_get_content_item(config , kw);
  return config_content_item_alloc_complete_stringlist(item , copy);
}



/**
   In the case the keyword has been mentioned several times the last
   occurence will be returned.
*/
stringlist_type * config_alloc_stringlist(const config_type * config , const char * kw) {
  config_content_item_type * item = config_get_content_item(config , kw);
  return config_content_item_alloc_stringlist(item , true);
}


/**
   Now accepts kw-items which have not been added with append_arg == false.
*/

char * config_alloc_joined_string(const config_type * config , const char * kw, const char * sep) {
  config_content_item_type * item = config_get_content_item(config , kw);
  return config_content_item_alloc_joined_string(item , sep);
}





/**
   Return the number of times a keyword has been set - dies on unknown
   'kw'. If the append_arg attribute has been set to false the
   function will return 0 or 1 irrespective of how many times the item
   has been set in the config file.
*/


int config_get_occurences(const config_type * config, const char * kw) {
  if (config_has_content_item( config , kw ))
    return config_content_item_get_size(config_get_content_item(config , kw));
  else
    return 0;
}


int config_get_occurence_size( const config_type * config , const char * kw , int occurence) {
  config_content_item_type      * item = config_get_content_item(config , kw);
  config_content_node_type * node = config_content_item_iget_node( item , occurence );
  return config_content_node_get_size( node );
}



/**
   Allocates a hash table for situations like this:

ENV   PATH              /some/path
ENV   LD_LIBARRY_PATH   /some/other/path
ENV   MALLOC            STRICT
....

the returned hash table will be: {"PATH": "/some/path", "LD_LIBARRY_PATH": "/some/other_path" , "MALLOC": "STRICT"}

It is enforced that:

 * item is allocated with append_arg = true
 * item is allocated with argc_minmax = 2,2
 
 The hash takes copy of the values in the hash so the config object
 can safefly be freed (opposite if copy == false).
*/


hash_type * config_alloc_hash(const config_type * config , const char * kw) {
  bool copy = true;
  config_content_item_type * item = config_get_content_item(config , kw);
  return config_content_item_alloc_hash(item , copy);
}





/**
   This function adds an alias to an existing item; so that the
   value+++ of an item can be referred to by two different names.
*/


void config_add_alias(config_type * config , const char * src , const char * alias) {
  if (config_has_schema_item(config , src)) {
    config_schema_item_type * item = config_get_schema_item(config , src);
    config_insert_schema_item(config , alias , item , true);
  } else
    util_abort("%s: item:%s not recognized \n",__func__ , src);
}



void config_install_message(config_type * config , const char * kw, const char * message) {
  hash_insert_hash_owned_ref(config->messages , kw , util_alloc_string_copy(message) , free);
}




#include "config_get.c"



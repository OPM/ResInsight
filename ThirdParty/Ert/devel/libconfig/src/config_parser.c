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
#include <ert/util/subst_list.h>
#include <ert/util/vector.h>
#include <ert/util/path_stack.h>

#include <ert/config/config_parser.h>
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











struct config_parser_struct {
  hash_type             * schema_items;
  hash_type             * messages;                  /* Can print a (warning) message when a keyword is encountered. */
};


















/*
  The last argument (config_file) is only used for printing
  informative error messages, and can be NULL. The config_cwd is
  essential if we are looking up a filename, otherwise it can be NULL.

  Returns a string with an error description, or NULL if the supplied
  arguments were OK. The string is allocated here, but is assumed that
  calling scope will free it.
*/

static config_content_node_type * config_content_item_set_arg__(subst_list_type * define_list ,
                                                                config_error_type * parse_errors ,
                                                                config_content_item_type * item ,
                                                                stringlist_type * token_list ,
                                                                const config_path_elm_type * path_elm ,
                                                                const char * config_file ) {

  config_content_node_type * new_node = NULL;
  int argc = stringlist_get_size( token_list ) - 1;

  if (argc == 1 && (strcmp(stringlist_iget(token_list , 1) , CLEAR_STRING) == 0)) {
    config_content_item_clear(item);
  } else {
    const config_schema_item_type * schema_item = config_content_item_get_schema( item );

    /* Filtering based on DEFINE statements */
    if (subst_list_get_size( define_list ) > 0) {
      int iarg;
      for (iarg = 0; iarg < argc; iarg++) {
        char * filtered_copy = subst_list_alloc_filtered_string( define_list , stringlist_iget(token_list , iarg + 1));
        stringlist_iset_owned_ref( token_list , iarg + 1 , filtered_copy);
      }
    }


    /* Filtering based on environment variables */
    if (config_schema_item_expand_envvar( schema_item )) {
      int iarg;
      for (iarg = 0; iarg < argc; iarg++) {
        int    env_offset = 0;
        while (true) {
          char * env_var = util_isscanf_alloc_envvar(  stringlist_iget(token_list , iarg + 1) , env_offset );
          if (env_var == NULL)
            break;

          {
            const char * env_value = getenv( &env_var[1] );
            if (env_value != NULL) {
              char * new_value = util_string_replace_alloc( stringlist_iget( token_list , iarg + 1 ) , env_var , env_value );
              stringlist_iset_owned_ref( token_list , iarg + 1 , new_value );
            } else {
              env_offset += 1;
              fprintf(stderr,"** Warning: environment variable: %s is not defined \n", env_var);
            }
          }

          free( env_var );
        }
      }
    }

    {
      if (config_schema_item_validate_set(schema_item , token_list , config_file,  path_elm , parse_errors)) {
        new_node = config_content_item_alloc_node( item , config_content_item_get_path_elm( item ));
        config_content_node_set(new_node , token_list);
      }
    }
  }
  return new_node;
}





/*****************************************************************/



config_parser_type * config_alloc() {
  config_parser_type *config       = util_malloc(sizeof * config );
  config->schema_items    = hash_alloc();
  config->messages        = hash_alloc();
  return config;
}










void config_free(config_parser_type * config) {

  hash_free(config->schema_items);
  hash_free(config->messages);

  free(config);
}



static void config_insert_schema_item(config_parser_type * config , const char * kw , const config_schema_item_type * item , bool ref) {
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


config_schema_item_type * config_add_schema_item(config_parser_type * config ,
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

config_schema_item_type * config_add_key_value( config_parser_type * config , const char * key , bool required , config_item_types item_type) {
  config_schema_item_type * item = config_add_schema_item( config , key , required );
  config_schema_item_set_argc_minmax( item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , item_type );
  return item;
}



bool config_has_schema_item(const config_parser_type * config , const char * kw) {
  return hash_has_key(config->schema_items , kw);
}


config_schema_item_type * config_get_schema_item(const config_parser_type * config , const char * kw) {
  return hash_get(config->schema_items , kw);
}

/*
  Due to the possibility of aliases we must go through the canonical
  keyword which is internalized in the schema_item.
*/









static void config_validate_content_item(const config_parser_type * config , config_content_type * content , const config_content_item_type * item) {
  const config_schema_item_type *  schema_item = config_content_item_get_schema( item );
  const char * schema_kw = config_schema_item_get_kw( schema_item );

  {
    int i;
    for (i = 0; i < config_schema_item_num_required_children(schema_item); i++) {
      const char * required_child = config_schema_item_iget_required_child( schema_item , i );
      if (!config_content_has_item(content , required_child)) {
        char * error_message = util_alloc_sprintf("When:%s is set - you also must set:%s.",schema_kw , required_child);
        config_error_add( config_content_get_errors( content ) , error_message );
        free( error_message );
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
              if (!config_content_has_item(content , req_child )) {
                char * error_message = util_alloc_sprintf("When:%s is set to:%s - you also must set:%s.",schema_kw , value , req_child );
                config_error_add( config_content_get_errors( content ) , error_message );
                free( error_message );
              }
            }
          }
        }
      }
    }
  }
}



static void config_validate(config_parser_type * config, config_content_type * content , const char * filename) {
  int size = hash_get_size(config->schema_items);
  char ** key_list = hash_alloc_keylist(config->schema_items);
  int ikey;
  for (ikey = 0; ikey < size; ikey++) {
    const config_schema_item_type *  schema_item = config_get_schema_item( config , key_list[ikey]);
    const char * content_key = config_schema_item_get_kw( schema_item );
    if (config_content_has_item( content , content_key)) {
      const config_content_item_type * item = config_content_get_item(content , content_key);
      config_validate_content_item(config , content , item );
    } else {
      if (config_schema_item_required( schema_item)) {  /* The item is not set ... */
        char * error_message = util_alloc_sprintf("Item:%s must be set - parsing:%s",content_key , config_content_get_config_file( content , true ));
        config_error_add( config_content_get_errors( content ) , error_message );
        free( error_message );
      }
    }
  }
  util_free_stringlist(key_list , size);
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


static void config_parse__(config_parser_type * config ,
                           config_content_type * content ,
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
    if (!config_content_add_file( content , abs_filename ))
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
    current_path_elm = config_content_add_path_elm( content , config_path );
    path_stack_push_cwd( path_stack );
    if (config_path != NULL) {
      util_chdir( config_path );
      free( config_path );
    }
  }


  {
    const char * comment_end = comment_string ? "\n" : NULL;
    basic_parser_type * parser = basic_parser_alloc(" \t" , "\"", NULL , NULL , comment_string , comment_end);
    FILE * stream = util_fopen(config_file , "r");
    bool   at_eof = false;

    while (!at_eof) {
      int    active_tokens;
      stringlist_type * token_list;
      char  *line_buffer;


      line_buffer  = util_fscanf_alloc_line(stream , &at_eof);
      if (line_buffer != NULL) {
        token_list = basic_parser_tokenize_buffer(parser , line_buffer , true);
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
                config_parse__(config , content , path_stack , include_file , comment_string , include_kw , define_kw , unrecognized, false); /* Recursive call */
              else {
                char * error_message = util_alloc_sprintf("%s file:%s not found" , include_kw , include_file);
                config_error_add( config_content_get_errors( content ) , error_message );
                free( error_message );
              }
            }
          } else if ((define_kw != NULL) && (strcmp(define_kw , kw) == 0)) {
            /* Treating the define keyword. */
            if (active_tokens < 3)
              util_abort("%s: keyword:%s must have exactly one (or more) arguments. \n",__func__ , define_kw);
            {
              char * key   = util_alloc_string_copy( stringlist_iget(token_list ,1) );
              char * value = stringlist_alloc_joined_substring( token_list , 2 , active_tokens , " ");

              {
                char * filtered_value = subst_list_alloc_filtered_string( config_content_get_define_list( content ) , value);
                config_content_add_define( content , key , filtered_value );
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
              else if (unrecognized == CONFIG_UNRECOGNIZED_ERROR) {
                char * error_message = util_alloc_sprintf("Keyword:%s is not recognized" , kw);
                config_error_add( config_content_get_errors( content ) , error_message );
                free( error_message );
              }
            }

            if (config_has_schema_item(config , kw)) {
              config_schema_item_type * schema_item = config_get_schema_item( config , kw );
              char * config_cwd;
              util_alloc_file_components( config_file , &config_cwd , NULL , NULL );

              if (!config_content_has_item( content , kw ))
                config_content_add_item( content , schema_item , current_path_elm);


              {
                subst_list_type * define_list = config_content_get_define_list( content );
                config_content_item_type * content_item = config_content_get_item( content , config_schema_item_get_kw( schema_item ) );
                config_content_node_type * new_node = config_content_item_set_arg__(define_list , config_content_get_errors( content ) , content_item , token_list , current_path_elm , config_file );
                if (new_node)
                  config_content_add_node( content , new_node );
                
              }
            }
          }
        }
        stringlist_free(token_list);
        free(line_buffer);
      }
    }
    if (validate)
      config_validate(config , content , config_file);
    fclose(stream);
    basic_parser_free( parser );
  }
  free(config_file);
  path_stack_pop( path_stack );
  config_content_pop_path_stack( content );
}












config_content_type * config_parse(config_parser_type * config ,
                                   const char * filename,
                                   const char * comment_string ,
                                   const char * include_kw ,
                                   const char * define_kw ,
                                   const hash_type * pre_defined_kw_map,
                                   config_schema_unrecognized_enum unrecognized_behaviour,
                                   bool validate) {

  config_content_type * content = config_content_alloc( );

  if(pre_defined_kw_map != NULL) {
    hash_iter_type * keys = hash_iter_alloc(pre_defined_kw_map);

    while(!hash_iter_is_complete(keys)) {
      const char * key = hash_iter_get_next_key(keys);
      const char * value = hash_get(pre_defined_kw_map, key);
      config_content_add_define( content , key , value );
    }

    hash_iter_free(keys);
  }
//

  if (util_file_readable( filename )) {
    path_stack_type * path_stack = path_stack_alloc();
    {
      config_content_set_config_file( content , filename );
      config_content_set_invoke_path( content );
      config_parse__(config , content , path_stack , filename , comment_string , include_kw , define_kw , unrecognized_behaviour , validate);
    }
    path_stack_free( path_stack );
  } else {
    char * error_message = util_alloc_sprintf("Could not open file:%s for parsing" , filename);
    config_error_add( config_content_get_errors( content ) , error_message );
    free( error_message );
  }

  if (config_error_count( config_content_get_errors( content ) ) == 0)
    config_content_set_valid( content );

  return content;
}




/*****************************************************************/




/**
   This function adds an alias to an existing item; so that the
   value+++ of an item can be referred to by two different names.
*/


void config_add_alias(config_parser_type * config , const char * src , const char * alias) {
  if (config_has_schema_item(config , src)) {
    config_schema_item_type * item = config_get_schema_item(config , src);
    config_insert_schema_item(config , alias , item , true);
  } else
    util_abort("%s: item:%s not recognized \n",__func__ , src);
}



void config_install_message(config_parser_type * config , const char * kw, const char * message) {
  hash_insert_hash_owned_ref(config->messages , kw , util_alloc_string_copy(message) , free);
}




#include "config_get.c"



/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'conf_new.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <assert.h>
#include <string.h>
#include <util.h>
#include <hash.h>
#include <set.h>
#include <vector.h>
#include <int_vector.h>
#include <conf_new.h>
#include <parser.h>



/******************************************************************************/


/**
  Defining parameters for the file inclusion and tokenizing.
*/
#define __CONF_INCLUDE    "include"
#define __CONF_WHITESPACE " \t\n\r,;"
#define __CONF_COM_START  "--"
#define __CONF_COM_STOP   "\n"
#define __CONF_SPECIAL    "[]{}="
#define __CONF_QUOTERS    "'\""
#define __CONF_DELETE     NULL

#define __CONF_VEC_START "["
#define __CONF_VEC_STOP  "]"
#define __CONF_EXP_START "{"
#define __CONF_EXP_STOP  "}"
#define __CONF_ASSIGN    "="



/**
  Defining numbers for safe run time casting.
*/
#define __CONF_SPEC_ID 132489012
#define __CONF_ITEM_ID 342349032
#define __CONF_ID      314234239



/******************************************************************************/


/**
  TODO

  Data structures should have some parsing info etc.
  Should have a special structure for the error messages.
*/



struct conf_spec_struct
{
  int                    __id;      /** Used for safe run time casting.       */
  char                 * name;      /** Name used to identify this class.     */
  const conf_spec_type * super;     /** Mother. Can be NULL for the root.     */
  hash_type            * specs;     /** Hash of conf_spec_type's.             */
  validator_ftype      * validator; /** Function used to validate a conf_type.*/
};



struct conf_struct
{
  int               __id;           /** Used for safe run time casting.       */
  conf_type       * super;          /** Mother. Can be NULL for the root.     */
  char            * type;           /** Name used to identify the class.      */
  char            * name;           /** Name used to identify the instance.   */
  vector_type     * confs;          /** Vector of conf_type's.                */
  hash_type       * items;          /** Hash of conf_item_type's.             */
};



struct conf_item_struct
{
  int               __id;           /** Used for safe run time casting.       */
  conf_type       * super;          /** Mother. Can not be NULL.              */
  stringlist_type * values;         /** Raw tokens read from file.            */
};



/******************************************************************************/



static
bool strings_are_equal(
  const char * string_a,
  const char * string_b)
{
  if( strcmp(string_a, string_b) == 0)
    return true;
  else
    return false;
}



/**
  Check if a string is equal one of the characters in the special set.
*/
static
bool string_is_special(
  const char * str)
{
  int num_special_chars = strlen(__CONF_SPECIAL);
  if( num_special_chars != 1 )
  {
    /**
      Cannot be in the special set if it's not a single character.
    */
    return false;
  }
  else
  {
    for(int i=0; i<num_special_chars; i++)
    {
      if( str[0] == __CONF_SPECIAL[i])
        return true;
    }
    return false;
  }
}



/**
  Find the realpath of file included from another file. 
  Returns CONF_OK and stores the realpath in *include_file_realpath
  if the realpath can be resolved. If the realopath cannot be resolved,
  the function returns CONF_UNABLE_TO_OPEN_FILE and *include_file_realpath
  is set to NULL.

  The calling scope is responsible for free'ing *include_file_realpath if 
  CONF_OK is returned.
*/
static
int alloc_included_realpath(
  const char  * current_file_realpath,
  const char  * include_file,
  char       ** include_file_realpath
)
{
  assert(current_file_realpath != NULL);
  assert(include_file          != NULL);
  assert(include_file_realpath != NULL);

  bool realpath_ok;
  char * path;

  if( util_is_abs_path(include_file) )
    path = util_alloc_string_copy(include_file);
  else
  {
    util_alloc_file_components(current_file_realpath, &path, NULL, NULL);
    if( path == NULL )
      path = util_alloc_string_copy("/");
    path = util_strcat_realloc(path, "/");
    path = util_strcat_realloc(path, include_file);
  }

  realpath_ok = util_try_alloc_realpath(path);
  if( realpath_ok )
    *include_file_realpath = util_alloc_realpath(path);
  else
    *include_file_realpath = NULL;
                                                      
  free(path);

  if( realpath_ok )
    return CONF_OK;
  else
    return CONF_UNABLE_TO_OPEN_FILE;
}




/**
  Recursively builds a stringlist of tokens from files, and an associated
  stringlist of the source file for each token.

  Do not use this function directly, use create_token_buffer.

  NOTE: This function wants the realpath of the file to tokenize!
*/
static
int create_token_buffer__(
  stringlist_type     ** tokens,      /** Memory to the created pointer.      */
  stringlist_type     ** src_files,   /** Memory to the created pointer.      */
  const set_type       * sources,     /** Real path of files already sourced. */
  const parser_type    * parser   ,   /** Parser to use.                      */
  const char           * filename ,   /** Real path of file to parse.         */
  stringlist_type      * errors       /** Storage for error messages.         */
)
{
  int status;

  stringlist_type * tokens__;
  stringlist_type * src_files__;
  
  /**
    Assert that we've got a realpath.
  */
  assert( util_is_abs_path(filename) );

  /**
    Check if the file has been parsed before.
  */
  if( !set_has_key(sources, filename) )
    status = CONF_OK;
  else
  {
    char * err = util_alloc_sprintf("Circular inclusion of \"%s\".", filename);
    stringlist_append_owned_ref(errors, err);
    status = CONF_CIRCULAR_INCLUDE_ERROR;
  }


  /**
    Check that we can open the file.  
  */
  if( status == CONF_OK && !util_fopen_test(filename, "r") )
  { 
    char * err = util_alloc_sprintf("Unable to open file \"%s\".", filename);
    stringlist_append_owned_ref(errors, err);
    status = CONF_UNABLE_TO_OPEN_FILE;
  }


  if( status != CONF_OK )
  {
    /**
      Create empty tokens__ and src_files__ list to return.
    */
    tokens__    = stringlist_alloc_new();
    src_files__ = stringlist_alloc_new();
  }
  else
  {
    set_type * sources__   = set_copyc(sources);
    set_add_key(sources__, filename);

    tokens__    = parser_tokenize_file(parser , filename, true);
    src_files__ = stringlist_alloc_new(); 

    /**
      Set the sources. Let the first token own the memory.
    */
    for(int i=0; i<stringlist_get_size(tokens__); i++)
    {
      if(i == 0)
        stringlist_append_copy(src_files__, filename);
      else
        stringlist_append_ref(src_files__, stringlist_iget(src_files__, 0));
    }

    /**
      Handle any include statements by recursion.
    */
    {
      int i = stringlist_find_first(tokens__, __CONF_INCLUDE);
      while( i != -1)
      {
        const char * include_file;
        char       * include_file_realpath;
        stringlist_type * included_tokens    = NULL;
        stringlist_type * included_src_files = NULL;

        /**
          Check that the next token actually exists.
        */
        if( i+1 == stringlist_get_size(tokens__) )
        {
          char * err = util_alloc_sprintf("Unexpected end of file in \"%s\". "
                                          "Expected a file name after \"%s\".",
                                          filename, __CONF_INCLUDE);
          stringlist_append_owned_ref(errors, err);
          status = CONF_UNEXPECTED_EOF_ERROR;

          /**
           Delete the last tokens, so that the output is still meaningful.
          */
          stringlist_idel(tokens__, i);
          stringlist_idel(src_files__, i);
          break;
        }

        include_file = stringlist_iget(tokens__, i+1);

        /**
          Check that the file exists.
        */
        status = alloc_included_realpath(filename, 
                                         include_file,
                                         &include_file_realpath);
        if( status == CONF_OK )
        {
          /**
            Recursive call.
          */
          int last_status = create_token_buffer__(&included_tokens,
                                                  &included_src_files,
                                                  sources__, parser,
                                                  include_file_realpath,
                                                  errors);
          if( last_status != CONF_OK)
            status = last_status;

          free(include_file_realpath);
        }
        else
        {
          char * err = util_alloc_sprintf("Unable to open file \"%s\" included "
                                          "from file \"%s\".", 
                                          include_file, filename);
          stringlist_append_owned_ref(errors, err);
          status = CONF_UNABLE_TO_OPEN_FILE;
        }


        /**
          Delete the "include" and filename token. Do the same for src_files__.
        */
        stringlist_idel(tokens__, i+1);
        stringlist_idel(tokens__, i);
        stringlist_idel(src_files__, i+1);
        stringlist_idel(src_files__, i);

        if(included_tokens != NULL && included_src_files != NULL)
        {
          /**
            Replace the deleted items.
          */
          stringlist_insert_stringlist_copy(tokens__, included_tokens, i);
          stringlist_free(included_tokens);
          stringlist_insert_stringlist_copy(src_files__, included_src_files, i);
          stringlist_free(included_src_files);
        }
        

        /**
          Find next include statement.
        */
        i = stringlist_find_first(tokens__, __CONF_INCLUDE);
      }
    }

    set_free(sources__);
  }

  *tokens    = tokens__;
  *src_files = src_files__;
  return status;
}



/**
  Creates a stringlist of tokens suitable for parsing with the config
  parser from a file. All include statements are resolved, and circular
  include statements are detected.

  If no errors are detected CONF_OK will be returned. Otherwise, the last
  detected error will be returned. Error messages are available in *errors.
  
  Note that the recursion does not halt on errors. Thus, the tokens are fully
  useable even though errors may have been encountered.

  The calling scope is responsible for free'ing *tokens, *src_files and 
  *errors when they are no longer needed.
*/
static
int create_token_buffer(
  stringlist_type ** tokens,    /** Memory for the tokens.                    */
  stringlist_type ** src_files, /** Memory for source file for each token.    */
  stringlist_type ** errors,    /** Memory to return error messages in.       */
  const char       * filename   /** Filename to create tokens from.           */
)
{
  int status;
  stringlist_type * tokens__   ; 
  stringlist_type * src_files__;

  stringlist_type * errors__ = stringlist_alloc_new(); 

  if( util_fopen_test(filename, "r") )
  {
    parser_type * parser   = parser_alloc(__CONF_WHITESPACE,
                                          __CONF_QUOTERS,
                                          __CONF_SPECIAL,
                                          __CONF_DELETE,
                                          __CONF_COM_START,
                                          __CONF_COM_STOP);
    
    set_type * sources           = set_alloc_empty();
    char     * realpath_filename = util_alloc_realpath(filename);

    status = create_token_buffer__(&tokens__, &src_files__, sources,
                                   parser, realpath_filename, errors__);

    free(realpath_filename);
    set_free(sources);
    parser_free(parser);
  }
  else
  {
    tokens__    = stringlist_alloc_new();
    src_files__ = stringlist_alloc_new();

    char * err = util_alloc_sprintf("Unable to open file \"%s\".", filename);
    stringlist_append_owned_ref(errors__, err);

    status      = CONF_UNABLE_TO_OPEN_FILE;
  }

  *tokens    = tokens__;
  *src_files = src_files__;
  *errors    = errors__;

  return status;
}



/******************************************************************************/



static
conf_type * conf_alloc_root()
{

  conf_type * conf = util_malloc(sizeof * conf);

  conf->__id  = __CONF_ID;
  conf->super = NULL;
  conf->type  = util_alloc_string_copy("root");
  conf->name  = util_alloc_string_copy("root");

  conf->confs = vector_alloc_new();
  conf->items = hash_alloc();

  return conf;
}



static
conf_type * conf_safe_cast(
  void * conf
)
{
  conf_type * __conf = (conf_type *) conf;
  if( __conf->__id != __CONF_ID )
    util_abort("%s: Internal error. Run time cast failed.\n", __func__);
  return __conf;
}



void conf_free(
  conf_type * conf
)
{
  hash_free(conf->items);
  vector_free(conf->confs);

  free(conf->type);
  free(conf->name);
  free(conf);
}



static
void conf_free__(
  void * conf
)
{
  conf_type * conf__ = conf_safe_cast(conf);
  conf_free(conf__);
}



static
conf_item_type * conf_item_safe_cast(
  void * item
)
{
  conf_item_type * __item = (conf_item_type *) item;
  if( __item->__id != __CONF_ITEM_ID)
    util_abort("%s: Internal error. Run time cast failed.\n", __func__);
  return __item;
}



static
void conf_item_free(
  conf_item_type * item
)
{
  stringlist_free(item->values);
  free(item);
}



static
void conf_item_free__(
  void * item
)
{
  conf_item_type * item__ = conf_item_safe_cast(item);
  conf_item_free(item__);
}



/******************************************************************************/



static
conf_item_type * conf_insert_item(
  conf_type      * conf,
  const char     * name
)
{
  assert(conf != NULL);
  assert(name != NULL);

  conf_item_type * item = util_malloc(sizeof * item );
  item->__id   = __CONF_ITEM_ID;
  item->values = stringlist_alloc_new();

  hash_insert_hash_owned_ref(conf->items, name, item, conf_item_free__);

  return item;
}



static
conf_type * conf_append_child(
  conf_type  * conf,
  const char * type,
  const char * name
)
{
  assert(conf != NULL);
  assert(type != NULL );
  assert(name != NULL );

  conf_type * child = util_malloc(sizeof * child );

  child->__id  = __CONF_ID;
  child->super = conf;
  child->type  = util_alloc_string_copy(type);
  child->name  = util_alloc_string_copy(name);

  child->confs = vector_alloc_new();
  child->items = hash_alloc();

  vector_append_owned_ref(conf->confs, child, conf_free__);

  return child;
}



static
conf_type * conf_get_super(
  conf_type * conf
)
{
  return conf->super;
}



static
void conf_item_append_data_item(
  conf_item_type * item,
  const char     * data
)
{
  stringlist_append_copy(item->values, data);  
}



static
void conf_item_append_data(
  conf_item_type        * item,
  const stringlist_type * data
)
{
  stringlist_append_stringlist_copy(item->values, data);
}



/******************************************************************************/



/**
  Create a shallow copy stringlist of tokens that belong to an item
  starting at *position__. The memory pointed by position__ is updated
  so that it contains the position of the first token AFTER the tokens
  belonging to the item.
*/
static
int get_item_tokens(
  const stringlist_type * tokens,
  stringlist_type      ** item_tokens,
  int                   * position__
)
{
  int status = CONF_OK;

  int item_size  = 0;
  int start      = *position__;
  int position   = *position__;
  int num_tokens = stringlist_get_size(tokens);


  const char * current_token = stringlist_iget(tokens, position);
  if( strings_are_equal(current_token, __CONF_VEC_START) )
  {
    position++;
    start = position;
    bool matched_delimiters = false;
    while( position < num_tokens && !matched_delimiters )
    {
      current_token = stringlist_iget(tokens, position);
      if( strings_are_equal(current_token, __CONF_VEC_STOP) )
      {
        matched_delimiters = true;
        position++;
      }
      else
      {
        item_size++;
        position++;
      }
    }
    
    if( !matched_delimiters )
    {
      /**
        TODO

        This is an error. Should add a message.
      */
      status = CONF_PARSE_ERROR;
    }
  }
  else
  {
    item_size = 1;
    position++;
  }

  *item_tokens = stringlist_alloc_shallow_copy_with_limits(tokens, start,
                                                           item_size);
  *position__ = position;

  return status;
}



static
int get_conf_tokens(
  const stringlist_type * tokens,
  stringlist_type      ** conf_tokens,
  int                   * position__
)
{
  int status = CONF_OK;

  int conf_size  = 0;
  int start      = *position__;
  int position   = *position__;
  int num_tokens = stringlist_get_size(tokens);


  const char * current_token = stringlist_iget(tokens, position);
  if( strings_are_equal(current_token, __CONF_EXP_START) )
  {
    position++;
    start = position;
    int depth = 1;
    while( position < num_tokens && depth > 0 )
    {
      current_token = stringlist_iget(tokens, position);
      if( strings_are_equal(current_token, __CONF_EXP_START) )
      {
        position++;
        conf_size++;
        depth++;
      }
      else if( strings_are_equal(current_token, __CONF_EXP_STOP) )
      {
        position++;
        depth--;
        if(depth > 0)
          conf_size++;
      }
      else
      {
        position++;
        conf_size++;
      }
    }
    
    if( depth > 0 )
    {
      /**
        TODO

        This is an error. Should add a message.
      */
      status = CONF_PARSE_ERROR;
    }
  }
  else
  {
    conf_size = 0;
    position++;
  }

  *conf_tokens = stringlist_alloc_shallow_copy_with_limits(tokens, start,
                                                           conf_size);
  *position__ = position;

  return status;
}



static
int conf_alloc_from_tokens(
  const stringlist_type * tokens,
  stringlist_type       * errors,
  conf_type            ** conf__
)
{
  assert(tokens != NULL);
  assert(errors != NULL);

  typedef enum {IN_ROOT, IN_CLASS, IN_ITEM} PARSER_STATE;
  PARSER_STATE state = IN_ROOT;

  int position   = 0;
  int num_tokens = stringlist_get_size(tokens);

  conf_type * root = conf_alloc_root();

  conf_type      * current_conf  = root;
  conf_item_type * current_item;

  while(position <  num_tokens)
  {
    /**
      On entering this loop, stringlist_iget(tokens, position)
      shall always be a non-special token.
    */

    const char * current_token = stringlist_iget(tokens, position);
    const char * look_ahead_one;
    const char * look_ahead_two;
  }
}
  

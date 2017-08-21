/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'util_env.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "ert/util/build_config.h"

#include <ert/util/util.h>
#include <ert/util/util_env.h>
#include <ert/util/buffer.h>

#ifdef HAVE_POSIX_SETENV
#define PATHVAR_SPLIT ":"

void util_unsetenv( const char * variable ) {
  unsetenv( variable );
}

void util_setenv( const char * variable , const char * value) {
  int overwrite = 1;
  setenv( variable , value , overwrite );
}

#else

#include <Windows.h>

#define PATHVAR_SPLIT ";"
void util_setenv( const char * variable , const char * value) {
  SetEnvironmentVariable( variable , NULL );
}

void util_unsetenv( const char * variable ) {
  util_setenv( variable , NULL );
}
#endif

/**
   Will return a NULL terminated list char ** of the paths in the PATH
   variable. 
*/

char ** util_alloc_PATH_list() {
  char ** path_list = NULL;
  char *  path_env  = getenv("PATH");
  if (path_env != NULL) {
    int     path_size;
    
    util_split_string(path_env , PATHVAR_SPLIT , &path_size , &path_list);
    path_list = util_realloc( path_list , (path_size + 1) * sizeof * path_list);
    path_list[path_size] = NULL;
  } else {
    path_list = util_malloc( sizeof * path_list);
    path_list[0] = NULL;
  }
  return path_list;
}

/**
   This function searches through the content of the (currently set)
   PATH variable, and allocates a string containing the full path
   (first match) to the executable given as input. 

   * If the entered executable already is an absolute path, a copy of
     the input is returned *WITHOUT* consulting the PATH variable (or
     checking that it exists).

   * If the executable starts with "./" getenv("PWD") is prepended. 

   * If the executable is not found in the PATH list NULL is returned.
*/
   

char * util_alloc_PATH_executable(const char * executable) {
  if (util_is_abs_path(executable)) {
    if (util_is_executable(executable))
      return util_alloc_string_copy(executable);
    else
      return NULL;
  } else if (strncmp(executable , "./" , 2) == 0) {
    char * cwd = util_alloc_cwd();
    char * path = util_alloc_filename(cwd , &executable[2] , NULL);

    /* The program has been invoked as ./xxxx */
    if (!(util_is_file(path) && util_is_executable( path ))) {
      free( path );
      path = NULL;
    }
    free( cwd );

    return path;
  } else {
    char * full_path  = NULL;
    char ** path_list = util_alloc_PATH_list();
    int ipath = 0;

    while (true) {
      if (path_list[ipath] != NULL)  {
        char * current_attempt = util_alloc_filename(path_list[ipath] , executable , NULL);
      
        if ( util_is_file( current_attempt ) && util_is_executable( current_attempt )) {
          full_path = current_attempt;
          break;
        } else {
          free(current_attempt);
          ipath++;
        }
      } else
        break;
    }
    
    util_free_NULL_terminated_stringlist(path_list);
    return full_path;
  }
}





/**
   This function updates an environment variable representing a path,
   before actually updating the environment variable the current value
   is checked, and the following rules apply:
   
   1. If @append == true, and @value is already included in the
      environment variable; nothing is done.

   2. If @append == false, and the variable already starts with
      @value, nothing is done.

   A pointer to the updated(?) environment variable is returned.
*/

const char * util_update_path_var(const char * variable, const char * value, bool append) {
  const char * current_value = getenv( variable );
  if (current_value == NULL)
    /* The (path) variable is not currently set. */
    util_setenv( variable , value );
  else {
    bool    update = true; 

    {
      char ** path_list;
      int     num_path;
      util_split_string( current_value , ":" , &num_path , &path_list);
      if (append) {
        int i;
        for (i = 0; i < num_path; i++) {
          if (util_string_equal( path_list[i] , value)) 
            update = false;                            /* The environment variable already contains @value - no point in appending it at the end. */
        } 
      } else {
        if (util_string_equal( path_list[0] , value)) 
          update = false;                              /* The environment variable already starts with @value. */
      }
      util_free_stringlist( path_list , num_path );
    }
    
    if (update) {
      char  * new_value;
      if (append)
        new_value = util_alloc_sprintf("%s:%s" , current_value , value);
      else
        new_value = util_alloc_sprintf("%s:%s" , value , current_value);
      util_setenv( variable , new_value );
      free( new_value );
    }
    
  }
  return getenv( variable );
}



/**
   This is a thin wrapper around the setenv() call, with the twist
   that all $VAR expressions in the @value parameter are replaced with
   getenv() calls, so that the function call:

      util_setenv("PATH" , "$HOME/bin:$PATH")

   Should work as in the shell. If the variables referred to with ${}
   in @value do not exist the literal string, i.e. '$HOME' is
   retained. 

   If @value == NULL a call to unsetenv( @variable ) will be issued.
*/

const char * util_interp_setenv( const char * variable , const char * value) {
  char * interp_value = util_alloc_envvar( value );
  if (interp_value != NULL) {
    util_setenv( variable , interp_value);
    free( interp_value );
  } else
    util_unsetenv( variable );
  
  return getenv( variable );
}



/**
   This function will take a string as input, and then replace all if
   $VAR expressions with the corresponding environment variable. If
   the environament variable VAR is not set, the string literal $VAR
   is retained. The return value is a newly allocated string. 

   If the input value is NULL - the function will just return NULL;
*/


char * util_alloc_envvar( const char * value ) {
  if (value == NULL)
    return NULL;
  else {
    buffer_type * buffer = buffer_alloc( 1024 );               /* Start by filling up a buffer instance with 
                                                                  the current content of @value. */
    buffer_fwrite_char_ptr( buffer , value );
    buffer_rewind( buffer );
    
    
    while (true) {
      if (buffer_strchr( buffer , '$')) {
        const char * data = buffer_get_data( buffer );
        int offset        = buffer_get_offset( buffer ) + 1;    /* Points at the first character following the '$' */
        int var_length = 0;
        
        /* Find the length of the variable name */
        while (true) {
          char c;
          c = data[offset + var_length];
          if (!(isalnum( c ) || c == '_'))      /* Any character which is NOT in the set [a-Z,0-9_] marks the end of the variable. */
            break;             
          
          if (c == '\0')                        /* The end of the string. */
            break;
          
          var_length += 1;
        }

        {
          char * var_name        = util_alloc_substring_copy( data , offset - 1 , var_length + 1);  /* Include the leading $ */
          const char * var_value = getenv( &var_name[1] );
          
          if (var_value != NULL)
            buffer_search_replace( buffer , var_name , var_value);                                      /* The actual string replacement. */
          else  
            buffer_fseek( buffer , var_length , SEEK_CUR );                                      /* The variable is not defined, and we leave the $name. */
          
          free( var_name );
        }
      } else break;  /* No more $ to replace */
    }
    
    
    buffer_shrink_to_fit( buffer );
    {
      char * expanded_value = buffer_get_data( buffer );
      buffer_free_container( buffer );
      return expanded_value;
    }
  }
}

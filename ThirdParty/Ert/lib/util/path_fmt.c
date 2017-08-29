/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'path_fmt.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "ert/util/build_config.h"
#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/util/node_ctype.h>
#include <ert/util/path_fmt.h>

/**
The basic idea of the path_fmt_type is that it should be possible for
a user to specify an arbirtrary path *WITH* embedded format
strings. It is implemented with the the help av variable length
argument lists. This has the following disadvantages:

 o The code gets ugly - really ugly.

 o It is difficult to provide type-safety on user input.

Example:


path_fmt_type * path_fmt = path_fmt_alloc_directory_fmt("/tmp/ECLIPSE/%s/Run-%d");


Here we have allocated a path_fmt instance which will require two
additional arguments when a full path is created, a string for the
"%s" placeholder and an integer for the %d placeholder:

char * path = path_fmt_alloc(path_fmt , "BaseCase" , 67);

=> path = /tmp/ECLIPSE/Basecase/Run-67

*/

#define PATH_FMT_ID 7519200
   

struct path_fmt_struct {
  UTIL_TYPE_ID_DECLARATION;
  char *fmt;
  char *file_fmt;
  bool  is_directory;
};


static UTIL_SAFE_CAST_FUNCTION( path_fmt , PATH_FMT_ID)

void path_fmt_reset_fmt(path_fmt_type * path , const char * fmt) {
  path->fmt = util_realloc_string_copy(path->fmt , fmt);
  if (path->is_directory) 
    path->file_fmt = util_alloc_sprintf("%s/%%s" , fmt);
}



static path_fmt_type * path_fmt_alloc__(const char * fmt , bool is_directory) {
  path_fmt_type * path = (path_fmt_type*)util_malloc(sizeof * path );
  UTIL_TYPE_ID_INIT(path , PATH_FMT_ID);
  path->fmt            = NULL;
  path->file_fmt       = NULL;
  path->is_directory   = is_directory;
  
  path_fmt_reset_fmt(path , fmt);
  return path;
}


/**

  This function is used to allocate a path_fmt instance which is
  intended to hold a directory, if the second argument is true, the
  resulting directory will be automatically created when
  path_fmt_alloc_path() is later invoked.

  Example:
  -------
  path_fmt_type * path_fmt = path_fmt_alloc_directory_fmt("/tmp/scratch/member%d/%d.%d" , true);
  ....
  ....
  char * path = path_fmt_alloc_path(path_fmt , 10 , 12 , 15);
  char * file = path_fmt_alloc_file(path_fmt ,  8 , 12 , 17, "SomeFile");

  After the two last function calls we will have:

   o path = "/tmp/scratch/member10/12.15" - and this directory has
     been created. 

   o file = "/tmp/scratch/member8/12.17/SomeFile - and the directory
     /tmp/scratch/member8/12.17 has been created.

     
  Observe that the functionality is implemented with the help av
  variable length argument lists, and **NO** checking of argument list
  versus format string is performed.
*/


path_fmt_type * path_fmt_alloc_directory_fmt(const char * fmt) {
  return path_fmt_alloc__(fmt , true);
}



/* 
   Most general. Can afterwards be used to allocate strings
   representing both directories and files.
*/

path_fmt_type * path_fmt_alloc_path_fmt(const char * fmt) {
  return path_fmt_alloc__(fmt , false );
}



/**
   Present the user with a prompt, and reads format specifier string
   from stdin. Currently not possible to specify argument types.
*/

path_fmt_type * path_fmt_scanf_alloc(const char * prompt , int types , const node_ctype * type_list, bool is_directory) {
  char * fmt;
  fmt = util_scanf_alloc_string(prompt);
  return path_fmt_alloc__(fmt , is_directory);
}

path_fmt_type * path_fmt_copyc(const path_fmt_type *path) {
  path_fmt_type *new_path = path_fmt_alloc__(path->fmt , path->is_directory);
  return new_path;
}

char * path_fmt_alloc_path_va(const path_fmt_type * path ,bool auto_mkdir,  va_list ap) {
 char * new_path = util_alloc_sprintf_va(path->fmt , ap );
 if (auto_mkdir)
   if (! util_is_directory(new_path) )
     util_make_path(new_path);
  return new_path;
}


char * path_fmt_alloc_path(const path_fmt_type * path , bool auto_mkdir , ...) {
  char * new_path;
  va_list ap;
  va_start(ap , auto_mkdir);
  new_path = path_fmt_alloc_path_va(path ,auto_mkdir ,  ap);
  va_end(ap);
  return new_path;
}



/**
  This function is used to allocate a filename (full path) from a
  path_fmt instance: 

  Eaxample:

    path_fmt_type * path_fmt = path_fmt_alloc_directory("/tmp/path%d/X.%02d");
    char * file = path_fmt_alloc_file(path_fmt , 100 , 78 , "SomeFile.txt")

  This will allocate the filename: /tmp/path100/X.78/SomeFile.txt; if
  it does not already exist, the underlying directory will be
  created. Observe that there is nothing special about the filename
  argument (i.e. 'SomeFile.txt' in the current example), it is just
  the last argument to the path_fmt_alloc_file() function call -
  however it must be a string; i.e. if you are making a purely numeric
  filename you must convert to a string.

  Observe that the handling of the variable length argument lists gets
  seriously ugly.

  -----------------------------------------------------------------
  
  If auto_mkdir == true the function behaves in two different ways
  depending on whether the path_instance was allocated as a directory
  or as a path:

   * [Directory]: When the path_fmt instance was allocated as a
     directory, "/%s" format decriptor will be appended to the format.

   * [Path]: The resulting string will be split on "/", and the path
     component will be created.
*/


char * path_fmt_alloc_file(const path_fmt_type * path , bool auto_mkdir , ...) {
  if (path->is_directory) {
    char * filename;
    va_list tmp_va , ap;
    va_start(ap , auto_mkdir);
    UTIL_VA_COPY(tmp_va , ap);
    filename = util_alloc_sprintf_va( path->file_fmt , tmp_va );
    if (auto_mkdir) {
      const char * __path = util_alloc_sprintf_va( path->fmt , tmp_va );
      if (! util_is_directory(__path)) 
        util_make_path( __path );
      free((char *) __path );
    }
    va_end(ap);
    return filename;
  } else {

    char * filename;
    va_list tmp_va , ap;
    va_start(ap , auto_mkdir);
    UTIL_VA_COPY(tmp_va , ap);
    filename = util_alloc_sprintf_va( path->fmt , tmp_va );
    if (auto_mkdir) {
      char * __path;
      util_alloc_file_components(filename , &__path , NULL , NULL);
      util_make_path(__path);
      free(__path);
    }
    va_end(ap);

    return filename;
  }
}



/**
   This function is used to assert that the format in a path_fmt
   instance is according to specification. What is checked is that the
   format string contains %d, %lfg and %s in as specified in the
   input_types vector.

   Observe that %s is mapped to void_pointer - as the node_ctype does not
   have typed pointers.
*/


/*
void path_fmt_assert_fmt(const path_fmt_type * path , int num_input , const node_ctype * input_types) {
  int input_nr = 0;
  int char_nr  = 0;
  do {
    if (path->fmt[char_nr] == '%') {

    }
  }
}
*/

/*
  fmt == NULL
  -----------
  The function will return NULL, possibly freeing the incoming
  path_fmt instance if that is different from NULL.


  fmt != NULL
  -----------
  The current path_fmt instance will be updated, or a new created. The
  new/updated path_fmt instance is returned.
*/

path_fmt_type * path_fmt_realloc_path_fmt( path_fmt_type * path_fmt, const char * fmt ) {
  if (fmt == NULL) {
    if (path_fmt != NULL)
      path_fmt_free( path_fmt );
    return NULL;
  } else {
    if (path_fmt == NULL)
      return path_fmt_alloc_path_fmt( fmt );
    else {
      path_fmt_reset_fmt( path_fmt , fmt );
      return path_fmt;
    }
  }
}


const char * path_fmt_get_fmt(const path_fmt_type * path) {
  if (path == NULL)
    return NULL;
  else
    return path->fmt;
}


void path_fmt_free(path_fmt_type * path) {
  free(path->fmt);
  if (path->is_directory)
    free(path->file_fmt);
  free(path);
}


void path_fmt_free__( void * arg ) {
  path_fmt_type * path_fmt = path_fmt_safe_cast( arg );
  path_fmt_free( path_fmt );
}

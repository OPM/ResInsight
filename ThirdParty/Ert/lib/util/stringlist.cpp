/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'stringlist.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdio.h>
#include <stdlib.h>

#include <ert/util/ert_api_config.hpp>
#ifdef ERT_HAVE_OPENDIR
#include <sys/types.h>
#include <dirent.h>
#endif

#ifdef ERT_HAVE_GLOB
#include <glob.h>
#else
#include <Windows.h>
#endif

#include <ert/util/util.h>
#include <ert/util/stringlist.hpp>
#include <ert/util/vector.hpp>

#define STRINGLIST_TYPE_ID 671855

/**
   This file implements a very thin wrapper around a list (vector) of
   strings, and the total number of strings. It is mostly to avoid
   sending both argc and argv.

   Most of the functionality is implemented through vector.c and
   stateless functions in util.c
*/

#ifdef __cplusplus
extern "C" {
#endif

struct stringlist_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type * strings;
};




static void stringlist_fprintf__(const stringlist_type * stringlist, const char * sep , FILE * stream) {
  int length = vector_get_size( stringlist->strings );
  if (length > 0) {
    int i;
    for (i=0; i < length - 1; i++) {
      const char * s = stringlist_iget(stringlist , i);
      fprintf(stream , "%s%s", s  , sep);
    }

    fprintf(stream , "%s", stringlist_iget( stringlist , length - 1 ));
  }
}


void stringlist_fprintf(const stringlist_type * stringlist, const char * sep , FILE * stream) {
  stringlist_fprintf__(stringlist , sep , stream);
}


void stringlist_fprintf_fmt(const stringlist_type * stringlist, const stringlist_type * fmt_list , FILE * stream) {
  if (stringlist_get_size(stringlist) == stringlist_get_size( fmt_list )) {
    int i;
    for (i=0; i < stringlist_get_size( stringlist); i++)
      fprintf(stream , stringlist_iget( fmt_list , i) , stringlist_iget( stringlist , i ));
  } util_abort("%s: length of stringlist:%d   length of fmt_list:%d - must be equal \n",__func__ , stringlist_get_size( stringlist ) , stringlist_get_size( fmt_list ));
}





/**
   This function appends a copy of s into the stringlist.
*/
void stringlist_append_copy(stringlist_type * stringlist , const char * s) {
  if (s)
    vector_append_buffer(stringlist->strings , s , strlen(s) + 1);
  else
    vector_append_ref(stringlist->strings, NULL );
}

/*****************************************************************/

void stringlist_iset_copy(stringlist_type * stringlist , int index , const char * s) {
  vector_iset_buffer(stringlist->strings , index , s , strlen(s) + 1);
}

void stringlist_iset_ref(stringlist_type * stringlist , int index , const char * s) {
  vector_iset_ref(stringlist->strings , index , s);
}

void stringlist_iset_owned_ref(stringlist_type * stringlist , int index , const char * s) {
  vector_iset_owned_ref(stringlist->strings , index , s , free);
}

/*****************************************************************/

void stringlist_insert_copy(stringlist_type * stringlist , int index , const char * s) {
  vector_insert_buffer(stringlist->strings , index , s , strlen(s) + 1);
}

void stringlist_insert_ref(stringlist_type * stringlist , int index , const char * s) {
  vector_insert_ref(stringlist->strings , index , s);
}

void stringlist_insert_owned_ref(stringlist_type * stringlist , int index , const char * s) {
  vector_insert_owned_ref(stringlist->strings , index , s , free);
}




static stringlist_type * stringlist_alloc_empty( bool alloc_vector ) {
  stringlist_type * stringlist = (stringlist_type*)util_malloc(sizeof * stringlist );
  UTIL_TYPE_ID_INIT( stringlist , STRINGLIST_TYPE_ID);

  if (alloc_vector)
    stringlist->strings = vector_alloc_new();
  else
    stringlist->strings = NULL;

  return stringlist;
}




stringlist_type * stringlist_alloc_new() {
  return stringlist_alloc_empty( true );
}



stringlist_type * stringlist_alloc_argv_copy(const char ** argv , int argc) {
  int iarg;
  stringlist_type * stringlist = stringlist_alloc_empty( true);
  for (iarg = 0; iarg < argc; iarg++)
    stringlist_append_copy( stringlist , argv[iarg]);

  return stringlist;
}





/*
  Can not use vector_deep copy - because we might not have the
  constructor registered, in the node_data instance; but in this case
  we know the copy constructor.
*/


stringlist_type * stringlist_alloc_deep_copy_with_limits(const stringlist_type * src, int offset , int num_strings) {
  stringlist_type * copy = stringlist_alloc_empty( true );
  int i;
  for (i = 0; i < num_strings; i++)
    stringlist_append_copy( copy , stringlist_iget( src , i + offset));
  return copy;
}


stringlist_type * stringlist_alloc_deep_copy_with_offset(const stringlist_type * src, int offset) {
  return stringlist_alloc_deep_copy_with_limits( src , offset , stringlist_get_size( src ) - offset );
}

stringlist_type * stringlist_alloc_deep_copy(const stringlist_type * src) {
  return stringlist_alloc_deep_copy_with_offset( src , 0 );
}




void stringlist_append_stringlist_copy(stringlist_type * stringlist , const stringlist_type * src) {
  int i;
  for (i = 0; i < stringlist_get_size( src ); i++)
    stringlist_append_copy(stringlist , stringlist_iget(src , i));
}



/**
  Insert a copy of a stringlist in some position.

  Can probably be made more efficient.
*/

void stringlist_insert_stringlist_copy(stringlist_type * stringlist, const stringlist_type * src, int pos) {
  int size_old  = stringlist_get_size(stringlist);

  /** Cannot use assert_index here. */
  if(pos < 0 || pos > size_old)
    util_abort("%s: Position %d is out of bounds. Min: 0 Max: %d\n", pos, size_old);
  {
  stringlist_type * start = stringlist_alloc_new();
  stringlist_type * end   = stringlist_alloc_new();
  stringlist_type * newList   = stringlist_alloc_new();
  int i;

  for( i=0; i<pos; i++)
    stringlist_append_copy(start, stringlist_iget(stringlist, i));

  for( i=pos; i<size_old; i++)
    stringlist_append_copy(end  , stringlist_iget(stringlist, i));

  stringlist_append_stringlist_copy(newList, start);
  stringlist_append_stringlist_copy(newList, src  );
  stringlist_append_stringlist_copy(newList, end  );

  stringlist_clear(stringlist);
  stringlist_append_stringlist_copy(stringlist, newList);

  stringlist_free(newList);
  stringlist_free(start);
  stringlist_free(end);
  }
}

void stringlist_deep_copy( stringlist_type * target , const stringlist_type * src) {
  stringlist_clear( target );
  {
    int i;
    for ( i=0; i < stringlist_get_size( src ); i++)
      stringlist_append_copy( target , stringlist_iget( src , i ));
  }
}



/**
    Frees all the memory contained by the stringlist.
*/
void stringlist_clear(stringlist_type * stringlist) {
  vector_clear( stringlist->strings );
}


void stringlist_free(stringlist_type * stringlist) {
  stringlist_clear(stringlist);
  vector_free(stringlist->strings);
  free(stringlist);
}


static UTIL_SAFE_CAST_FUNCTION(stringlist , STRINGLIST_TYPE_ID);
       UTIL_IS_INSTANCE_FUNCTION(stringlist , STRINGLIST_TYPE_ID)

void stringlist_free__(void * __stringlist) {
  stringlist_type * stringlist = stringlist_safe_cast(__stringlist);
  stringlist_free( stringlist );
}


void stringlist_idel(stringlist_type * stringlist , int index) {
  vector_idel( stringlist->strings , index);
}


char * stringlist_pop( stringlist_type * stringlist) {
  return (char*)vector_pop_back( stringlist->strings );
}


const char * stringlist_iget(const stringlist_type * stringlist , int index) {
  return (const char*)vector_iget(stringlist->strings ,index);
}

const char * stringlist_front(const stringlist_type * stringlist) {
  return (const char*)vector_iget(stringlist->strings , 0);
}

const char * stringlist_back(const stringlist_type * stringlist) {
  return (const char*)vector_iget(stringlist->strings , vector_get_size( stringlist->strings ) - 1);
}


int stringlist_iget_as_int( const stringlist_type * stringlist , int index , bool * valid) {
  const char * string_value = stringlist_iget( stringlist , index );
  int value = -1;

  if (valid != NULL)
    *valid = false;

  if (util_sscanf_int(string_value , &value))
    if (valid != NULL)
      *valid = true;

  return value;
}


double stringlist_iget_as_double( const stringlist_type * stringlist , int index , bool * valid) {
  const char * string_value = stringlist_iget( stringlist , index );
  double value = -1.0;

  if (valid != NULL)
    *valid = false;

  if (util_sscanf_double(string_value , &value))
    if (valid != NULL)
      *valid = true;

  return value;
}


bool stringlist_iget_as_bool( const stringlist_type * stringlist, int index, bool * valid) {
  const char * string_value = stringlist_iget( stringlist, index);
  bool value = false;

  if (valid != NULL)
    *valid = false;

  if (util_sscanf_bool(string_value , &value))
    if (valid != NULL)
      *valid = true;

  return value;
}

const char * stringlist_get_last( const stringlist_type * stringlist ) {
  return (const char*)vector_get_last( stringlist->strings );
}


bool stringlist_iequal( const stringlist_type * stringlist , int index, const char * s ) {
  return util_string_equal( stringlist_iget( stringlist , index ) , s);
}


/**
   Will return NULL if you ask for something beyond the limits of the
   stringlist (will die on negative index - that is NEVER OK).
*/
const char * stringlist_safe_iget( const stringlist_type * stringlist , int index) {
  if (index < 0)
    util_abort("%s: negative index:%d is NOT allowed \n",__func__ , index);

  if (index < stringlist_get_size( stringlist ))
    return stringlist_iget( stringlist , index);
  else
    return NULL;
}



char * stringlist_iget_copy(const stringlist_type * stringlist , int index) {
  return util_alloc_string_copy(stringlist_iget(stringlist , index));
}


int stringlist_get_size(const stringlist_type * stringlist) {
  return vector_get_size(stringlist->strings);
}


/*
  Return NULL if the list has zero entries.
*/
static char ** stringlist_alloc_char__(const stringlist_type * stringlist, bool deep_copy) {
  char ** strings = NULL;
  int size = stringlist_get_size( stringlist );
  if (size > 0) {
    int i;
    strings = (char**)util_calloc(size , sizeof * strings );
    for (i = 0; i <size; i++) {
      if (deep_copy)
        strings[i] = stringlist_iget_copy( stringlist , i);
      else
        strings[i] = (char *) stringlist_iget( stringlist , i);
    }
  }
  return strings;
}

char ** stringlist_alloc_char_copy(const stringlist_type * stringlist) {
  return stringlist_alloc_char__( stringlist , true );
}


char ** stringlist_alloc_char_ref(const stringlist_type * stringlist) {
  return stringlist_alloc_char__( stringlist , false );
}




/**
    Scans the stringlist (linear scan) to see if it contains (at
    least) one occurence of 's'. Will never return true if the input
    string @s equals NULL, altough the stringlist itself can contain
    NULL elements.
*/

bool stringlist_contains(const stringlist_type * stringlist , const char * s) {
  int  size     = stringlist_get_size( stringlist );
  int  index    = 0;
  bool contains = false;

  while ((index < size) && (!contains)) {
    const char * istring = stringlist_iget(stringlist , index);
    if (istring != NULL)
      if (strcmp(istring , s) == 0) contains = true;
    index++;
  }

  return contains;
}



/**
  Finds the indicies of the entries matching 's'.
*/
int_vector_type * stringlist_find(const stringlist_type * stringlist, const char * s) {
  int_vector_type * indicies = int_vector_alloc(0, -1);
  int  size     = stringlist_get_size( stringlist );
  int  index    = 0;

  while (index < size ) {
    const char * istring = stringlist_iget(stringlist , index);
    if (istring != NULL)
      if (strcmp(istring , s) == 0)
        int_vector_append(indicies, index);
    index++;
  }
  return indicies;
}


/**
  Find the index of the first index matching 's'.
  Returns -1 if 's' cannot be found.
*/
int stringlist_find_first(const stringlist_type * stringlist, const char * s) {
  bool found = false;
  int size   = stringlist_get_size( stringlist );
  int index  = 0;

  while( index < size && !found )
  {
    const char * istring = stringlist_iget(stringlist , index);
    if (istring != NULL)
      if (strcmp(istring , s) == 0)
      {
        found = true;
        break;
      }
    index++;
  }

  if(found)
    return index;
  else
    return -1;
}



bool stringlist_equal(const stringlist_type * s1 , const stringlist_type *s2) {
  int size1 = stringlist_get_size( s1 );
  int size2 = stringlist_get_size( s2 );
  if (size1 == size2) {
    bool equal = true;
    int i;
    for ( i = 0; i < size1; i++) {
      if (strcmp(stringlist_iget(s1 , i) , stringlist_iget(s2 , i)) != 0) {
        equal = false;
        break;
      }
    }
    return equal;
  } else
    return false;
}


/**
   The interval is halfopen: [start_index , end_index).
*/

/* Based on buffer?? */

char * stringlist_alloc_joined_substring( const stringlist_type * s , int start_index , int end_index , const char * sep ) {
  if (start_index >= stringlist_get_size( s ))
    return util_alloc_string_copy("");
  {
    char * string = NULL;
    int i;

    /* Start with allocating a string long enough to hold all the substrings. */
    {
      int sep_length   = strlen( sep );
      int total_length = 0;
      for (i=start_index; i < end_index; i++)
        total_length += (strlen(stringlist_iget( s , i)) + sep_length);

      total_length += (1 - sep_length);
      string    = (char*)util_malloc( total_length * sizeof * string );
      string[0] = '\0';
    }

    for (i = start_index; i < end_index; i ++) {
      strcat( string , stringlist_iget( s , i));
      if (i < (end_index - 1))
        strcat( string , sep );
    }

    return string;
  }
}


char * stringlist_alloc_joined_string(const stringlist_type * s , const char * sep) {
  return stringlist_alloc_joined_substring( s , 0 , stringlist_get_size( s ) , sep );
}

/**
   This function will allocate a stringlist instance based on
   splitting the input string. If the input string is NULL the
   function will return a stringlist instance with zero elements.

   Observe that the splitting is based on __ANY__ character in @sep;
   NOT the full exact string @sep.

   The newly allocated stringlist will take ownership of the strings
   in the list. The actual functionality is in the util_split_string()
   function.
*/



stringlist_type * stringlist_alloc_from_split( const char * input_string , const char * sep ) {
  stringlist_type * slist = stringlist_alloc_new();
  if (input_string != NULL) {
    char ** items;
    int     num_items , i;
    util_split_string( input_string , sep , &num_items , &items);
    for ( i =0; i < num_items; i++)
      stringlist_append_copy( slist , items[i] );
    util_free_stringlist( items , num_items );
  }
  return slist;
}

/*****************************************************************/



void stringlist_fwrite(const stringlist_type * s, FILE * stream) {
  int i;
  int size = stringlist_get_size( s );
  util_fwrite_int( size , stream);
  for (i=0; i < size; i++)
    util_fwrite_string(stringlist_iget(s , i) , stream);
}

/*
   When a stringlist is loaded from file the current content of the
   stringlist is discarded; and the stringlist becomes the owner of
   all the data read in.
*/
void  stringlist_fread(stringlist_type * s, FILE * stream) {
  int size = util_fread_int(stream);
  int i;
  stringlist_clear(s);
  for (i=0; i < size; i++) {
    char * tmp = util_fread_alloc_string(stream);
    stringlist_append_copy( s , tmp);
    free(tmp);
  }
}





stringlist_type * stringlist_fread_alloc(FILE * stream) {
  stringlist_type * s = stringlist_alloc_empty( true );
  stringlist_fread(s , stream);
  return s;
}


static int strcmp__(const void * __s1, const void * __s2) {
  const char * s1 = (const char *) __s1;
  const char * s2 = (const char *) __s2;
  return strcmp( s1, s2);
}



/**
   Will sort the stringlist inplace. The prototype of the comparison
   function is

     int (cmp) (const void * , const void *);

   i.e. ths strings are implemented as (void *). If string_cmp == NULL
   the sort function will use the ordinary strcmp() function for
   comparison.
*/


void stringlist_sort(stringlist_type * s , string_cmp_ftype * string_cmp)
{
  if (string_cmp == NULL)
    vector_sort( s->strings , strcmp__ );
  else
    vector_sort( s->strings , string_cmp );
}


void stringlist_python_sort( stringlist_type * s , int cmp_flag) {
  if (cmp_flag == 0)
    stringlist_sort( s , NULL);
  else if (cmp_flag == 1)
    stringlist_sort( s , (string_cmp_ftype *) util_strcmp_int);
  else if (cmp_flag == 2)
    stringlist_sort( s , (string_cmp_ftype *) util_strcmp_float);
  else
    util_abort("%s: unrecognized cmp_flag:%d \n",__func__ , cmp_flag );
}


void stringlist_reverse( stringlist_type * s ) {
  vector_inplace_reverse( s->strings );
}


/*****************************************************************/

/*
  This function uses the stdlib function glob() to select file/path
  names matching a pattern. The stringlist is cleared when the
  function starts.
*/

#ifdef ERT_HAVE_GLOB
int stringlist_select_matching(stringlist_type * names , const char * pattern) {
  int match_count = 0;
  stringlist_clear( names );

  {
    size_t i;
    glob_t * pglob = (glob_t*)util_malloc( sizeof * pglob );
    int glob_flags = 0;
    glob( pattern , glob_flags , NULL , pglob);
    match_count = pglob->gl_pathc;
    for (i=0; i < pglob->gl_pathc; i++)
      stringlist_append_copy( names , pglob->gl_pathv[i] );
    globfree( pglob );  /* Only frees the _internal_ data structures of the pglob object. */
    free( pglob );
  }
  return match_count;
}
#endif


int stringlist_select_matching_files(stringlist_type * names , const char * path , const char * file_pattern) {
#ifdef ERT_HAVE_GLOB
  char * pattern  = util_alloc_filename( path , file_pattern , NULL );
  int match_count = stringlist_select_matching( names , pattern );
  free( pattern );
  return match_count;
#else
  {
    WIN32_FIND_DATA file_data;
    HANDLE          file_handle;
    char * pattern  = util_alloc_filename( path , file_pattern , NULL );

    stringlist_clear( names );
    file_handle = FindFirstFile( pattern , &file_data );
    if (file_handle != INVALID_HANDLE_VALUE) {
      do {
        char * full_path = util_alloc_filename( path , file_data.cFileName , NULL);
        stringlist_append_copy( names , full_path );
        free( full_path );
      } while (FindNextFile( file_handle , &file_data) != 0);
    }
    FindClose( file_handle );
    free( pattern );

    return stringlist_get_size( names );
  }
#endif
}


int stringlist_select_files(stringlist_type * names, const char * path, file_pred_ftype * predicate, const void * pred_arg) {
  stringlist_clear(names);
  char * path_arg = path ? util_alloc_string_copy(path) : util_alloc_cwd();

#ifdef ERT_HAVE_OPENDIR
  DIR * dir = opendir(path_arg);
  if (!dir) {
    free(path_arg);
    return 0;
  }

  while (true) {
    struct dirent * entry = readdir(dir);
    if (!entry)
      break;

    if (util_string_equal(entry->d_name, "."))
      continue;

    if (util_string_equal(entry->d_name, ".."))
      continue;

    if (predicate && !predicate(entry->d_name, pred_arg))
      continue;

    {
      char * fname = util_alloc_filename(path, entry->d_name, NULL);
      stringlist_append_copy(names, fname);
      free(fname);
    }
  }

  closedir(dir);

#else

  WIN32_FIND_DATA file_data;
  HANDLE          file_handle;
  char * pattern  = util_alloc_filename( path_arg , "*", NULL );

  file_handle = FindFirstFile( pattern , &file_data );
  if (file_handle != INVALID_HANDLE_VALUE) {
    do {
      if (util_string_equal(file_data.cFileName, "."))
        continue;

      if (util_string_equal(file_data.cFileName, ".."))
        continue;

      if (predicate && !predicate(file_data.cFileName, pred_arg))
        continue;
      {
        char * tmp_fname = util_alloc_filename(path, file_data.cFileName, NULL);
        stringlist_append_copy(names, tmp_fname);
        free(tmp_fname);
      }
    } while (FindNextFile( file_handle , &file_data) != 0);
    FindClose( file_handle );
  }
  free( pattern );

#endif

  free(path_arg);
  return stringlist_get_size(names);
}


int stringlist_append_matching_elements(stringlist_type * target , const stringlist_type * src , const char * pattern) {
      int ielm;
    int match_count = 0;
    for (ielm = 0; ielm < stringlist_get_size( src ); ielm++) {
      const char * item = stringlist_iget( src , ielm );
      if (util_fnmatch( pattern , item ) == 0) {
        stringlist_append_copy( target , item );
        match_count++;
      }
    }
    return match_count;
}

int stringlist_select_matching_elements(stringlist_type * target , const stringlist_type * src , const char * pattern) {
  stringlist_clear( target );
  return stringlist_append_matching_elements( target , src , pattern );
}


static int void_strcmp(const void* s1, const void *s2) {
  return strcmp((char*)s1, (char*)s2);
}

bool stringlist_unique(const stringlist_type * stringlist )
{
  bool unique = true;
  stringlist_type * cpy = stringlist_alloc_deep_copy(stringlist);

  stringlist_sort(cpy, void_strcmp);
  for (int i = 0; i < stringlist_get_size(cpy) - 1; i++) {
    const char* s1 = stringlist_iget(cpy, i);
    const char* s2 = stringlist_iget(cpy, i+1);
    if (strcmp(s1,s2) == 0) {
      unique = false;
      break;
    }
  }
  stringlist_free(cpy);
  return unique;
}


#ifdef __cplusplus
}
#endif

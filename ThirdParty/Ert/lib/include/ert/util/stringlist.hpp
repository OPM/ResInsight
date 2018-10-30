/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'stringlist.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_STRINGLIST_H
#define ERT_STRINGLIST_H

#include <stdbool.h>
#include <stdio.h>

#include <ert/util/ert_api_config.hpp>
#include <ert/util/type_macros.hpp>
#include <ert/util/int_vector.hpp>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct stringlist_struct stringlist_type;
typedef int  ( string_cmp_ftype)  (const void * , const void *);
typedef bool ( file_pred_ftype) (const char *, const void *);

  int stringlist_select_files(stringlist_type * names, const char * path, file_pred_ftype * predicate, const void * pred_arg);

  const      char * stringlist_get_last( const stringlist_type * stringlist );
  char            * stringlist_pop( stringlist_type * stringlist);
  void              stringlist_deep_copy( stringlist_type * target , const stringlist_type * src);
  stringlist_type * stringlist_alloc_deep_copy_with_limits(const stringlist_type * src , int offset, int num_strings);
  stringlist_type * stringlist_alloc_deep_copy_with_offset(const stringlist_type * src , int offset);
  stringlist_type * stringlist_alloc_deep_copy( const stringlist_type * src );

  stringlist_type * stringlist_alloc_new(void);
  void              stringlist_free__(void * );
  void              stringlist_free(stringlist_type *);
  void              stringlist_clear(stringlist_type * );

  void              stringlist_append_copy(stringlist_type * , const char *);

  const      char * stringlist_safe_iget( const stringlist_type * stringlist , int index);
  bool              stringlist_unique(const stringlist_type * stringlist );
  bool              stringlist_iequal( const stringlist_type * stringlist , int index, const char * s );
  const      char * stringlist_iget(const stringlist_type * , int);
  int               stringlist_iget_as_int( const stringlist_type * stringlist , int index , bool * valid);
  double            stringlist_iget_as_double( const stringlist_type * stringlist , int index , bool * valid);
  bool              stringlist_iget_as_bool( const stringlist_type * stringlist, int index, bool * valid);
  char            * stringlist_iget_copy(const stringlist_type * stringlist , int );
  char            * stringlist_alloc_joined_string(const stringlist_type *  , const char * );
  char            * stringlist_alloc_joined_substring( const stringlist_type * s , int start_index , int end_index , const char * sep );
  const char * stringlist_front(const stringlist_type * stringlist);
  const char * stringlist_back(const stringlist_type * stringlist);



  void              stringlist_iset_copy(stringlist_type *, int index , const char *);
  void              stringlist_iset_ref(stringlist_type *, int index , const char *);
  void              stringlist_iset_owned_ref(stringlist_type *, int index , const char *);

  void              stringlist_insert_copy(stringlist_type *, int index , const char *);
  void              stringlist_insert_ref(stringlist_type *, int index , const char *);
  void              stringlist_insert_owned_ref(stringlist_type *, int index , const char *);

  void              stringlist_idel(stringlist_type * stringlist , int index);

  int               stringlist_get_size(const stringlist_type * );
  void              stringlist_fprintf(const stringlist_type * , const char * , FILE *);
  void              stringlist_fprintf_fmt(const stringlist_type * stringlist, const stringlist_type * fmt_list , FILE * stream);


  stringlist_type * stringlist_alloc_argv_copy(const char **      , int );
  stringlist_type * stringlist_alloc_argv_ref (const char **      , int );
  stringlist_type * stringlist_alloc_argv_owned_ref(const char ** argv , int argc);
  stringlist_type * stringlist_alloc_from_split( const char * input_string , const char * sep );
  stringlist_type * stringlist_fread_alloc(FILE * );

  void              stringlist_append_stringlist_copy(stringlist_type *  , const stringlist_type * );
  void              stringlist_insert_stringlist_copy(stringlist_type *  , const stringlist_type *, int);

  bool              stringlist_equal(const stringlist_type *  , const stringlist_type *);
  bool              stringlist_contains(const stringlist_type *  , const char * );
  int_vector_type * stringlist_find(const stringlist_type *, const char *);
  int               stringlist_find_first(const stringlist_type * , const char * );
  int               stringlist_get_argc(const stringlist_type * );
  char           ** stringlist_alloc_char_copy(const stringlist_type * );
  char           ** stringlist_alloc_char_ref(const stringlist_type * stringlist);
  void              stringlist_fread(stringlist_type * , FILE * );
  void              stringlist_fwrite(const stringlist_type * , FILE * );
  void              stringlist_sort(stringlist_type * , string_cmp_ftype * string_cmp);
  void              stringlist_reverse( stringlist_type * s );
  void              stringlist_python_sort( stringlist_type * s , int cmp_flag);

#ifdef ERT_HAVE_GLOB
  int               stringlist_select_matching(stringlist_type * names , const char * pattern);
#endif
  int               stringlist_select_matching_files(stringlist_type * names , const char * path , const char * file_pattern);
  int               stringlist_select_matching_elements(stringlist_type * target , const stringlist_type * src , const char * pattern);
  int stringlist_append_matching_elements(stringlist_type * target , const stringlist_type * src , const char * pattern);
  UTIL_IS_INSTANCE_HEADER(stringlist);

#ifdef __cplusplus
}
#endif
#endif

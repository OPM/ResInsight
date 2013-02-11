/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_include.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ctype.h>
#include <time.h>

#include <ert/util/util.h>
#include <ert/util/parser.h>
#include <ert/util/stringlist.h>

#include <ert/sched/sched_util.h>
#include <ert/sched/sched_kw_include.h>
#include <ert/sched/sched_macros.h>


/**
   This file implemtents support for the INCLUDE keyword in the
   SCHEDULE files. Observe that the implementation is extremely
   minimal, all it does is:

    1. Recognize the INCLUDE keyword.
    2. Internalize the string representing the included file.

   It does NOT descent into any recursive parsing. The main reason to
   have this support is to let the untyped parser "safely" read up to
   the next "/" (that approach ill be completely fooled by path
   separators in INCLUDE statements).
*/

#define SCHED_KW_INCLUDE_ID 1085006


struct sched_kw_include_struct {
  UTIL_TYPE_ID_DECLARATION;
  char * include_file;      /* The file to include ... */
};


static sched_kw_include_type * sched_kw_include_alloc_empty() {
  sched_kw_include_type * kw = util_malloc( sizeof * kw );
  UTIL_TYPE_ID_INIT(kw , SCHED_KW_INCLUDE_ID);
  kw->include_file           = NULL;
  return kw;
}


static void sched_kw_include_set_file( sched_kw_include_type * kw , const char * file) {
  kw->include_file = util_alloc_string_copy( file );
}



sched_kw_include_type * sched_kw_include_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_include_type * kw    = sched_kw_include_alloc_empty();
  stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , 0 , token_index );
  if (line_tokens == NULL)
    util_abort("%s: fatal error when parsing INCLUDE \n",__func__);
    
  if (stringlist_get_size( line_tokens ) != 1)
    util_abort("%s: fatal error when parsing INCLUDE \n",__func__);
  
  sched_kw_include_set_file( kw , stringlist_iget( line_tokens , 0 ));
  
  return kw;
}




void sched_kw_include_free( sched_kw_include_type * kw ) {
  util_safe_free( kw->include_file );
  free( kw );
}


void sched_kw_include_fprintf( const sched_kw_include_type * kw , FILE * stream ) {
  fprintf(stream , "INCLUDE\n");
  fprintf(stream , "   \'%s\' /\n\n" , kw->include_file);
}


sched_kw_include_type * sched_kw_include_copyc( const sched_kw_include_type * kw ) {
  sched_kw_include_type * copy = sched_kw_include_alloc_empty();
  sched_kw_include_set_file( copy , kw->include_file );
  return copy;
}



KW_IMPL(include)

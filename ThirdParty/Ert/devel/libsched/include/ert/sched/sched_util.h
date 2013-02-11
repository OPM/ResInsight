/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_util.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_UTIL_H__
#define __SCHED_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <time.h>

#include <ert/util/stringlist.h>

#define SCHED_KW_DEFAULT_ITEM "*"

char            * sched_util_alloc_line(FILE *, bool *);
void              sched_util_parse_line(const char * , int * , char *** , int , bool *);


void              sched_util_fprintf_int(bool ,          int , int , FILE *);
void              sched_util_fprintf_dbl(bool , double , int , int , FILE *);
double            sched_util_atof(const char *);
int               sched_util_atoi(const char *);
void              sched_util_fprintf_qst(bool , const char * , int , FILE *);
void              sched_util_fprintf_tokenlist(int num_token , const char ** token_list , const bool * def);
void              sched_util_skip_trailing_tokens( const stringlist_type * tokens , int * __token_index );
void              sched_util_skip_newline( const stringlist_type * tokens , int * __token_index );
stringlist_type * sched_util_alloc_line_tokens( const stringlist_type * tokens , bool untyped , int num_tokens , int * __token_index);
void              sched_util_init_default(const stringlist_type * line_tokens , bool * def);

#ifdef __cplusplus
}
#endif
#endif

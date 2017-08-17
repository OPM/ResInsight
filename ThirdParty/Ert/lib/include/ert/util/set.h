/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'set.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_SET_H
#define ERT_SET_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


typedef struct set_struct set_type;
typedef struct set_iter_struct set_iter_type;

void         set_clear( set_type * set );
void         set_remove_key(set_type * , const char * );
set_type   * set_alloc(int , const char ** );
set_type   * set_alloc_empty();
bool         set_add_key(set_type * , const char * );
bool         set_has_key(const set_type * set, const char * );
void         set_free(set_type * );
void         set_free__(void * );
int          set_get_size(const set_type *);
char      ** set_alloc_keylist(const set_type * );
void         set_fwrite(const set_type * , FILE * );
void         set_fread(set_type * , FILE * );
set_type   * set_fread_alloc(FILE *);
void         set_fprintf(const set_type * , const char * sep , FILE * );
void         set_intersect(set_type * , const set_type * );
void         set_union(set_type * , const set_type * );
void         set_minus(set_type * , const set_type * );
set_type   * set_copyc(const set_type *);


set_iter_type * set_iter_alloc(const set_type * set);
void set_iter_free(set_iter_type * set_iter);
bool set_iter_is_complete(const set_iter_type * set_iter);
const char * set_iter_get_next_key(set_iter_type * set_iter);


#ifdef __cplusplus
}
#endif
#endif

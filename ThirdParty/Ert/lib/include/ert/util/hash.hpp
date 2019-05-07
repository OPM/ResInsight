/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'hash.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_HASH_H
#define ERT_HASH_H
#ifdef __cplusplus
extern"C" {
#endif

#include <stdlib.h>

#include <ert/util/stringlist.hpp>
#include <ert/util/type_macros.hpp>
#include <ert/util/node_data.hpp>

typedef struct hash_struct      hash_type;
typedef struct hash_iter_struct hash_iter_type;
typedef void (hash_apply_ftype) (void * );

UTIL_SAFE_CAST_HEADER(hash);
UTIL_SAFE_CAST_HEADER_CONST(hash);

hash_type       * hash_alloc(void);
void              hash_iter_complete(hash_type * );
void              hash_free(hash_type *);
void              hash_free__(void *);
void              hash_insert_ref(hash_type * , const char * , const void *);
void              hash_insert_copy(hash_type *, const char * , const void *, copyc_ftype *, free_ftype *);
void              hash_insert_string(hash_type *, const char *, const char *);
bool              hash_has_key(const hash_type *, const char *);
void            * hash_pop( hash_type * hash , const char * key);
void            * hash_safe_get( const hash_type * hash , const char * key );
void            * hash_get(const hash_type *, const char *);
char            * hash_get_string(const hash_type * , const char *);
void              hash_del(hash_type *, const char *);
void              hash_safe_del(hash_type * , const char * );
void              hash_clear(hash_type *);
int               hash_get_size(const hash_type *);
void              hash_set_keylist(const hash_type * , char **);
char           ** hash_alloc_keylist(const hash_type *);
stringlist_type * hash_alloc_stringlist(const hash_type * );

char           ** hash_alloc_sorted_keylist (const hash_type *hash , int ( hash_get_cmp_value ) (const void *));
char           ** hash_alloc_key_sorted_list(const hash_type *hash, int (*cmp)(const void *, const void *));
bool              hash_key_list_compare(const hash_type * hash1, const hash_type * hash2);
void              hash_insert_hash_owned_ref(hash_type *, const char * , const void *, free_ftype *);
void              hash_resize(hash_type *hash, int new_size);

hash_iter_type  * hash_iter_alloc(const hash_type *);
void              hash_iter_free(hash_iter_type *);
bool              hash_iter_is_complete(const hash_iter_type *);
const      char * hash_iter_get_next_key(hash_iter_type *);
void            * hash_iter_get_next_value(hash_iter_type *);
void              hash_iter_restart( hash_iter_type * iter );

hash_type       * hash_alloc_from_options(const stringlist_type *);
bool              hash_add_option( hash_type * hash, const char * key_value);

int               hash_inc_counter(hash_type * hash , const char * counter_key);
int               hash_get_counter(const hash_type * hash , const char * key);
void              hash_insert_int(hash_type * , const char * , int);
int               hash_get_int(const hash_type * , const char *);
void              hash_insert_double(hash_type * , const char * , double);
double            hash_get_double(const hash_type * , const char *);
void              hash_apply( hash_type * hash , hash_apply_ftype * func);

UTIL_IS_INSTANCE_HEADER(hash);

#ifdef __cplusplus
}
#endif
#endif

/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'set.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <ert/util/type_macros.h>
#include <ert/util/set.h>
#include <ert/util/hash.h>
#include <ert/util/util.h>


#define SET_TYPE_ID 816523
struct set_struct {
  UTIL_TYPE_ID_DECLARATION;
  hash_type * key_hash;
};


static UTIL_SAFE_CAST_FUNCTION( set , SET_TYPE_ID )


set_type * set_alloc(int size, const char ** keyList) {
  set_type * set = malloc(sizeof * set);
  UTIL_TYPE_ID_INIT( set , SET_TYPE_ID );
  set->key_hash  = hash_alloc_unlocked();
  {
    int ikey;
    for (ikey = 0; ikey < size; ikey++)
      set_add_key(set , keyList[ikey]);
  }
  return set;
}


set_type * set_copyc(const set_type * set)
{
  int         size     = set_get_size(set);
  char     ** keylist  = set_alloc_keylist(set);
  set_type  * set_copy = set_alloc(size, (const char **) keylist);
  int key_nr;

  for( key_nr = 0; key_nr < size; key_nr++)
    free(keylist[key_nr]);
  free(keylist);

  return set_copy;
}


void set_clear( set_type * set ) {
  hash_clear( set->key_hash );
}


set_type * set_alloc_empty() { return set_alloc(0 , NULL); }


bool set_add_key(set_type * set, const char * key) {
  if (hash_has_key(set->key_hash , key))
    return false;
  else {
    hash_insert_int(set->key_hash , key , 1);
    return true;
  }
}

void set_remove_key(set_type * set, const char * key) {
  if (!hash_has_key(set->key_hash , key)) {
    fprintf(stderr, "%s: set does not have key: %s - aborting \n",__func__ , key);
    abort();
  }
  hash_del(set->key_hash  , key);
}


bool set_has_key(const set_type * set, const char * key) {
  return hash_has_key(set->key_hash, key);
}


void set_fprintf(const set_type * set, const char * sep , FILE * stream) {
  const int size = set_get_size(set);
  int i;
  char ** key_list = set_alloc_keylist(set);
  for (i=0; i < size; i++) {
    if (i < (size - 1))
      fprintf(stream , "%s%s",key_list[i] , sep);
    else
      fprintf(stream , "%s",key_list[i]);
    free(key_list[i]);
  }
  free(key_list);
}



int set_get_size(const set_type *set) { return hash_get_size(set->key_hash); }


char ** set_alloc_keylist(const set_type * set) {
  return hash_alloc_keylist(set->key_hash);
}




void set_free(set_type * set) {
  hash_free(set->key_hash);
  free(set);
}


void set_free__(void * arg) {
  set_type * set = set_safe_cast( arg );
  set_free( set );
}





void set_fwrite(const set_type * set, FILE * stream) {
  char ** key_list = set_alloc_keylist(set);
  int size = set_get_size(set);
  int i;
  fwrite(&size , sizeof size , 1 , stream);
  for (i=0; i < size; i++) {
    util_fwrite_string(key_list[i] , stream);
    free(key_list[i]);
  }
  free(key_list);
}


void set_fread(set_type * set , FILE * stream) {
  int size, i;
  fread(&size , sizeof size , 1 , stream);
  for (i=0; i < size; i++) {
    char * key = util_fread_alloc_string(stream);
    set_add_key(set , key);
    free(key);
  }
}



set_type * set_fread_alloc(FILE * stream) {
  set_type * set = set_alloc_empty();
  set_fread(set , stream);
  return set;
}


/**
   set1 is updated to *ONLY* contain elements which are both in
   set1 and set2.
*/

void set_intersect(set_type * set1 , const set_type * set2) {
  char ** key_list1 = set_alloc_keylist(set1);
  int size1 = set_get_size(set1);
  int i;
  for ( i=0; i < size1; i++) {
    if (!set_has_key(set2 , key_list1[i]))
      set_remove_key(set1 , key_list1[i]);
    free(key_list1[i]);
  }
  free(key_list1);
}





/**
   set1 is updated to contain all elements which are (originally) in
   either set1 or set2.
*/

void set_union(set_type * set1 , const set_type * set2) {
  char ** key_list2 = set_alloc_keylist(set2);
  int size2 = set_get_size(set2);
  int i;
  for ( i=0; i < size2; i++) {
    set_add_key(set1 , key_list2[i]);
    free(key_list2[i]);
  }
  free(key_list2);
}



/**
  set1 is updated so that any keys that are both in set1 and set2
  are removed from set1.
*/

void set_minus(set_type * set1, const set_type * set2) {
  char ** key_list2 = set_alloc_keylist(set2);
  int size2 = set_get_size(set2);
  int i;
  for ( i=0; i < size2; i++) {
    if( set_has_key(set1 , key_list2[i]) )
      set_remove_key(set1, key_list2[i]);
    free(key_list2[i]);
  }
  free(key_list2);
}



/**
  This is a **VERY** simple iteration object.

  Do **NOT** use with multi-threading.
*/


struct set_iter_struct
{
  hash_iter_type * hash_iter;
};


set_iter_type * set_iter_alloc(const set_type * set)
{
  set_iter_type * set_iter = util_malloc(sizeof * set_iter);
  set_iter->hash_iter = hash_iter_alloc(set->key_hash);
  return set_iter;
}


void set_iter_free(set_iter_type * set_iter)
{
  hash_iter_free(set_iter->hash_iter);
  free(set_iter);
}



bool set_iter_is_complete(const set_iter_type * set_iter)
{
  return hash_iter_is_complete(set_iter->hash_iter);
}


const char * set_iter_get_next_key(set_iter_type * set_iter)
{
  return hash_iter_get_next_key(set_iter->hash_iter);
}

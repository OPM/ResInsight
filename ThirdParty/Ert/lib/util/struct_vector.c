/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'struct_vector.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/type_macros.h>
#include <ert/util/struct_vector.h>


#define STRUCT_VECTOR_TYPE_ID 772562097

struct struct_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  int size;
  int element_size;
  int alloc_size;

  char * data;
};

UTIL_IS_INSTANCE_FUNCTION( struct_vector , STRUCT_VECTOR_TYPE_ID)


static void struct_vector_resize( struct_vector_type * struct_vector , int new_alloc_size) {
  struct_vector->data = (char*)util_realloc( struct_vector->data , struct_vector->element_size * new_alloc_size );
  struct_vector->alloc_size = new_alloc_size;
}


void struct_vector_reserve( struct_vector_type * struct_vector , int reserve_size) {
  if (reserve_size > struct_vector->alloc_size)
    struct_vector_resize( struct_vector , reserve_size );
}



struct_vector_type * struct_vector_alloc( int element_size ) {
  if (element_size <= 0) {
    util_abort("%s: fatal error - can not create container for zero size objects\n",__func__);
    return NULL;
  }

  {
    struct_vector_type * vector = (struct_vector_type*)util_malloc( sizeof * vector );
    UTIL_TYPE_ID_INIT( vector , STRUCT_VECTOR_TYPE_ID );
    vector->size = 0;
    vector->alloc_size = 0;
    vector->element_size = element_size;
    vector->data = NULL;

    struct_vector_resize( vector , 10 );

    return vector;
  }
}

void struct_vector_free( struct_vector_type * struct_vector) {
  free( struct_vector->data );
  free( struct_vector );
}


int struct_vector_get_size( const struct_vector_type * struct_vector) {
  return struct_vector->size;
}



void struct_vector_append( struct_vector_type * struct_vector , void * value) {
  if (struct_vector->size == struct_vector->alloc_size)
    struct_vector_resize( struct_vector , 2*struct_vector->alloc_size + 1);

  {
    size_t offset = struct_vector->size * struct_vector->element_size;
    memcpy( &struct_vector->data[offset] , value , struct_vector->element_size);
    struct_vector->size++;
  }
}


void * struct_vector_get_data( const struct_vector_type * struct_vector ) {
  return struct_vector->data;
}


void struct_vector_iget( const struct_vector_type * struct_vector , int index , void * value) {
  if (index < struct_vector->size) {
    size_t offset = index * struct_vector->element_size;
    memcpy( value , &struct_vector->data[offset] , struct_vector->element_size);
  } else
    util_abort("%s: fatal error - invalid index:%d size:%d\n",__func__ , index , struct_vector->size);
}


void * struct_vector_iget_ptr( const struct_vector_type * struct_vector , int index ) {
  if (index < struct_vector->size) {
    size_t offset = index * struct_vector->element_size;
    return &struct_vector->data[offset];
  } else
    util_abort("%s: fatal error - invalid index:%d size:%d\n",__func__ , index , struct_vector->size);
  return NULL;
}


void struct_vector_reset( struct_vector_type * struct_vector ) {
  struct_vector->size = 0;
}


void struct_vector_sort( struct_vector_type * struct_vector , struct_vector_cmp_ftype * cmp) {
  qsort(struct_vector->data , struct_vector->size , struct_vector->element_size ,  cmp);
}

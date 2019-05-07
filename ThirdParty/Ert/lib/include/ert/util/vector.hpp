/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'vector.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_VECTOR_H
#define ERT_VECTOR_H

#include <ert/util/node_data.hpp>
#include <ert/util/type_macros.hpp>
#include <ert/util/int_vector.hpp>

#ifdef __cplusplus
extern "C" {
#endif

  typedef void ( vector_func_type ) (void * , void *);
  typedef int  ( vector_cmp_ftype)  (const void * , const void *);

  typedef struct vector_struct vector_type;


  vector_type * vector_alloc_new(void);
  void          vector_grow_NULL( vector_type * vector , int new_size );
  vector_type * vector_alloc_NULL_initialized( int size );

  int           vector_append_ref( vector_type * , const void *);
  int           vector_append_owned_ref( vector_type * , const void * , free_ftype * del);
  int           vector_append_copy(vector_type * , const void *, copyc_ftype *, free_ftype *);

  void          vector_iset_ref( vector_type * , int , const void *);
  void          vector_iset_owned_ref( vector_type * , int , const void * , free_ftype * del);
  void          vector_iset_copy(vector_type * , int , const void *, copyc_ftype *, free_ftype *);

  void          vector_safe_iset_copy(vector_type * vector , int index , const void * data, copyc_ftype * copyc , free_ftype * del);
  void          vector_safe_iset_owned_ref(vector_type * vector , int index , const void * data, free_ftype * del);
  void          vector_safe_iset_ref(vector_type * vector , int index , const void * data);

  void          vector_insert_ref( vector_type * , int , const void *);
  void          vector_insert_owned_ref( vector_type * , int , const void * , free_ftype * del);
  void          vector_insert_copy(vector_type * , int , const void *, copyc_ftype *, free_ftype *);
  void          vector_insert_buffer(vector_type * vector , int index , const void * buffer, int buffer_size);

  void          vector_push_front_ref( vector_type * ,  const void *);
  void          vector_push_front_owned_ref( vector_type * ,  const void * , free_ftype * del);
  void          vector_push_front_copy(vector_type * ,  const void *, copyc_ftype *, free_ftype *);


  void          vector_clear(vector_type * vector);
  void          vector_free(vector_type * );
  void          vector_free__( void * arg );
  void          vector_append_buffer(vector_type * , const void * , int);
  void          vector_push_buffer(vector_type * , const void * , int);
  void        * vector_safe_iget(const vector_type * vector, int index);
  const void  * vector_safe_iget_const(const vector_type * vector, int index);
  const void  * vector_iget_const(const vector_type * , int );
  void        * vector_iget(const vector_type * , int );
  void          vector_idel(vector_type * vector , int index);
  void          vector_shrink( vector_type * vector , int new_size );
  void        * vector_get_last(const vector_type * );
  const void  * vector_get_last_const(const vector_type * );
  int           vector_get_size( const vector_type * );
  void        * vector_pop_back(vector_type * );
  void        * vector_pop_front(vector_type * );
  void          vector_sort(vector_type * vector , vector_cmp_ftype * cmp);
  int_vector_type * vector_alloc_sort_perm(const vector_type * vector , vector_cmp_ftype * cmp);
  void          vector_permute(vector_type * vector , const int_vector_type * perm_vector);
  void          vector_inplace_reverse(vector_type * vector);
  vector_type * vector_alloc_copy(const vector_type * src , bool deep_copy);

  void          vector_iset_buffer(vector_type * vector , int index , const void * buffer, int buffer_size);
  int           vector_find( const vector_type * vector , const void * ptr);

  UTIL_IS_INSTANCE_HEADER( vector );
  UTIL_SAFE_CAST_HEADER( vector );

#ifdef __cplusplus
}
#endif
#endif

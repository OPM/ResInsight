/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'vector_template.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __size_t_VECTOR_H__
#define __size_t_VECTOR_H__
#ifdef __cplusplus 
extern "C" {
#endif
#include <stdio.h>

#include <buffer.h>
#include <util.h>

typedef struct size_t_vector_struct size_t_vector_type;
typedef size_t (size_t_ftype) (size_t);

  
  int                  size_t_vector_lookup_bin( const size_t_vector_type * limits , size_t value , int guess);
  int                  size_t_vector_lookup_bin__( const size_t_vector_type * limits , size_t value , int guess);
  void                 size_t_vector_inplace_div( size_t_vector_type * vector , const size_t_vector_type * inv_factor);
  void                 size_t_vector_inplace_mul( size_t_vector_type * vector , const size_t_vector_type * factor);
  void                 size_t_vector_inplace_add( size_t_vector_type * vector , const size_t_vector_type * delta);
  void                 size_t_vector_set_read_only( size_t_vector_type * vector , bool read_only);
  bool                 size_t_vector_get_read_only( const size_t_vector_type * vector );
  void                 size_t_vector_resize( size_t_vector_type * vector , int new_alloc_size );
  void                 size_t_vector_memcpy(  size_t_vector_type * target , const size_t_vector_type * src);
  void size_t_vector_memcpy_data_block( size_t_vector_type * target , const size_t_vector_type * src , int target_offset , int src_offset , int len);
  bool                 size_t_vector_growable( const size_t_vector_type * vector);
  void                 size_t_vector_select_unique(size_t_vector_type * vector);
  size_t_vector_type * size_t_vector_alloc( int init_size , size_t );
  size_t_vector_type * size_t_vector_alloc_private_wrapper(int init_size, size_t default_value , size_t * data , int alloc_size);
  size_t_vector_type * size_t_vector_alloc_shared_wrapper(int init_size, size_t default_value , size_t * data , int alloc_size);
  size_t_vector_type * size_t_vector_alloc_strided_copy( const size_t_vector_type * src , int start , int stop , int stride );
  size_t_vector_type * size_t_vector_alloc_copy( const size_t_vector_type * src);
  void                 size_t_vector_imul(size_t_vector_type * vector, int index, size_t factor);
  void                 size_t_vector_scale(size_t_vector_type * vector, size_t factor);
  size_t               size_t_vector_iget(const size_t_vector_type * , int);
  size_t               size_t_vector_safe_iget(const size_t_vector_type * , int);
  size_t               size_t_vector_get_min(const size_t_vector_type * vector);
  size_t               size_t_vector_get_max(const size_t_vector_type * vector);
  int                  size_t_vector_get_min_index(const size_t_vector_type * vector, bool reverse);
  int                  size_t_vector_get_max_index(const size_t_vector_type * vector, bool reverse);
  size_t               size_t_vector_iadd( size_t_vector_type * vector , int index , size_t delta);
  void                 size_t_vector_iset(size_t_vector_type *       , int , size_t);
  void                 size_t_vector_idel_block( size_t_vector_type * vector , int index , int block_size);
  size_t               size_t_vector_idel( size_t_vector_type * vector , int index);
  void                 size_t_vector_append(size_t_vector_type *     , size_t);
  void                 size_t_vector_free(size_t_vector_type *);  
  void                 size_t_vector_free__(void *);  
  void                 size_t_vector_free_data(size_t_vector_type *);  
  void                 size_t_vector_reset(size_t_vector_type *); 
  void                 size_t_vector_reset__(void * __vector);
  int                  size_t_vector_size(const size_t_vector_type * );
  size_t               size_t_vector_pop(size_t_vector_type * vector);
  size_t               size_t_vector_get_first(const size_t_vector_type * vector);
  size_t               size_t_vector_get_last(const size_t_vector_type * );
  size_t *             size_t_vector_get_ptr(const size_t_vector_type * );
  size_t *             size_t_vector_alloc_data_copy( const size_t_vector_type * vector );
  const size_t *       size_t_vector_get_const_ptr(const size_t_vector_type * );
  void                 size_t_vector_set_many(size_t_vector_type *  , int  , const size_t *  , int );
  void                 size_t_vector_set_all(size_t_vector_type * vector , size_t value);
  void                 size_t_vector_append_many(size_t_vector_type * vector , const size_t * data , int length);
  void                 size_t_vector_shrink(size_t_vector_type * );
  size_t               size_t_vector_sum(const size_t_vector_type * );
  size_t               size_t_vector_get_default(const size_t_vector_type * );
  void                 size_t_vector_set_default(size_t_vector_type * vector, size_t default_value);
  void                 size_t_vector_append_default(size_t_vector_type * vector , size_t default_value);
  void                 size_t_vector_iset_default(size_t_vector_type * vector , int index , size_t default_value);
  bool                 size_t_vector_is_sorted( const size_t_vector_type * vector , bool reverse);
  int                  size_t_vector_index(const size_t_vector_type * vector , size_t value);
  int                  size_t_vector_index_sorted(const size_t_vector_type * vector , size_t value);
  void                 size_t_vector_sort(size_t_vector_type * vector);
  void                 size_t_vector_rsort(size_t_vector_type * vector);
  void                 size_t_vector_permute(size_t_vector_type * vector , const int * perm);
  int *                size_t_vector_alloc_sort_perm(const size_t_vector_type * vector);
  int *                size_t_vector_alloc_rsort_perm(const size_t_vector_type * vector);
  void                 size_t_vector_fprintf(const size_t_vector_type * vector , FILE * stream , const char * name , const char * fmt);
  void                 size_t_vector_fwrite(const size_t_vector_type * vector , FILE * stream);
  void                 size_t_vector_buffer_fread(size_t_vector_type * vector , buffer_type * buffer);
  size_t_vector_type * size_t_vector_fread_alloc( FILE * stream );
  size_t_vector_type * size_t_vector_buffer_fread_alloc( buffer_type * buffer );
  void                 size_t_vector_buffer_fwrite(const size_t_vector_type * vector , buffer_type * buffer);
  void                 size_t_vector_fread( size_t_vector_type * vector , FILE * stream );
  void                 size_t_vector_fwrite_data( const size_t_vector_type * vector , FILE * stream );
  void                 size_t_vector_fread_data( size_t_vector_type * vector , int size, FILE * stream);
  bool                 size_t_vector_equal(const size_t_vector_type * vector1 , const size_t_vector_type * vector2);
  void                 size_t_vector_apply(size_t_vector_type * vector , size_t_ftype *func);
  int                  size_t_vector_count_equal( const size_t_vector_type * vector , size_t cmp_value);
  int                  size_t_vector_element_size( const size_t_vector_type * vector );
  
  UTIL_SAFE_CAST_HEADER( size_t_vector );

#ifdef __cplusplus 
}
#endif
#endif
//





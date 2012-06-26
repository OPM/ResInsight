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

#ifndef __long_VECTOR_H__
#define __long_VECTOR_H__
#ifdef __cplusplus 
extern "C" {
#endif
#include <stdio.h>

#include <buffer.h>
#include <type_macros.h>

typedef struct long_vector_struct long_vector_type;
typedef long (long_ftype) (long);

  
  int                  long_vector_lookup_bin( const long_vector_type * limits , long value , int guess);
  int                  long_vector_lookup_bin__( const long_vector_type * limits , long value , int guess);
  void                 long_vector_inplace_div( long_vector_type * vector , const long_vector_type * inv_factor);
  void                 long_vector_inplace_mul( long_vector_type * vector , const long_vector_type * factor);
  void                 long_vector_inplace_add( long_vector_type * vector , const long_vector_type * delta);
  void                 long_vector_set_read_only( long_vector_type * vector , bool read_only);
  bool                 long_vector_get_read_only( const long_vector_type * vector );
  void                 long_vector_resize( long_vector_type * vector , int new_alloc_size );
  void                 long_vector_memcpy(  long_vector_type * target , const long_vector_type * src);
  void long_vector_memcpy_data_block( long_vector_type * target , const long_vector_type * src , int target_offset , int src_offset , int len);
  bool                 long_vector_growable( const long_vector_type * vector);
  void                 long_vector_select_unique(long_vector_type * vector);
  long_vector_type * long_vector_alloc( int init_size , long );
  long_vector_type * long_vector_alloc_private_wrapper(int init_size, long default_value , long * data , int alloc_size);
  long_vector_type * long_vector_alloc_shared_wrapper(int init_size, long default_value , long * data , int alloc_size);
  long_vector_type * long_vector_alloc_strided_copy( const long_vector_type * src , int start , int stop , int stride );
  long_vector_type * long_vector_alloc_copy( const long_vector_type * src);
  void                 long_vector_imul(long_vector_type * vector, int index, long factor);
  void                 long_vector_scale(long_vector_type * vector, long factor);
  long               long_vector_iget(const long_vector_type * , int);
  long               long_vector_safe_iget(const long_vector_type * , int);
  long               long_vector_get_min(const long_vector_type * vector);
  long               long_vector_get_max(const long_vector_type * vector);
  int                  long_vector_get_min_index(const long_vector_type * vector, bool reverse);
  int                  long_vector_get_max_index(const long_vector_type * vector, bool reverse);
  long               long_vector_iadd( long_vector_type * vector , int index , long delta);
  void                 long_vector_iset(long_vector_type *       , int , long);
  void                 long_vector_idel_block( long_vector_type * vector , int index , int block_size);
  long               long_vector_idel( long_vector_type * vector , int index);
  void                 long_vector_append(long_vector_type *     , long);
  void                 long_vector_free(long_vector_type *);  
  void                 long_vector_free__(void *);  
  void                 long_vector_free_data(long_vector_type *);  
  void                 long_vector_reset(long_vector_type *); 
  void                 long_vector_reset__(void * __vector);
  int                  long_vector_size(const long_vector_type * );
  long               long_vector_pop(long_vector_type * vector);
  long               long_vector_get_first(const long_vector_type * vector);
  long               long_vector_get_last(const long_vector_type * );
  long *             long_vector_get_ptr(const long_vector_type * );
  long *             long_vector_alloc_data_copy( const long_vector_type * vector );
  const long *       long_vector_get_const_ptr(const long_vector_type * );
  void                 long_vector_set_many(long_vector_type *  , int  , const long *  , int );
  void                 long_vector_set_all(long_vector_type * vector , long value);
  void                 long_vector_append_many(long_vector_type * vector , const long * data , int length);
  void                 long_vector_shrink(long_vector_type * );
  long               long_vector_sum(const long_vector_type * );
  long               long_vector_get_default(const long_vector_type * );
  void                 long_vector_set_default(long_vector_type * vector, long default_value);
  void                 long_vector_append_default(long_vector_type * vector , long default_value);
  void                 long_vector_iset_default(long_vector_type * vector , int index , long default_value);
  bool                 long_vector_is_sorted( const long_vector_type * vector , bool reverse);
  int                  long_vector_index(const long_vector_type * vector , long value);
  int                  long_vector_index_sorted(const long_vector_type * vector , long value);
  void                 long_vector_sort(long_vector_type * vector);
  void                 long_vector_rsort(long_vector_type * vector);
  void                 long_vector_permute(long_vector_type * vector , const int * perm);
  int *                long_vector_alloc_sort_perm(const long_vector_type * vector);
  int *                long_vector_alloc_rsort_perm(const long_vector_type * vector);
  void                 long_vector_fprintf(const long_vector_type * vector , FILE * stream , const char * name , const char * fmt);
  void                 long_vector_fwrite(const long_vector_type * vector , FILE * stream);
  void                 long_vector_buffer_fread(long_vector_type * vector , buffer_type * buffer);
  long_vector_type * long_vector_fread_alloc( FILE * stream );
  long_vector_type * long_vector_buffer_fread_alloc( buffer_type * buffer );
  void                 long_vector_buffer_fwrite(const long_vector_type * vector , buffer_type * buffer);
  void                 long_vector_fread( long_vector_type * vector , FILE * stream );
  void                 long_vector_fwrite_data( const long_vector_type * vector , FILE * stream );
  void                 long_vector_fread_data( long_vector_type * vector , int size, FILE * stream);
  bool                 long_vector_equal(const long_vector_type * vector1 , const long_vector_type * vector2);
  void                 long_vector_apply(long_vector_type * vector , long_ftype *func);
  int                  long_vector_count_equal( const long_vector_type * vector , long cmp_value);
  int                  long_vector_element_size( const long_vector_type * vector );
  
  UTIL_SAFE_CAST_HEADER( long_vector );

#ifdef __cplusplus 
}
#endif
#endif
//





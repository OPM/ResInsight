/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_kw.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_KW_H
#define ERT_ECL_KW_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <ert/util/buffer.h>
#include <ert/util/type_macros.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_type.h>

  UTIL_IS_INSTANCE_HEADER(ecl_kw);


  typedef struct ecl_kw_struct      ecl_kw_type;

  typedef enum {
    ECL_KW_READ_OK = 0,
    ECL_KW_READ_FAIL = 1,
    ECL_KW_READ_SKIP = 2
  } ecl_read_status_enum;

/*
  The size of an ecl_kw instance is denoted with an integer. The
  choice of int to store the size obviously limits the maximum size to
  INT_MAX elements. This choice is an historical mistake - it should
  probably have been size_t; however the ecl_kw datastructure is
  tightly bound to the on-disk binary format supplied by Eclipse, and
  there the number of elements is stored as a signed(?) 32 bit
  integer - so using int for size does make some sense-
*/

#define ECL_KW_MAX_SIZE INT_MAX

/*
  Character data in ECLIPSE files comes as an array of fixed-length
  string. Each of these strings is 8 characters long. The type name,
  i.e. 'REAL', 'INTE', ... , come as 4 character strings.
*/
#define ECL_KW_HEADER_DATA_SIZE   ECL_STRING8_LENGTH + ECL_TYPE_LENGTH + 4
#define ECL_KW_HEADER_FORTIO_SIZE ECL_KW_HEADER_DATA_SIZE + 8



  int            ecl_kw_first_different( const ecl_kw_type * kw1 , const ecl_kw_type * kw2 , int offset, double abs_epsilon , double rel_epsilon);
  size_t         ecl_kw_fortio_size( const ecl_kw_type * ecl_kw );
  void *         ecl_kw_get_ptr(const ecl_kw_type *ecl_kw);
  void           ecl_kw_set_data_ptr(ecl_kw_type * ecl_kw , void * data);
  void           ecl_kw_fwrite_data(const ecl_kw_type *_ecl_kw , fortio_type *fortio);
  bool           ecl_kw_fread_realloc_data(ecl_kw_type *ecl_kw, fortio_type *fortio);
  ecl_data_type  ecl_kw_get_data_type(const ecl_kw_type *);
  size_t         ecl_kw_get_sizeof_ctype(const ecl_kw_type *);
  const char   * ecl_kw_get_header8(const ecl_kw_type *);
  const char   * ecl_kw_get_header(const ecl_kw_type * ecl_kw );
  ecl_kw_type  * ecl_kw_alloc_empty(void);
  ecl_read_status_enum ecl_kw_fread_header(ecl_kw_type *, fortio_type *);
  void           ecl_kw_set_header_name(ecl_kw_type * , const char * );
  bool           ecl_kw_fseek_kw(const char * , bool , bool , fortio_type *);
  bool           ecl_kw_fseek_last_kw(const char * , bool  , fortio_type *);
  void           ecl_kw_inplace_update_file(const ecl_kw_type * , const char * , int ) ;
  void           ecl_kw_fskip(fortio_type *);
  void           ecl_kw_alloc_data(ecl_kw_type  *);
  void           ecl_kw_alloc_double_data(ecl_kw_type * ecl_kw , double * values);
  void           ecl_kw_alloc_float_data(ecl_kw_type * ecl_kw , float * values);
  bool           ecl_kw_fread_realloc(ecl_kw_type *, fortio_type *);
  void           ecl_kw_fread(ecl_kw_type * , fortio_type * );
  ecl_kw_type *  ecl_kw_fread_alloc(fortio_type *);
  void           ecl_kw_free_data(ecl_kw_type *);
  void           ecl_kw_fread_indexed_data(fortio_type * fortio, offset_type data_offset, ecl_data_type, int element_count, const int_vector_type* index_map, char* buffer);
  void           ecl_kw_free(ecl_kw_type *);
  void           ecl_kw_free__(void *);
  ecl_kw_type *  ecl_kw_alloc_copy (const ecl_kw_type *);
  ecl_kw_type *  ecl_kw_alloc_sub_copy( const ecl_kw_type * src, const char * new_kw , int offset , int count);
  const void  *  ecl_kw_copyc__(const void *);
  ecl_kw_type *  ecl_kw_alloc_slice_copy( const ecl_kw_type * src, int index1, int index2, int stride);
  void           ecl_kw_resize( ecl_kw_type * ecl_kw, int new_size);
  //void        * ecl_kw_get_data_ref(const ecl_kw_type *);
  void        *  ecl_kw_alloc_data_copy(const ecl_kw_type * );
  void           ecl_kw_memcpy(ecl_kw_type *, const ecl_kw_type *);
  void           ecl_kw_get_memcpy_data(const ecl_kw_type *, void *);
  void           ecl_kw_get_memcpy_float_data(const ecl_kw_type *ecl_kw , float *target);
  void           ecl_kw_get_memcpy_double_data(const ecl_kw_type *ecl_kw , double *target);
  void           ecl_kw_get_memcpy_int_data(const ecl_kw_type *ecl_kw , int *target);
  void           ecl_kw_set_memcpy_data(ecl_kw_type * , const void *);
  void           ecl_kw_fwrite(const ecl_kw_type *,  fortio_type *);
  void           ecl_kw_iget(const ecl_kw_type *, int , void *);
  void           ecl_kw_iset(ecl_kw_type *ecl_kw , int i , const void *iptr);
  void           ecl_kw_iset_char_ptr( ecl_kw_type * ecl_kw , int index, const char * s);
  void           ecl_kw_iset_string8(ecl_kw_type * ecl_kw , int index , const char *s8);
  const char  *  ecl_kw_iget_char_ptr( const ecl_kw_type * ecl_kw , int i);
  void        *  ecl_kw_iget_ptr(const ecl_kw_type *, int);
  int            ecl_kw_get_size(const ecl_kw_type *);
  bool           ecl_kw_ichar_eq(const ecl_kw_type *, int , const char *);
  ecl_kw_type *  ecl_kw_alloc( const char * header , int size , ecl_data_type );
  ecl_kw_type *  ecl_kw_alloc_new(const char * ,  int , ecl_data_type , const void * );
  ecl_kw_type *  ecl_kw_alloc_new_shared(const char * ,  int , ecl_data_type , void * );
  void           ecl_kw_fwrite_param(const char * , bool  , const char * ,  ecl_data_type , int , void * );
  void           ecl_kw_fwrite_param_fortio(fortio_type *, const char * ,  ecl_data_type , int , void * );
  void           ecl_kw_summarize(const ecl_kw_type * ecl_kw);
  void           ecl_kw_fread_double_param(const char * , bool , double *);
  float          ecl_kw_iget_as_float(const ecl_kw_type * ecl_kw , int i);
  double         ecl_kw_iget_as_double(const ecl_kw_type * ecl_kw , int i);
  void           ecl_kw_get_data_as_double(const ecl_kw_type *, double *);
  void           ecl_kw_get_data_as_float(const ecl_kw_type * ecl_kw , float * float_data);
  bool           ecl_kw_name_equal( const ecl_kw_type * ecl_kw , const char * name);
  bool           ecl_kw_header_eq(const ecl_kw_type *ecl_kw1 , const ecl_kw_type * ecl_kw2);
  bool           ecl_kw_equal(const ecl_kw_type *ecl_kw1, const ecl_kw_type *ecl_kw2);
  bool           ecl_kw_size_and_type_equal( const ecl_kw_type *ecl_kw1 , const ecl_kw_type * ecl_kw2 );
  bool           ecl_kw_icmp_string( const ecl_kw_type * ecl_kw , int index, const char * other_string);
  bool           ecl_kw_numeric_equal(const ecl_kw_type *ecl_kw1, const ecl_kw_type *ecl_kw2 , double abs_diff , double rel_diff);
  bool           ecl_kw_block_equal( const ecl_kw_type * ecl_kw1 , const ecl_kw_type * ecl_kw2 , int cmp_elements);
  bool           ecl_kw_data_equal( const ecl_kw_type * ecl_kw , const void * data);
  bool           ecl_kw_content_equal( const ecl_kw_type * ecl_kw1 , const ecl_kw_type * ecl_kw2);
  bool           ecl_kw_fskip_data__( ecl_data_type, int, fortio_type *);
  bool           ecl_kw_fskip_data(ecl_kw_type *ecl_kw, fortio_type *fortio);
  bool           ecl_kw_fread_data(ecl_kw_type *ecl_kw, fortio_type *fortio);
  void           ecl_kw_fskip_header( fortio_type * fortio);


  bool ecl_kw_is_kw_file(fortio_type * fortio);

  int        ecl_kw_element_sum_int( const ecl_kw_type * ecl_kw );
  double     ecl_kw_element_sum_float( const ecl_kw_type * ecl_kw );
  void       ecl_kw_inplace_inv(ecl_kw_type * my_kw);
  void       ecl_kw_element_sum(const ecl_kw_type * , void * );
  void       ecl_kw_max_min(const ecl_kw_type * , void * , void *);
  void     * ecl_kw_get_void_ptr(const ecl_kw_type * ecl_kw);

  ecl_kw_type * ecl_kw_buffer_alloc(buffer_type * buffer);
  void          ecl_kw_buffer_store(const ecl_kw_type * ecl_kw , buffer_type * buffer);

  void ecl_kw_fprintf_data( const ecl_kw_type * ecl_kw , const char * fmt , FILE * stream);
  void ecl_kw_memcpy_data( ecl_kw_type * target , const ecl_kw_type * src);

  bool ecl_kw_assert_numeric( const ecl_kw_type * kw );
  bool ecl_kw_assert_binary( const ecl_kw_type * kw1, const ecl_kw_type * kw2);

  void ecl_kw_scalar_set_bool( ecl_kw_type * ecl_kw , bool bool_value);
  void ecl_kw_scalar_set__(ecl_kw_type * ecl_kw , const void * value);
  void ecl_kw_scalar_set_float_or_double( ecl_kw_type * ecl_kw , double value );


#define ECL_KW_SCALAR_SET_TYPED_HEADER( ctype ) void ecl_kw_scalar_set_ ## ctype( ecl_kw_type * ecl_kw , ctype value);
  ECL_KW_SCALAR_SET_TYPED_HEADER( int )
  ECL_KW_SCALAR_SET_TYPED_HEADER( float )
  ECL_KW_SCALAR_SET_TYPED_HEADER( double )
#undef ECL_KW_SCALAR_SET_TYPED_HEADER

  ecl_kw_type * ecl_kw_alloc_scatter_copy( const ecl_kw_type * src_kw , int target_size , const int * mapping, void * def_value);

  void ecl_kw_inplace_add( ecl_kw_type * target_kw , const ecl_kw_type * add_kw);
  void ecl_kw_inplace_sub( ecl_kw_type * target_kw , const ecl_kw_type * sub_kw);
  void ecl_kw_inplace_div( ecl_kw_type * target_kw , const ecl_kw_type * div_kw);
  void ecl_kw_inplace_mul( ecl_kw_type * target_kw , const ecl_kw_type * mul_kw);
  void ecl_kw_inplace_abs( ecl_kw_type * kw );

  void ecl_kw_inplace_add_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * add_kw);
  void ecl_kw_inplace_sub_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * sub_kw);
  void ecl_kw_inplace_mul_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * mul_kw);
  void ecl_kw_inplace_div_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * div_kw);
  void ecl_kw_copy_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * src_kw);

  bool ecl_kw_assert_binary_numeric( const ecl_kw_type * kw1, const ecl_kw_type * kw2);
#define ECL_KW_ASSERT_TYPED_BINARY_OP_HEADER( ctype ) bool ecl_kw_assert_binary_ ## ctype( const ecl_kw_type * kw1 , const ecl_kw_type * kw2)
  ECL_KW_ASSERT_TYPED_BINARY_OP_HEADER( int );
  ECL_KW_ASSERT_TYPED_BINARY_OP_HEADER( float );
  ECL_KW_ASSERT_TYPED_BINARY_OP_HEADER( double );
#undef  ECL_KW_ASSERT_TYPED_BINARY_OP_HEADER

#define ECL_KW_SCALE_TYPED_HEADER( ctype ) void ecl_kw_scale_ ## ctype (ecl_kw_type * ecl_kw , ctype scale_factor)
  ECL_KW_SCALE_TYPED_HEADER( int );
  ECL_KW_SCALE_TYPED_HEADER( float );
  ECL_KW_SCALE_TYPED_HEADER( double );
#undef ECL_KW_SCALE_TYPED_HEADER
  void ecl_kw_scale_float_or_double( ecl_kw_type * ecl_kw , double scale_factor );


#define ECL_KW_SHIFT_TYPED_HEADER( ctype ) void ecl_kw_shift_ ## ctype (ecl_kw_type * ecl_kw , ctype shift_factor)
  ECL_KW_SHIFT_TYPED_HEADER( int );
  ECL_KW_SHIFT_TYPED_HEADER( float );
  ECL_KW_SHIFT_TYPED_HEADER( double );
#undef ECL_KW_SHIFT_TYPED_HEADER
  void ecl_kw_shift_float_or_double( ecl_kw_type * ecl_kw , double shift_value );


#define ECL_KW_IGET_TYPED_HEADER(type) type ecl_kw_iget_ ## type(const ecl_kw_type * , int)
  ECL_KW_IGET_TYPED_HEADER(double);
  ECL_KW_IGET_TYPED_HEADER(float);
  ECL_KW_IGET_TYPED_HEADER(int);
#undef ECL_KW_IGET_TYPED_HEADER
  bool  ecl_kw_iget_bool( const ecl_kw_type * ecl_kw , int i );


#define ECL_KW_ISET_TYPED_HEADER(type) void ecl_kw_iset_ ## type(ecl_kw_type * , int , type )
  ECL_KW_ISET_TYPED_HEADER(double);
  ECL_KW_ISET_TYPED_HEADER(float);
  ECL_KW_ISET_TYPED_HEADER(int);
#undef ECL_KW_ISET_TYPED_HEADER
  void ecl_kw_iset_bool( ecl_kw_type * ecl_kw , int i , bool bool_value);


#define ECL_KW_GET_TYPED_PTR_HEADER(type) type * ecl_kw_get_ ## type ## _ptr(const ecl_kw_type *)
  ECL_KW_GET_TYPED_PTR_HEADER(double);
  ECL_KW_GET_TYPED_PTR_HEADER(float);
  ECL_KW_GET_TYPED_PTR_HEADER(int);
  ECL_KW_GET_TYPED_PTR_HEADER(bool);
#undef ECL_KW_GET_TYPED_PTR_HEADER


#define ECL_KW_SET_INDEXED_HEADER(ctype ) void ecl_kw_set_indexed_ ## ctype( ecl_kw_type * ecl_kw, const int_vector_type * index_list , ctype value)
  ECL_KW_SET_INDEXED_HEADER( double );
  ECL_KW_SET_INDEXED_HEADER( float  );
  ECL_KW_SET_INDEXED_HEADER( int    );
#undef ECL_KW_SET_INDEXED_HEADER


#define ECL_KW_SHIFT_INDEXED_HEADER(ctype) void ecl_kw_shift_indexed_ ## ctype( ecl_kw_type * ecl_kw, const int_vector_type * index_list , ctype shift)
  ECL_KW_SHIFT_INDEXED_HEADER( int    );
  ECL_KW_SHIFT_INDEXED_HEADER( float  );
  ECL_KW_SHIFT_INDEXED_HEADER( double );
#undef ECL_KW_SHIFT_INDEXED_HEADER


#define ECL_KW_SCALE_INDEXED_HEADER(ctype) void ecl_kw_scale_indexed_ ## ctype( ecl_kw_type * ecl_kw, const int_vector_type * index_list , ctype scale)
  ECL_KW_SCALE_INDEXED_HEADER( int    );
  ECL_KW_SCALE_INDEXED_HEADER( float  );
  ECL_KW_SCALE_INDEXED_HEADER( double );
#undef ECL_KW_SCALE_INDEXED_HEADER


#define ECL_KW_MAX_MIN_HEADER( ctype ) void ecl_kw_max_min_ ## ctype( const ecl_kw_type * ecl_kw , ctype * _max , ctype * _min)
  ECL_KW_MAX_MIN_HEADER( int );
  ECL_KW_MAX_MIN_HEADER( float );
  ECL_KW_MAX_MIN_HEADER( double );
#undef ECL_KW_MAX_MIN_HEADER

  void ecl_kw_fix_uninitialized(ecl_kw_type * ecl_kw , int nx , int ny , int nz, const int * actnum);

#include <ert/ecl/ecl_kw_grdecl.h>

#ifdef __cplusplus
}
#endif
#endif

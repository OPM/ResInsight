/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'matrix.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_MATRIX_H
#define ERT_MATRIX_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/ert_api_config.h>
#include <ert/util/rng.h>
#include <ert/util/type_macros.h>
#include <ert/util/bool_vector.h>

#ifdef HAVE_THREAD_POOL
#include <ert/util/thread_pool.h>
#endif

#ifdef _MSC_VER
#define __forceinline inline
#elif __GNUC__
/* Also clang defines the __GNUC__ symbol */
#define __forceinline inline __attribute__((always_inline))
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct matrix_struct {
  UTIL_TYPE_ID_DECLARATION;
  char                  * name;           /* A name of the matrix - for printing - can be NULL. */
  double                * data;           /* The actual storage */
  bool                    data_owner;     /* is this matrix instance the owner of data? */
  size_t                  data_size;      /* What is the length of data (number of double values). */

  int                     rows;           /* The number of rows in the matrix. */
  int                     columns;        /* The number of columns in the matrix. */
  int                     alloc_rows;
  int                     alloc_columns;
  int                     row_stride;     /* The distance in data between two conscutive row values. */
  int                     column_stride;  /* The distance in data between to consecutive column values. */

  /*
     Observe that the stride is considered an internal property - if
     the matrix is stored to disk and then recovered the strides might
     change, and also matrix_alloc_copy() will not respect strides.
  */
};

typedef struct matrix_struct matrix_type;

  __forceinline size_t GET_INDEX( const matrix_type * m , size_t i , size_t j) {return m->row_stride *i + m->column_stride *j;}
  matrix_type * matrix_fread_alloc(FILE * stream);
  void          matrix_fread(matrix_type * matrix , FILE * stream);
  void          matrix_fwrite(const matrix_type * matrix , FILE * stream);
  bool          matrix_check_dims( const matrix_type * m , int rows , int columns);
  void          matrix_fscanf_data( matrix_type * matrix , bool row_major_order , FILE * stream );
  void          matrix_fprintf( const matrix_type * matrix , const char * fmt , FILE * stream );
  void          matrix_dump_csv( const matrix_type * matrix  ,const char * filename);
  void          matrix_pretty_fprint(const matrix_type * matrix , const char * name , const char * fmt , FILE * stream);
  void matrix_pretty_fprint_submat(const matrix_type * matrix , const char * name , const char * fmt , FILE * stream, int m, int M, int n, int N);
  matrix_type * matrix_alloc(int rows, int columns);
  matrix_type * matrix_alloc_identity(int dim);
  matrix_type * matrix_safe_alloc(int rows, int columns);
  bool          matrix_resize(matrix_type * matrix , int rows , int columns , bool copy_content);
  bool          matrix_safe_resize(matrix_type * matrix , int rows , int columns , bool copy_content);
  matrix_type * matrix_alloc_sub_copy( const matrix_type * src , int row_offset , int column_offset , int rows, int columns);
  matrix_type * matrix_alloc_copy(const matrix_type * src);
  matrix_type * matrix_alloc_column_compressed_copy(const matrix_type * src, const bool_vector_type * mask);
  void          matrix_column_compressed_memcpy(matrix_type * target, const matrix_type * src, const bool_vector_type * mask);
  matrix_type * matrix_safe_alloc_copy(const matrix_type * src);
  matrix_type * matrix_realloc_copy(matrix_type * T , const matrix_type * src);

  matrix_type * matrix_alloc_shared(const matrix_type * src , int row , int column , int rows , int columns);
  void          matrix_free(matrix_type * matrix);
  void          matrix_safe_free( matrix_type * matrix );
  void          matrix_pretty_print(const matrix_type * matrix , const char * name , const char * fmt);
  void          matrix_set(matrix_type * matrix, double value);
  void          matrix_set_name( matrix_type * matrix , const char * name);
  void          matrix_scale(matrix_type * matrix, double value);
  void          matrix_shift(matrix_type * matrix, double value);

  void          matrix_assign(matrix_type * A , const matrix_type * B);
  void          matrix_inplace_add(matrix_type * A , const matrix_type * B);
  void          matrix_inplace_sub(matrix_type * A , const matrix_type * B);
  void          matrix_inplace_mul(matrix_type * A , const matrix_type * B);
  void          matrix_inplace_div(matrix_type * A , const matrix_type * B);
  void          matrix_sub(matrix_type * A , const matrix_type * B , const matrix_type * C);
  void          matrix_mul( matrix_type * A , const matrix_type * B , const matrix_type * C);
  void          matrix_transpose(const matrix_type * A , matrix_type * T);
  void          matrix_inplace_add_column(matrix_type * A , const matrix_type * B, int colA , int colB);
  void          matrix_inplace_sub_column(matrix_type * A , const matrix_type * B, int colA , int colB);
  void          matrix_inplace_transpose(matrix_type * A );

  void          matrix_iset_safe(matrix_type * matrix , int i , int j, double value);
  void          matrix_iset(matrix_type * matrix , int i , int j, double value);
  double        matrix_iget(const matrix_type * matrix , int i , int j);
  double        matrix_iget_safe(const matrix_type * matrix , int i , int j);
  void          matrix_iadd(matrix_type * matrix , int i , int j , double value);
  void          matrix_isub(matrix_type * matrix , int i , int j , double value);
  void          matrix_imul(matrix_type * matrix , int i , int j , double value);


  void          matrix_inplace_matmul(matrix_type * A, const matrix_type * B);
  void          matrix_inplace_matmul_mt1(matrix_type * A, const matrix_type * B , int num_threads);
#ifdef HAVE_THREAD_POOL
  void          matrix_inplace_matmul_mt2(matrix_type * A, const matrix_type * B , thread_pool_type * thread_pool);
#endif

  void          matrix_shift_column(matrix_type * matrix , int column, double shift);
  void          matrix_shift_row(matrix_type * matrix , int row , double shift);
  double        matrix_get_column_sum(const matrix_type * matrix , int column);
  double        matrix_get_row_sum(const matrix_type * matrix , int column);
  double        matrix_get_column_sum2(const matrix_type * matrix , int column);
  double        matrix_get_row_abssum(const matrix_type * matrix , int row);
  double        matrix_get_column_abssum(const matrix_type * matrix , int column);
  double        matrix_get_row_sum2(const matrix_type * matrix , int column);
  void          matrix_subtract_row_mean(matrix_type * matrix);
  void          matrix_subtract_and_store_row_mean(matrix_type * matrix, matrix_type * row_mean);
  void          matrix_scale_column(matrix_type * matrix , int column  , double scale_factor);
  void          matrix_scale_row(matrix_type * matrix , int row  , double scale_factor);
  void          matrix_set_const_column(matrix_type * matrix , const double value , int column);
  void          matrix_copy_column(matrix_type * target_matrix, const matrix_type * src_matrix , int src_column, int target_column);
  void          matrix_set_const_row(matrix_type * matrix , const double value , int row);

  double      * matrix_get_data(const matrix_type * matrix);
  double        matrix_orthonormality( const matrix_type * matrix );

  matrix_type * matrix_alloc_steal_data(int rows , int columns , double * data , int data_size);
  void          matrix_set_column(matrix_type * matrix , const double * data , int column);
  void          matrix_set_many_on_column(matrix_type * matrix , int row_offset , int elements , const double * data , int column);
  void          matrix_ensure_rows(matrix_type * matrix, int rows, bool copy_content);
  void          matrix_shrink_header(matrix_type * matrix , int rows , int columns);
  void          matrix_full_size( matrix_type * matrix );
  int           matrix_get_rows(const matrix_type * matrix);
  int           matrix_get_columns(const matrix_type * matrix);
  int           matrix_get_row_stride(const matrix_type * matrix);
  int           matrix_get_column_stride(const matrix_type * matrix);
  void          matrix_get_dims(const matrix_type * matrix ,  int * rows , int * columns , int * row_stride , int * column_stride);
  bool          matrix_is_quadratic(const matrix_type * matrix);
  bool          matrix_equal( const matrix_type * m1 , const matrix_type * m2);
  bool          matrix_columns_equal( const matrix_type * m1 , int col1 , const matrix_type * m2 , int col2);

  void          matrix_diag_set_scalar(matrix_type * matrix , double value);
  void          matrix_diag_set(matrix_type * matrix , const double * diag);
  void          matrix_random_init(matrix_type * matrix , rng_type * rng);
  void          matrix_matlab_dump(const matrix_type * matrix, const char * filename);

  void          matrix_imul_col( matrix_type * matrix , int column , double factor);
  double        matrix_column_column_dot_product(const matrix_type * m1 , int col1 , const matrix_type * m2 , int col2);
  double        matrix_row_column_dot_product(const matrix_type * m1 , int row1 , const matrix_type * m2 , int col2);
  matrix_type * matrix_alloc_view(double * data , int rows , int columns);
  matrix_type * matrix_alloc_transpose( const matrix_type * A);
  void          matrix_copy_row(matrix_type * target_matrix, const matrix_type * src_matrix , int target_row, int src_row);
  void          matrix_copy_block( matrix_type * target_matrix , int target_row , int target_column , int rows , int columns,
                                   const matrix_type * src_matrix , int src_row , int src_column);

  void          matrix_scalar_set( matrix_type * matrix , double value);
  void          matrix_inplace_diag_sqrt(matrix_type *Cd);
  void          matrix_create_identiy(int n,matrix_type *Id);
  double        matrix_trace(const matrix_type *matrix);
  double        matrix_diag_std(const matrix_type * Sk,double mean);
  double        matrix_det2( const matrix_type * A);
  double        matrix_det3( const matrix_type * A);
  double        matrix_det4( const matrix_type * A);

  #ifdef ERT_HAVE_ISFINITE
  bool          matrix_is_finite(const matrix_type * matrix);
  void          matrix_assert_finite( const matrix_type * matrix );
  #endif

  UTIL_SAFE_CAST_HEADER( matrix );

#ifdef __cplusplus
}
#endif
#endif

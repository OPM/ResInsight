/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'matrix.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <ert/util/ert_api_config.h>
#include <ert/util/thread_pool.h>
#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/arg_pack.h>
#include <ert/util/rng.h>

/**
   This is V E R Y  S I M P L E matrix implementation. It is not
   designed to be fast/efficient or anything. It is purely a minor
   support functionality for the enkf program, and should N O T be
   considered as general matrix functionality.
*/



/**
  Many of matrix functions can potentially involve laaarge amounts of
  memory. The functions:

   o matrix_alloc(), matrix_resize() and matrix_alloc_copy() will
     abort with util_abort() if the memory requirements can not be
     satisfied.

   o The corresponding functions matrix_safe_alloc(),
     matrix_safe_resize() and matrix_safe_alloc_copy() will not abort,
     instead NULL or an unchanged matrix will be returned. When using
     these functons it is the responsability of the calling scope to
     check return values.

  So the expression "safe" should be interpreted as 'can not abort' -
  however the responsability of the calling scope is greater when it
  comes to using these functions - things can surely blow up!
*/

#ifdef __cplusplus
extern "C" {
#endif

#define MATRIX_TYPE_ID 712108

/*#define GET_INDEX(m,i,j) (m->row_stride * (i) + m->column_stride * (j))*/

/*
 This GET_INDEX function has been forcely inlined for performance.
*/
/*static size_t GET_INDEX( const matrix_type * m , size_t i , size_t j) {
  return m->row_stride *i + m->column_stride *j;
}*/

static size_t MATRIX_DATA_SIZE( const matrix_type * m) {
  size_t col    = m->columns;
  size_t stride = m->column_stride;

  return col*stride;
}




static void matrix_init_header(matrix_type * matrix , int rows , int columns , int row_stride , int column_stride) {

  if (!((column_stride * columns <= row_stride) || (row_stride * rows <= column_stride)))
    util_abort("%s: invalid stride combination \n",__func__);

  matrix->data_size     = 0;
  matrix->alloc_rows    = rows;
  matrix->alloc_columns = columns;
  matrix->row_stride    = row_stride;
  matrix->column_stride = column_stride;

  matrix_full_size( matrix );
}


/**
   This is the low-level function allocating storage. If the input
   flag 'safe_mode' is equal to true, the function will return NULL if
   the allocation fails, otherwise the function will abort() if the
   allocation fails.

   Before returning all elements will be initialized to zero.

   1. It is based on first free() of the original pointer, and then
       subsequently calling malloc() to get new storage. This is to
       avoid prohibitive temporary memory requirements during the
       realloc() call.

    2. If the malloc() fails the function will return NULL, i.e. you
       will NOT keep the original data pointer. I.e. in this case the
       matrix will be invalid. It is the responsability of the calling
       scope to do the right thing.

    3. realloc() functionality - i.e. keeping the original content of
       the matrix is implemented at higher level. The memory layout of
       the matrix will in general change anyway; so the promise made
       by realloc() is not very interesting.
*/

static void matrix_realloc_data__( matrix_type * matrix , bool safe_mode ) {
  if (matrix->data_owner) {
    size_t data_size = MATRIX_DATA_SIZE( matrix );
    if (matrix->data_size == data_size) return;
    if (matrix->data != NULL)
      free(matrix->data);

    if (safe_mode) {
      /*
         If safe_mode == true it is 'OK' to fail in the allocation,
         otherwise we use util_malloc() which will abort if the memory
         is not available.
      */
      matrix->data = (double*)malloc( sizeof * matrix->data * data_size );
    } else
      matrix->data = (double*)util_malloc( sizeof * matrix->data * data_size );


    /* Initializing matrix content to zero. */
    if (matrix->data != NULL) {
      size_t i;
                for (i = 0; i < data_size; i++)
        matrix->data[i] = 0;
    } else
      data_size = 0;

    /**
       Observe that if the allocation failed the matrix will
       be returned with data == NULL, and data_size == 0.
    */
    matrix->data_size = data_size;
  } else
    util_abort("%s: can not manipulate memory when is not data owner\n",__func__);
}


UTIL_SAFE_CAST_FUNCTION( matrix , MATRIX_TYPE_ID )

/**
   The matrix objecty is NOT ready for use after this function.
*/
static matrix_type * matrix_alloc_empty( ) {
  matrix_type * matrix  = (matrix_type*)util_malloc( sizeof * matrix );
  UTIL_TYPE_ID_INIT( matrix , MATRIX_TYPE_ID );
  matrix->name = NULL;
  return matrix;
}

/*
  The freshly allocated matrix is explicitly initialized to zero. If
  the variable safe_mode equals true the function will return NULL if
  the allocation of data fails, otherwise it will abort() if the
  allocation fails.
*/
static matrix_type * matrix_alloc_with_stride(int rows , int columns , int row_stride , int column_stride, bool safe_mode) {
  matrix_type * matrix = NULL;
  if ((rows > 0) && (columns > 0)) {
    matrix = matrix_alloc_empty();
    matrix->data      = NULL;
    matrix->data_size = 0;
    matrix_init_header( matrix , rows , columns , row_stride , column_stride);
    matrix->data_owner    = true;
    matrix_realloc_data__( matrix  , safe_mode );
    if (safe_mode) {
      if (matrix->data == NULL) {
        /* Allocation failed - we return NULL */
        matrix_free(matrix);
        matrix = NULL;
      }
    }
  }
  return matrix;
}


void matrix_set_name( matrix_type * matrix , const char * name) {
  matrix->name = util_realloc_string_copy( matrix->name , name );
}


/**
   This function will allocate a matrix object where the data is
   shared with the 'src' matrix. A matrix allocated in this way can be
   used with all the matrix_xxx functions, but you should be careful
   when exporting the data pointer to e.g. lapack routines.
*/

matrix_type * matrix_alloc_shared(const matrix_type * src , int row , int column , int rows , int columns) {
  if (((row + rows) > src->rows) || ((column + columns) > src->columns))
    util_abort("%s: Invalid matrix subsection src:[%d,%d]  Offset:[%d,%d]  SubSize:[%d,%d] \n",
               __func__,
               src->rows , src->columns,
               row,column,
               rows,columns);

  {
    matrix_type * matrix = matrix_alloc_empty();

    matrix_init_header( matrix , rows , columns , src->row_stride , src->column_stride);
    matrix->data          = &src->data[ GET_INDEX(src , row , column) ];
    matrix->data_owner    = false;

    return matrix;
  }
}


matrix_type * matrix_alloc_view(double * data , int rows , int columns) {
  matrix_type * matrix = matrix_alloc_empty();

  matrix_init_header( matrix , rows , columns , 1 , rows);
  matrix->data          = data;
  matrix->data_owner    = false;

  return matrix;
}


/**
   This function will allocate a matrix structure; this matrix
   structure will TAKE OWNERSHIP OF THE SUPPLIED DATA. This means that
   it is (at the very best) highly risky to use the data pointer in
   the calling scope after the matrix has been allocated. If the
   supplied pointer is too small it is immediately realloced ( in
   which case the pointer in the calling scope will be immediately
   invalid).
*/

matrix_type * matrix_alloc_steal_data(int rows , int columns , double * data , int data_size) {
  matrix_type * matrix = matrix_alloc_empty();
  matrix_init_header( matrix , rows , columns , 1 , rows );
  matrix->data_size  = data_size;           /* Can in general be different from rows * columns */
  matrix->data_owner = true;
  matrix->data       = data;
  if (data_size < rows * columns)
    matrix_realloc_data__(matrix , false);

  return matrix;
}



/*****************************************************************/

static matrix_type * matrix_alloc__(int rows, int columns , bool safe_mode) {
  return matrix_alloc_with_stride( rows , columns , 1 , rows , safe_mode );    /* Must be the stride (1,rows) to use the lapack routines. */
}

matrix_type * matrix_alloc(int rows, int columns) {
  return matrix_alloc__( rows , columns , false );
}

matrix_type * matrix_safe_alloc(int rows, int columns) {
  return matrix_alloc__( rows , columns , true );
}

matrix_type * matrix_alloc_identity(int dim) {
  if (dim < 1)
    util_abort("%s: identity matrix must have positive size. \n",__func__);

  matrix_type * idty = matrix_alloc(dim, dim);
  for (int i = 0; i < dim; ++i)
    matrix_iset(idty, i, i, 1);
  return idty;
}

/*****************************************************************/

/**
   Will not respect strides - that is considered low level data
   layout.
*/
static matrix_type * matrix_alloc_copy__( const matrix_type * src , bool safe_mode) {
  matrix_type * copy = matrix_alloc__( matrix_get_rows( src ), matrix_get_columns( src ) , safe_mode);
  if (copy != NULL)
    matrix_assign(copy , src);
  return copy;
}


matrix_type * matrix_alloc_copy(const matrix_type * src) {
  return matrix_alloc_copy__(src , false );
}

matrix_type * matrix_alloc_column_compressed_copy(const matrix_type * src, const bool_vector_type * mask) {
  if (bool_vector_size( mask ) != matrix_get_columns( src ))
    util_abort("%s: size mismatch. Src matrix has %d rows  mask has:%d elements\n", __func__ , matrix_get_rows( src ) , bool_vector_size( mask ));
  {
    int target_columns = bool_vector_count_equal( mask , true );
    matrix_type * target = matrix_alloc( matrix_get_rows( src ) , target_columns );

    matrix_column_compressed_memcpy( target , src , mask );
    return target;
  }
}


void matrix_column_compressed_memcpy(matrix_type * target, const matrix_type * src, const bool_vector_type * mask) {
  if (bool_vector_count_equal( mask , true ) != matrix_get_columns( target ))
    util_abort("%s: size mismatch. \n",__func__);

  if (bool_vector_size( mask ) != matrix_get_columns( src))
    util_abort("%s: size mismatch. \n",__func__);

  {
    int target_col = 0;
    int src_col;
    for (src_col = 0; src_col < bool_vector_size( mask ); src_col++) {
      if (bool_vector_iget( mask , src_col)) {
        matrix_copy_column( target , src , target_col , src_col);
        target_col++;
      }
    }
  }
}



matrix_type * matrix_realloc_copy(matrix_type * T , const matrix_type * src) {
  if (T == NULL)
    return matrix_alloc_copy( src );
  else {
    matrix_resize( T , src->rows , src->columns , false );
    matrix_assign( T , src );
    return T;
  }
}




/**
   Will return NULL if allocation of the copy failed.
*/

matrix_type * matrix_safe_alloc_copy(const matrix_type * src) {
  return matrix_alloc_copy__(src , true);
}

void matrix_copy_block( matrix_type * target_matrix , int target_row , int target_column , int rows , int columns,
                        const matrix_type * src_matrix , int src_row , int src_column) {
  matrix_type * target_view = matrix_alloc_shared(target_matrix , target_row , target_column , rows , columns);
  matrix_type * src_view    = matrix_alloc_shared( src_matrix , src_row , src_column , rows , columns);
  matrix_assign( target_view , src_view );
  matrix_free( target_view );
  matrix_free( src_view );
}

matrix_type * matrix_alloc_sub_copy( const matrix_type * src , int row_offset , int column_offset , int rows, int columns) {
  matrix_type * copy = matrix_alloc( rows, columns );
  matrix_copy_block( copy , 0 , 0 , rows , columns , src , row_offset , column_offset );
  return copy;
}


/*****************************************************************/

static bool matrix_resize__(matrix_type * matrix , int rows , int columns , bool copy_content , bool safe_mode) {
  if (!matrix->data_owner)
    util_abort("%s: sorry - can not resize shared matrizes. \n",__func__);
  {
    bool resize_OK = true;

    if ((rows != matrix->rows) || (columns != matrix->columns)) {
      int copy_rows            = util_int_min( rows    , matrix->rows );
      int copy_columns         = util_int_min( columns , matrix->columns);
      matrix_type * copy_view  = NULL;
      matrix_type * copy       = NULL;

      if (copy_content) {
        copy_view = matrix_alloc_shared( matrix , 0 , 0 , copy_rows , copy_columns);         /* This is the part of the old matrix which should be copied over to the new. */
        copy      = matrix_alloc_copy__( copy_view , safe_mode );                            /* Now copy contains the part of the old matrix which should be copied over - with private storage. */
      }
      {
        int old_rows , old_columns, old_row_stride , old_column_stride;
        matrix_get_dims( matrix , &old_rows , &old_columns , &old_row_stride , &old_column_stride);        /* Storing the old header information - in case the realloc() fails. */

        matrix_init_header(matrix , rows , columns , 1 , rows);                                            /* Resetting the header for the matrix */
        matrix_realloc_data__(matrix , safe_mode);
        if (matrix->data != NULL) {  /* Realloc succeeded */
          if (copy_content) {
            matrix_type * target_view = matrix_alloc_shared(matrix , 0 , 0 , copy_rows , copy_columns);
            matrix_assign( target_view , copy);
            matrix_free( target_view );
          }
        } else {
          /* Failed to realloc new storage; RETURNING AN INVALID MATRIX */
          matrix_init_header(matrix , old_rows , old_columns , old_row_stride , old_column_stride);
          resize_OK = false;
        }
      }

      if (copy_content) {
        matrix_free(copy_view);
        matrix_free(copy);
      }
    }
    return resize_OK;
  }
}


/**
    If copy content is true the content of the old matrix is carried
    over to the new one, otherwise the new matrix is cleared.

    Will always return true (or abort).
*/
bool matrix_resize(matrix_type * matrix , int rows , int columns , bool copy_content) {
  return matrix_resize__(matrix , rows , columns , copy_content , false);
}


/**
   Return true if the resize succeded, otherwise it will return false
   and leave the matrix unchanged. When resize implies expanding a
   dimension, the newly created elements will be explicitly
   initialized to zero.

   If copy_content is set to false the new matrix will be fully
   initialized to zero.
*/

bool matrix_safe_resize(matrix_type * matrix , int rows , int columns , bool copy_content) {
  return matrix_resize__(matrix , rows , columns , copy_content , true);
}



/**
    This function will ensure that the matrix has at least 'rows'
    rows. If the present matrix already has >= rows it will return
    immediately, otherwise the matrix will be resized.
*/

void matrix_ensure_rows(matrix_type * matrix, int rows, bool copy_content) {
  if (matrix->rows < rows)
    matrix_resize( matrix , rows , matrix->columns , copy_content);
}



/**
    This function will reduce the size of the matrix. It will only
    affect the headers, and not touch the actual memory of the matrix.
*/

void matrix_shrink_header(matrix_type * matrix , int rows , int columns) {

  if (rows <= matrix->rows)
    matrix->rows = rows;

  if (columns <= matrix->columns)
    matrix->columns = columns;

}


void matrix_full_size( matrix_type * matrix ) {
  matrix->rows    = matrix->alloc_rows;
  matrix->columns = matrix->alloc_columns;
}


/*****************************************************************/

static void matrix_free_content(matrix_type * matrix) {
  if (matrix->data_owner)
    util_safe_free(matrix->data);
  util_safe_free( matrix->name );
}

void matrix_free(matrix_type * matrix) {
  matrix_free_content( matrix );
  free(matrix);
}


void matrix_safe_free( matrix_type * matrix ) {
  if (matrix != NULL)
  matrix_free( matrix );
}


/*****************************************************************/
void matrix_pretty_fprint_submat(const matrix_type * matrix , const char * name , const char * fmt , FILE * stream, int m, int M, int n, int N) {
  int i,j;

 if (m<0 || m>M || M >= matrix->rows || n<0 || n>N || N >= matrix->columns)
         util_abort("%s: matrix:%s not compatible with print subdimensions. \n",__func__ , matrix->name);

 fprintf(stream ,  "%s =" , name);
  for (i=m; i < M; i++) {
    fprintf(stream , " [");
    for (j=n; j < N; j++)
      fprintf(stream , fmt , matrix_iget(matrix , i,j));
    fprintf(stream , "]\n");
  }
}
/*****************************************************************/

void matrix_pretty_fprint(const matrix_type * matrix , const char * name , const char * fmt , FILE * stream) {
  int i,j;
  for (i=0; i < matrix->rows; i++) {

    if (i == (matrix->rows / 2))
      fprintf(stream ,  "%s =" , name);
    else {
      int l;
      for (l = 0; l < strlen(name) + 2; l++)
        fprintf(stream ,  " ");
    }

    fprintf(stream , " [");
    for (j=0; j < matrix->columns; j++)
      fprintf(stream , fmt , matrix_iget(matrix , i,j));
    fprintf(stream , "]\n");
  }
}


void matrix_pretty_print(const matrix_type * matrix , const char * name , const char * fmt) {
  matrix_pretty_fprint(matrix , name , fmt , stdout );
}


void matrix_fprintf( const matrix_type * matrix , const char * fmt , FILE * stream ) {
  int i,j;
  for (i=0; i < matrix->rows; i++) {
    for (j=0; j < matrix->columns; j++)
      fprintf(stream , fmt , matrix_iget( matrix , i , j));
    fprintf(stream , "\n");
  }
}


void matrix_dump_csv( const matrix_type * matrix  ,const char * filename) {
  FILE * stream = util_fopen(filename , "w");
  for (int i=0; i < matrix->rows; i++) {
    for (int j=0; j < matrix->columns - 1; j++)
      fprintf(stream , "%g, " , matrix_iget( matrix , i , j));
    fprintf(stream , "%g\n" , matrix_iget( matrix , i , matrix->columns - 1));
  }
  fclose( stream );
}


void matrix_fwrite(const matrix_type * matrix , FILE * stream) {
  util_fwrite_int( matrix->rows , stream );
  util_fwrite_int( matrix->columns , stream );

  if (matrix->column_stride == matrix->rows)
    util_fwrite( matrix->data , sizeof * matrix->data , matrix->columns * matrix->rows , stream , __func__);
  else {
    int column;
    for (column=0; column < matrix->columns; column++) {
      if (matrix->row_stride == 1) {
        const double * column_data = &matrix->data[ column * matrix->column_stride ];
        util_fwrite( column_data , sizeof * column_data , matrix->rows , stream , __func__);
      } else {
        int row;
        for (row=0; row < matrix->rows; row++)
          util_fwrite_double( matrix->data[ GET_INDEX( matrix , row , column )] , stream);
      }
    }
  }
}


void matrix_fread(matrix_type * matrix , FILE * stream) {
  int rows    = util_fread_int( stream );
  int columns = util_fread_int( stream );

  matrix_resize( matrix , rows , columns , false);
  if (matrix->column_stride == matrix->rows)
    util_fread( matrix->data , sizeof * matrix->data , matrix->columns * matrix->rows , stream , __func__);
  else {
    int column;
    for (column=0; column < matrix->columns; column++) {
      if (matrix->row_stride == 1) {
        double * column_data = &matrix->data[ column * matrix->column_stride ];
        util_fread( column_data , sizeof * column_data , matrix->rows , stream , __func__);
      } else {
        int row;
        for (row=0; row < matrix->rows; row++)
          matrix->data[ GET_INDEX( matrix , row , column )] = util_fread_double( stream );
      }
    }
  }
}

matrix_type * matrix_fread_alloc(FILE * stream) {
  matrix_type * matrix = matrix_alloc(1,1);
  matrix_fread(matrix , stream);
  return matrix;
}


/**
     [ a11   a12  ]
     [ a21   a22  ]




   row_major_order == true
   -----------------------
   a_11
   a_12
   a_21
   a_22


   row_major_order == false
   -----------------------
   a_11
   a_12
   a_21
   a_22


   The @orw_major_order parameter ONLY affects the layout on the file,
   and NOT the memory layout of the matrix.
*/


static void __fscanf_and_set( matrix_type * matrix , int row , int col , FILE * stream) {
  double value;
  if (fscanf(stream , "%lg" , &value) == 1)
    matrix_iset( matrix , row , col , value );
  else
    util_abort("%s: reading of matrix failed at row:%d  col:%d \n",__func__ , row , col);
}


void matrix_fscanf_data( matrix_type * matrix , bool row_major_order , FILE * stream ) {
  int row,col;
  if (row_major_order) {
    for (row = 0; row < matrix->columns; row++) {
      for (col = 0; col < matrix->columns; col++) {
        __fscanf_and_set( matrix , row , col ,stream);
      }
    }
  } else {
    for (row = 0; row < matrix->columns; row++) {
      for (col = 0; col < matrix->columns; col++) {
        __fscanf_and_set( matrix , row , col , stream);
      }
    }
  }
}



/*****************************************************************/
/* Functions which manipulate one element in the matrix.         */

static void matrix_assert_ij( const matrix_type * matrix , int i , int j) {
  if ((i < 0) || (i >= matrix->rows) || (j < 0) || (j >= matrix->columns))
    util_abort("%s: (i,j) = (%d,%d) invalid. Matrix size: %d x %d \n",__func__ , i,j,matrix->rows , matrix->columns);
}


static void matrix_assert_equal_rows( const matrix_type * m1 , const matrix_type * m2) {
  if (m1->rows != m2->rows)
    util_abort("%s: size mismatch in binary matrix operation %d %d \n",__func__ , m1->rows , m2->rows);
}


static void matrix_assert_equal_columns( const matrix_type * m1 , const matrix_type * m2) {
  if (m1->columns != m2->columns)
    util_abort("%s: size mismatch in binary matrix operation %d %d \n",__func__ , m1->columns , m2->columns);
}


void matrix_iset(matrix_type * matrix , int i , int j, double value) {
  matrix->data[ GET_INDEX(matrix , i,j) ] = value;
}



void matrix_iset_safe(matrix_type * matrix , int i , int j, double value) {
  matrix_assert_ij( matrix , i , j );
  matrix_iset( matrix , i , j , value );
}


double matrix_iget(const matrix_type * matrix , int i , int j) {
  return matrix->data[ GET_INDEX(matrix , i, j) ];
}


double matrix_iget_safe(const matrix_type * matrix , int i , int j) {
  matrix_assert_ij( matrix , i , j );
  return matrix_iget( matrix , i , j );
}


void  matrix_iadd(matrix_type * matrix , int i , int j , double value) {
  matrix->data[ GET_INDEX(matrix , i,j) ] += value;
}


void  matrix_isub(matrix_type * matrix , int i , int j , double value) {
  matrix->data[ GET_INDEX(matrix , i,j) ] -= value;
}


void  matrix_imul(matrix_type * matrix , int i , int j , double value) {
  matrix->data[ GET_INDEX(matrix , i,j) ] *= value;
}


/*****************************************************************/
/* One scalar operating on all the elements in the matrix         */

void matrix_set(matrix_type * matrix, double value) {
  int i,j;
  for (j=0; j < matrix->columns; j++)
    for (i=0; i < matrix->rows; i++)
      matrix_iset(matrix , i , j , value);
}


void matrix_shift(matrix_type * matrix, double value) {
  int i,j;
  for (j=0; j < matrix->columns; j++)
    for (i=0; i < matrix->rows; i++)
      matrix_iadd(matrix , i , j , value);
}


void matrix_scale(matrix_type * matrix, double value) {
  int i,j;
  for (j=0; j < matrix->columns; j++)
    for (i=0; i < matrix->rows; i++)
      matrix_imul(matrix , i , j , value);
}

/*****************************************************************/
/* Functions working on rows & columns                           */

void matrix_set_many_on_column(matrix_type * matrix , int row_offset , int elements , const double * data , int column) {
  if ((row_offset + elements) <= matrix->rows) {
    if (matrix->row_stride == 1)  /* Memory is continous ... */
      memcpy( &matrix->data[ GET_INDEX( matrix , row_offset , column) ] , data , elements * sizeof * data);
    else {
      int i;
      for (i = 0; i < elements; i++)
        matrix->data[ row_offset + GET_INDEX( matrix , i , column) ] = data[i];
    }
  } else
    util_abort("%s: range violation \n" , __func__);
}

void matrix_set_column(matrix_type * matrix , const double * data , int column) {
  matrix_set_many_on_column( matrix , 0 , matrix->rows , data , column );
}


void matrix_set_const_column(matrix_type * matrix , const double value , int column) {
  int row;
  for (row = 0; row < matrix->rows; row++)
    matrix->data[ GET_INDEX( matrix , row , column) ] = value;
}


void matrix_set_const_row(matrix_type * matrix , const double value , int row) {
  int column;
  for (column = 0; column < matrix->columns; column++)
    matrix->data[ GET_INDEX( matrix , row , column) ] = value;
}


void matrix_copy_column(matrix_type * target_matrix, const matrix_type * src_matrix , int target_column, int src_column) {
  matrix_assert_equal_rows( target_matrix , src_matrix );
  {
    int row;
    for(row = 0; row < target_matrix->rows; row++)
      target_matrix->data[ GET_INDEX( target_matrix, row , target_column)] = src_matrix->data[ GET_INDEX( src_matrix, row, src_column)];
  }
}


void matrix_scale_column(matrix_type * matrix , int column  , double scale_factor) {
  int row;
  for (row = 0; row < matrix->rows; row++)
    matrix->data[ GET_INDEX( matrix , row , column) ] *= scale_factor;
}

void matrix_scale_row(matrix_type * matrix , int row  , double scale_factor) {
  int column;
  for (column = 0; column < matrix->columns; column++)
    matrix->data[ GET_INDEX( matrix , row , column) ] *= scale_factor;
}

void matrix_copy_row(matrix_type * target_matrix, const matrix_type * src_matrix , int target_row, int src_row) {
  matrix_assert_equal_columns( target_matrix , src_matrix );
  {
    int col;
    for(col = 0; col < target_matrix->columns; col++)
      target_matrix->data[ GET_INDEX( target_matrix , target_row , col)] = src_matrix->data[ GET_INDEX( src_matrix, src_row, col)];
  }
}


/*****************************************************************/
/* Functions for dot products between rows/columns in matrices. */

double matrix_column_column_dot_product(const matrix_type * m1 , int col1 , const matrix_type * m2 , int col2) {
  if (m1->rows != m2->rows)
    util_abort("%s: size mismatch \n",__func__);

  if (col1 >= m1->columns || col2 >= m2->columns)
    util_abort("%s: size mismatch \n",__func__);
  {
    int row;
    double sum = 0;
    for( row = 0; row < m1->rows; row++)
      sum += m1->data[ GET_INDEX(m1 , row , col1) ] * m2->data[ GET_INDEX(m2, row , col2) ];

    return sum;
  }
}


double matrix_row_column_dot_product(const matrix_type * m1 , int row1 , const matrix_type * m2 , int col2) {
  if (m1->columns != m2->rows)
    util_abort("%s: size mismatch: m1:[%d,%d]   m2:[%d,%d] \n",__func__ , matrix_get_rows( m1 ) , matrix_get_columns( m1 ) , matrix_get_rows( m2 ) , matrix_get_columns( m2 ));

  {
    int k;
    double sum = 0;
    for( k = 0; k < m1->columns; k++)
      sum += m1->data[ GET_INDEX(m1 , row1 , k) ] * m2->data[ GET_INDEX(m2, k , col2) ];

    return sum;
  }
}




/*****************************************************************/
/* Matrix - matrix operations                                    */


/* Implements assignement: A = B */
void matrix_assign(matrix_type * A , const matrix_type * B) {
  if ((A->rows == B->rows) && (A->columns == B->columns)) {
    int i,j;

    if (A->row_stride == B->row_stride) {
      if (A->columns == A->row_stride)  /** Memory is just one continous block */
        memcpy( A->data , B->data , A->rows * A->columns * sizeof * A->data);
      else {
        /* Copying columns of data */
        for (j = 0; j < A->columns; j++)
          memcpy( &A->data[ GET_INDEX(A , 0 , j)] , &B->data[ GET_INDEX(B , 0 , j) ] , A->rows * sizeof * A->data);
      }
    } else {
      /* Copying element by element */
      for (j = 0; j < A->columns; j++)
        for (i=0; i < A->rows; i++)
          A->data[ GET_INDEX(A,i,j) ] = B->data[ GET_INDEX(B,i,j) ];
    }
  } else
    util_abort("%s: size mismatch A:[%d,%d]  B:[%d,%d] \n",__func__ , A->rows , A->columns , B->rows , B->columns);
}



void matrix_inplace_sub_column(matrix_type * A , const matrix_type * B, int colA , int colB) {
  if ((A->rows == B->rows) &&
      (colA < A->columns) &&
      (colB < B->columns)) {
    int row;

    for (row = 0; row < A->rows; row++)
      A->data[ GET_INDEX(A , row , colA)] -= B->data[ GET_INDEX(B , row , colB)];

  } else
    util_abort("%s: size mismatch \n",__func__);
}

void matrix_inplace_add_column(matrix_type * A , const matrix_type * B, int colA , int colB) {
  if ((A->rows == B->rows) &&
      (colA < A->columns) &&
      (colB < B->columns)) {
    int row;

    for (row = 0; row < A->rows; row++)
      A->data[ GET_INDEX(A , row , colA)] += B->data[ GET_INDEX(B , row , colB)];

  } else
    util_abort("%s: size mismatch \n",__func__);
}

/* Updates matrix A by adding in matrix B - elementwise. */
void matrix_inplace_add(matrix_type * A , const matrix_type * B) {
  if ((A->rows == B->rows) && (A->columns == B->columns)) {
    int i,j;

    for (j = 0; j < A->columns; j++)
      for (i=0; i < A->rows; i++)
        A->data[ GET_INDEX(A,i,j) ] += B->data[ GET_INDEX(B,i,j) ];

  } else
    util_abort("%s: size mismatch \n",__func__);
}


/* Updates matrix A by multiplying in matrix B - elementwise - i.e. Schur product. */
void matrix_inplace_mul(matrix_type * A , const matrix_type * B) {
  if ((A->rows == B->rows) && (A->columns == B->columns)) {
    int i,j;

    for (j = 0; j < A->columns; j++)
      for (i=0; i < A->rows; i++)
        A->data[ GET_INDEX(A,i,j) ] *= B->data[ GET_INDEX(B,i,j) ];

  } else
    util_abort("%s: size mismatch \n",__func__);
}


/*
  Schur product:  A = B * C
*/

void matrix_mul( matrix_type * A , const matrix_type * B , const matrix_type * C) {
  if ((A->rows == B->rows) && (A->columns == B->columns) && (A->rows == C->rows) && (A->columns == C->columns)) {
    int i,j;

    for (j = 0; j < A->columns; j++)
      for (i=0; i < A->rows; i++)
        A->data[ GET_INDEX(A,i,j) ] = B->data[ GET_INDEX(B,i,j) ] * C->data[ GET_INDEX(B,i,j) ];

  } else
    util_abort("%s: size mismatch \n",__func__);
}



/* Updates matrix A by subtracting matrix B - elementwise. */
void matrix_inplace_sub(matrix_type * A , const matrix_type * B) {
  if ((A->rows == B->rows) && (A->columns == B->columns)) {
    int i,j;

    for (j = 0; j < A->columns; j++)
      for (i=0; i < A->rows; i++)
        A->data[ GET_INDEX(A,i,j) ] -= B->data[ GET_INDEX(B,i,j) ];

  } else
    util_abort("%s: size mismatch  A:[%d,%d]   B:[%d,%d]\n",__func__ ,
               matrix_get_rows(A),
               matrix_get_columns(A),
               matrix_get_rows(B),
               matrix_get_columns(B));
}


/* Does the matrix operation:

   A = B - C

   elementwise.
*/
void matrix_sub(matrix_type * A , const matrix_type * B , const matrix_type * C) {
  if ((A->rows == B->rows) && (A->columns == B->columns) && (A->rows == C->rows) && (A->columns == C->columns)) {
    int i,j;

    for (j = 0; j < A->columns; j++)
      for (i=0; i < A->rows; i++)
        A->data[ GET_INDEX(A,i,j) ] = B->data[ GET_INDEX(B,i,j) ] - C->data[ GET_INDEX(B,i,j) ];

  } else
    util_abort("%s: size mismatch \n",__func__);
}




/* Updates matrix A by dividing matrix B - elementwise. */
void matrix_inplace_div(matrix_type * A , const matrix_type * B) {
  if ((A->rows == B->rows) && (A->columns == B->columns)) {
    int i,j;

    for (j = 0; j < A->columns; j++)
      for (i=0; i < A->rows; i++)
        A->data[ GET_INDEX(A,i,j) ] /= B->data[ GET_INDEX(B,i,j) ];

  } else
    util_abort("%s: size mismatch \n",__func__);
}


/**
   Observe that A and T should not overlap, i.e. the call

         matrix_transpose(X , X)

   will fail in mysterious ways.
*/

void matrix_transpose(const matrix_type * A , matrix_type * T) {
  if ((A->columns == T->rows) && (A->rows == T->columns)) {
    int i,j;
    for (i=0; i < A->rows; i++) {
      for (j=0; j < A->columns; j++) {
        size_t src_index    = GET_INDEX(A , i , j );
        size_t target_index = GET_INDEX(T , j , i );

        T->data[ target_index ] = A->data[ src_index ];
      }
    }
  } else
    util_abort("%s: size mismatch\n",__func__);
}


void matrix_inplace_transpose(matrix_type * A ) {
  matrix_type * B = matrix_alloc_transpose( A );
  matrix_free_content( A );
  memcpy( A , B , sizeof * A );
  free( B );
}



matrix_type * matrix_alloc_transpose( const matrix_type * A) {
  matrix_type * B = matrix_alloc( matrix_get_columns( A ) , matrix_get_rows( A ));
  matrix_transpose( A , B );
  return B;
}



/**
   For this function to work the following must be satisfied:

     columns in A == rows in B == columns in B

   For general matrix multiplactions where A = B * C all have
   different dimensions you can use matrix_matmul() (which calls the
   BLAS routine dgemm());
*/


void matrix_inplace_matmul(matrix_type * A, const matrix_type * B) {
  if ((A->columns == B->rows) && (B->rows == B->columns)) {
    double * tmp = (double*)util_malloc( sizeof * A->data * A->columns );
    int i,j,k;

    for (i=0; i < A->rows; i++) {

      /* Clearing the tmp vector */
      for (k=0; k < B->rows; k++)
        tmp[k] = 0;

      for (j=0; j < B->rows; j++) {
        double scalar_product = 0;
        for (k=0; k < A->columns; k++)
          scalar_product += A->data[ GET_INDEX(A,i,k) ] * B->data[ GET_INDEX(B,k,j) ];

        /* Assign first to tmp[j] */
        tmp[j] = scalar_product;
      }
      for (j=0; j < A->columns; j++)
        A->data[ GET_INDEX(A , i, j) ] = tmp[j];
    }
    free(tmp);
  } else
    util_abort("%s: size mismatch: A:[%d,%d]   B:[%d,%d]\n",__func__ , matrix_get_rows(A) , matrix_get_columns(A) , matrix_get_rows(B) , matrix_get_columns(B));
}

/*****************************************************************/
/* If the current build has a thread_pool implementation enabled a
   proper matrix_implace_matmul_mt() function will be built, otherwise
   only the serial version matrix_inplace_matmul() will be used.  */


#ifdef ERT_HAVE_THREAD_POOL

static void * matrix_inplace_matmul_mt__(void * arg) {

  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  int row_offset         =                     arg_pack_iget_int( arg_pack , 0 );
  int rows               =                     arg_pack_iget_int( arg_pack , 1 );
  matrix_type * A        = (matrix_type*)      arg_pack_iget_ptr( arg_pack , 2 );
  const matrix_type * B  = (const matrix_type*)arg_pack_iget_const_ptr( arg_pack , 3 );

  matrix_type * A_view = matrix_alloc_shared( A , row_offset , 0 , rows , matrix_get_columns( A ));
  matrix_inplace_matmul( A_view , B );
  matrix_free( A_view );
  return NULL;
}

/**
   Observe that the calling scope is responsible for passing a
   thread_pool in suitable state to this function. This implies one of
   the following:

     1. The thread_pool is newly created, with the @start_queue
        argument set to false.

     2. The thread_pool has been joined __without__ an interevening
        call to thread_pool_restart().

   If the thread_pool has not been correctly prepared, according to
   this specification, it will be crash and burn.
*/

void matrix_inplace_matmul_mt2(matrix_type * A, const matrix_type * B , thread_pool_type * thread_pool){
  int num_threads  = thread_pool_get_max_running( thread_pool );
  arg_pack_type    ** arglist = (arg_pack_type**)util_malloc( num_threads * sizeof * arglist );
  int it;
  thread_pool_restart( thread_pool );
  {
    int rows       = matrix_get_rows( A ) / num_threads;
    int rows_mod   = matrix_get_rows( A ) % num_threads;
    int row_offset = 0;

    for (it = 0; it < num_threads; it++) {
      int row_size;
      arglist[it] = arg_pack_alloc();
      row_size = rows;
      if (it < rows_mod)
        row_size += 1;

      arg_pack_append_int(arglist[it] , row_offset );
      arg_pack_append_int(arglist[it] , row_size   );
      arg_pack_append_ptr(arglist[it] , A );
      arg_pack_append_const_ptr(arglist[it] , B );

      thread_pool_add_job( thread_pool , matrix_inplace_matmul_mt__ , arglist[it]);
      row_offset += row_size;
    }
  }
  thread_pool_join( thread_pool );

  for (it = 0; it < num_threads; it++)
    arg_pack_free( arglist[it] );
  free( arglist );
}

void matrix_inplace_matmul_mt1(matrix_type * A, const matrix_type * B , int num_threads){
  thread_pool_type  * thread_pool = thread_pool_alloc( num_threads , false );
  matrix_inplace_matmul_mt2( A , B , thread_pool );
  thread_pool_free( thread_pool );
}

#else

void matrix_inplace_matmul_mt1(matrix_type * A, const matrix_type * B , int num_threads){
  matrix_inplace_matmul( A , B );
}

#endif




/*****************************************************************/
/* Row/column functions                                          */

double matrix_get_row_sum(const matrix_type * matrix , int row) {
  double sum = 0;
  int j;
  for (j=0; j < matrix->columns; j++)
    sum += matrix->data[ GET_INDEX( matrix , row , j ) ];
  return sum;
}


double matrix_get_column_sum(const matrix_type * matrix , int column) {
  double sum = 0;
  int i;
  for (i=0; i < matrix->rows; i++)
    sum += matrix->data[ GET_INDEX( matrix , i , column ) ];
  return sum;
}


double matrix_get_column_abssum(const matrix_type * matrix , int column) {
  double sum = 0;
  int i;
  for (i=0; i < matrix->rows; i++)
    sum += fabs( matrix->data[ GET_INDEX( matrix , i , column ) ] );
  return sum;
}


double matrix_get_row_sum2(const matrix_type * matrix , int row) {
  double sum2 = 0;
  int j;
  for ( j=0; j < matrix->columns; j++) {
    double m = matrix->data[ GET_INDEX( matrix , row , j ) ];
    sum2 += m*m;
  }
  return sum2;
}


double matrix_get_row_abssum(const matrix_type * matrix , int row) {
  double sum_abs = 0;
  int j;
  for ( j=0; j < matrix->columns; j++) {
    double m = matrix->data[ GET_INDEX( matrix , row , j ) ];
    sum_abs += fabs( m );
  }
  return sum_abs;
}

/**
   Return the sum of the squares on column.
*/
double matrix_get_column_sum2(const matrix_type * matrix , int column) {
  double sum2 = 0;
  int i;
  for ( i=0; i < matrix->rows; i++) {
    double m = matrix->data[ GET_INDEX( matrix , i , column ) ];
    sum2 += m*m;
  }
  return sum2;
}



void matrix_shift_column(matrix_type * matrix , int column, double shift) {
  int i;
  for ( i=0; i < matrix->rows; i++)
    matrix->data[ GET_INDEX( matrix , i , column) ] += shift;
}


void matrix_shift_row(matrix_type * matrix , int row , double shift) {
   int j;
  for ( j=0; j < matrix->columns; j++)
    matrix->data[ GET_INDEX( matrix , row , j ) ] += shift;
}



/**
   For each row in the matrix we will do the operation

     R -> R - <R>
*/

void matrix_subtract_row_mean(matrix_type * matrix) {
		int i;
		for ( i=0; i < matrix->rows; i++) {
    double row_mean = matrix_get_row_sum(matrix , i) / matrix->columns;
    matrix_shift_row( matrix , i , -row_mean);
  }
}

void matrix_subtract_and_store_row_mean(matrix_type * matrix, matrix_type * row_mean) {
        int i;
        for ( i=0; i < matrix->rows; i++) {
    double mean = matrix_get_row_sum(matrix , i) / matrix->columns;
    matrix_shift_row( matrix , i , -mean);
    matrix_iset(row_mean , i , 0, mean );
  }
}

void matrix_imul_col( matrix_type * matrix , int column , double factor) {
   int i;
   for ( i=0; i < matrix->rows; i++)
    matrix_imul( matrix , i , column , factor );
}


/*****************************************************************/
/**
   This function will return the double data pointer of the matrix,
   when you use this explicitly you ARE ON YOUR OWN.
*/

double * matrix_get_data(const matrix_type * matrix) {
  return matrix->data;
}

/**
   The query functions below can be used to ask for the dimensions &
   strides of the matrix.
*/

int matrix_get_rows(const matrix_type * matrix) {
  return matrix->rows;
}

int matrix_get_columns(const matrix_type * matrix) {
  return matrix->columns;
}

int matrix_get_row_stride(const matrix_type * matrix) {
  return matrix->row_stride;
}

int matrix_get_column_stride(const matrix_type * matrix) {
  return matrix->column_stride;
}


void matrix_get_dims(const matrix_type * matrix ,  int * rows , int * columns , int * row_stride , int * column_stride) {

  *rows          = matrix->rows;
  *columns       = matrix->columns;
  *row_stride    = matrix->row_stride;
  *column_stride = matrix->column_stride;

}


bool matrix_is_quadratic(const matrix_type * matrix) {
  if (matrix->rows == matrix->columns)
    return true;
  else
    return false;
}

/**
   Goes through all the elements in the matrix - and return true if they are all finite.
*/

#ifdef ERT_HAVE_ISFINITE

bool matrix_is_finite(const matrix_type * matrix) {
  int i,j;
  for (i = 0; i < matrix->rows; i++)
    for (j =0; j< matrix->columns; j++)
      if ( !isfinite( matrix->data[ GET_INDEX( matrix , i , j) ])) {
        printf("%s(%d,%d) = %g \n",matrix->name , i,j,matrix->data[ GET_INDEX( matrix , i , j) ]);
        return false;
      }

  return true;
}

void matrix_assert_finite( const matrix_type * matrix ) {
  if (!matrix_is_finite( matrix )) {
    if ((matrix->rows * matrix->columns) < 400)
      matrix_pretty_fprint( matrix , matrix->name , " %6.3f" , stdout);

    util_abort("%s: matrix:%s is not finite. \n",__func__ , matrix->name);
  }
}

#endif



/**
   This function will return the largest deviance from orthonormal
   conditions for the matrix - i.e. when this function returns
   0.000000 the matrix is perfectly orthonormal; otherwise it is the
   responsability of the calling scope to evaluate.
*/

double matrix_orthonormality( const matrix_type * matrix ) {
  double max_dev = 0.0;
  int col1,col2;
  for (col1=0; col1 < matrix->columns; col1++) {
    for (col2=col1; col2 < matrix->columns; col2++) {
      double dot_product = matrix_column_column_dot_product( matrix , col1 , matrix , col2);
      double dev;

      if (col1 == col2)
        dev = fabs( dot_product - 1.0 );
      else
        dev = fabs( dot_product );

      if (dev > max_dev)
        max_dev = dev;
    }
  }
  return max_dev;
}





/**
   Return true if the two matrices m1 and m2 are equal. The equality
   test is based on element-by-element memcmp() comparison, i.e. the
   there is ZERO numerical tolerance in the comparison.

   If the two matrices do not have equal dimension false is returned.
*/

bool matrix_equal( const matrix_type * m1 , const matrix_type * m2) {
  if (! ((m1->rows == m2->rows) && (m1->columns == m2->columns)))
    return false;
  {
    int i,j;
    for (i=0; i < m1->rows; i++) {
      for (j=0; j < m1->columns; j++) {
        int index1 = GET_INDEX(m1 , i , j);
        int index2 = GET_INDEX(m2 , i , j);
        double d1 = m1->data[ index1 ];
        double d2 = m2->data[ index2 ];

        if (d1 != d2)
          return false;
      }
    }
  }

  /** OK - we came all the way through - they are equal. */
  return true;
}


bool matrix_columns_equal( const matrix_type * m1 , int col1 , const matrix_type * m2 , int col2) {
  if (m1->rows != m2->rows)
    return false;

  {
    int row;
    for (row=0; row < m1->rows; row++) {
      if (memcmp( &m1->data[ GET_INDEX(m1 , row , col1)]  , &m2->data[ GET_INDEX(m2 , row , col2)] , sizeof * m1->data) != 0)
        return false;
    }
  }

  return true;
}


/*****************************************************************/
/* Various special matrices */


/**
    Will set the diagonal elements in matrix to the values in diag,
    and all remaining elements to zero. Assumes that matrix is
    rectangular.
*/

void matrix_diag_set(matrix_type * matrix , const double * diag) {
  if (matrix->rows == matrix->columns) {
    int i;
        matrix_set(matrix , 0);
    for ( i=0; i < matrix->rows; i++)
      matrix->data[ GET_INDEX(matrix , i , i) ] = diag[i];
  } else
    util_abort("%s: size mismatch \n",__func__);
}


/**
   Will set the scalar @value on all the diagonal elements of the
   matrix; all off-diagonal elements are explicitly set to zero.
*/

void matrix_diag_set_scalar(matrix_type * matrix , double value) {
  if (matrix->rows == matrix->columns) {
    int i;
        matrix_set(matrix , 0);
    for ( i=0; i < matrix->rows; i++)
      matrix->data[ GET_INDEX(matrix , i , i) ] = value;
  } else
    util_abort("%s: size mismatch \n",__func__);
}


void matrix_scalar_set( matrix_type * matrix , double value) {
  int i,j;
  for (j=0; j < matrix->columns; j++)
    for (i=0; i < matrix->rows; i++)
      matrix->data[ GET_INDEX(matrix , i , j) ] = value;
}



/**
   Fills the matrix with uniformly distributed random numbers in
   [0,1).
*/

void matrix_random_init(matrix_type * matrix , rng_type * rng) {
  int i,j;
  for (j=0; j < matrix->columns; j++)
    for (i=0; i < matrix->rows; i++)
      matrix->data[ GET_INDEX(matrix , i , j) ] = rng_get_double( rng );
}



void matrix_clear( matrix_type * matrix ) {
  matrix_set( matrix , 0 );
}



/**
   This function dumps the following binary file:

   rows
   columns
   data(1,1)
   data(2,1)
   data(3,1)
   ....
   data(1,2)
   data(2,2)
   ....

   Not exactly a matlab format.

   The following matlab code can be used to instatiate a matrix based
   on the file:

     function m = load_matrix(filename)
     fid  = fopen(filename);
     dims = fread(fid , 2 , 'int32');
     m    = fread(fid , [dims(1) , dims(2)] , 'double');
     fclose(fid);


   >> A = load_matrix( 'filename' );
*/


void matrix_matlab_dump(const matrix_type * matrix, const char * filename) {
  FILE * stream = util_fopen( filename , "w");
  int i,j;
  util_fwrite_int( matrix->rows    , stream);
  util_fwrite_int( matrix->columns , stream);

  for (j=0; j < matrix->columns; j++)
    for (i=0; i < matrix->rows; i++)
      util_fwrite_double( matrix->data[ GET_INDEX(matrix , i , j) ] , stream);

  fclose(stream);
}


// Comment
void matrix_inplace_diag_sqrt(matrix_type *Cd)
{
  int nrows = Cd->rows;

  if (Cd->rows != Cd->columns) {
    util_abort("%s: size mismatch \n",__func__);
  }
  else{
    int i;
    for ( i=0; i<nrows; i++)
      {
        Cd->data[GET_INDEX(Cd , i , i)] = sqrt(Cd->data[GET_INDEX(Cd , i , i)]);
      }
  }
}



double matrix_trace(const matrix_type *matrix) {

  int nrows  = matrix->rows;
  double sum = 0;

  if (matrix->rows != matrix->columns) {
    util_abort("%s: matrix is not square \n",__func__);
  }
  else{
    int i;
    for ( i=0; i<nrows; i++)
      {
        sum = sum + matrix->data[GET_INDEX(matrix , i , i)];
      }
  }
  return sum;
}


bool matrix_check_dims( const matrix_type * m , int rows , int columns) {
  if (m) {
    if ((m->rows == rows) && (m->columns == columns))
      return true;
    else
      return false;
  } else {
    util_abort("%s: internal error - trying to dereference NULL matrix pointer \n",__func__);
    return false;
  }
}


double matrix_diag_std(const matrix_type * Sk,double mean)
{

  if (Sk->rows != Sk->columns) {
    util_abort("%s: matrix is not square \n",__func__);
    return 0;
  }
  else{
    int nrows  = Sk->rows;
    double std = 0;
    int i;

    for ( i=0; i<nrows; i++) {
      double d = Sk->data[GET_INDEX(Sk , i , i)] - mean;
      std += d*d;
    }


    std = sqrt(std / nrows);
    return std;
  }
}

/**
   The matrix_det3() and matrix_det4() are explicit implementations of
   the determinant of a 3x3 and 4x4 matrices. The ecl_grid class uses
   these determinants determine whether a point is inside a cell. By
   using this explicit implementation the ecl_grid library has no
   LAPACK dependency.
*/

double matrix_det2( const matrix_type * A) {
  if ((A->rows == 2) && (A->columns == 2)) {
    double a00 = A->data[GET_INDEX(A,0,0)];
    double a01 = A->data[GET_INDEX(A,0,1)];
    double a10 = A->data[GET_INDEX(A,1,0)];
    double a11 = A->data[GET_INDEX(A,1,1)];

    return a00 * a11 - a10 * a01;
  } else {
    util_abort("%s: hardcoded for 2x2 matrices A is: %d x %d \n",__func__, A->rows , A->columns);
    return 0;
  }
}

double matrix_det3( const matrix_type * A) {
  if ((A->rows == 3) && (A->columns == 3)) {
    double a = A->data[GET_INDEX(A,0,0)];
    double b = A->data[GET_INDEX(A,0,1)];
    double c = A->data[GET_INDEX(A,0,2)];

    double d = A->data[GET_INDEX(A,1,0)];
    double e = A->data[GET_INDEX(A,1,1)];
    double f = A->data[GET_INDEX(A,1,2)];

    double g = A->data[GET_INDEX(A,2,0)];
    double h = A->data[GET_INDEX(A,2,1)];
    double i = A->data[GET_INDEX(A,2,2)];

    return a*e*i + b*f*g + c*d*h - c*e*g - b*d*i - a*f*h;
  } else {
    util_abort("%s: hardcoded for 3x3 matrices A is: %d x %d \n",__func__, A->rows , A->columns);
    return 0;
  }
}


double matrix_det4( const matrix_type * A) {
  if ((A->rows == 4) && (A->columns == 4)) {
    double a00 = A->data[GET_INDEX(A,0,0)];
    double a01 = A->data[GET_INDEX(A,0,1)];
    double a02 = A->data[GET_INDEX(A,0,2)];
    double a03 = A->data[GET_INDEX(A,0,3)];
    double a10 = A->data[GET_INDEX(A,1,0)];
    double a11 = A->data[GET_INDEX(A,1,1)];
    double a12 = A->data[GET_INDEX(A,1,2)];
    double a13 = A->data[GET_INDEX(A,1,3)];
    double a20 = A->data[GET_INDEX(A,2,0)];
    double a21 = A->data[GET_INDEX(A,2,1)];
    double a22 = A->data[GET_INDEX(A,2,2)];
    double a23 = A->data[GET_INDEX(A,2,3)];
    double a30 = A->data[GET_INDEX(A,3,0)];
    double a31 = A->data[GET_INDEX(A,3,1)];
    double a32 = A->data[GET_INDEX(A,3,2)];
    double a33 = A->data[GET_INDEX(A,3,3)];

    /*
    double det = (a00*(a11*(a22*a33 - a23*a32)-a12*(a21*a33 - a23*a31)+a13*(a21*a32 - a22*a31)) -
                  a01*(a10*(a22*a33 - a23*a32)-a12*(a20*a33 - a23*a30)+a13*(a20*a32 - a22*a30)) +
                  a02*(a10*(a21*a33 - a23*a31)-a11*(a20*a33 - a23*a30)+a13*(a20*a31 - a21*a30)) -
                  a03*(a10*(a21*a32 - a22*a31)-a11*(a20*a32 - a22*a30)+a12*(a20*a31 - a21*a30)));
    */
    double det = 0;

    {
      double factors[24] = {   a00*a12*a23*a31,
                               a00*a13*a21*a32,
                               a00*a11*a22*a33,
                               a01*a10*a23*a32,
                               a01*a12*a20*a33,
                               a01*a13*a22*a30,
                               a02*a10*a21*a33,
                               a02*a11*a23*a30,
                               a02*a13*a20*a31,
                               a03*a10*a22*a31,
                               a03*a11*a20*a32,
                               a03*a12*a21*a30
                              -a02*a13*a21*a30,
                              -a03*a10*a21*a32,
                              -a03*a11*a22*a30,
                              -a03*a12*a20*a31,
                              -a00*a11*a23*a32,
                              -a00*a12*a21*a33,
                              -a00*a13*a22*a31,
                              -a01*a10*a22*a33,
                              -a01*a12*a23*a30,
                              -a01*a13*a20*a32,
                              -a02*a10*a23*a31,
                              -a02*a11*a20*a33};
      int i;

      for (i = 0; i < 12; i++)
        det += (factors[i] + factors[i + 12]);
    }

    return det;
  } else {
    util_abort("%s: hardcoded for 4x4 matrices A is: %d x %d \n",__func__, A->rows , A->columns);
    return 0;
  }
}


#ifdef __cplusplus
}
#endif

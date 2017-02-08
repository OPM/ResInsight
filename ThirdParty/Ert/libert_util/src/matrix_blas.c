/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'matrix_blas.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************/
void  dgemm_(char * , char * , int * , int * , int * , double * , double * , int * , double * , int *  , double * , double * , int *);
void  dgemv_(char * , int * , int * , double * , double * , int * , const double * , int * , double * , double * , int * );
/*****************************************************************/



/**
   y = alpha * op(A)*x + beta * y

   alpha,beta: scalars
   x,y       : vectors
   A         : matrix


   x and y are entered as (double * ).
*/


void matrix_dgemv(const matrix_type * A , const double *x , double * y, bool transA , double alpha , double beta) {
  int m    = matrix_get_rows( A );
  int n    = matrix_get_columns( A );
  int lda  = matrix_get_column_stride( A );
  int incx = 1;
  int incy = 1;

  char transA_c;
  if (transA)
    transA_c = 'T';
  else
    transA_c = 'N';

  dgemv_(&transA_c , &m , &n , &alpha , matrix_get_data( A ) , &lda , x , &incx , &beta , y , &incy);
}


/**
   y = A*x
*/

void matrix_mul_vector(const matrix_type * A , const double * x , double * y) {
  matrix_dgemv(A , x , y , false , 1 , 0);
}



static void dgemm_debug(const matrix_type *C , const matrix_type *A , const matrix_type * B , bool transA, bool transB) {
  printf("\nC =  [%d , %d]\n",matrix_get_rows( C ) , matrix_get_columns(C));

  printf("A: [%d , %d]", matrix_get_rows( A ) , matrix_get_columns(A));
  if (transA)
    printf("^T");

  printf("\nB: [%d , %d]", matrix_get_rows( B ) , matrix_get_columns(B));
  if (transB)
    printf("^T");

  printf("\n\n");
  printf("[%d ,%d] = ",matrix_get_rows( C ) , matrix_get_columns(C));
  if (transA)
    printf("[%d ,%d] x ",matrix_get_rows( A ) , matrix_get_columns(A));
  else
    printf("[%d ,%d] x ",matrix_get_columns( A ) , matrix_get_rows(A));


  if (transB)
    printf("[%d ,%d]\n",matrix_get_rows( B ) , matrix_get_columns(B));
  else
    printf("[%d ,%d]\n",matrix_get_columns( B ) , matrix_get_rows(B));


}



/**
   C = alpha * op(A) * op(B)  +  beta * C

   op(·) can either be unity or Transpose.
*/

void matrix_dgemm(matrix_type *C , const matrix_type *A , const matrix_type * B , bool transA, bool transB , double alpha , double beta) {
  int m   = matrix_get_rows( C );
  int n   = matrix_get_columns( C );
  int lda = matrix_get_column_stride( A );
  int ldb = matrix_get_column_stride( B );
  int ldc = matrix_get_column_stride( C );
  char transA_c;
  char transB_c;
  int  k , innerA, innerB , outerA , outerB;

  if (transA)
    k = matrix_get_rows( A );
  else
    k = matrix_get_columns( A );


  if (transA) {
    innerA = matrix_get_rows(A);
    outerA = matrix_get_columns(A);
    transA_c = 'T';
  } else {
    innerA = matrix_get_columns(A);
    outerA = matrix_get_rows(A);
    transA_c = 'N';
  }


  if (transB) {
    innerB   = matrix_get_columns( B );
    outerB   = matrix_get_rows( B );
    transB_c = 'T';
  } else {
    transB_c = 'N';
    innerB = matrix_get_rows( B );
    outerB = matrix_get_columns( B );
  }

  /*
    This is the dimension check which must pass:

    --------------------------------------------------
          A   |         B   |  Columns(A) = Rows(B)
    Trans(A)  |   Trans(B)  |  Rows(A)    = Columns(B)
          A   |   Trans(B)  |  Columns(A) = Columns(B)
    Trans(A)  |         B   |  Rows(A)    = Rows(B)
    --------------------------------------------------

    --------------------------------------------------
              A         | Rows(A)    = Rows(C)
        Trans(A)        | Columns(A) = Rows(C)
              B         | Columns(B) = Columns(C)
        Trans(B)        | Rows(B)    = Columns(B)
    --------------------------------------------------

  */

  if (innerA != innerB) {
    dgemm_debug(C,A,B,transA , transB);
    util_abort("%s: matrix size mismatch between A and B \n", __func__);
  }


  if (outerA != matrix_get_rows( C )) {
    dgemm_debug(C,A,B,transA , transB);
    printf("outerA:%d  rows(C):%d \n",outerA , matrix_get_rows( C ));
    util_abort("%s: matrix size mismatch between A and C \n",__func__);
  }


  if (outerB != matrix_get_columns( C )) {
    dgemm_debug(C,A,B,transA , transB);
    util_abort("%s: matrix size mismatch between B and C \n",__func__);
  }

  if (ldc < util_int_max(1 , m)) {
    dgemm_debug(C,A,B,transA , transB);
    fprintf(stderr,"Tried to capture blas message: \"** On entry to DGEMM parameter 13 had an illegal value\"\n");
    fprintf(stderr,"m:%d  ldc:%d  ldc should be >= max(1,%d) \n",m,ldc,m);
    util_abort("%s: invalid value for ldc\n",__func__);
  }


  dgemm_(&transA_c ,                  //  1
         &transB_c ,                  //  2
         &m ,                         //  3
         &n ,                         //  4
         &k ,                         //  5
         &alpha ,                     //  6
         matrix_get_data( A ) ,       //  7
         &lda ,                       //  8
         matrix_get_data( B ) ,       //  9
         &ldb ,                       // 10
         &beta ,                      // 11
         matrix_get_data( C ) ,       // 12
         &ldc);                       // 13
}



void matrix_matmul_with_transpose(matrix_type * C, const matrix_type * A , const matrix_type * B , bool transA , bool transB) {
  matrix_dgemm( C , A , B , transA , transB , 1 , 0);
}


/*
   This function does a general matrix multiply of A * B, and stores
   the result in C.
*/

void matrix_matmul(matrix_type * C, const matrix_type * A , const matrix_type * B) {
  matrix_dgemm( C , A , B , false , false , 1 , 0);
}


/**
   Allocates new matrix C = A·B
*/

matrix_type * matrix_alloc_matmul(const matrix_type * A, const matrix_type * B) {
  matrix_type * C = matrix_alloc( matrix_get_rows( A ) , matrix_get_columns( B ));
  matrix_matmul( C , A , B );
  return C;
}





/*****************************************************************/
/**
   Will calculate the Gram matrix: G = X'*X
*/

void matrix_gram_set( const matrix_type * X , matrix_type * G, bool col) {
  int G_rows = matrix_get_rows( G );
  int G_cols = matrix_get_columns( G );
  int X_rows = matrix_get_rows( X );
  int X_cols = matrix_get_columns( X );
  if (col) {
    // Calculate X' · X
    if ((G_rows == G_cols) && (X_cols == G_rows))
      matrix_dgemm( G , X , X , true , false , 1 , 0);
    else
      util_abort("%s: dimension mismatch \n",__func__);
  } else {
    // Calculate X · X'
    if ((G_rows == G_cols) && (X_rows == G_rows))
      matrix_dgemm( G , X , X , false , true , 1 , 0);
    else
      util_abort("%s: dimension mismatch \n",__func__);
  }
}


/**
   If col == true:   G = X' · X
      col == false:  G = X  · X'
*/


matrix_type * matrix_alloc_gram( const matrix_type * X , bool col) {
  int X_rows    = matrix_get_rows( X );
  int X_columns = matrix_get_columns( X );
  matrix_type * G;

  if (col)
    G = matrix_alloc( X_columns , X_columns );
  else
    G = matrix_alloc( X_rows , X_rows );

  matrix_gram_set( X , G , col);
  return G;
}

#ifdef __cplusplus
}
#endif

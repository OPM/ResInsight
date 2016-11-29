/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'matrix_blas.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_MATRIX_BLAS
#define ERT_MATRIX_BLAS
#include <stdbool.h>

#include <ert/util/matrix.h>


#ifdef __cplusplus
extern "C" {
#endif


void          matrix_dgemm(matrix_type *C , const matrix_type *A , const matrix_type * B , bool transA, bool transB , double alpha , double beta);
void          matrix_matmul_with_transpose(matrix_type * C, const matrix_type * A , const matrix_type * B , bool transA , bool transB);
void          matrix_matmul(matrix_type * A, const matrix_type *B , const matrix_type * C);
matrix_type * matrix_alloc_matmul(const matrix_type * A, const matrix_type * B);
void          matrix_dgemv(const matrix_type * A , const double * x , double * y , bool transA , double alpha , double beta);
void          matrix_mul_vector(const matrix_type * A , const double * x , double * y);
void          matrix_gram_set( const matrix_type * X , matrix_type * G, bool col);
matrix_type * matrix_alloc_gram( const matrix_type * X , bool col);


#ifdef __cplusplus
}
#endif

#endif

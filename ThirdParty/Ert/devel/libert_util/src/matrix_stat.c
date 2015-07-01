/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'matrix_stat.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/thread_pool.h>
#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/matrix_lapack.h>
#include <ert/util/matrix_stat.h>


llsq_result_enum matrix_stat_llsq_estimate( matrix_type * beta , const matrix_type * X0 , const matrix_type * Y0 , const matrix_type * S) {
  if (matrix_get_rows( beta ) != matrix_get_columns( X0 ))
    return LLSQ_INVALID_DIM;

  if (matrix_get_rows( X0 ) != matrix_get_rows( Y0 ))
    return LLSQ_INVALID_DIM;

  if (S && matrix_get_rows( S ) != matrix_get_rows( Y0 ))
    return LLSQ_INVALID_DIM;

  if (matrix_get_rows(beta) > matrix_get_rows( X0 ))
    return LLSQ_UNDETERMINED;

  {
    int num_data  = matrix_get_rows( X0 );
    int num_var   = matrix_get_columns( X0 );
    matrix_type * XX = matrix_alloc( num_var , num_var );
    matrix_type * A  = matrix_alloc( num_var , num_data );

    matrix_type * X,*Y;

    if (S == NULL) {
      X = (matrix_type *) X0;
      Y = (matrix_type *) Y0;
    } else {
      X = matrix_alloc_copy( X0 );
      Y = matrix_alloc_copy( Y0 );

      {
        int row,col;
        for (row = 0; row < matrix_get_rows( X0 ); row++) {
          double sigma = matrix_iget(S , row , 0);
          double weigth = 1.0 / (sigma * sigma );

          for (col = 0; col < matrix_get_columns( X0 ); col++)
            matrix_imul( X , row , col , weigth );

          matrix_imul( Y , row , col , weigth );
        }
      }
    }

    matrix_matmul_with_transpose( XX , X , X , true , false );
    matrix_inv( XX );
    matrix_matmul_with_transpose( A , XX , X , false , true );
    matrix_matmul(beta , A , Y);

    matrix_free( XX );
    matrix_free( A );
    if (S) {
      matrix_free( X );
      matrix_free( Y );
    }
  }

  return LLSQ_SUCCESS;
}




llsq_result_enum matrix_stat_polyfit( matrix_type * beta , const matrix_type * X0 , const matrix_type * Y0 , const matrix_type * S) {
  int num_data = matrix_get_rows( X0 );
  int num_var  = matrix_get_rows( beta );
  llsq_result_enum result;
  matrix_type * X = matrix_alloc( num_data , num_var );
  int row,col;

  for (row = 0; row < matrix_get_rows( X0 ); row++) {
    double x1 = matrix_iget( X0 , row , 0 );
    double xp = 1;
    for (col = 0; col < num_var; col++) {
      matrix_iset(X , row , col , xp);
      xp *= x1;
    }
  }

  result = matrix_stat_llsq_estimate( beta , X , Y0 , S);
  matrix_free( X );
  return result;
}


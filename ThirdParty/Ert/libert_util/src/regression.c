/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'regression.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/matrix_lapack.h>
#include <ert/util/regression.h>



/**
   Will normalize the the data in X and Y:

     1. Y -> Y - <Y>
     2. For each column: X -> X - <X>
     3. For each column: ||X|| = 1

   The mean Y value is returned from the function, and the mean X and
   normalization factor for X is returned, for each column, in the
   matrices (row vectors) X_mean and X_norm.
*/


double regression_scale(matrix_type * X , matrix_type * Y , matrix_type * X_mean , matrix_type * X_norm) {
  int nvar = matrix_get_columns( X );
  int nsample = matrix_get_rows( X );

  if ( matrix_get_rows( Y ) != nsample)
    util_abort("%s: dimension mismatch X:[%d,%d]  Y:%d \n",__func__ , nsample , nvar , matrix_get_rows( Y ));

  if ( matrix_get_columns( X_norm ) != nvar)
    util_abort("%s: dimension mismtach X_norm \n",__func__);

  if ( matrix_get_columns( X_mean ) != nvar)
    util_abort("%s: dimension mismtach X_mean \n",__func__);

  {
    double y_mean = matrix_get_column_sum( Y , 0 ) / nsample;
    matrix_shift_column( Y , 0 , -y_mean );

    {
      int col;
      for (col = 0; col < nvar; col++) {
        double mean = matrix_get_column_sum(X , col ) / nsample;
        matrix_shift_column(X , col , -mean);
        matrix_iset( X_mean , 0 , col , mean );
      }

      for (col=0; col < nvar; col++) {
        double norm = 1.0/sqrt( (1.0 / (nsample - 1)) * matrix_get_column_sum2( X , col ));
        matrix_iset( X_norm , 0 , col , norm );
      }
    }
    return y_mean;
  }
}


double regression_unscale(const matrix_type * beta , const matrix_type * X_norm , const matrix_type * X_mean , double Y_mean , matrix_type * beta0) {
  int    nvars = matrix_get_rows( beta0 );
  int    k;
  double yshift = 0;

  for (k=0; k < nvars; k++) {
    double scaled_beta = matrix_iget( beta , k , 0 ) * matrix_iget( X_norm , 0 , k);
    matrix_iset( beta0 , k , 0 , scaled_beta);
    yshift += scaled_beta * matrix_iget( X_mean , 0 , k );
  }
  return Y_mean - yshift;
}


/**
   Performs an ordinary least squares estimation of the parameter
   vector beta.

   beta = inv(X'·X)·X'·y
*/

void regression_augmented_OLS( const matrix_type * X , const matrix_type * Y , const matrix_type* Z, matrix_type * beta) {
  /*
    Solves the following especial augmented regression problem:

    [Y ; 0] = [X ; Z] beta + epsilon

    where 0 is the zero matrix of same size as Y.

    The solution to this OLS is:

     inv(X'X + Z'Z) * X' * Y

    The semicolon denotes row concatenation and the apostrophe the transpose.

   */
  int nvar = matrix_get_columns( X );
  matrix_type * Xt   = matrix_alloc_transpose( X );
  matrix_type * Xinv = matrix_alloc( nvar ,  nvar);
  matrix_matmul( Xinv , Xt , X );

  matrix_type * Zt  = matrix_alloc_transpose( Z );
  matrix_type * ZtZ = matrix_alloc( nvar ,  nvar);
  matrix_matmul( ZtZ , Zt , Z );

  // Xinv <- X'X + Z'Z
  matrix_inplace_add(Xinv, ZtZ);

  // Sometimes the inversion fails - add a small regularization to diagonal
  for (int i = 0; i < nvar; ++i)
    matrix_iadd(Xinv, i, i, 1e-10);

  matrix_inv( Xinv ); // Xinv is always invertible
  {
    matrix_type * tmp = matrix_alloc_matmul( Xinv , Xt );
    matrix_matmul( beta , tmp , Y );
    matrix_free( tmp );
  }

  matrix_free( Xt );
  matrix_free( Xinv );
  matrix_free( Zt );
  matrix_free( ZtZ );
}


/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'ert_util_matrix_stat.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/matrix.h>
#include <ert/util/matrix_stat.h>
#include <ert/util/test_util.h>
#include <ert/util/util.h>



void test_invalid_dimensions() {
  matrix_type * X = matrix_alloc(10,2);
  matrix_type * Y = matrix_alloc(11,1);
  matrix_type * S = matrix_alloc(10,1);
  matrix_type * beta = matrix_alloc(2,1);

  test_assert_true( matrix_stat_llsq_estimate( beta , X , Y , S )    == LLSQ_INVALID_DIM );
  test_assert_true( matrix_stat_llsq_estimate( beta , X , Y , NULL ) == LLSQ_INVALID_DIM );

  matrix_resize(beta , 4 , 1 , false );
  matrix_resize(Y , 3 , 1 , false );
  matrix_resize(X , 3 , 4 , false );
  test_assert_true( matrix_stat_llsq_estimate( beta , X , Y , NULL ) == LLSQ_UNDETERMINED );

  matrix_free( Y );
  matrix_free( S );
  matrix_free( beta );
  matrix_free( X );
}


void test_no_sigma() {
  const double A = 4.00;
  const double B = -2;
  const double C = 0.25;

  const double xmin = 0;
  const double xmax = 1;
  int size        = 20;
  int P           = 3;
  matrix_type * X = matrix_alloc(size,P);
  matrix_type * Y = matrix_alloc(size,1);
  matrix_type * beta = matrix_alloc(P,1);

  int i;
  for (i = 0; i < size; i++) {
    double x = xmin + i * (xmax - xmin) / (size - 1);

    double y = A + B*x + C*x*x;
    matrix_iset( X , i , 0 , 1 );
    matrix_iset( X , i , 1 , x );
    matrix_iset( X , i , 2 , x*x );

    matrix_iset( Y , i , 0 , y);
  }

  test_assert_true( matrix_stat_llsq_estimate( beta , X , Y , NULL ) == LLSQ_SUCCESS );

  test_assert_double_equal( A , matrix_iget( beta , 0 , 0 ));
  test_assert_double_equal( B , matrix_iget( beta , 1 , 0 ));
  test_assert_double_equal( C , matrix_iget( beta , 2 , 0 ));


  matrix_free( Y );
  matrix_free( beta );
  matrix_free( X );
}


void test_with_sigma() {
  const double A = 4.00;
  const double B = -2;
  const double C = 0.25;

  const double xmin = 0;
  const double xmax = 1;
  int size        = 20;
  int P           = 3;
  matrix_type * X = matrix_alloc(size,P);
  matrix_type * Y = matrix_alloc(size,1);
  matrix_type * beta = matrix_alloc(P,1);
  matrix_type * S = matrix_alloc(size,1);

  int i;
  for (i = 0; i < size; i++) {
    double x = xmin + i * (xmax - xmin) / (size - 1);

    double y = A + B*x + C*x*x;
    matrix_iset( X , i , 0 , 1 );
    matrix_iset( X , i , 1 , x );
    matrix_iset( X , i , 2 , x*x );

    matrix_iset( Y , i , 0 , y);
    matrix_iset( S , i , 0 , 1);
  }

  test_assert_true( matrix_stat_llsq_estimate( beta , X , Y , S ) == LLSQ_SUCCESS );

  test_assert_double_equal( A , matrix_iget( beta , 0 , 0 ));
  test_assert_double_equal( B , matrix_iget( beta , 1 , 0 ));
  test_assert_double_equal( C , matrix_iget( beta , 2 , 0 ));

  matrix_free( S );
  matrix_free( Y );
  matrix_free( beta );
  matrix_free( X );
}


void test_polyfit() {
  const double A = 4.00;
  const double B = -2;
  const double C = 0.25;

  const double xmin = 0;
  const double xmax = 1;
  int size        = 20;
  int P           = 3;
  matrix_type * X = matrix_alloc(size,1);
  matrix_type * Y = matrix_alloc(size,1);
  matrix_type * beta = matrix_alloc(P,1);

  int i;
  for (i = 0; i < size; i++) {
    double x = xmin + i * (xmax - xmin) / (size - 1);

    double y = A + B*x + C*x*x;
    matrix_iset( X , i , 0 , x );
    matrix_iset( Y , i , 0 , y);
  }

  test_assert_true( matrix_stat_polyfit( beta , X , Y , NULL) == LLSQ_SUCCESS );

  test_assert_double_equal( A , matrix_iget( beta , 0 , 0 ));
  test_assert_double_equal( B , matrix_iget( beta , 1 , 0 ));
  test_assert_double_equal( C , matrix_iget( beta , 2 , 0 ));

  matrix_free( Y );
  matrix_free( beta );
  matrix_free( X );
}





int main() {
  util_install_signals();
  test_invalid_dimensions();
  test_no_sigma();
  test_with_sigma();
  test_polyfit();
  exit(0);
}

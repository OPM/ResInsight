/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ert_util_matrix.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <math.h>

#include <ert/util/bool_vector.h>
#include <ert/util/test_util.h>
#include <ert/util/statistics.h>
#include <ert/util/test_work_area.h>
#include <ert/util/matrix.h>
#include <ert/util/rng.h>
#include <ert/util/mzran.h>


void test_resize() {
  matrix_type * m1 = matrix_alloc(5,5);
  matrix_type * m2 = matrix_alloc(5,5);
  rng_type * rng = rng_alloc( MZRAN , INIT_DEFAULT );

  matrix_random_init( m1 , rng );
  matrix_assign( m2 , m1 );

  test_assert_true( matrix_equal( m1 , m2 ));
  matrix_resize( m1 , 5 , 5 , false );
  test_assert_true( matrix_equal( m1 , m2 ));
  matrix_resize( m1 , 5 , 5 , true );
  test_assert_true( matrix_equal( m1 , m2 ));

  rng_free( rng );
  matrix_free( m1 );
  matrix_free( m2 );
}


void test_column_equal() {
  matrix_type * m1 = matrix_alloc(5,5);
  matrix_type * m2 = matrix_alloc(5,5);
  matrix_type * m3 = matrix_alloc(6,5);
  rng_type * rng = rng_alloc( MZRAN , INIT_DEFAULT );

  matrix_random_init( m1 , rng );
  matrix_assign( m2 , m1 );

  test_assert_true( matrix_columns_equal( m1 , 2 , m2 , 2 ));
  test_assert_false( matrix_columns_equal( m1 , 2 , m2 , 3 ));
  test_assert_false( matrix_columns_equal( m1 , 2 , m3 , 3 ));

  rng_free( rng );
  matrix_free( m1 );
  matrix_free( m2 );
  matrix_free( m3 );
}




void test_create_invalid() {
  test_assert_NULL( matrix_alloc(0, 100));
  test_assert_NULL( matrix_alloc(100, 0));
  test_assert_NULL( matrix_alloc(0, 0));
  test_assert_NULL( matrix_alloc(-1, -1));
}



void test_dims() {
  const int rows = 10;
  const int columns = 13;
  matrix_type * m = matrix_alloc(rows , columns);

  test_assert_true(  matrix_check_dims(m , rows , columns));
  test_assert_false( matrix_check_dims(m , rows + 1 , columns));
  test_assert_false( matrix_check_dims(m , rows , columns + 1));

  matrix_free( m );
}




void test_readwrite() {
  test_work_area_type * test_area = test_work_area_alloc("matrix-test");
  {
    rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
    matrix_type * m1 = matrix_alloc(3  , 3);
    matrix_type * m2 = matrix_alloc(3  , 3);
    matrix_random_init( m1 , rng );
    matrix_assign(m2 , m1);

    test_assert_true( matrix_equal( m1 , m2 ) );
    {
      FILE * stream = util_fopen("m1" , "w");
      matrix_fwrite( m1 , stream );
      fclose( stream );
    }
    matrix_random_init( m1 , rng );
    test_assert_false( matrix_equal( m1 , m2 ) );
    {
      FILE * stream = util_fopen("m1" , "r");
      matrix_free( m1 );
      m1 = matrix_alloc(1,1);
      printf("-----------------------------------------------------------------\n");
      matrix_fread( m1 , stream );
      test_assert_int_equal( matrix_get_rows(m1) , matrix_get_rows( m2));
      test_assert_int_equal( matrix_get_columns(m1) , matrix_get_columns( m2));
      util_fseek( stream , 0 , SEEK_SET);
      {
        matrix_type * m3 = matrix_fread_alloc( stream );
        test_assert_true( matrix_equal( m2 , m3 ));
        matrix_free( m3 );
      }
      fclose( stream );
    }
    test_assert_true( matrix_equal( m1 , m2 ) );

    matrix_free( m2 );
    matrix_free( m1 );
    rng_free( rng );
  }
  test_work_area_free( test_area );
}


void test_diag_std() {
  const int N = 25;
  double_vector_type * data = double_vector_alloc( 0,0);
  rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
  matrix_type * m = matrix_alloc( N , N );
  double sum1 = 0;
  double sum2 = 0;
  int i;

  for (i=0; i < N; i++) {
    double R = rng_get_double( rng );
    matrix_iset(m , i , i , R);
    double_vector_iset( data , i , R );

    sum1 += R;
    sum2 += R*R;
  }
  {
    double mean = sum1 / N;
    double std = sqrt( sum2 / N - mean * mean );

    test_assert_double_equal( std , matrix_diag_std( m , mean ));
    test_assert_double_equal( statistics_std( data ) , matrix_diag_std( m , mean ));
    test_assert_double_equal( statistics_mean( data ) , mean );
  }
  matrix_free( m );
  rng_free( rng );
}



void test_masked_copy() {
  const int N = 25;
  bool_vector_type * mask = bool_vector_alloc(N , true);
  rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
  matrix_type * m1 = matrix_alloc( N , N );
  matrix_random_init( m1 , rng );

  bool_vector_iset( mask , 0 , false );
  bool_vector_iset( mask , 10 , false );

  {
    matrix_type * m2 = matrix_alloc_column_compressed_copy( m1 , mask );
    matrix_type * m3 = matrix_alloc( N , N - 2 );

    test_assert_int_equal( matrix_get_rows( m1 ) , matrix_get_rows( m2 ));
    test_assert_int_equal( matrix_get_columns( m1 ) , matrix_get_columns( m2 ) + 2);

    matrix_column_compressed_memcpy( m3 , m1 , mask );
    {
      int src_col;
      int target_col = 0;
      for (src_col = 0; src_col < N; src_col++) {
        if (bool_vector_iget( mask , src_col)) {
          test_assert_true( matrix_columns_equal( m1 , src_col , m3 , target_col ));
          target_col++;
        }
      }
    }

    test_assert_true( matrix_equal( m2 , m3 ));
    matrix_free( m3 );
    matrix_free( m2 );
  }

  matrix_free( m1 );
  rng_free( rng );
}


void test_inplace_sub_column() {
  const int N = 25;
  rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
  matrix_type * m1 = matrix_alloc( N , N );
  matrix_type * m2 = matrix_alloc( N , N );

  matrix_random_init( m1 , rng );
  matrix_assign( m2 , m1 );
  matrix_inplace_sub_column( m1 , m2 , 0 , 0 );
  {
    int row;
    for (row = 0; row < N; row++) {
      double diff = matrix_iget( m1 , row , 0);
      test_assert_true( fabs( diff ) < 1e-6);
    }
  }
}


int main( int argc , char ** argv) {
  test_create_invalid();
  test_resize();
  test_column_equal();
  test_dims();

  test_readwrite();
  test_diag_std();
  test_masked_copy();
  test_inplace_sub_column();
  exit(0);
}

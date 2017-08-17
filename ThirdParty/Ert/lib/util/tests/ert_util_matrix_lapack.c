/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'ert_util_matrix_lapack.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/matrix_lapack.h>




void test_det4() {
  matrix_type * m = matrix_alloc(4  , 4 );
  rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
  for (int i=0; i < 10; i++) {
    matrix_random_init( m , rng );
    {
      double det4 = matrix_det4( m );
      double det = matrix_det( m );

      test_assert_double_equal( det , det4 );
    }
  }

  matrix_free( m );
  rng_free( rng );
}


void test_det3() {
  matrix_type * m = matrix_alloc(3  , 3 );
  rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
  matrix_random_init( m , rng );

  {
    double det3 = matrix_det3( m );
    double det = matrix_det( m );

    test_assert_double_equal( det , det3 );
  }

  matrix_free( m );
  rng_free( rng );
}


void test_det2() {
  matrix_type * m = matrix_alloc(2,2);
  rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
  matrix_random_init( m , rng );
  {
    double det2 = matrix_det2( m );
    double det = matrix_det( m );

    test_assert_double_equal( det , det2 );
  }
  matrix_free( m );
  rng_free( rng );
}



int main( int argc , char ** argv) {
  test_det2();
  test_det3();
  test_det4();
  exit(0);
}

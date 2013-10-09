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

#include <ert/util/test_util.h>
#include <ert/util/matrix.h>



int main( int argc , char ** argv) {
  const int rows = 10;
  const int columns = 13;
  matrix_type * m = matrix_alloc(rows , columns);

  test_assert_true( matrix_check_dims(m , rows , columns));
  test_assert_false( matrix_check_dims(m , rows + 1 , columns));
  test_assert_false( matrix_check_dims(m , rows , columns + 1));

  matrix_free( m );
  exit(0);
}

/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_vector_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>

#include <ert/util/vector.h>
#include <ert/util/test_util.h>



int test_iset( ) {
  vector_type * vector = vector_alloc_new(  );
  vector_iset_ref( vector , 2 , vector );

  
  test_assert_true( vector_get_size( vector ) == 3 );
  test_assert_true( vector_iget( vector , 0 ) == NULL );
  test_assert_true( vector_iget( vector , 1 ) == NULL );
  test_assert_true( vector_iget( vector , 2 ) == vector );
  vector_free( vector );
  return 0;
}


void test_reverse() {
  const char * val1 = "value1";
  const char * val2 = "value2";
  const char * val3 = "value3";
  const char * val4 = "value4";
  
  vector_type * vector1 = vector_alloc_new(  );
  vector_type * vector2 = vector_alloc_new(  );

  vector_append_ref( vector1 , val1 );
  vector_append_ref( vector1 , val2 );
  vector_append_ref( vector1 , val3 );
  vector_append_ref( vector1 , val4 );

  vector_append_ref( vector2 , val1 );
  vector_append_ref( vector2 , val2 );
  vector_append_ref( vector2 , val3 );
  vector_append_ref( vector2 , val4 );

  vector_inplace_reverse( vector1 );

  {
    int i;
    int size = vector_get_size( vector1 );
    for (i=0; i < vector_get_size( vector1 ); i++)
      test_assert_ptr_equal( vector_iget_const( vector2 , i ) , vector_iget_const( vector1 , size - 1 - i ));
  }
  vector_free( vector1 );
  vector_free( vector2 );
}





int main(int argc , char ** argv) {
  test_iset( );
  test_reverse( );
  exit(0);
}

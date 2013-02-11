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

void assert_equal( bool equal ) {
  if (!equal)
    exit(1);
}


int test_iset( ) {
  vector_type * vector = vector_alloc_new( 0 );
  vector_iset_ref( vector , 2 , vector );
  
  assert_equal( vector_get_size( vector ) == 3 );
  assert_equal( vector_iget( vector , 0 ) == NULL );
  assert_equal( vector_iget( vector , 1 ) == NULL );
  assert_equal( vector_iget( vector , 2 ) == vector );
  vector_free( vector );
  return 0;
}



int main(int argc , char ** argv) {
  test_iset( );
  
  exit(0);
}

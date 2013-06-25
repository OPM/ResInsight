/*                
   Copyri                ght (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_type_vector_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/int_vector.h>
#include <ert/util/test_util.h>

void assert_equal( bool equal ) {
  if (!equal)
    exit(1);
}


int main(int argc , char ** argv) {
  
  int_vector_type * int_vector = int_vector_alloc( 0 , 99);
  
  int_vector_iset( int_vector , 2 , 0);       
  int_vector_insert( int_vector , 2 , 77 );   
  int_vector_iset( int_vector , 5 , -10);     
  
  int_vector_fprintf( int_vector , stdout , "int_vector" , "%3d");
  assert_equal( int_vector_iget(int_vector , 0 ) == 99 );
  assert_equal( int_vector_iget(int_vector , 1 ) == 99 );
  assert_equal( int_vector_iget(int_vector , 2 ) == 77 );
  assert_equal( int_vector_iget(int_vector , 3 ) == 00 );
  assert_equal( int_vector_iget(int_vector , 4 ) == 99 );
  assert_equal( int_vector_iget(int_vector , 5 ) == -10 );
  
  {
    int N1 = 100000;
    int N2 = 10*N1;
    int_vector_type * v1 = int_vector_alloc( N1 , 0 );
    int_vector_type * v2;
    int * data1 = int_vector_get_ptr( v1 );
    int_vector_iset( v1 , N1 - 1, 99);

    int_vector_free_container( v1 );
    v2 = int_vector_alloc( N2 , 0 );
    int_vector_iset(v2 , N2 - 1, 77 );
    
    test_assert_int_equal(  data1[N1-1] , 99);
    int_vector_free( v2 );
    free( data1 );
  }                 
  
  
  test_assert_true( int_vector_init_range( int_vector , 100 , 1000 , 115 ) );
  test_assert_int_equal( int_vector_iget( int_vector , 0 ) , 100);
  test_assert_int_equal( int_vector_iget( int_vector , 1 ) , 215);
  test_assert_int_equal( int_vector_iget( int_vector , 2 ) , 330);
  test_assert_int_equal( int_vector_iget( int_vector , 3 ) , 445);
  test_assert_int_equal( int_vector_get_last( int_vector ) , 1000);
  
  test_assert_false( int_vector_init_range( int_vector , 100 , -1000 , 115 ) );
  test_assert_int_equal( int_vector_iget( int_vector , 0 ) , 100);
  test_assert_int_equal( int_vector_iget( int_vector , 1 ) , 215);
  test_assert_int_equal( int_vector_iget( int_vector , 2 ) , 330);
  test_assert_int_equal( int_vector_iget( int_vector , 3 ) , 445);
  test_assert_int_equal( int_vector_get_last( int_vector ) , 1000);

  {
    int_vector_type * v1 = int_vector_alloc(0,0);
    int_vector_type * v2 = int_vector_alloc(0,0);
    int_vector_append(v1 , 10);
    int_vector_append(v1 , 15);
    int_vector_append(v1 , 20);

    int_vector_append(v2 , 1);
    int_vector_append(v2 , 2);
    int_vector_append(v2 , 3);

    int_vector_append_vector( v1 , v2 );
    test_assert_int_equal( int_vector_size( v1 ) , 6 );
    test_assert_int_equal( int_vector_iget (v1 ,  0 ), 10 );
    test_assert_int_equal( int_vector_iget (v1 ,  1 ), 15 );
    test_assert_int_equal( int_vector_iget (v1 ,  2 ), 20 );
                                                               
    test_assert_int_equal( int_vector_iget (v1 ,  3 ), 1 );
    test_assert_int_equal( int_vector_iget (v1 ,  4 ), 2 );
    test_assert_int_equal( int_vector_iget (v1 ,  5 ), 3 );

    int_vector_free( v1 );
    int_vector_free( v2 );
  }
  
  exit(0);
}

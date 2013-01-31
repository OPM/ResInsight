/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
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
  
  
  exit(0);
}

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

#include <ert/util/int_vector.hpp>
#include <ert/util/double_vector.hpp>
#include <ert/util/test_util.hpp>

void assert_equal( bool equal ) {
  if (!equal)
    exit(1);
}


void test_range_fill_int() {
  int_vector_type * int_vector = int_vector_alloc(0,0);

  int_vector_range_fill( int_vector , 10 , 10 , 40 );  /* 10,20,30,40 */
  test_assert_int_equal( int_vector_size( int_vector ), 4);
  test_assert_int_equal( int_vector_iget( int_vector , 0 ) , 10 );
  test_assert_int_equal( int_vector_iget( int_vector , 1 ) , 20 );
  test_assert_int_equal( int_vector_iget( int_vector , 2 ) , 30 );
  test_assert_int_equal( int_vector_iget( int_vector , 3 ) , 40 );

  int_vector_range_fill( int_vector , 10 , 10 , 44 );  /* 10,20,30,40 */
  test_assert_int_equal( int_vector_size( int_vector ), 4);
  test_assert_int_equal( int_vector_iget( int_vector , 0 ) , 10 );
  test_assert_int_equal( int_vector_iget( int_vector , 1 ) , 20 );
  test_assert_int_equal( int_vector_iget( int_vector , 2 ) , 30 );
  test_assert_int_equal( int_vector_iget( int_vector , 3 ) , 40 );

  int_vector_range_fill( int_vector , 4 , -1 , 0 );  /* 4,3,2,1,0 */
  test_assert_int_equal( int_vector_size( int_vector ), 5);
  test_assert_int_equal( int_vector_iget( int_vector , 0 ) , 4 );
  test_assert_int_equal( int_vector_iget( int_vector , 1 ) , 3 );
  test_assert_int_equal( int_vector_iget( int_vector , 2 ) , 2 );
  test_assert_int_equal( int_vector_iget( int_vector , 3 ) , 1 );
  test_assert_int_equal( int_vector_iget( int_vector , 4 ) , 0 );

  int_vector_range_fill( int_vector , 20 , -10 , -3 );  /* 20,10,0 */
  test_assert_int_equal( int_vector_size( int_vector ), 3);
  test_assert_int_equal( int_vector_iget( int_vector , 0 ) , 20 );
  test_assert_int_equal( int_vector_iget( int_vector , 1 ) , 10 );
  test_assert_int_equal( int_vector_iget( int_vector , 2 ) ,  0 );

  int_vector_free( int_vector );
}


void test_range_fill_double() {
  double_vector_type * double_vector = double_vector_alloc(0,0);
  double_vector_range_fill( double_vector , 1,2,10 ); /* 1 , 3 , 5 , 7 , 9 */

  test_assert_double_equal( double_vector_size( double_vector ), 5);
  test_assert_double_equal( double_vector_iget( double_vector , 0 ) , 1 );
  test_assert_double_equal( double_vector_iget( double_vector , 1 ) , 3 );
  test_assert_double_equal( double_vector_iget( double_vector , 2 ) , 5 );
  test_assert_double_equal( double_vector_iget( double_vector , 3 ) , 7 );
  test_assert_double_equal( double_vector_iget( double_vector , 4 ) , 9 );

  double_vector_range_fill( double_vector , 3,3,9 ); /* 3,6,9 */
  test_assert_double_equal( double_vector_size( double_vector ), 3);
  test_assert_double_equal( double_vector_iget( double_vector , 0 ) , 3 );
  test_assert_double_equal( double_vector_iget( double_vector , 1 ) , 6 );
  test_assert_double_equal( double_vector_iget( double_vector , 2 ) , 9 );

  double_vector_free( double_vector );
}


void test_range_fill() {
  test_range_fill_int();
  test_range_fill_double();
}


void test_contains() {
  int_vector_type * int_vector = int_vector_alloc( 0 , 100);

  test_assert_false( int_vector_contains( int_vector , 100 ));
  int_vector_iset( int_vector , 0 , 77 );
  test_assert_false( int_vector_contains( int_vector , 100 ));
  test_assert_true( int_vector_contains( int_vector , 77 ));

  int_vector_iset( int_vector , 10 , 33 );
  test_assert_true( int_vector_contains( int_vector , 100 ));
  test_assert_true( int_vector_contains( int_vector , 77 ));
  test_assert_true( int_vector_contains( int_vector , 33 ));

  int_vector_free( int_vector );
}

void test_contains_sorted() {
  int_vector_type * int_vector = int_vector_alloc( 0 , 100);

  int_vector_append( int_vector , 99 );
  int_vector_append( int_vector , 89 );
  int_vector_append( int_vector , 79 );
  int_vector_append( int_vector , 109 );

  int_vector_sort( int_vector );

  test_assert_false( int_vector_contains( int_vector , 0 ));
  test_assert_false( int_vector_contains( int_vector , 100 ));
  test_assert_true( int_vector_contains( int_vector , 89 ));
  test_assert_true( int_vector_contains( int_vector , 109 ));
}



void test_div() {
  int_vector_type * int_vector = int_vector_alloc( 0 , 100);
  int_vector_iset( int_vector , 10 , 100 );
  int_vector_div( int_vector , 10 );
  {
    int i;
    for (i=0; i < int_vector_size( int_vector ); i++)
      test_assert_int_equal( 10 , int_vector_iget( int_vector , i ));
  }
}

void test_memcpy_from_data() {
  int_vector_type * int_vector = int_vector_alloc( 10 , 77 );
  int data[5] = {1,2,3,4,5};

  int_vector_memcpy_from_data( int_vector , data , 5 );
  test_assert_int_equal( 5 , int_vector_size( int_vector ));

  for (int i=0; i < int_vector_size( int_vector ); i++)
    test_assert_int_equal( i + 1 , int_vector_iget( int_vector , i ));

  int_vector_free( int_vector );
}


void test_alloc() {
  const int size = 100;
  const int default_value = 77;
  int_vector_type * v = int_vector_alloc(size , default_value);

  test_assert_int_equal(size , int_vector_size(v));
  for (int i=0; i < size; i++)
    test_assert_int_equal( default_value , int_vector_iget( v , i));


  int_vector_free( v);
}


void int_vector_iget_invalid( void * arg ) {
  int_vector_type * ivec = int_vector_safe_cast( arg );
  int_vector_iget(ivec , -1 );
}


void test_abort() {
  int_vector_type * ivec = int_vector_alloc(0,0);
  test_assert_util_abort( "int_vector_assert_index" , int_vector_iget_invalid , ivec );
  int_vector_free( ivec );
}



void test_shift() {
  int default_value = 88;
  int_vector_type * v = int_vector_alloc(0,default_value);

  int_vector_append(v , 1 );
  int_vector_append(v , 2 );
  int_vector_append(v , 3 );
  test_assert_int_equal( 1 , int_vector_iget( v , 0 ));
  test_assert_int_equal( 2 , int_vector_iget( v , 1 ));
  test_assert_int_equal( 3 , int_vector_iget( v , 2 ));

  int_vector_rshift(v , 3 );
  test_assert_int_equal( 6 , int_vector_size( v ));
  test_assert_int_equal( default_value , int_vector_iget( v , 0 ));
  test_assert_int_equal( default_value , int_vector_iget( v , 1 ));
  test_assert_int_equal( default_value , int_vector_iget( v , 2 ));
  test_assert_int_equal( 1 , int_vector_iget( v , 3 ));
  test_assert_int_equal( 2 , int_vector_iget( v , 4 ));
  test_assert_int_equal( 3 , int_vector_iget( v , 5 ));

  int_vector_lshift(v,4);
  test_assert_int_equal( 2 , int_vector_size( v ));
  test_assert_int_equal( 2 , int_vector_iget( v , 0 ));
  test_assert_int_equal( 3 , int_vector_iget( v , 1 ));

  int_vector_free( v );
}

void test_idel_insert() {
  int_vector_type * vec = int_vector_alloc(0,0);

  int_vector_append(vec , 1 );
  int_vector_append(vec , 2 );
  int_vector_append(vec , 2 );
  int_vector_append(vec , 3 );

  int_vector_fprintf(vec , stdout , "Vec0" , "%2d");
  int_vector_idel(vec , 1 );
  int_vector_fprintf(vec , stdout , "Vec1" , "%2d");
  int_vector_idel(vec , 1 );
  int_vector_fprintf(vec , stdout , "Vec2" , "%2d");

  test_assert_int_equal( 2 , int_vector_size( vec ));
  test_assert_int_equal( 1 , int_vector_iget( vec , 0 ));
  test_assert_int_equal( 3 , int_vector_iget( vec , 1 ));

  int_vector_insert(vec , 1 , 2 );
  int_vector_insert(vec , 1 , 2 );
  test_assert_int_equal( 4 , int_vector_size( vec ));
  test_assert_int_equal( 1 , int_vector_iget( vec , 0 ));
  test_assert_int_equal( 2 , int_vector_iget( vec , 1 ));
  test_assert_int_equal( 2 , int_vector_iget( vec , 2 ));
  test_assert_int_equal( 3 , int_vector_iget( vec , 3 ));

  int_vector_free( vec );
}



void test_iset_block() {
  int_vector_type * vec = int_vector_alloc(0,0);

  int_vector_iset_block( vec , 10 , 10 , 77 );
  test_assert_int_equal( int_vector_size( vec ) , 20 );
  {
    int i;
    for (i=10; i < 20; i++)
      test_assert_int_equal( int_vector_iget( vec , i ) , 77 );
  }
  int_vector_iset_block( vec , 10 , 0 , 177 );
  test_assert_int_equal( int_vector_iget( vec , 10 ) , 77 );

  int_vector_iset_block( vec , 10 , -11 , 66 );
  {
    int i;
    for (i=0; i <= 10; i++)
      test_assert_int_equal( int_vector_iget( vec , i ) , 66 );
  }
  int_vector_free( vec );
}


void test_resize() {
  int i;
  int def = 77;
  int_vector_type * vec = int_vector_alloc(0,def);
  int_vector_resize( vec , 10 );
  test_assert_int_equal( int_vector_size( vec ) , 10 );
  for (i=0; i < 10; i++)
    test_assert_int_equal( int_vector_iget( vec , i ) , def );

  int_vector_iset_block( vec , 5 , 5 , 5 );
  for (i=5; i < 10; i++)
    test_assert_int_equal( int_vector_iget( vec , i ) , 5 );

  int_vector_resize( vec , 5 );
  test_assert_int_equal( int_vector_size( vec ) , 5 );
  for (i=0; i < 5; i++)
    test_assert_int_equal( int_vector_iget( vec , i ) , def );

  int_vector_resize( vec , 10 );
  test_assert_int_equal( int_vector_size( vec ) , 10 );
  for (i=0; i < 10; i++)
    test_assert_int_equal( int_vector_iget( vec , i ) , def );

  int_vector_free( vec );
}


void test_del() {
  int_vector_type * vec = int_vector_alloc(0,0);

  test_assert_int_equal( int_vector_del_value( vec , 77) , 0 );

  int_vector_append(vec , 1);
  int_vector_append(vec , 2);
  int_vector_append(vec , 3);
  int_vector_append(vec , 1);
  int_vector_append(vec , 2);
  int_vector_append(vec , 3);
  int_vector_append(vec , 1);
  int_vector_append(vec , 1);

  int_vector_fprintf( vec , stdout , "int_vector" , "%3d");

  test_assert_int_equal( int_vector_del_value( vec , 77) ,  0);
  test_assert_int_equal( int_vector_del_value( vec , 2) , 2 );
  test_assert_int_equal( 6 , int_vector_size( vec ));

  int_vector_fprintf( vec , stdout , "int_vector" , "%3d");

  test_assert_int_equal( 1 , int_vector_iget(vec , 0));
  test_assert_int_equal( 3 , int_vector_iget(vec , 1));
  test_assert_int_equal( 1 , int_vector_iget(vec , 2));
  test_assert_int_equal( 3 , int_vector_iget(vec , 3));
  test_assert_int_equal( 1 , int_vector_iget(vec , 4));
  test_assert_int_equal( 1 , int_vector_iget(vec , 5));

  test_assert_int_equal( 4 , int_vector_del_value( vec , 1 ));
  test_assert_int_equal( 2 , int_vector_size( vec ));

  test_assert_int_equal( 3 , int_vector_iget(vec , 0));
  test_assert_int_equal( 3 , int_vector_iget(vec , 1));
  int_vector_free( vec );
}


void test_insert_double() {
  double_vector_type * vec = double_vector_alloc(0,0);
  double_vector_append( vec , 1 );
  double_vector_insert( vec , 0 , 0 );

  test_assert_double_equal( 0 , double_vector_iget( vec , 0 ));
  test_assert_double_equal( 1 , double_vector_iget( vec , 1 ));

  double_vector_free( vec );
}


void test_empty() {
  int_vector_type * vec = int_vector_alloc(0,0);
  int_vector_sort( vec );
  int_vector_select_unique( vec );
  int_vector_free( vec );
}


void test_equal_index() {
  int_vector_type * v1 = int_vector_alloc(0,0);
  int_vector_type * v2 = int_vector_alloc(0,0);
  int_vector_type * v3 = int_vector_alloc(0,0);

  for (int i=0; i < 5; i++) {
    int_vector_iset(v1,i,i);
    int_vector_iset(v2,i,i);
    int_vector_iset(v3,i,i+1);
  }

  test_assert_int_equal( int_vector_first_equal(v1,v2,10), -2);
  test_assert_int_equal( int_vector_first_not_equal(v1,v2,10), -2);

  test_assert_int_equal( int_vector_first_equal(v1,v2,0), 0);
  test_assert_int_equal( int_vector_first_not_equal(v1,v2,0), -1);


  int_vector_iset(v1,0,77);
  test_assert_int_equal( int_vector_first_equal(v1,v2,0), 1);
  test_assert_int_equal( int_vector_first_not_equal(v1,v2,0),0);
  test_assert_int_equal( int_vector_first_equal(v1,v3,0), -1);
  test_assert_int_equal( int_vector_first_not_equal(v1,v1,0), -1);

  int_vector_free(v1);
  int_vector_free(v2);
  int_vector_free(v3);
}



int main(int argc , char ** argv) {

  int_vector_type * int_vector = int_vector_alloc( 0 , 99);

  test_abort();
  test_assert_int_equal( -1 , int_vector_index(int_vector , 100));
  test_assert_int_equal( -1 , int_vector_index_sorted(int_vector , 100));

  test_assert_true( int_vector_is_instance( int_vector ));
  test_assert_false( double_vector_is_instance( int_vector ));
  int_vector_iset( int_vector , 2 , 0);
  int_vector_insert( int_vector , 2 , 77 );
  int_vector_iset( int_vector , 5 , -10);

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


  int_vector_init_range( int_vector , 100 , 1000 , 115 );
  test_assert_int_equal( int_vector_iget( int_vector , 0 ) , 100);
  test_assert_int_equal( int_vector_iget( int_vector , 1 ) , 215);
  test_assert_int_equal( int_vector_iget( int_vector , 2 ) , 330);
  test_assert_int_equal( int_vector_iget( int_vector , 3 ) , 445);
  test_assert_int_equal( int_vector_get_last( int_vector ) , 905);

  int_vector_init_range( int_vector , 100 , -1000 , -115 );
  test_assert_int_equal( int_vector_iget( int_vector , 0 ) , 100);
  test_assert_int_equal( int_vector_iget( int_vector , 1 ) , -15);
  test_assert_int_equal( int_vector_iget( int_vector , 2 ) , -130);
  test_assert_int_equal( int_vector_iget( int_vector , 3 ) , -245);
  test_assert_int_equal( int_vector_get_last( int_vector ) , -935);

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
  test_contains();
  test_contains_sorted();
  test_shift();
  test_alloc();
  test_div();
  test_memcpy_from_data();
  test_idel_insert();
  test_del();
  test_range_fill();
  test_iset_block();
  test_resize();
  test_empty();
  test_insert_double();
  test_equal_index();
  exit(0);
}

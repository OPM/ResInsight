/*
   Copyright (C) 2012  Statoil ASA, Norway. 
   
   The file 'test_util.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/test_util.h>

void test_error_exit( const char * fmt , ...) {
  char * s;
  va_list ap;
  va_start(ap , fmt);
  s = util_alloc_sprintf_va(fmt , ap);
  va_end(ap);

  fprintf( stderr , s );
  exit(1);
}


bool test_string_equal( const char * s1 , const char * s2 ) {
  bool equal = true;
  if (s1 == NULL && s2 == NULL)
    return true;
  else {
    if (s1 == NULL)
      equal = false;
    if (s2 == NULL)
      equal = false;

    if (equal && (strcmp(s1,s2) != 0))
      equal = false;
  }
  return equal;
}



void test_assert_string_equal__( const char * s1 , const char * s2 , const char * file, int line) {
  bool equal = test_string_equal( s1 , s2 );
  if (!equal) 
    test_error_exit( "%s:%d => String are different s1:[%s]  s2:[%s]\n" , file , line , s1 , s2 );
}



void test_assert_int_equal__( int i1 , int i2 , const char * file , int line) {
  if (i1 != i2) 
    test_error_exit( "%s:%d => Integers are different i1:[%d]  i2:[%d]\n" , file , line , i1 , i2 );
}


void test_assert_int_not_equal__( int i1 , int i2 , const char * file , int line) {
  if (i1 == i2) 
    test_error_exit( "%s:%d => Integers are equal i1:[%d]  i2:[%d]\n" , file , line , i1 , i2 );
}


void test_assert_bool_equal__( bool b1 , bool b2 , const char * file , int line) {
  if (b1 != b2) 
    test_error_exit( "%s:%d => Booleans are different b1:[%d]  b2:[%d]\n" , file , line , b1 , b2 );
}



/*****************************************************************/

void test_assert_time_t_equal__( time_t t1 , time_t t2 , const char * file , int line) {
  if (t1 != t2) 
    test_error_exit("%s:%d => time_t values are different t1:%d  t2:[%d]" , file , line , t1 , t2);
}


void test_assert_time_t_not_equal__( time_t t1 , time_t t2 , const char * file , int line) {
  if (t1 == t2) 
    test_error_exit("%s:%d => time_t values are different t1:%d  t2:[%d]" , file , line , t1 , t2);
}

/*****************************************************************/

void test_assert_true__( bool value, const char * file , int line) {
  if (!value) 
    test_error_exit("%s:%d => assert( true ) failed" , file , line);
}


void test_assert_false__( bool value, const char * file , int line) {
  if (value) 
    test_error_exit("%s:%d => assert( false ) failed" , file , line);
}


/*****************************************************************/

void test_assert_ptr_equal__( const void * p1 , const void * p2 , const char * file , int line) {
  bool equal = (p1 == p2);
  if (!equal) 
    test_error_exit( "%s:%d => Pointers are different p1:[%p]  p2:[%p]\n" , file , line , p1 , p2 );
}


void test_assert_NULL__( const void * p , const char * file , int line) {
  if (p != NULL) 
    test_error_exit( "%s:%d => Pointer is != NULL \n" , file , line , p);
}


void test_assert_not_NULL__( const void * p , const char * file , int line) {
  if (p == NULL) 
    test_error_exit( "%s:%d => Pointer is NULL \n" , file , line , p);
}


void test_assert_mem_equal__( const void * p1 , const void * p2 , size_t byte_size , const char * file , int line) {
  if (memcmp(p1 ,p2 , byte_size) != 0)
    test_error_exit( "%s:%d => Memory regions have different content \n" , file , line);
}


void test_assert_mem_not_equal__( const void * p1 , const void * p2 , size_t byte_size , const char * file , int line) {
  if (memcmp(p1 ,p2 , byte_size) == 0)
    test_error_exit( "%s:%d => Memory regions have the same content \n" , file , line);
}

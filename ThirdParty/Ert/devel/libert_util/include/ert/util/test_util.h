/*
   Copyright (C) 2012  Statoil ASA, Norway. 
   
   The file 'test_util.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

  

  void  test_error_exit( const char * fmt , ...);
  bool  test_string_equal( const char * s1 , const char * s2 );

#define test_assert_string_equal( s1 , s2 ) test_assert_string_equal__(s1 , s2 , __FILE__ , __LINE__)  
  void test_assert_string_equal__( const char * s1 , const char * s2 , const char * file , int line);

#define test_assert_int_equal( i1 , i2 ) test_assert_int_equal__( (i1) , (i2) , __FILE__ , __LINE__  )
  void  test_assert_int_equal__( int i1 , int i2 , const char * file , int line );

#define test_assert_int_not_equal( i1 , i2 ) test_assert_int_not_equal__( (i1) , (i2) , __FILE__ , __LINE__  )
  void  test_assert_int_not_equal__( int i1 , int i2 , const char * file , int line );

#define test_assert_bool_equal( b1 , b2 ) test_assert_bool_equal__( (b1) , (b2) , __FILE__ , __LINE__ )
  void test_assert_bool_equal__( bool b1 , bool b2 , const char * file , int line);

#define test_assert_true( value ) test_assert_true__( (value) , __FILE__ , __LINE__);
  void test_assert_true__( bool value, const char * file , int line);
  
#define test_assert_false( value ) test_assert_false__( (value) , __FILE__ , __LINE__);
  void test_assert_false__( bool value, const char * file , int line);
  
#define test_assert_time_t_equal( t1 , t2) test_assert_time_t_equal__((t1) , (t2) , __FILE__ , __LINE__);
  void test_assert_time_t_equal__( time_t t1 , time_t t2 , const char * file , int line);

#define test_assert_time_t_not_equal( t1 , t2) test_assert_time_t_not_equal__((t1) , (t2) , __FILE__ , __LINE__);
  void test_assert_time_t_not_equal__( time_t t1 , time_t t2 , const char * file , int line);

#define test_assert_ptr_equal( p1 , p2 ) test_assert_ptr_equal__( (p1) , (p2) , __FILE__ , __LINE__);  
  void test_assert_ptr_equal__( const void * p1 , const void * p2 , const char * file , int line);

#define test_assert_NULL( p ) test_assert_NULL__( (p) , __FILE__ , __LINE__);
  void test_assert_NULL__( const void * p , const char * file , int line);

#define test_assert_not_NULL( p ) test_assert_not_NULL__( (p) , __FILE__ , __LINE__);
  void test_assert_not_NULL__( const void * p , const char * file , int line);
  
#define test_assert_mem_equal( p1 , p2 , byte_size ) test_assert_mem_equal__( (p1) , (p2) , (byte_size), __FILE__ , __LINE__);
  void test_assert_mem_equal__( const void * p1 , const void * p2 , size_t byte_size , const char * file , int line);

#define test_assert_mem_not_equal( p1 , p2 , byte_size ) test_assert_mem_not_equal__( (p1) , (p2) , (byte_size), __FILE__ , __LINE__);
  void test_assert_mem_not_equal__( const void * p1 , const void * p2 , size_t byte_size , const char * file , int line);

#ifdef __cplusplus
}
#endif
#endif

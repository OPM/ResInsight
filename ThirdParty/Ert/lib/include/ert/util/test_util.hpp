/*
   Copyright (C) 2012  Equinor ASA, Norway.

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


#ifndef ERT_TEST_UTIL_H
#define ERT_TEST_UTIL_H

#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <ert/util/ert_api_config.hpp>

#if defined(__APPLE__)
#include <wchar.h>
#include <time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


  void  test_error_exit( const char * fmt , ...);
  void * test_argpack_is_stringlist( void * arg );
  void * thread_pool_test_func1( void * arg );

#define test_exit( fmt, ...) test_exit__( __FILE__ , __LINE__ , fmt , __VA_ARGS__);
  void  test_exit__(const char * file , int line , const char * fmt , ...);

#define test_assert_string_equal( s1 , s2 ) test_assert_string_equal__(s1 , s2 , __FILE__ , __LINE__)
  void  test_assert_string_equal__( const char * s1 , const char * s2 , const char * file , int line);
#define test_assert_string_not_equal( s1 , s2 ) test_assert_string_not_equal__(s1 , s2 , __FILE__ , __LINE__)
  void  test_assert_string_not_equal__( const char * s1 , const char * s2 , const char * file , int line);

  bool  test_check_string_equal( const char *s1 , const char * s2);

#define test_assert_int_equal( i1 , i2 ) test_assert_int_equal__( (i1) , (i2) , __FILE__ , __LINE__  )
  void  test_assert_int_equal__( int i1 , int i2 , const char * file , int line );

#define test_assert_int_not_equal( i1 , i2 ) test_assert_int_not_equal__( (i1) , (i2) , __FILE__ , __LINE__  )
  void  test_assert_int_not_equal__( int i1 , int i2 , const char * file , int line );

#define test_assert_long_equal( i1 , i2 ) test_assert_long_equal__( (i1) , (i2) , __FILE__ , __LINE__  )
  void  test_assert_long_equal__( long i1 , long i2 , const char * file , long line );

#define test_assert_long_not_equal( i1 , i2 ) test_assert_long_not_equal__( (i1) , (i2) , __FILE__ , __LINE__  )
  void  test_assert_long_not_equal__( long i1 , long i2 , const char * file , long line );

#define test_assert_uint_equal( i1 , i2 ) test_assert_uint_equal__( (i1) , (i2) , __FILE__ , __LINE__  )
  void  test_assert_uint_equal__( unsigned int i1 , unsigned int i2 , const char * file , int line );

#define test_assert_uint_not_equal( i1 , i2 ) test_assert_uint_not_equal__( (i1) , (i2) , __FILE__ , __LINE__  )
  void  test_assert_uint_not_equal__( unsigned int i1 , unsigned int i2 , const char * file , int line );

#define test_assert_size_t_equal( s1 , s2 ) test_assert_size_t_equal__( (s1) , (s2) , __FILE__ , __LINE__  )
  void  test_assert_size_t_equal__( size_t s1 , size_t s2 , const char * file , int line );

#define test_assert_size_t_not_equal( s1 , s2 ) test_assert_size_t_not_equal__( (s1) , (s2) , __FILE__ , __LINE__  )
  void  test_assert_size_t_not_equal__( size_t s1 , size_t s2 , const char * file , int line );


#define test_assert_double_equal( d1 , d2 ) test_assert_double_equal__( (d1) , (d2) , __FILE__ , __LINE__  )
  void  test_assert_double_equal__( double d1 , double d2 , const char * file , int line );
  bool  test_check_double_equal( double d1 , double d2);

#define test_assert_float_equal( d1 , d2 ) test_assert_float_equal__( (d1) , (d2) , __FILE__ , __LINE__  )
  void  test_assert_float_equal__( float d1 , float d2 , const char * file , int line );
  bool  test_check_float_equal( float d1 , float d2);

#define test_assert_double_not_equal( d1 , d2 ) test_assert_double_not_equal__( (d1) , (d2) , __FILE__ , __LINE__  )
  void  test_assert_double_not_equal__( double d1 , double d2 , const char * file , int line );

#define test_assert_bool_equal( b1 , b2 ) test_assert_bool_equal__( (b1) , (b2) , __FILE__ , __LINE__ )
  void test_assert_bool_equal__( bool b1 , bool b2 , const char * file , int line);

#define test_assert_bool_not_equal( b1 , b2 ) test_assert_bool_not_equal__( (b1) , (b2) , __FILE__ , __LINE__ )
  void test_assert_bool_not_equal__( bool b1 , bool b2 , const char * file , int line);

#define test_assert_true( value ) test_assert_true__( (value) , __FILE__ , __LINE__)
  void test_assert_true__( bool value, const char * file , int line);

#define test_assert_false( value ) test_assert_false__( (value) , __FILE__ , __LINE__)
  void test_assert_false__( bool value, const char * file , int line);

#define test_assert_time_t_equal( t1 , t2) test_assert_time_t_equal__((t1) , (t2) , __FILE__ , __LINE__)
  void test_assert_time_t_equal__( time_t t1 , time_t t2 , const char * file , int line);

#define test_assert_time_t_not_equal( t1 , t2) test_assert_time_t_not_equal__((t1) , (t2) , __FILE__ , __LINE__)
  void test_assert_time_t_not_equal__( time_t t1 , time_t t2 , const char * file , int line);

#define test_assert_ptr_equal( p1 , p2 ) test_assert_ptr_equal__( (p1) , (p2) , __FILE__ , __LINE__)
  void test_assert_ptr_equal__( const void * p1 , const void * p2 , const char * file , int line);

#define test_assert_ptr_not_equal(p1 , p2) test_assert_ptr_not_equal__( (p1) , (p2) , __FILE__ , __LINE__)
  void test_assert_ptr_not_equal__( const void * p1 , const void * p2 , const char * file , int line);

#define test_assert_NULL( p ) test_assert_NULL__( (p) , __FILE__ , __LINE__)
  void test_assert_NULL__( const void * p , const char * file , int line);

#define test_assert_not_NULL( p ) test_assert_not_NULL__( (p) , __FILE__ , __LINE__)
  void test_assert_not_NULL__( const void * p , const char * file , int line);

#define test_assert_mem_equal( p1 , p2 , byte_size ) test_assert_mem_equal__( (p1) , (p2) , (byte_size), __FILE__ , __LINE__)
  void test_assert_mem_equal__( const void * p1 , const void * p2 , size_t byte_size , const char * file , int line);

#define test_assert_mem_not_equal( p1 , p2 , byte_size ) test_assert_mem_not_equal__( (p1) , (p2) , (byte_size), __FILE__ , __LINE__)
  void test_assert_mem_not_equal__( const void * p1 , const void * p2 , size_t byte_size , const char * file , int line);

#define test_assert_file_content( input_file , expected) test_assert_file_content__( input_file , expected , __FILE__ , __LINE__)
  void test_assert_file_content__( const char * input_file , const char * expected, const char * src_file , int line);

  void test_install_SIGNALS(void);

  jmp_buf * util_abort_test_jump_buffer();
  void   test_util_addr2line();
  void   test_assert_util_abort(const char * function_name , void call_func (void *) , void * arg);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#define test_assert_throw(expr , exception_type )                       \
  {                                                                     \
    bool throw_ok = false;                                              \
    try {                                                               \
      expr;                                                             \
    }                                                                   \
    catch (std::exception &e) {                                         \
      if (dynamic_cast<exception_type *>(&e))                           \
        throw_ok = true;                                                \
    }                                                                   \
    if (!throw_ok)                                                      \
      test_error_exit("Correct exception not thrown at %s:%d\n",__FILE__ , __LINE__); \
  }

#define test_assert_std_string_equal(s0, s1)                        \
  {                                                                 \
    if (s0.compare(s1) != 0)                                        \
      test_error_exit("Strings not equal at%s:%d (%s != %s)\n",     \
                      __FILE__ , __LINE__, s0.c_str(), s1.c_str()); \
  }


#endif


#endif

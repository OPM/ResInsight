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
#include <signal.h>

#include "ert/util/build_config.h"
#include <ert/util/util.h>
#include <ert/util/arg_pack.h>
#include <ert/util/test_util.h>
#include <ert/util/stringlist.h>
#include <ert/util/util.h>

void test_error_exit( const char * fmt , ...) {
  char * s;
  va_list ap;
  va_start(ap , fmt);
  s = util_alloc_sprintf_va(fmt , ap);
  va_end(ap);
  fprintf(stderr, "%s", s);
  exit(1);
}


void test_exit__(const char * file , int line , const char * fmt , ...) {
  fprintf(stderr , "Error at %s:%d:\n",file,line);
  {
    char * s;
    va_list ap;
    va_start(ap , fmt);
    s = util_alloc_sprintf_va(fmt , ap);
    va_end(ap);
    fprintf(stderr, "%s", s);
    exit(1);
  }
}


bool test_check_string_equal( const char * s1 , const char * s2 ) {
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
  bool equal = test_check_string_equal( s1 , s2 );
  if (!equal)
    test_error_exit( "%s:%d => String are different s1:[%s]  s2:[%s]\n" , file , line , s1 , s2 );
}


void test_assert_string_not_equal__( const char * s1 , const char * s2 , const char * file, int line) {
  bool equal = test_check_string_equal( s1 , s2 );
  if (equal)
    test_error_exit( "%s:%d => String are equal s1:[%s]  s2:[%s]\n" , file , line , s1 , s2 );
}


void test_assert_long_equal__( long i1 , long i2 , const char * file , long line) {
  if (i1 != i2)
    test_error_exit( "%s:%d => Long values are different i1:[%d]  i2:[%d]\n" , file , line , i1 , i2 );
}


void test_assert_long_not_equal__( long i1 , long i2 , const char * file , long line) {
  if (i1 == i2)
    test_error_exit( "%s:%d => Long values are equal i1:[%d]  i2:[%d]\n" , file , line , i1 , i2 );
}


void test_assert_int_equal__( int i1 , int i2 , const char * file , int line) {
  if (i1 != i2)
    test_error_exit( "%s:%d => Integers are different i1:[%d]  i2:[%d]\n" , file , line , i1 , i2 );
}


void test_assert_int_not_equal__( int i1 , int i2 , const char * file , int line) {
  if (i1 == i2)
    test_error_exit( "%s:%d => Integers are equal i1:[%d]  i2:[%d]\n" , file , line , i1 , i2 );
}


void test_assert_uint_equal__( unsigned int i1 , unsigned int i2 , const char * file , int line) {
  if (i1 != i2)
    test_error_exit( "%s:%d => Unsigned integers are different i1:[%d]  i2:[%d]\n" , file , line , i1 , i2 );
}


void test_assert_uint_not_equal__( unsigned int i1 , unsigned int i2 , const char * file , int line) {
  if (i1 == i2)
    test_error_exit( "%s:%d => Unsigned integers are equal i1:[%d]  i2:[%d]\n" , file , line , i1 , i2 );
}


void test_assert_bool_equal__( bool b1 , bool b2 , const char * file , int line) {
  if (b1 != b2)
    test_error_exit( "%s:%d => Booleans are different b1:[%d]  b2:[%d]\n" , file , line , b1 , b2 );
}


void test_assert_bool_not_equal__( bool b1 , bool b2 , const char * file , int line) {
  if (b1 == b2)
    test_error_exit( "%s:%d => Booleans are equal b1:[%d]  b2:[%d]\n" , file , line , b1 , b2 );
}


/*****************************************************************/

void test_assert_size_t_equal__( size_t s1 , size_t s2 , const char * file , int line) {
  if (s1 != s2)
    test_error_exit("%s:%d => size_t values are different s1:%zu  s2:[%zu]\n" , file , line , s1 , s2);
}


void test_assert_size_t_not_equal__( size_t s1 , size_t s2 , const char * file , int line) {
  if (s1 == s2)
    test_error_exit("%s:%d => size_t values are different s1:%zu  s2:[%zu]\n" , file , line , s1 , s2);
}


/*****************************************************************/

void test_assert_time_t_equal__( time_t t1 , time_t t2 , const char * file , int line) {
  if (t1 != t2)
    test_error_exit("%s:%d => time_t values are different t1:%d  t2:[%d]\n" , file , line , t1 , t2);
}


void test_assert_time_t_not_equal__( time_t t1 , time_t t2 , const char * file , int line) {
  if (t1 == t2)
    test_error_exit("%s:%d => time_t values are different t1:%d  t2:[%d]\n" , file , line , t1 , t2);
}



/*****************************************************************/

void test_assert_true__( bool value, const char * file , int line) {
  if (!value)
    test_error_exit("%s:%d => assert( true ) failed\n" , file , line);
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


void test_assert_ptr_not_equal__( const void * p1 , const void * p2 , const char * file , int line) {
  bool equal = (p1 == p2);
  if (equal)
    test_error_exit( "%s:%d => Pointers are equal p1:[%p]  p2:[%p]\n" , file , line , p1 , p2 );
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
  if (memcmp(p1 ,p2 , byte_size) != 0) {
    const char * char_ptr1 = (const char *) p1;
    const char * char_ptr2 = (const char *) p2;
    size_t offset = 0;

    while( char_ptr1[offset] == char_ptr2[offset])
      offset++;

    test_error_exit( "%s:%d => Memory regions have different content. First difference at offset:%ld\n" , file , line , offset);
  }
}


void test_assert_mem_not_equal__( const void * p1 , const void * p2 , size_t byte_size , const char * file , int line) {
  if (memcmp(p1 ,p2 , byte_size) == 0)
    test_error_exit( "%s:%d => Memory regions have the same content \n" , file , line);
}


bool test_check_double_equal( double d1 , double d2) {
  const double tolerance = 1e-5;
  return util_double_approx_equal__( d1 , d2 , tolerance , 0);
}


void test_assert_double_equal__( double d1 , double d2, const char * file , int line) {
  if (!test_check_double_equal(d1 , d2))
    test_error_exit( "%s:%d => double values:%g %g are not sufficiently similar\n" , file , line , d1 , d2);
}


void test_assert_double_not_equal__( double d1 , double d2, const char * file , int line) {
  if (test_check_double_equal(d1 , d2))
    test_error_exit( "%s:%d => double values:%15.12g %15.12g are equal.\n" , file , line , d1 , d2);
}

bool test_check_float_equal( float d1 , float d2) {
  const float tolerance = 1e-4;
  return util_float_approx_equal__( d1 , d2 , tolerance , 0);
}


void test_assert_float_equal__( float d1 , float d2, const char * file , int line) {
  if (!test_check_float_equal(d1 , d2))
    test_error_exit( "%s:%d => float values:%g %g are not sufficiently similar\n" , file , line , d1 , d2);
}


void test_assert_float_not_equal__( float d1 , float d2, const char * file , int line) {
  if (test_check_float_equal(d1 , d2))
    test_error_exit( "%s:%d => float values:%15.12g %15.12g are equal.\n" , file , line , d1 , d2);
}


void test_assert_file_content__( const char * input_file , const char * expected, const char * src_file , int line) {
  if (util_file_exists( input_file )) {
    char * content = util_fread_alloc_file_content(input_file, NULL);
    if (!util_string_equal( content , expected))
      test_error_exit("%s:%d  content difference \n",src_file , line);
    free( content );
  } else
    test_error_exit("%s:%d => No such file:%s \n", src_file , line , input_file);
}


/*****************************************************************/

void test_install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the program with SIGTERM (the default kill signal) you will get a backtrace.
                                             Killing with SIGKILL (-9) will not give a backtrace.*/
}


/*****************************************************************/

#ifdef HAVE_BACKTRACE
#include <execinfo.h>


void test_util_addr2line() {
  const char * file = __FILE__;
  const char * func = __func__;
  int    line;
  const int max_bt = 50;
  void *bt_addr[max_bt];
  int size;
  char * func_name , * file_name;
  int line_nr;

  line = __LINE__ + 2;
  size = backtrace(bt_addr , max_bt);
  test_assert_int_equal( size , 4 );
  test_assert_true( util_addr2line_lookup( bt_addr[0] , &func_name , &file_name , &line_nr));
  test_assert_string_equal( func_name , func );
  test_assert_int_equal( line , line_nr );
  test_assert_string_equal( file_name , file );
}


void test_assert_util_abort(const char * function_name , void call_func (void *) , void * arg) {
  bool util_abort_intercepted = false;

  {
    jmp_buf * context = util_abort_test_jump_buffer();
    util_abort_test_set_intercept_function( function_name );

    if (setjmp(*context) == 0)
      call_func( arg );
    else
      util_abort_intercepted = true;

    util_abort_test_set_intercept_function( NULL );

 }

  if (!util_abort_intercepted) {
    fprintf(stderr,"Expected call to util_abort() from:%s missing \n",function_name);
    test_assert_true( util_abort_intercepted );
  }
}

#else

/*
  These are non-functional stubs.
*/

bool util_addr2line_lookup(const void * bt_addr , char ** func_name , char ** file_name , int * line_nr)
{
  return false;
}

jmp_buf * util_abort_test_jump_buffer()
{
  return NULL;
}

void      util_abort_test_set_intercept_function(const char * function)
{
  return;
}


#endif


/*****************************************************************/

#ifdef HAVE_PTHREAD
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * thread_pool_test_func1( void * arg ) {
  int * value = (int *) arg;
  pthread_mutex_lock( &mutex );
  value[0]++;
  pthread_mutex_unlock( &mutex );
  return NULL;
}
#endif


void * test_argpack_is_stringlist( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  void * arg0 = arg_pack_iget_ptr( arg_pack , 0 );
  test_assert_true( stringlist_is_instance( arg0 ) );
  return NULL;
}

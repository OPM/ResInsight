/*
 * File:   ert_util_abort_gnu_tests.c
 * Author: kflik
 *
 * Created on August 16, 2013, 9:41 AM
 */

#include <unistd.h>
#include <stdlib.h>
#include <ert/util/util.h>
#include <ert/util/test_util.h>


void      test_assert_util_abort(const char * function_name , void (void *) , void * arg);


void call_util_abort(void * arg) {
  util_abort("%s: I am calling util_abort - should be intercepted\n",__func__ );
}

void test_intercept() {
  test_assert_util_abort( "call_util_abort" , call_util_abort , NULL );
}

int main(int argc, char** argv) {
  test_intercept();
  exit(0);
}


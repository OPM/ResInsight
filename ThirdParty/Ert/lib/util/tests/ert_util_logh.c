/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ert_util_logh.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/util.h>
#include <ert/util/log.h>

#define LOG_FILE "log.txt"


void test_open() {
  test_work_area_type * work_area = test_work_area_alloc("util/logh");
  {
    log_type * logh = log_open( NULL , 0 );
    test_assert_int_equal( 0 , log_get_msg_count( logh ));
    test_assert_false( log_is_open( logh ));
    log_reopen( logh , LOG_FILE );
    test_assert_true( log_is_open( logh ));

    log_close( logh );
  }

  {
    log_type * logh = log_open( LOG_FILE , 0 );
    test_assert_not_NULL(logh);
    log_close( logh );
  }

  {
    log_type * logh = log_open( LOG_FILE , 1 );
    test_assert_true( log_is_open( logh ));
    log_add_message( logh , 1 , NULL , "Message" , false);
    test_assert_int_equal( 1 , log_get_msg_count( logh ));
    log_close( logh );
  }

  test_work_area_free( work_area );
}


void test_delete_empty() {
  test_work_area_type * work_area = test_work_area_alloc("logh_delete_empty");
  {
    log_type * logh = log_open( LOG_FILE , 0 );
    test_assert_not_NULL(logh);
    log_close( logh );

    test_assert_false( util_file_exists( LOG_FILE ));
  }

  {
    log_type * logh = log_open( LOG_FILE , 0 );
    log_reopen( logh , "LOG2.txt");
    log_close( logh );

    test_assert_false( util_file_exists( LOG_FILE ));
  }

  {
    log_type * logh = log_open( LOG_FILE , 1 );
    log_add_message( logh , 1 , NULL , "Message" , false);
    log_close( logh );
    test_assert_true( util_file_exists( LOG_FILE ));

    logh = log_open( LOG_FILE , 1 );
    log_close( logh );
    test_assert_true( util_file_exists( LOG_FILE ));
  }

  test_work_area_free( work_area );
}


/*
   Someone else deletes the file before closing - that should not kill the thing.
*/

void test_file_deleted() {
  log_type * logh = log_open( LOG_FILE , 1 );
  log_add_message( logh , 1 , NULL , "Message" , false);
  util_unlink( LOG_FILE );
  test_assert_false( util_file_exists( LOG_FILE ));
  log_close( logh );
  test_assert_false( util_file_exists( LOG_FILE ));
}

int main(int argc , char ** argv) {
  test_open();
  test_delete_empty();
  test_file_deleted( );
  exit(0);
}

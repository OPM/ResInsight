/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'enkf_ui_return_type.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>
#include <ert/util/ui_return.h>




void test_create() {
  ui_return_status_enum status = UI_RETURN_OK;
  ui_return_type * ui_return = ui_return_alloc(status);
  test_assert_true( ui_return_is_instance( ui_return ));
  test_assert_int_equal( status , ui_return_get_status(ui_return));
  ui_return_free( ui_return );
}


void test_default() {
  ui_return_status_enum status = UI_RETURN_OK;
  ui_return_type * ui_return = ui_return_alloc(status);

  test_assert_int_equal( 0 , ui_return_get_error_count(ui_return));
  test_assert_NULL( ui_return_get_first_error( ui_return));
  test_assert_NULL( ui_return_get_last_error( ui_return));
  test_assert_NULL( ui_return_get_help(ui_return));
  ui_return_free( ui_return);
}

void test_errors_inconsistent() {
  ui_return_status_enum status = UI_RETURN_OK;
  ui_return_type * ui_return = ui_return_alloc(status);

  test_assert_int_equal( 0 , ui_return_get_error_count(ui_return));
  test_assert_false( ui_return_add_error( ui_return , "ERROR1"));
  test_assert_int_equal( 0 , ui_return_get_error_count(ui_return));
  ui_return_free( ui_return);
}


void test_errors_consistent() {
  ui_return_status_enum status = UI_RETURN_FAIL;
  ui_return_type * ui_return = ui_return_alloc(status);

  test_assert_int_equal( 0 , ui_return_get_error_count(ui_return));
  test_assert_true( ui_return_add_error( ui_return , "ERROR1"));
  test_assert_int_equal(1, ui_return_get_error_count(ui_return));
  test_assert_string_equal("ERROR1", ui_return_get_first_error(ui_return));
  test_assert_string_equal("ERROR1", ui_return_get_last_error(ui_return));

  test_assert_true(ui_return_add_error(ui_return, "ERROR2"));
  test_assert_int_equal(2, ui_return_get_error_count(ui_return));
  test_assert_string_equal("ERROR1", ui_return_get_first_error(ui_return));
  test_assert_string_equal("ERROR2", ui_return_get_last_error(ui_return));

  test_assert_string_equal("ERROR1" , ui_return_iget_error(ui_return , 0));
  test_assert_string_equal("ERROR2" , ui_return_iget_error(ui_return , 1));

  ui_return_free( ui_return);
}


void test_help() {
  ui_return_type * ui_return = ui_return_alloc(UI_RETURN_OK);

  ui_return_add_help(ui_return , "HELP1");
  test_assert_string_equal( "HELP1" , ui_return_get_help(ui_return));

  ui_return_add_help(ui_return , "HELP2");
  test_assert_string_equal( "HELP1 HELP2" , ui_return_get_help(ui_return));

  ui_return_free( ui_return);
}

int main(int argc , char ** argv) {
  test_create();
  test_default();
  test_errors_inconsistent();
  exit(0);
}

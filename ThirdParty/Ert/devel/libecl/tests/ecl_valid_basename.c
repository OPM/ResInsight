/*

 *
 *  Created on: Sep 2, 2013
 *      Author: joaho
 */
/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_lgr_test.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>
#include <ert/ecl/ecl_util.h>




int main(int argc , char ** argv) {
    test_assert_true( ecl_util_valid_basename("ECLIPSE.DATA"));
    test_assert_true( ecl_util_valid_basename("ECLIPS100.DATA"));
    test_assert_true( ecl_util_valid_basename("eclipse100.data"));
    test_assert_true( ecl_util_valid_basename("MYPATH/ECLIPSE.DATA"));
    test_assert_true( ecl_util_valid_basename("mypath/ECLIPSE.DATA"));
    test_assert_false( ecl_util_valid_basename("ECLiPS100.DATa"));
    test_assert_false( ecl_util_valid_basename("mypath/eclipse.DATA"));

    test_assert_true( ecl_util_valid_basename_fmt("ECL_%d.DATA"));
    test_assert_true( ecl_util_valid_basename_fmt("ECL_%04d.DATA"));
    test_assert_true( ecl_util_valid_basename_fmt("mypath/ECL_%04d.DATA"));
    test_assert_true( ecl_util_valid_basename_fmt("MYPATH/ECL_%04d.DATA"));
    test_assert_true( ecl_util_valid_basename_fmt("MYPATH/ECL_%04d.DATA"));
    test_assert_false( ecl_util_valid_basename_fmt("ECL_%d.dATA"));
    test_assert_false( ecl_util_valid_basename_fmt("ECL_%s.DATA"));
    test_assert_false( ecl_util_valid_basename_fmt("mypath/ECL_%d.dATA"));

    exit(0);
}


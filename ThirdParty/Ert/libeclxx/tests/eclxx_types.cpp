/*
   Copyright (C) 2017  Statoil ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdexcept>
#include <fstream>


#include <ert/util/TestArea.hpp>
#include <ert/util/test_util.hpp>
#include <ert/ecl/ecl_type.h>
#include <ert/ecl/FortIO.hpp>


void test_ECL_INT() {
    ecl_data_type dt = ECL_INT;
    test_assert_int_equal( dt.element_size , sizeof(int) );
    test_assert_int_equal( dt.type , ECL_INT_TYPE );
}


void test_ECL_FLOAT() {
    ecl_data_type dt = ECL_FLOAT;
    test_assert_int_equal( dt.element_size , sizeof(float) );
    test_assert_int_equal( dt.type , ECL_FLOAT_TYPE );
}


void test_ECL_DOUBLE() {
    ecl_data_type dt = ECL_DOUBLE;
    test_assert_int_equal( dt.element_size , sizeof(double) );
    test_assert_int_equal( dt.type , ECL_DOUBLE_TYPE );
}


void test_ECL_CHAR() {
    ecl_data_type dt = ECL_CHAR;
    test_assert_int_equal( dt.element_size , ECL_STRING8_LENGTH + 1 );
    test_assert_int_equal( dt.type , ECL_CHAR_TYPE );
}



int main(int argc , char ** argv) {
    test_ECL_INT();
    test_ECL_FLOAT();
    test_ECL_DOUBLE();
    test_ECL_CHAR();
}

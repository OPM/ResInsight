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

#include <ert/util/test_util.hpp>
#include <ert/ecl/EclFilename.hpp>


void cmp(const char * expected, const std::string& value) {
    test_assert_string_equal( expected , value.c_str());
}


int main (int argc, char **argv) {
    cmp( "BASE.X0067" , ERT::EclFilename("BASE" , ECL_RESTART_FILE , 67 ));
    cmp( "BASE.F0067" , ERT::EclFilename("BASE" , ECL_RESTART_FILE , 67 , true));

    cmp( "BASE.EGRID" , ERT::EclFilename("BASE" , ECL_EGRID_FILE ));
    cmp( "BASE.FEGRID" , ERT::EclFilename("BASE" , ECL_EGRID_FILE , true));

    cmp( "BASE.EGRID" , ERT::EclFilename("BASE" , ECL_EGRID_FILE , 67));
    cmp( "BASE.FEGRID" , ERT::EclFilename("BASE" , ECL_EGRID_FILE , 67 , true));


    try {
        ERT::EclFilename("BASE" , ECL_RESTART_FILE );
        test_assert_true( false );
    } catch (...) {
        test_assert_true( true );
    }


    cmp( "PATH/BASE.X0067" , ERT::EclFilename("PATH", "BASE" , ECL_RESTART_FILE , 67 ));
    cmp( "PATH/BASE.F0067" , ERT::EclFilename("PATH", "BASE" , ECL_RESTART_FILE , 67 , true));

    cmp( "PATH/BASE.EGRID" , ERT::EclFilename("PATH", "BASE" , ECL_EGRID_FILE ));
    cmp( "PATH/BASE.FEGRID" , ERT::EclFilename("PATH", "BASE" , ECL_EGRID_FILE , true));

    cmp( "PATH/BASE.EGRID" , ERT::EclFilename("PATH", "BASE" , ECL_EGRID_FILE , 67));
    cmp( "PATH/BASE.FEGRID" , ERT::EclFilename("PATH", "BASE" , ECL_EGRID_FILE , 67 , true));


    try {
        ERT::EclFilename("PATH", "BASE" , ECL_RESTART_FILE );
        test_assert_true( false );
    } catch (...) {
        test_assert_true( true );
    }

    test_assert_int_equal( ECL_EGRID_FILE , ERT::EclFiletype("CASE.EGRID" ) );
    test_assert_int_equal( ECL_RESTART_FILE , ERT::EclFiletype("CASE.F0098") );
    test_assert_int_equal( ECL_UNIFIED_RESTART_FILE , ERT::EclFiletype("CASE.UNRST") );
}

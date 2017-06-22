/*
  Copyright 2015 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>

#include <ert/ecl/Smspec.hpp>

void test_smspec_copy() {
    std::string kw( "FOPT" );
    ERT::smspec_node field( kw );

    ERT::smspec_node copy( field );
}

void test_smspec_get() {
    std::string kw( "FOPT" );
    ERT::smspec_node field( kw );

    test_assert_true( field.get() != nullptr );
}

void test_smspec_move() {
    std::string kw( "FOPT" );
    ERT::smspec_node field( kw );
    ERT::smspec_node move_ctor( std::move( field ) );
    auto move_assignment = std::move( move_ctor );
}

void test_smspec_wg() {
    std::string wkw( "WWCT" );
    std::string gkw( "GWCT" );
    std::string wg( "OP1" );
    std::string gr( "WG1" );

    ERT::smspec_node well( ECL_SMSPEC_WELL_VAR, wg, wkw );
    ERT::smspec_node group( ECL_SMSPEC_GROUP_VAR, gr, gkw );

    test_assert_string_equal(well.key1() , "WWCT:OP1");
    test_assert_string_equal(well.keyword() , "WWCT");
    test_assert_true(well.wgname() == wg);
    test_assert_true(well.type() == ECL_SMSPEC_WELL_VAR );

    test_assert_string_equal(group.key1(), "GWCT:WG1");
    test_assert_string_equal(group.keyword() , "GWCT");
    test_assert_true(group.wgname() == gr);
    test_assert_true(group.type() == ECL_SMSPEC_GROUP_VAR );
}

void test_smspec_field() {
    std::string kw( "FOPT" );
    ERT::smspec_node field( kw );

    test_assert_string_equal( field.key1() , "FOPT" );
    test_assert_true( field.keyword() == kw );
    test_assert_true( field.type() == ECL_SMSPEC_FIELD_VAR );
}

void test_smspec_block() {
    std::string kw( "BPR" );
    int dims[ 3 ] = { 10, 10, 10 };
    int ijk[ 3 ] = { 5, 5, 5 };

    ERT::smspec_node block( kw, dims, ijk );

    // Observe that ERT::smspec_node( ) constructor is considered part
    // of the developer API, with offset zero indices, whereas the
    // key1() string is meant for user consumption and has offset 1 -
    // what a mess!
    test_assert_string_equal( block.key1(), "BPR:6,6,6" );
    test_assert_true( block.keyword() == kw );
    test_assert_true( block.type() == ECL_SMSPEC_BLOCK_VAR );
    test_assert_true( block.num() == 556 );
}

void test_smspec_misc() {
    ERT::smspec_node tcpu( "TCPU" );
    test_assert_true( tcpu.type() == ECL_SMSPEC_MISC_VAR );
}


void test_smspec_region() {
    std::string kw( "ROIP" );
    int dims[ 3 ] = { 10, 10, 10 };
    ERT::smspec_node region( kw, dims, 0 );

    test_assert_true( region.keyword() == kw );
    test_assert_true( region.type() == ECL_SMSPEC_REGION_VAR );
    test_assert_true( region.num() == 0 );
}

void test_smspec_completion() {
    std::string kw( "CWIT" );
    std::string wg( "WELL1" );
    int dims[ 3 ] = { 10, 10, 10 };
    int ijk[ 3 ] = { 1, 1, 1 };
    ERT::smspec_node completion( kw, wg, dims, ijk );

    test_assert_true( completion.keyword() == kw );
    test_assert_true( completion.type() == ECL_SMSPEC_COMPLETION_VAR );
    test_assert_true( completion.num() == 112 );
}

int main (int argc, char **argv) {
    test_smspec_copy();
    test_smspec_move();
    test_smspec_get();
    test_smspec_wg();
    test_smspec_field();
    test_smspec_block();
    test_smspec_region();
    test_smspec_completion();
    test_smspec_misc();
}

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

#include <stdexcept>
#include <fstream>

#include <ert/util/test_util.h>

#include <ert/ecl/EclKW.hpp>
#include <ert/ecl/FortIO.hpp>

void test_kw_name() {
    ERT::EclKW< int > kw1( "short", 1 );
    ERT::EclKW< int > kw2( "verylong", 1 );

    test_assert_string_equal( kw1.name(), "short" );
    test_assert_string_equal( kw2.name(), "verylong" );
}

void test_kw_vector_assign() {
    std::vector< int > vec = { 1, 2, 3, 4, 5 };
    ERT::EclKW< int > kw( "XYZ", vec );

    test_assert_size_t_equal( kw.size(), vec.size() );

    for( size_t i = 0; i < kw.size(); ++i ) {
        test_assert_int_equal( kw.at( i ), vec[ i ] );
        test_assert_int_equal( kw[ i ] , vec[ i ] );
    }

    for( size_t i = 0; i < kw.size(); ++i ) {
        kw[i] *= 2;
        test_assert_int_equal( kw[ i ] , 2*vec[ i ] );
    }
}

void test_kw_vector_string() {
    std::vector< const char* > vec = {
        "short",
        "sweet",
        "padded  ",
        "verylongkeyword"
    };

    ERT::EclKW< const char* > kw( "XYZ", vec );

    test_assert_size_t_equal( kw.size(), vec.size() );

    test_assert_string_equal( kw.at( 0 ), "short   " );
    test_assert_string_equal( kw.at( 1 ), "sweet   " );
    test_assert_string_equal( kw.at( 2 ), vec.at( 2 ) );
    test_assert_string_equal( kw.at( 3 ), "verylong" );
    test_assert_string_not_equal( kw.at( 2 ), "verylongkeyword" );
}

void test_move_semantics_no_crash() {
    std::vector< int > vec = { 1, 2, 3, 4, 5 };
    ERT::EclKW< int > kw1( "XYZ", vec );

    ERT::EclKW< int > kw2( std::move( kw1 ) );
    test_assert_true( kw1.get() == nullptr );
}

void test_exception_assing_ref_wrong_type() {
    auto* ptr = ecl_kw_alloc( "XYZ", 1, ECL_INT );

    try {
        ERT::EclKW< double > kw( ptr );
        test_assert_true( false );
    } catch (...) {
        ERT::EclKW< int > kw( ptr );
    }
}

void test_resize() {
    ERT::EclKW< int > kw1( "short", 1 );

    test_assert_int_equal( kw1.size() , 1 );
    kw1.resize( 100 );
    test_assert_int_equal( kw1.size() , 100 );
}


int main (int argc, char **argv) {
    test_kw_name();
    test_kw_vector_assign();
    test_kw_vector_string();
    test_move_semantics_no_crash();
    test_exception_assing_ref_wrong_type();
    test_resize();
}


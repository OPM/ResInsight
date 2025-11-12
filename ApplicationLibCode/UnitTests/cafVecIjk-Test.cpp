/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "cafVecIjk.h"

using namespace caf;

TEST( VecIjk0Test, Construction )
{
    VecIjk0 vec0( 0, 1, 2 );

    EXPECT_EQ( 0u, vec0.i() );
    EXPECT_EQ( 1u, vec0.j() );
    EXPECT_EQ( 2u, vec0.k() );
}

TEST( VecIjk0Test, ToOneBased )
{
    VecIjk0 vec0( 0, 1, 2 );
    VecIjk1 vec1 = vec0.toOneBased();

    EXPECT_EQ( 1u, vec1.i() );
    EXPECT_EQ( 2u, vec1.j() );
    EXPECT_EQ( 3u, vec1.k() );
}

TEST( VecIjk0Test, ToString )
{
    VecIjk0 vec0( 5, 10, 15 );
    EXPECT_EQ( "5, 10, 15", vec0.toString() );
}

TEST( VecIjk1Test, Construction )
{
    VecIjk1 vec1( 1, 2, 3 );

    EXPECT_EQ( 1u, vec1.i() );
    EXPECT_EQ( 2u, vec1.j() );
    EXPECT_EQ( 3u, vec1.k() );
}

TEST( VecIjk1Test, ToZeroBased )
{
    VecIjk1 vec1( 1, 2, 3 );
    VecIjk0 vec0 = vec1.toZeroBased();

    EXPECT_EQ( 0u, vec0.i() );
    EXPECT_EQ( 1u, vec0.j() );
    EXPECT_EQ( 2u, vec0.k() );
}

TEST( VecIjk1Test, ToString )
{
    VecIjk1 vec1( 10, 20, 30 );
    EXPECT_EQ( "10, 20, 30", vec1.toString() );
}

TEST( VecIjkConversionTest, RoundTrip0to1to0 )
{
    VecIjk0 original( 5, 10, 15 );
    VecIjk1 asOne      = original.toOneBased();
    VecIjk0 backToZero = asOne.toZeroBased();

    EXPECT_EQ( original.i(), backToZero.i() );
    EXPECT_EQ( original.j(), backToZero.j() );
    EXPECT_EQ( original.k(), backToZero.k() );
}

TEST( VecIjkConversionTest, RoundTrip1to0to1 )
{
    VecIjk1 original( 5, 10, 15 );
    VecIjk0 asZero    = original.toZeroBased();
    VecIjk1 backToOne = asZero.toOneBased();

    EXPECT_EQ( original.i(), backToOne.i() );
    EXPECT_EQ( original.j(), backToOne.j() );
    EXPECT_EQ( original.k(), backToOne.k() );
}

TEST( VecIjkConversionTest, ZeroBasedConversion )
{
    VecIjk0 vec0( 0, 0, 0 );
    VecIjk1 vec1 = vec0.toOneBased();

    EXPECT_EQ( 1u, vec1.i() );
    EXPECT_EQ( 1u, vec1.j() );
    EXPECT_EQ( 1u, vec1.k() );
}

TEST( VecIjkConversionTest, LargeValues )
{
    size_t  large = 1000000;
    VecIjk0 vec0( large, large + 1, large + 2 );
    VecIjk1 vec1 = vec0.toOneBased();

    EXPECT_EQ( large + 1, vec1.i() );
    EXPECT_EQ( large + 2, vec1.j() );
    EXPECT_EQ( large + 3, vec1.k() );

    VecIjk0 backToZero = vec1.toZeroBased();
    EXPECT_EQ( large, backToZero.i() );
    EXPECT_EQ( large + 1, backToZero.j() );
    EXPECT_EQ( large + 2, backToZero.k() );
}

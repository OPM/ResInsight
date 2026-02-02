/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RigPadModel.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"

#include <algorithm>
#include <cmath>
#include <string>

//--------------------------------------------------------------------------------------------------
/// Test extendPropertyArray with only upper padding
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, ExtendPropertyArray_UpperOnly )
{
    // Create a 2x2x2 grid (8 cells)
    int                 nx = 2, ny = 2, nz = 2;
    std::vector<double> original( nx * ny * nz, 0.25 ); // PORO = 0.25

    // Add 2 upper padding layers
    int    nzUpper      = 2;
    int    nzLower      = 0;
    double upperDefault = 0.1;
    double lowerDefault = 0.0;

    auto result = RigPadModel::extendPropertyArray( original, nx, ny, nz, nzUpper, nzLower, upperDefault, lowerDefault );

    // Expected size: nx * ny * (nz + nzUpper + nzLower) = 2 * 2 * 4 = 16
    EXPECT_EQ( 16u, result.size() );

    // First 8 values (2 upper layers * 4 cells) should be upperDefault
    for ( int i = 0; i < 8; i++ )
    {
        EXPECT_DOUBLE_EQ( upperDefault, result[i] ) << "Upper padding cell " << i << " should be " << upperDefault;
    }

    // Next 8 values should be original data
    for ( int i = 8; i < 16; i++ )
    {
        EXPECT_DOUBLE_EQ( 0.25, result[i] ) << "Original cell " << ( i - 8 ) << " should be 0.25";
    }
}

//--------------------------------------------------------------------------------------------------
/// Test extendPropertyArray with only lower padding
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, ExtendPropertyArray_LowerOnly )
{
    // Create a 2x2x2 grid (8 cells)
    int                 nx = 2, ny = 2, nz = 2;
    std::vector<double> original( nx * ny * nz, 0.25 ); // PORO = 0.25

    // Add 3 lower padding layers
    int    nzUpper      = 0;
    int    nzLower      = 3;
    double upperDefault = 0.1;
    double lowerDefault = 0.0;

    auto result = RigPadModel::extendPropertyArray( original, nx, ny, nz, nzUpper, nzLower, upperDefault, lowerDefault );

    // Expected size: nx * ny * (nz + nzUpper + nzLower) = 2 * 2 * 5 = 20
    EXPECT_EQ( 20u, result.size() );

    // First 8 values should be original data
    for ( int i = 0; i < 8; i++ )
    {
        EXPECT_DOUBLE_EQ( 0.25, result[i] ) << "Original cell " << i << " should be 0.25";
    }

    // Last 12 values (3 lower layers * 4 cells) should be lowerDefault
    for ( int i = 8; i < 20; i++ )
    {
        EXPECT_DOUBLE_EQ( lowerDefault, result[i] ) << "Lower padding cell " << ( i - 8 ) << " should be " << lowerDefault;
    }
}

//--------------------------------------------------------------------------------------------------
/// Test extendPropertyArray with both upper and lower padding
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, ExtendPropertyArray_BothPadding )
{
    // Create a 3x3x2 grid (18 cells)
    int                 nx = 3, ny = 3, nz = 2;
    std::vector<double> original( nx * ny * nz );
    for ( int i = 0; i < nx * ny * nz; i++ )
    {
        original[i] = 0.3; // PORO = 0.3
    }

    // Add 1 upper and 2 lower padding layers
    int    nzUpper      = 1;
    int    nzLower      = 2;
    double upperDefault = 0.05;
    double lowerDefault = 0.02;

    auto result = RigPadModel::extendPropertyArray( original, nx, ny, nz, nzUpper, nzLower, upperDefault, lowerDefault );

    // Expected size: 3 * 3 * (2 + 1 + 2) = 45
    EXPECT_EQ( 45u, result.size() );

    // First 9 values (1 upper layer * 9 cells) should be upperDefault
    for ( int i = 0; i < 9; i++ )
    {
        EXPECT_DOUBLE_EQ( upperDefault, result[i] );
    }

    // Middle 18 values should be original data
    for ( int i = 9; i < 27; i++ )
    {
        EXPECT_DOUBLE_EQ( 0.3, result[i] );
    }

    // Last 18 values (2 lower layers * 9 cells) should be lowerDefault
    for ( int i = 27; i < 45; i++ )
    {
        EXPECT_DOUBLE_EQ( lowerDefault, result[i] );
    }
}

//--------------------------------------------------------------------------------------------------
/// Test extendPropertyArray with zero padding (no change)
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, ExtendPropertyArray_NoPadding )
{
    int                 nx = 2, ny = 2, nz = 2;
    std::vector<double> original( nx * ny * nz, 0.5 );

    auto result = RigPadModel::extendPropertyArray( original, nx, ny, nz, 0, 0, 0.0, 0.0 );

    EXPECT_EQ( original.size(), result.size() );
    for ( size_t i = 0; i < original.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( original[i], result[i] );
    }
}

//--------------------------------------------------------------------------------------------------
/// Test extendPropertyArray preserves original data order
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, ExtendPropertyArray_DataOrder )
{
    // Create a 2x2x2 grid with sequential values
    int                 nx = 2, ny = 2, nz = 2;
    std::vector<double> original( nx * ny * nz );
    for ( int i = 0; i < nx * ny * nz; i++ )
    {
        original[i] = static_cast<double>( i + 1 );
    }

    // Add 1 upper and 1 lower padding layer
    auto result = RigPadModel::extendPropertyArray( original, nx, ny, nz, 1, 1, 0.0, 99.0 );

    // Check that original data is preserved in the middle
    int xyCount = nx * ny;
    for ( int i = 0; i < nx * ny * nz; i++ )
    {
        EXPECT_DOUBLE_EQ( original[i], result[xyCount + i] ) << "Original data at index " << i << " should be preserved";
    }
}

//--------------------------------------------------------------------------------------------------
/// Test empty input arrays
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, ExtendPropertyArray_EmptyInput )
{
    std::vector<double> empty;

    auto result = RigPadModel::extendPropertyArray( empty, 2, 2, 0, 2, 2, 0.5, 0.5 );

    // Should have 2*2*(0+2+2) = 16 elements, all with padding values
    EXPECT_EQ( 16u, result.size() );
    for ( const auto& v : result )
    {
        EXPECT_DOUBLE_EQ( 0.5, v );
    }
}

//--------------------------------------------------------------------------------------------------
/// Test large grid dimensions
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, ExtendPropertyArray_LargeGrid )
{
    // Simulate a typical sector model size
    int                 nx = 50, ny = 50, nz = 20;
    std::vector<double> original( nx * ny * nz, 0.2 );

    // Add padding
    int nzUpper = 5;
    int nzLower = 10;

    auto result = RigPadModel::extendPropertyArray( original, nx, ny, nz, nzUpper, nzLower, 0.1, 0.0 );

    size_t expectedSize = static_cast<size_t>( nx ) * ny * ( nz + nzUpper + nzLower );
    EXPECT_EQ( expectedSize, result.size() );

    // Verify structure: upper padding, original, lower padding
    int xyCount = nx * ny;

    // Sample check on upper padding
    EXPECT_DOUBLE_EQ( 0.1, result[0] );
    EXPECT_DOUBLE_EQ( 0.1, result[nzUpper * xyCount - 1] );

    // Sample check on original
    EXPECT_DOUBLE_EQ( 0.2, result[nzUpper * xyCount] );
    EXPECT_DOUBLE_EQ( 0.2, result[( nzUpper + nz ) * xyCount - 1] );

    // Sample check on lower padding
    EXPECT_DOUBLE_EQ( 0.0, result[( nzUpper + nz ) * xyCount] );
    EXPECT_DOUBLE_EQ( 0.0, result[result.size() - 1] );
}

//--------------------------------------------------------------------------------------------------
/// Test getPropsDefaultValue returns physically correct defaults from saturation tables.
/// A minimal deck with SWOF/SGOF tables is parsed, and the function should return
/// non-zero values for saturation endpoint keywords (not the old hardcoded 0.0).
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, GetPropsDefaultValue_FromSaturationTables )
{
    // Minimal deck with OIL/WATER/GAS phases and SWOF+SGOF tables
    std::string deckString = R"(
RUNSPEC
OIL
WATER
GAS
TABDIMS
  1 /
EQLDIMS
  1 /
DIMENS
  2 2 1 /

GRID
DX
  4*100 /
DY
  4*100 /
DZ
  4*10 /
TOPS
  4*2000 /

PROPS
SWOF
  0.2  0.0  1.0  0.0
  0.5  0.3  0.5  0.0
  1.0  1.0  0.0  0.0
/
SGOF
  0.0  0.0  1.0  0.0
  0.3  0.2  0.5  0.0
  0.8  1.0  0.0  0.0
/

SOLUTION
EQUIL
  2000 200 2200 0 1800 0 /

SCHEDULE
)";

    auto deck = Opm::Parser{}.parseString( deckString );

    // SWATINIT should default to 1.0 (padding cells fully water-saturated)
    EXPECT_DOUBLE_EQ( 1.0, RigPadModel::getPropsDefaultValue( deck, "SWATINIT" ) );

    // SWU (max water saturation) should be 1.0 from the SWOF table (last Sw entry)
    EXPECT_DOUBLE_EQ( 1.0, RigPadModel::getPropsDefaultValue( deck, "SWU" ) );

    // SWL (connate water) should be 0.2 from the SWOF table (first Sw entry)
    EXPECT_DOUBLE_EQ( 0.2, RigPadModel::getPropsDefaultValue( deck, "SWL" ) );

    // SGU (max gas saturation) should be 0.8 from the SGOF table (last Sg entry)
    EXPECT_DOUBLE_EQ( 0.8, RigPadModel::getPropsDefaultValue( deck, "SGU" ) );

    // KRW (max water relperm) should be 1.0 from the SWOF table
    EXPECT_DOUBLE_EQ( 1.0, RigPadModel::getPropsDefaultValue( deck, "KRW" ) );

    // KRG (max gas relperm) should be 1.0 from the SGOF table
    EXPECT_DOUBLE_EQ( 1.0, RigPadModel::getPropsDefaultValue( deck, "KRG" ) );

    // KRO (max oil relperm) should be 1.0 from the SWOF table
    EXPECT_DOUBLE_EQ( 1.0, RigPadModel::getPropsDefaultValue( deck, "KRO" ) );

    // Directional variant (SWL with X suffix) should map to same value as SWL
    EXPECT_DOUBLE_EQ( 0.2, RigPadModel::getPropsDefaultValue( deck, "SWLX" ) );

    // Imbibition variant (ISWL) should map to SWL
    EXPECT_DOUBLE_EQ( 0.2, RigPadModel::getPropsDefaultValue( deck, "ISWL" ) );

    // Unknown keyword should return 0.0
    EXPECT_DOUBLE_EQ( 0.0, RigPadModel::getPropsDefaultValue( deck, "UNKNOWN_KEYWORD" ) );
}

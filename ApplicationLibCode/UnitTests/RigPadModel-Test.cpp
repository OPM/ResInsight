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

#include <algorithm>
#include <cmath>

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
/// Test extendIntPropertyArray
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, ExtendIntPropertyArray )
{
    // Create a 2x2x3 grid (12 cells)
    int              nx = 2, ny = 2, nz = 3;
    std::vector<int> original( nx * ny * nz, 1 ); // SATNUM = 1

    // Add 2 upper and 1 lower padding layers
    int nzUpper      = 2;
    int nzLower      = 1;
    int upperDefault = 2;
    int lowerDefault = 3;

    auto result = RigPadModel::extendIntPropertyArray( original, nx, ny, nz, nzUpper, nzLower, upperDefault, lowerDefault );

    // Expected size: 2 * 2 * (3 + 2 + 1) = 24
    EXPECT_EQ( 24u, result.size() );

    // First 8 values (2 upper layers * 4 cells) should be upperDefault
    for ( int i = 0; i < 8; i++ )
    {
        EXPECT_EQ( upperDefault, result[i] );
    }

    // Middle 12 values should be original data
    for ( int i = 8; i < 20; i++ )
    {
        EXPECT_EQ( 1, result[i] );
    }

    // Last 4 values (1 lower layer * 4 cells) should be lowerDefault
    for ( int i = 20; i < 24; i++ )
    {
        EXPECT_EQ( lowerDefault, result[i] );
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
/// Test makeVerticalPillars
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, MakeVerticalPillars )
{
    // Create COORD for a 2x2 grid (3x3 pillars)
    int nx = 2, ny = 2;
    // Each pillar has 6 values: x_top, y_top, z_top, x_bot, y_bot, z_bot
    std::vector<double> coord( ( nx + 1 ) * ( ny + 1 ) * 6 );

    // Set up non-vertical pillars
    for ( int j = 0; j <= ny; j++ )
    {
        for ( int i = 0; i <= nx; i++ )
        {
            int idx        = ( j * ( nx + 1 ) + i ) * 6;
            coord[idx]     = i * 100.0; // x_top
            coord[idx + 1] = j * 100.0; // y_top
            coord[idx + 2] = 1000.0; // z_top
            coord[idx + 3] = i * 100.0 + 5.0; // x_bot (offset by 5)
            coord[idx + 4] = j * 100.0 + 3.0; // y_bot (offset by 3)
            coord[idx + 5] = 2000.0; // z_bot
        }
    }

    RigPadModel::makeVerticalPillars( coord, nx, ny );

    // Check that all pillars are now vertical
    for ( int j = 0; j <= ny; j++ )
    {
        for ( int i = 0; i <= nx; i++ )
        {
            int idx = ( j * ( nx + 1 ) + i ) * 6;
            EXPECT_DOUBLE_EQ( coord[idx], coord[idx + 3] ) << "x_bot should equal x_top for pillar (" << i << "," << j << ")";
            EXPECT_DOUBLE_EQ( coord[idx + 1], coord[idx + 4] ) << "y_bot should equal y_top for pillar (" << i << "," << j << ")";
            // Z values should be unchanged
            EXPECT_DOUBLE_EQ( 1000.0, coord[idx + 2] );
            EXPECT_DOUBLE_EQ( 2000.0, coord[idx + 5] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test enforceMonotonicZcorn
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, EnforceMonotonicZcorn )
{
    // Create a simple 1x1x2 grid
    int nx = 1, ny = 1, nz = 2;
    // ZCORN has 2*nx * 2*ny values per half-layer = 4 values
    // Total: 2*nz layers = 4 layers, so 16 values
    std::vector<double> zcorn( nx * ny * nz * 8 );

    // Set up non-monotonic Z values for corner column 0
    // Layer 0 top: 1000, Layer 0 bottom: 1100, Layer 1 top: 1050 (wrong!), Layer 1 bottom: 1200
    int xyCorners = 2 * nx * 2 * ny; // = 4

    // Layer 0 top corners (all same for simplicity)
    for ( int i = 0; i < xyCorners; i++ )
        zcorn[i] = 1000.0;
    // Layer 0 bottom corners
    for ( int i = 0; i < xyCorners; i++ )
        zcorn[xyCorners + i] = 1100.0;
    // Layer 1 top corners (non-monotonic - less than layer 0 bottom)
    for ( int i = 0; i < xyCorners; i++ )
        zcorn[2 * xyCorners + i] = 1050.0;
    // Layer 1 bottom corners
    for ( int i = 0; i < xyCorners; i++ )
        zcorn[3 * xyCorners + i] = 1200.0;

    double minDist = 0.1;
    RigPadModel::enforceMonotonicZcorn( zcorn, nx, ny, nz, minDist );

    // Check that all Z values are monotonically increasing
    for ( int corner = 0; corner < xyCorners; corner++ )
    {
        double prevZ = zcorn[corner];
        for ( int layer = 1; layer < 2 * nz; layer++ )
        {
            double currZ = zcorn[layer * xyCorners + corner];
            EXPECT_GE( currZ, prevZ + minDist - 1e-10 ) << "Z value at layer " << layer << " should be >= " << ( prevZ + minDist );
            prevZ = currZ;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test fillZcornGaps
//--------------------------------------------------------------------------------------------------
TEST( RigPadModel, FillZcornGaps )
{
    // Create a simple 1x1x2 grid
    int nx = 1, ny = 1, nz = 2;
    // ZCORN: 4 corners per half-layer, 4 half-layers = 16 values
    std::vector<double> zcorn( nx * ny * nz * 8 );

    int xyCorners = 2 * nx * 2 * ny; // = 4

    // Layer 0 top: 1000, Layer 0 bottom: 1100
    // Layer 1 top: 1150 (gap of 50), Layer 1 bottom: 1200
    for ( int i = 0; i < xyCorners; i++ )
    {
        zcorn[0 * xyCorners + i] = 1000.0;
        zcorn[1 * xyCorners + i] = 1100.0;
        zcorn[2 * xyCorners + i] = 1150.0;
        zcorn[3 * xyCorners + i] = 1200.0;
    }

    RigPadModel::fillZcornGaps( zcorn, nx, ny, nz );

    // After filling, layer 0 bottom and layer 1 top should be averaged
    double expectedAvg = ( 1100.0 + 1150.0 ) / 2.0;
    for ( int i = 0; i < xyCorners; i++ )
    {
        EXPECT_DOUBLE_EQ( expectedAvg, zcorn[1 * xyCorners + i] ) << "Layer 0 bottom should be averaged";
        EXPECT_DOUBLE_EQ( expectedAvg, zcorn[2 * xyCorners + i] ) << "Layer 1 top should be averaged";
    }

    // Layer 0 top and layer 1 bottom should be unchanged
    for ( int i = 0; i < xyCorners; i++ )
    {
        EXPECT_DOUBLE_EQ( 1000.0, zcorn[0 * xyCorners + i] );
        EXPECT_DOUBLE_EQ( 1200.0, zcorn[3 * xyCorners + i] );
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

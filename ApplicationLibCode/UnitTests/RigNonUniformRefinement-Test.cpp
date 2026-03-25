/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RigNonUniformRefinement.h"

#include "RiaCellDividingTools.h"
#include "RigGridExportAdapter.h"

#include "cafVecIjk.h"

//--------------------------------------------------------------------------------------------------
/// Test fromUniform() matches expected uniform counts
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, FromUniform )
{
    auto nuRef = RigNonUniformRefinement::fromUniform( cvf::Vec3st( 3, 2, 1 ), cvf::Vec3st( 4, 5, 6 ) );

    // Total refined count should match uniform: sectorSize * refinement
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 4u * 3 );
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimJ ), 5u * 2 );
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimK ), 6u * 1 );

    // Each cell in dim 0 should have 3 subcells
    for ( size_t i = 0; i < 4; ++i )
    {
        EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, i ), 3u );
    }

    // Each cell in dim 1 should have 2 subcells
    for ( size_t j = 0; j < 5; ++j )
    {
        EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimJ, j ), 2u );
    }

    // Each cell in dim 2 should have 1 subcell
    for ( size_t k = 0; k < 6; ++k )
    {
        EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimK, k ), 1u );
    }

    EXPECT_TRUE( nuRef.hasNonUniformRefinement() );
}

//--------------------------------------------------------------------------------------------------
/// Test no refinement case
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, NoRefinement )
{
    auto nuRef = RigNonUniformRefinement::fromUniform( cvf::Vec3st( 1, 1, 1 ), cvf::Vec3st( 3, 3, 3 ) );

    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 3u );
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimJ ), 3u );
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimK ), 3u );
    EXPECT_FALSE( nuRef.hasNonUniformRefinement() );
}

//--------------------------------------------------------------------------------------------------
/// Test subcellCount and cumulativeOffset for mixed refinement
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, MixedRefinement )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 3, 1, 1 ) );

    // Cell 0: 2 subcells, Cell 1: 5 subcells, Cell 2: 1 subcell (default)
    auto fractions2 = RigNonUniformRefinement::widthsToCumulativeFractions( { 0.5, 0.5 } );
    auto fractions5 = RigNonUniformRefinement::widthsToCumulativeFractions( { 0.2, 0.4, 0.5, 0.6, 0.8 } );

    nuRef.setCumulativeFractions( RigNonUniformRefinement::DimI, 0, fractions2 );
    nuRef.setCumulativeFractions( RigNonUniformRefinement::DimI, 1, fractions5 );

    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 0 ), 2u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 1 ), 5u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 2 ), 1u );

    EXPECT_EQ( nuRef.cumulativeOffset( RigNonUniformRefinement::DimI, 0 ), 0u );
    EXPECT_EQ( nuRef.cumulativeOffset( RigNonUniformRefinement::DimI, 1 ), 2u );
    EXPECT_EQ( nuRef.cumulativeOffset( RigNonUniformRefinement::DimI, 2 ), 7u );

    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 8u ); // 2 + 5 + 1
    EXPECT_TRUE( nuRef.hasNonUniformRefinement() );
}

//--------------------------------------------------------------------------------------------------
/// Test mapRefinedToOriginal round-trips with cumulativeOffset
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, MapRefinedToOriginalRoundTrip )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 4, 1, 1 ) );

    // Cell 0: 3 subcells, Cell 1: 1 subcell, Cell 2: 2 subcells, Cell 3: 4 subcells
    nuRef.setCumulativeFractions( RigNonUniformRefinement::DimI, 0, RigNonUniformRefinement::widthsToCumulativeFractions( { 1.0, 1.0, 1.0 } ) );
    // Cell 1: default (1 subcell)
    nuRef.setCumulativeFractions( RigNonUniformRefinement::DimI, 2, RigNonUniformRefinement::widthsToCumulativeFractions( { 1.0, 1.0 } ) );
    nuRef.setCumulativeFractions( RigNonUniformRefinement::DimI,
                                  3,
                                  RigNonUniformRefinement::widthsToCumulativeFractions( { 1.0, 1.0, 1.0, 1.0 } ) );

    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 10u ); // 3 + 1 + 2 + 4

    // Verify round-trip: for each original cell, cumulativeOffset should map back correctly
    for ( size_t origIdx = 0; origIdx < 4; ++origIdx )
    {
        size_t offset                = nuRef.cumulativeOffset( RigNonUniformRefinement::DimI, origIdx );
        auto [mappedOrig, mappedSub] = nuRef.mapRefinedToOriginal( RigNonUniformRefinement::DimI, offset );
        EXPECT_EQ( mappedOrig, origIdx );
        EXPECT_EQ( mappedSub, 0u );
    }

    // Check specific refined indices
    auto [orig0, sub0] = nuRef.mapRefinedToOriginal( RigNonUniformRefinement::DimI, 0 ); // First subcell of cell 0
    EXPECT_EQ( orig0, 0u );
    EXPECT_EQ( sub0, 0u );

    auto [orig1, sub1] = nuRef.mapRefinedToOriginal( RigNonUniformRefinement::DimI, 2 ); // Last subcell of cell 0
    EXPECT_EQ( orig1, 0u );
    EXPECT_EQ( sub1, 2u );

    auto [orig2, sub2] = nuRef.mapRefinedToOriginal( RigNonUniformRefinement::DimI, 3 ); // Cell 1 (only subcell)
    EXPECT_EQ( orig2, 1u );
    EXPECT_EQ( sub2, 0u );

    auto [orig3, sub3] = nuRef.mapRefinedToOriginal( RigNonUniformRefinement::DimI, 4 ); // First subcell of cell 2
    EXPECT_EQ( orig3, 2u );
    EXPECT_EQ( sub3, 0u );

    auto [orig4, sub4] = nuRef.mapRefinedToOriginal( RigNonUniformRefinement::DimI, 9 ); // Last subcell of cell 3
    EXPECT_EQ( orig4, 3u );
    EXPECT_EQ( sub4, 3u );
}

//--------------------------------------------------------------------------------------------------
/// Test widthsToCumulativeFractions normalization
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, WidthsToCumulativeFractions )
{
    // Equal widths
    auto fractions = RigNonUniformRefinement::widthsToCumulativeFractions( { 1.0, 1.0, 1.0 } );
    EXPECT_EQ( fractions.size(), 3u );
    EXPECT_NEAR( fractions[0], 1.0 / 3.0, 1e-10 );
    EXPECT_NEAR( fractions[1], 2.0 / 3.0, 1e-10 );
    EXPECT_DOUBLE_EQ( fractions[2], 1.0 );

    // Non-equal widths
    auto fractions2 = RigNonUniformRefinement::widthsToCumulativeFractions( { 1.0, 3.0 } );
    EXPECT_EQ( fractions2.size(), 2u );
    EXPECT_NEAR( fractions2[0], 0.25, 1e-10 );
    EXPECT_DOUBLE_EQ( fractions2[1], 1.0 );

    // Empty input
    auto fractions3 = RigNonUniformRefinement::widthsToCumulativeFractions( {} );
    EXPECT_EQ( fractions3.size(), 1u );
    EXPECT_DOUBLE_EQ( fractions3[0], 1.0 );
}

//--------------------------------------------------------------------------------------------------
/// Test single cell edge case
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, SingleCell )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 1, 1, 1 ) );

    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 1u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 0 ), 1u );
    EXPECT_EQ( nuRef.cumulativeOffset( RigNonUniformRefinement::DimI, 0 ), 0u );

    auto [orig, sub] = nuRef.mapRefinedToOriginal( RigNonUniformRefinement::DimI, 0 );
    EXPECT_EQ( orig, 0u );
    EXPECT_EQ( sub, 0u );
}

//--------------------------------------------------------------------------------------------------
/// Test distributeWidthsAcrossCells: 3 equal widths across 3 cells → 1 subcell each
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, DistributeEqualWidthsAcrossEqualCells )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 3, 1, 1 ) );

    // 3 equal widths across 3 cells: each boundary lands on a cell boundary
    // so each cell gets exactly 1 subcell
    nuRef.distributeWidthsAcrossCells( RigNonUniformRefinement::DimI, 0, 2, { 1.0, 1.0, 1.0 } );

    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 0 ), 1u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 1 ), 1u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 2 ), 1u );
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 3u );
}

//--------------------------------------------------------------------------------------------------
/// Test distributeWidthsAcrossCells: 6 equal widths across 3 cells → 2 subcells each
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, DistributeDoubleWidthsAcrossCells )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 3, 1, 1 ) );

    // 6 equal widths across 3 cells: 2 subcell boundaries per cell
    nuRef.distributeWidthsAcrossCells( RigNonUniformRefinement::DimI, 0, 2, { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 } );

    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 0 ), 2u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 1 ), 2u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 2 ), 2u );
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 6u );
}

//--------------------------------------------------------------------------------------------------
/// Test distributeWidthsAcrossCells: uneven widths distributed across cells
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, DistributeUnevenWidthsAcrossCells )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 3, 1, 1 ) );

    // 5 widths [0.1, 0.1, 0.1, 0.1, 0.6] across 3 cells
    // Cumulative fractions: [0.1, 0.2, 0.3, 0.4, 1.0]
    // Cell 0 [0, 0.333]: boundaries 0.1, 0.2, 0.3 fall inside → 4 subcells
    // Cell 1 [0.333, 0.667]: boundary 0.4 falls inside → 2 subcells
    // Cell 2 [0.667, 1.0]: no interior boundaries → 1 subcell
    nuRef.distributeWidthsAcrossCells( RigNonUniformRefinement::DimI, 0, 2, { 0.1, 0.1, 0.1, 0.1, 0.6 } );

    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 0 ), 4u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 1 ), 2u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 2 ), 1u );
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 7u );
}

//--------------------------------------------------------------------------------------------------
/// Test distributeWidthsAcrossCells: single cell range
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, DistributeWidthsSingleCell )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 5, 1, 1 ) );

    // Apply 3 widths to just cell 2
    nuRef.distributeWidthsAcrossCells( RigNonUniformRefinement::DimI, 2, 2, { 0.2, 0.3, 0.5 } );

    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 0 ), 1u ); // Unaffected
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 1 ), 1u ); // Unaffected
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 2 ), 3u ); // 3 subcells
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 3 ), 1u ); // Unaffected
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 4 ), 1u ); // Unaffected
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 7u ); // 1+1+3+1+1
}

//--------------------------------------------------------------------------------------------------
/// Test distributeWidthsAcrossCells: many widths concentrate in first cell
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, DistributeConcentratedWidths )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 2, 1, 1 ) );

    // 4 widths [0.1, 0.1, 0.1, 0.7] across 2 cells
    // Cumulative fractions: [0.1, 0.2, 0.3, 1.0]
    // Cell 0 [0, 0.5]: boundaries 0.1, 0.2, 0.3 fall inside → 4 subcells
    // Cell 1 [0.5, 1.0]: no interior boundaries → 1 subcell
    nuRef.distributeWidthsAcrossCells( RigNonUniformRefinement::DimI, 0, 1, { 0.1, 0.1, 0.1, 0.7 } );

    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 0 ), 4u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 1 ), 1u );
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 5u );
}

//--------------------------------------------------------------------------------------------------
/// Test generateLogarithmicWidths: even count, verify symmetry and center < edge
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, GenerateLogarithmicWidthsSymmetry )
{
    auto widths = RigNonUniformRefinement::generateLogarithmicWidths( 6 );
    ASSERT_EQ( widths.size(), 6u );

    // Verify symmetry: widths[i] == widths[n-1-i]
    for ( size_t i = 0; i < widths.size() / 2; ++i )
    {
        EXPECT_DOUBLE_EQ( widths[i], widths[widths.size() - 1 - i] );
    }

    // Center cells should be smaller than edge cells
    EXPECT_LT( widths[2], widths[0] );
    EXPECT_LT( widths[3], widths[0] );
}

//--------------------------------------------------------------------------------------------------
/// Test generateLogarithmicWidths: odd count, verify symmetry around center
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, GenerateLogarithmicWidthsOdd )
{
    auto widths = RigNonUniformRefinement::generateLogarithmicWidths( 7 );
    ASSERT_EQ( widths.size(), 7u );

    // Verify symmetry: widths[i] == widths[n-1-i]
    for ( size_t i = 0; i < widths.size() / 2; ++i )
    {
        EXPECT_DOUBLE_EQ( widths[i], widths[widths.size() - 1 - i] );
    }

    // Center cell (index 3) should be the smallest
    for ( size_t i = 0; i < widths.size(); ++i )
    {
        if ( i != 3 )
        {
            EXPECT_LE( widths[3], widths[i] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Test generateLogarithmicWidths: single cell edge case
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, GenerateLogarithmicWidthsSingleCell )
{
    auto widths = RigNonUniformRefinement::generateLogarithmicWidths( 1 );
    ASSERT_EQ( widths.size(), 1u );
    EXPECT_DOUBLE_EQ( widths[0], 1.0 );
}

//--------------------------------------------------------------------------------------------------
/// Test generateEqualFractions: produces correct cumulative fractions
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, GenerateEqualFractions )
{
    auto fracs1 = RigNonUniformRefinement::generateEqualFractions( 1 );
    ASSERT_EQ( fracs1.size(), 1u );
    EXPECT_DOUBLE_EQ( fracs1[0], 1.0 );

    auto fracs4 = RigNonUniformRefinement::generateEqualFractions( 4 );
    ASSERT_EQ( fracs4.size(), 4u );
    EXPECT_NEAR( fracs4[0], 0.25, 1e-10 );
    EXPECT_NEAR( fracs4[1], 0.50, 1e-10 );
    EXPECT_NEAR( fracs4[2], 0.75, 1e-10 );
    EXPECT_DOUBLE_EQ( fracs4[3], 1.0 );

    // Edge case: 0 returns {1.0}
    auto fracs0 = RigNonUniformRefinement::generateEqualFractions( 0 );
    ASSERT_EQ( fracs0.size(), 1u );
    EXPECT_DOUBLE_EQ( fracs0[0], 1.0 );
}

//--------------------------------------------------------------------------------------------------
/// Test linear equal split: each cell gets exactly N equal subcells
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, LinearEqualSplitPattern )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 5, 1, 1 ) );

    // Apply N=3 equal subcells to cells 1-3 using generateEqualFractions
    auto equalFractions = RigNonUniformRefinement::generateEqualFractions( 3 );

    for ( size_t c = 1; c <= 3; ++c )
    {
        nuRef.setCumulativeFractions( RigNonUniformRefinement::DimI, c, equalFractions );
    }

    // Cells outside range should be unaffected
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 0 ), 1u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 4 ), 1u );

    // Cells in range should each have exactly 3 subcells
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 1 ), 3u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 2 ), 3u );
    EXPECT_EQ( nuRef.subcellCount( RigNonUniformRefinement::DimI, 3 ), 3u );

    // Total: 1 + 3 + 3 + 3 + 1 = 11
    EXPECT_EQ( nuRef.totalRefinedCount( RigNonUniformRefinement::DimI ), 11u );
}

//--------------------------------------------------------------------------------------------------
/// Test logarithmic distribution through distributeWidthsAcrossCells
//--------------------------------------------------------------------------------------------------
TEST( RigNonUniformRefinement, LogarithmicDistribution )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 3, 1, 1 ) );

    auto widths = RigNonUniformRefinement::generateLogarithmicWidths( 10 );
    ASSERT_EQ( widths.size(), 10u );

    nuRef.distributeWidthsAcrossCells( RigNonUniformRefinement::DimI, 0, 2, widths );

    // Total refined count should be >= number of widths (each width creates at least a boundary)
    size_t totalRefined = nuRef.totalRefinedCount( RigNonUniformRefinement::DimI );
    EXPECT_GE( totalRefined, 3u ); // At minimum 1 per cell
    EXPECT_TRUE( nuRef.hasNonUniformRefinement() );
}

//--------------------------------------------------------------------------------------------------
/// Test non-uniform geometry: equal fractions should match uniform createHexCornerCoords
//--------------------------------------------------------------------------------------------------
TEST( RiaCellDividingTools, NonUniformEqualFractionsMatchUniform )
{
    std::array<cvf::Vec3d, 8> corners = { cvf::Vec3d( 0, 0, 0 ),
                                          cvf::Vec3d( 1, 0, 0 ),
                                          cvf::Vec3d( 1, 1, 0 ),
                                          cvf::Vec3d( 0, 1, 0 ),
                                          cvf::Vec3d( 0, 0, 1 ),
                                          cvf::Vec3d( 1, 0, 1 ),
                                          cvf::Vec3d( 1, 1, 1 ),
                                          cvf::Vec3d( 0, 1, 1 ) };

    size_t nx = 3, ny = 2, nz = 2;

    // Uniform result
    auto uniformResult = RiaCellDividingTools::createHexCornerCoords( corners, nx, ny, nz );

    // Non-uniform with equal fractions
    std::vector<double> fracX = { 1.0 / 3, 2.0 / 3, 1.0 };
    std::vector<double> fracY = { 0.5, 1.0 };
    std::vector<double> fracZ = { 0.5, 1.0 };

    auto nonUniformResult = RiaCellDividingTools::createHexCornerCoords( corners, fracX, fracY, fracZ );

    ASSERT_EQ( uniformResult.size(), nonUniformResult.size() );

    for ( size_t i = 0; i < uniformResult.size(); ++i )
    {
        EXPECT_NEAR( uniformResult[i].x(), nonUniformResult[i].x(), 1e-10 ) << "Mismatch at index " << i << " (x)";
        EXPECT_NEAR( uniformResult[i].y(), nonUniformResult[i].y(), 1e-10 ) << "Mismatch at index " << i << " (y)";
        EXPECT_NEAR( uniformResult[i].z(), nonUniformResult[i].z(), 1e-10 ) << "Mismatch at index " << i << " (z)";
    }
}

//--------------------------------------------------------------------------------------------------
/// Test non-uniform fractions produce geometrically correct subcells
//--------------------------------------------------------------------------------------------------
TEST( RiaCellDividingTools, NonUniformFractionsGeometry )
{
    std::array<cvf::Vec3d, 8> corners = { cvf::Vec3d( 0, 0, 0 ),
                                          cvf::Vec3d( 10, 0, 0 ),
                                          cvf::Vec3d( 10, 10, 0 ),
                                          cvf::Vec3d( 0, 10, 0 ),
                                          cvf::Vec3d( 0, 0, 10 ),
                                          cvf::Vec3d( 10, 0, 10 ),
                                          cvf::Vec3d( 10, 10, 10 ),
                                          cvf::Vec3d( 0, 10, 10 ) };

    // Non-uniform X: 20% and 80%
    std::vector<double> fracX = { 0.2, 1.0 };
    std::vector<double> fracY = { 1.0 }; // No refinement
    std::vector<double> fracZ = { 1.0 }; // No refinement

    auto result = RiaCellDividingTools::createHexCornerCoords( corners, fracX, fracY, fracZ );

    // Should produce 2*1*1 = 2 subcells, each with 8 corners
    ASSERT_EQ( result.size(), 16u );

    // First subcell: x range [0, 2]
    EXPECT_NEAR( result[0].x(), 0.0, 1e-10 ); // corner 0 x
    EXPECT_NEAR( result[1].x(), 2.0, 1e-10 ); // corner 1 x

    // Second subcell: x range [2, 10]
    EXPECT_NEAR( result[8].x(), 2.0, 1e-10 ); // corner 0 x
    EXPECT_NEAR( result[9].x(), 10.0, 1e-10 ); // corner 1 x
}

//--------------------------------------------------------------------------------------------------
/// Test transformIjkToSectorCoordinates with non-uniform refinement
//--------------------------------------------------------------------------------------------------
TEST( RigGridExportAdapter, TransformIjkNonUniform )
{
    RigNonUniformRefinement nuRef( cvf::Vec3st( 3, 2, 1 ) );

    // I: cell 0 -> 2 subcells, cell 1 -> 3 subcells, cell 2 -> 1 subcell
    nuRef.setCumulativeFractions( RigNonUniformRefinement::DimI, 0, RigNonUniformRefinement::widthsToCumulativeFractions( { 1.0, 1.0 } ) );
    nuRef.setCumulativeFractions( RigNonUniformRefinement::DimI, 1, RigNonUniformRefinement::widthsToCumulativeFractions( { 1.0, 1.0, 1.0 } ) );
    // Cell 2: default (1 subcell)

    // J: cell 0 -> 1 subcell, cell 1 -> 2 subcells
    nuRef.setCumulativeFractions( RigNonUniformRefinement::DimJ, 1, RigNonUniformRefinement::widthsToCumulativeFractions( { 1.0, 1.0 } ) );

    caf::VecIjk0 sectorMin( 10, 20, 30 );
    caf::VecIjk0 sectorMax( 12, 21, 30 ); // 3 cells I, 2 cells J, 1 cell K

    // Test min coordinate (default): cell (10, 20, 30) -> offset(0), offset(0), offset(0) = (0, 0, 0)
    auto result = RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( 10, 20, 30 ), sectorMin, sectorMax, nuRef );
    ASSERT_TRUE( result.has_value() );
    EXPECT_EQ( result->x(), 0u );
    EXPECT_EQ( result->y(), 0u );
    EXPECT_EQ( result->z(), 0u );

    // Test cell (11, 20, 30) -> offset(1) = 2, offset(0) = 0, offset(0) = 0
    auto result2 = RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( 11, 20, 30 ), sectorMin, sectorMax, nuRef );
    ASSERT_TRUE( result2.has_value() );
    EXPECT_EQ( result2->x(), 2u );
    EXPECT_EQ( result2->y(), 0u );

    // Test max coordinate: cell (12, 21, 30) -> offset(3)-1 = 5, offset(2)-1 = 2, offset(1)-1 = 0
    auto result3 =
        RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( 12, 21, 30 ), sectorMin, sectorMax, nuRef, false, true );
    ASSERT_TRUE( result3.has_value() );
    EXPECT_EQ( result3->x(), 5u ); // 2 + 3 + 1 - 1 = 5
    EXPECT_EQ( result3->y(), 2u ); // 1 + 2 - 1 = 2
    EXPECT_EQ( result3->z(), 0u );

    // Test centering: cell (11, 20, 30) -> offset(1) + subcellCount(1)/2 = 2 + 1 = 3
    auto result4 = RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( 11, 20, 30 ), sectorMin, sectorMax, nuRef, true );
    ASSERT_TRUE( result4.has_value() );
    EXPECT_EQ( result4->x(), 3u ); // offset(1)=2, subcellCount(1)=3, center=2+3/2=3
    EXPECT_EQ( result4->y(), 0u ); // offset(0)=0, subcellCount(0)=1, center=0+1/2=0

    // Test out of bounds
    auto result5 = RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( 9, 20, 30 ), sectorMin, sectorMax, nuRef );
    EXPECT_FALSE( result5.has_value() );
}

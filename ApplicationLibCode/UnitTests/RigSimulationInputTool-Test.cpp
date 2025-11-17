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

#include "RigSimulationInputTool.h"

#include "RifOpmDeckTools.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"

#include <QString>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Helper to create an EQUALS DeckRecord
/// EQUALS format: FIELD VALUE I1 I2 J1 J2 K1 K2 (1-based Eclipse coordinates)
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createEqualsRecord( const std::string& fieldName, int value, int i1, int i2, int j1, int j2, int k1, int k2 )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", fieldName ) );
    items.push_back( RifOpmDeckTools::item( "VALUE", value ) );
    items.push_back( RifOpmDeckTools::item( "I1", i1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", i2 ) );
    items.push_back( RifOpmDeckTools::item( "J1", j1 ) );
    items.push_back( RifOpmDeckTools::item( "J2", j2 ) );
    items.push_back( RifOpmDeckTools::item( "K1", k1 ) );
    items.push_back( RifOpmDeckTools::item( "K2", k2 ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record with no overlap - should return error
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_NoOverlap )
{
    // Sector: sectorMin=(0, 15, 0), sectorMax=(19, 29, 9) (inclusive, cells [0-19, 15-29, 0-9])
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record: FIPNUM 1 1 20 1 14 1 10 (1-based Eclipse)
    // Converts to 0-based: I[0-19], J[0-13], K[0-9]
    // Sector J is [15-29], so no overlap in J dimension
    auto record = createEqualsRecord( "FIPNUM", 1, 1, 20, 1, 14, 1, 10 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record with partial overlap requiring clamping
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_PartialOverlapWithClamping )
{
    // Sector: sectorMin=(0, 15, 0), sectorMax=(19, 29, 9) (inclusive, cells [0-19, 15-29, 0-9])
    caf::VecIjk0 sectorMin( 0, 15, 0 );
    caf::VecIjk0 sectorMax( 19, 29, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record: FIPNUM 2 1 20 15 30 1 10 (1-based Eclipse)
    // Converts to 0-based: I[0-19], J[14-29], K[0-9]
    // Sector: I[0-19], J[15-29], K[0-9]
    // Intersection (clamped): I[0-19], J[15-29], K[0-9]
    auto record = createEqualsRecord( "FIPNUM", 2, 1, 20, 15, 30, 1, 10 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Check transformed coordinates (sector-relative, 1-based)
    // After clamping: I[0-19], J[15-29], K[0-9] (0-based)
    // Transformed to sector-relative (min at 0,15,0):
    //   I: (0-0)*1+1 = 1 to (19-0)*1+1 = 20
    //   J: (15-15)*1+1 = 1 to (29-15)*1+1 = 15
    //   K: (0-0)*1+1 = 1 to (9-0)*1+1 = 10
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 20, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 7 ).get<int>( 0 ) ); // K2

    // Check field name and value are preserved
    EXPECT_EQ( "FIPNUM", result->getItem( 0 ).get<std::string>( 0 ) );
    EXPECT_EQ( 2, result->getItem( 1 ).get<int>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record completely inside sector (no clamping)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_CompletelyInside )
{
    // Sector: min=(0, 0, 0), max=(19, 19, 9) (inclusive, cells [0-19, 0-19, 0-9])
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 19, 19, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record completely inside: FIPNUM 3 5 15 5 15 2 8 (1-based Eclipse)
    // Converts to 0-based: I[4-14], J[4-14], K[1-7]
    auto record = createEqualsRecord( "FIPNUM", 3, 5, 15, 5, 15, 2, 8 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // No clamping, direct transformation (sector min is 0,0,0)
    // I: (4-0)*1+1 = 5 to (14-0)*1+1 = 15
    // J: (4-0)*1+1 = 5 to (14-0)*1+1 = 15
    // K: (1-0)*1+1 = 2 to (7-0)*1+1 = 8
    EXPECT_EQ( 5, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 15, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 5, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 15, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 2, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 8, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record completely outside sector
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_CompletelyOutside )
{
    // Sector: min=(0, 0, 0), max=(9, 9, 9) (inclusive, cells [0-9, 0-9, 0-9])
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 9, 9, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record outside: FIPNUM 4 20 30 20 30 1 10 (1-based Eclipse)
    // Converts to 0-based: I[19-29], J[19-29], K[0-9]
    // No overlap with sector
    auto record = createEqualsRecord( "FIPNUM", 4, 20, 30, 20, 30, 1, 10 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test invalid EQUALS record (insufficient items)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_InvalidRecord )
{
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 9, 9, 9 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // Create a record with only 5 items (insufficient)
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "FIELD", std::string( "FIPNUM" ) ) );
    items.push_back( RifOpmDeckTools::item( "VALUE", 1 ) );
    items.push_back( RifOpmDeckTools::item( "I1", 1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", 10 ) );
    items.push_back( RifOpmDeckTools::item( "J1", 1 ) );
    // Missing J2, K1, K2

    Opm::DeckRecord record{ std::move( items ) };

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "insufficient items" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test EQUALS record at sector boundary
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputTool, ProcessEqualsRecord_AtBoundary )
{
    // Sector: min=(5, 5, 5), max=(14, 14, 14) (inclusive, cells [5-14, 5-14, 5-14])
    caf::VecIjk0 sectorMin( 5, 5, 5 );
    caf::VecIjk0 sectorMax( 14, 14, 14 );
    cvf::Vec3st  refinement( 1, 1, 1 );

    // EQUALS record exactly matching sector: 6 15 6 15 6 15 (1-based Eclipse)
    // Converts to 0-based: I[5-14], J[5-14], K[5-14]
    auto record = createEqualsRecord( "FIPNUM", 5, 6, 15, 6, 15, 6, 15 );

    auto result = RigSimulationInputTool::processEqualsRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Should span entire sector in all dimensions
    // Transformed: (5-5)*1+1 = 1 to (14-5)*1+1 = 10
    EXPECT_EQ( 1, result->getItem( 2 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 10, result->getItem( 3 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 4 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 10, result->getItem( 5 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 1, result->getItem( 6 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 10, result->getItem( 7 ).get<int>( 0 ) ); // K2
}

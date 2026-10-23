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

#include "RigSimulationInputTestHelpers.h"

#include "RifOpmDeckTools.h"

#include "RigSimulationInputAquiferTools.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"

#include <string>
#include <vector>

using RigSimulationInputTestHelpers::makeRef;

//--------------------------------------------------------------------------------------------------
/// Helper to create an AQUCON DeckRecord
/// AQUCON format: ID I1 I2 J1 J2 K1 K2 CONNECT_FACE ... (1-based Eclipse coordinates)
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord createAquconRecord( int aquiferId, int i1, int i2, int j1, int j2, int k1, int k2, const std::string& connectFace )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "ID", aquiferId ) );
    items.push_back( RifOpmDeckTools::item( "I1", i1 ) );
    items.push_back( RifOpmDeckTools::item( "I2", i2 ) );
    items.push_back( RifOpmDeckTools::item( "J1", j1 ) );
    items.push_back( RifOpmDeckTools::item( "J2", j2 ) );
    items.push_back( RifOpmDeckTools::item( "K1", k1 ) );
    items.push_back( RifOpmDeckTools::item( "K2", k2 ) );
    items.push_back( RifOpmDeckTools::item( "CONNECT_FACE", connectFace ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Test AQUCON record with partial overlap (based on user-provided data)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputAquiferTools, ProcessAquconRecord_PartialOverlap )
{
    // Sector: min=(40, 27, 21), max=(100, 72, 58) (inclusive, 0-based)
    // This corresponds to a sector with cells [40-100, 27-72, 21-58]
    caf::VecIjk0 sectorMin( 40, 27, 21 );
    caf::VecIjk0 sectorMax( 100, 72, 58 );
    auto         refinement = makeRef( sectorMin, sectorMax, cvf::Vec3st( 1, 1, 1 ) );

    // AQUCON record from user data: 1 57 57 28 36 46 58 'I+' (1-based Eclipse)
    // Converts to 0-based: I[56-56], J[27-35], K[45-57]
    // Sector is I[40-100], J[27-72], K[21-58]
    // Overlap: I[56-56], J[27-35], K[45-57] - all inside, no clamping needed
    auto record = createAquconRecord( 1, 57, 57, 28, 36, 46, 58, "I+" );

    auto result = RigSimulationInputAquiferTools::processAquconRecord( record, sectorMin, sectorMax, refinement );

    ASSERT_TRUE( result.has_value() );

    // Check transformed coordinates (sector-relative, 1-based)
    // I: (56-40)*1+1 = 17 to (56-40)*1+1 = 17
    // J: (27-27)*1+1 = 1 to (35-27)*1+1 = 9
    // K: (45-21)*1+1 = 25 to (57-21)*1+1 = 37
    EXPECT_EQ( 17, result->getItem( 1 ).get<int>( 0 ) ); // I1
    EXPECT_EQ( 17, result->getItem( 2 ).get<int>( 0 ) ); // I2
    EXPECT_EQ( 1, result->getItem( 3 ).get<int>( 0 ) ); // J1
    EXPECT_EQ( 9, result->getItem( 4 ).get<int>( 0 ) ); // J2
    EXPECT_EQ( 25, result->getItem( 5 ).get<int>( 0 ) ); // K1
    EXPECT_EQ( 37, result->getItem( 6 ).get<int>( 0 ) ); // K2

    // Check aquifer ID and connect face are preserved
    EXPECT_EQ( 1, result->getItem( 0 ).get<int>( 0 ) );
    EXPECT_EQ( "I+", result->getItem( 7 ).get<std::string>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Test AQUCON record with partial K overlap requiring clamping
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputAquiferTools, ProcessAquconRecord_PartialOverlapWithClamping )
{
    // Sector: min=(40, 27, 21), max=(100, 72, 58) (inclusive, 0-based)
    caf::VecIjk0 sectorMin( 40, 27, 21 );
    caf::VecIjk0 sectorMax( 100, 72, 58 );
    auto         refinement = makeRef( sectorMin, sectorMax, cvf::Vec3st( 1, 1, 1 ) );

    // AQUCON record: 1 79 79 41 67 5 11 'I+' (1-based Eclipse)
    // Converts to 0-based: I[78-78], J[40-66], K[4-10]
    // Sector K is [21-58], so K[4-10] is outside and should be skipped
    auto record = createAquconRecord( 1, 79, 79, 41, 67, 5, 11, "I+" );

    auto result = RigSimulationInputAquiferTools::processAquconRecord( record, sectorMin, sectorMax, refinement );

    // This should fail because K range [4-10] doesn't overlap with sector K range [21-58]
    EXPECT_FALSE( result.has_value() );
    EXPECT_TRUE( result.error().contains( "does not overlap" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test AQUCON record completely inside sector
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputAquiferTools, ProcessAquconRecord_CompletelyInside )
{
    // Sector: min=(40, 27, 21), max=(100, 72, 58) (inclusive, 0-based)
    caf::VecIjk0 sectorMin( 40, 27, 21 );
    caf::VecIjk0 sectorMax( 100, 72, 58 );
    auto         refinement = makeRef( sectorMin, sectorMax, cvf::Vec3st( 1, 1, 1 ) );

    // AQUCON record: 1 61 61 48 72 12 17 'I+' (1-based Eclipse)
    // Converts to 0-based: I[60-60], J[47-71], K[11-16]
    // However, K[11-16] is outside sector K[21-58], so this should fail
    auto record = createAquconRecord( 1, 61, 61, 48, 72, 12, 17, "I+" );

    auto result = RigSimulationInputAquiferTools::processAquconRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_FALSE( result.has_value() );
}

//--------------------------------------------------------------------------------------------------
/// Test AQUCON record without box definition (only aquifer ID)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputAquiferTools, ProcessAquconRecord_NoBoxDefinition )
{
    caf::VecIjk0 sectorMin( 0, 0, 0 );
    caf::VecIjk0 sectorMax( 9, 9, 9 );
    auto         refinement = makeRef( sectorMin, sectorMax, cvf::Vec3st( 1, 1, 1 ) );

    // Create a record with only aquifer ID (no box coordinates)
    std::vector<Opm::DeckItem> items;
    items.push_back( RifOpmDeckTools::item( "ID", 1 ) );

    Opm::DeckRecord record{ std::move( items ) };

    auto result = RigSimulationInputAquiferTools::processAquconRecord( record, sectorMin, sectorMax, refinement );

    EXPECT_TRUE( result.has_value() );
    EXPECT_EQ( result->size(), 1U );
    EXPECT_EQ( 1, result->getItem( 0 ).get<int>( 0 ) );
}

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

#include "RigNonUniformRefinement.h"
#include "RigSimulationInputConnectionTools.h"
#include "RigSimulationInputSettings.h"
#include "RigTransmissibilityEquations.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"

#include <cmath>
#include <optional>
#include <vector>

using RigSimulationInputTestHelpers::setSettingsRefinement;

//--------------------------------------------------------------------------------------------------
/// Helper: parse a keyword from deck text and return all its records
//--------------------------------------------------------------------------------------------------
static std::vector<Opm::DeckRecord> parseKeywordRecords( const std::string& deckText, const std::string& keywordName )
{
    Opm::Parser parser;
    auto        deck    = parser.parseString( deckText );
    const auto& keyword = deck[keywordName].back();

    std::vector<Opm::DeckRecord> records;
    for ( size_t i = 0; i < keyword.size(); ++i )
    {
        records.push_back( keyword.getRecord( i ) );
    }
    return records;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static RigSimulationInputSettings createSectorSettings( const cvf::Vec3st& refCounts )
{
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 9, 9, 9 ) );
    setSettingsRefinement( settings, refCounts );
    return settings;
}

//--------------------------------------------------------------------------------------------------
/// A Z-direction connection in a K-refined layer must be completed in every sub-cell, with explicit
/// CF and KH split between the sub-cells (regression: only every other refined layer was completed)
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompdatRecord_ZDirectionSplitAcrossKSubcells )
{
    auto settings = createSectorSettings( cvf::Vec3st( 1, 1, 2 ) );
    auto records  = parseKeywordRecords( "COMPDAT\n 'W' 3 4 2 2 'OPEN' 1* 10.0 0.216 100.0 2* 'Z' /\n/\n", "COMPDAT" );

    auto result = RigSimulationInputConnectionTools::processCompdatRecord( records[0], "W", settings );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 2u, result->size() );

    for ( size_t s = 0; s < 2; ++s )
    {
        const auto& rec = result->at( s );
        EXPECT_EQ( 3, rec.getItem( "I" ).get<int>( 0 ) );
        EXPECT_EQ( 4, rec.getItem( "J" ).get<int>( 0 ) );
        EXPECT_EQ( 3 + static_cast<int>( s ), rec.getItem( "K1" ).get<int>( 0 ) );
        EXPECT_EQ( 3 + static_cast<int>( s ), rec.getItem( "K2" ).get<int>( 0 ) );
        EXPECT_DOUBLE_EQ( 5.0, rec.getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).get<double>( 0 ) );
        EXPECT_DOUBLE_EQ( 50.0, rec.getItem( "Kh" ).get<double>( 0 ) );
        EXPECT_DOUBLE_EQ( 0.216, rec.getItem( "DIAMETER" ).get<double>( 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// With defaulted CF and KH the simulator computes the factors, so the full refined K range is kept in one record
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompdatRecord_DefaultedFactorsKeepsSingleRecord )
{
    auto settings = createSectorSettings( cvf::Vec3st( 1, 1, 2 ) );
    auto records  = parseKeywordRecords( "COMPDAT\n 'W' 3 4 2 3 'OPEN' 2* 0.216 /\n/\n", "COMPDAT" );

    auto result = RigSimulationInputConnectionTools::processCompdatRecord( records[0], "W", settings );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 1u, result->size() );
    EXPECT_EQ( 3, result->at( 0 ).getItem( "K1" ).get<int>( 0 ) );
    EXPECT_EQ( 6, result->at( 0 ).getItem( "K2" ).get<int>( 0 ) );
    EXPECT_FALSE( result->at( 0 ).getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).hasValue( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// An X-direction connection is split across the I sub-cells; J and K use the center sub-cell
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompdatRecord_XDirectionSplitAcrossISubcells )
{
    auto settings = createSectorSettings( cvf::Vec3st( 3, 2, 2 ) );
    auto records  = parseKeywordRecords( "COMPDAT\n 'W' 2 1 1 1 'OPEN' 1* 9.0 0.216 90.0 2* 'X' /\n/\n", "COMPDAT" );

    auto result = RigSimulationInputConnectionTools::processCompdatRecord( records[0], "W", settings );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 3u, result->size() );

    for ( size_t s = 0; s < 3; ++s )
    {
        const auto& rec = result->at( s );
        EXPECT_EQ( 4 + static_cast<int>( s ), rec.getItem( "I" ).get<int>( 0 ) );
        EXPECT_EQ( 2, rec.getItem( "J" ).get<int>( 0 ) );
        EXPECT_EQ( 2, rec.getItem( "K1" ).get<int>( 0 ) );
        EXPECT_EQ( 2, rec.getItem( "K2" ).get<int>( 0 ) );
        EXPECT_DOUBLE_EQ( 3.0, rec.getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).get<double>( 0 ) );
        EXPECT_DOUBLE_EQ( 30.0, rec.getItem( "Kh" ).get<double>( 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Non-uniform K refinement splits the CF by the sub-cell thickness fractions
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompdatRecord_NonUniformKRefinementScalesByFraction )
{
    RigSimulationInputSettings settings;
    settings.setMin( caf::VecIjk0( 0, 0, 0 ) );
    settings.setMax( caf::VecIjk0( 9, 9, 9 ) );
    auto refinement = std::make_unique<RigNonUniformRefinement>( cvf::Vec3st( 10, 10, 10 ) );
    refinement->setCumulativeFractions( RigRefinement::DimK, 1, { 0.25, 1.0 } );
    settings.setRefinement( std::move( refinement ) );

    auto records = parseKeywordRecords( "COMPDAT\n 'W' 3 4 2 2 'OPEN' 1* 8.0 0.216 /\n/\n", "COMPDAT" );

    auto result = RigSimulationInputConnectionTools::processCompdatRecord( records[0], "W", settings );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 2u, result->size() );
    EXPECT_EQ( 2, result->at( 0 ).getItem( "K1" ).get<int>( 0 ) );
    EXPECT_EQ( 3, result->at( 1 ).getItem( "K1" ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 2.0, result->at( 0 ).getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 6.0, result->at( 1 ).getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).get<double>( 0 ) );
    EXPECT_TRUE( result->at( 0 ).getItem( "Kh" ).defaultApplied( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Without refinement the record is passed through unchanged
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompdatRecord_NoRefinementUnchanged )
{
    auto settings = createSectorSettings( cvf::Vec3st( 1, 1, 1 ) );
    auto records  = parseKeywordRecords( "COMPDAT\n 'W' 3 4 2 5 'OPEN' 1* 10.0 0.216 100.0 2* 'Z' /\n/\n", "COMPDAT" );

    auto result = RigSimulationInputConnectionTools::processCompdatRecord( records[0], "W", settings );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 1u, result->size() );
    EXPECT_EQ( 2, result->at( 0 ).getItem( "K1" ).get<int>( 0 ) );
    EXPECT_EQ( 5, result->at( 0 ).getItem( "K2" ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 10.0, result->at( 0 ).getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).get<double>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// COMPSEGS cells are split along the connection direction with the MD interval divided between sub-cells
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompsegsRecord_SplitAcrossKSubcells )
{
    auto settings = createSectorSettings( cvf::Vec3st( 1, 1, 2 ) );

    Opm::Parser parser;
    auto        deck = parser.parseString( "COMPDAT\n 'W' 3 4 2 2 'OPEN' 1* 10.0 0.216 /\n/\n"
                                           "COMPSEGS\n 'W' /\n 3 4 2 1 100.0 120.0 /\n/\n" );

    std::vector<Opm::DeckKeyword> compdatKeywords = { deck["COMPDAT"].back() };
    auto                          directions      = RigSimulationInputConnectionTools::connectionDirections( compdatKeywords );

    const auto& compsegs = deck["COMPSEGS"].back();
    auto result = RigSimulationInputConnectionTools::processCompsegsRecord( compsegs.getRecord( 1 ), "W", false, settings, directions );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 2u, result->size() );

    EXPECT_EQ( 3, result->at( 0 ).getItem( 2 ).get<int>( 0 ) );
    EXPECT_EQ( 4, result->at( 1 ).getItem( 2 ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 100.0, result->at( 0 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 110.0, result->at( 0 ).getItem( 5 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 110.0, result->at( 1 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 120.0, result->at( 1 ).getItem( 5 ).get<double>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Refinement perpendicular to a Z well reduces the Peaceman radius given by PR, so CF increases
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompdatRecord_LateralRefinementCorrectsCFUsingPR )
{
    auto settings = createSectorSettings( cvf::Vec3st( 10, 10, 1 ) );
    auto records  = parseKeywordRecords( "COMPDAT\n 'W' 3 4 2 2 'OPEN' 1* 10.0 0.216 100.0 1.0 1* 'Z' 20.0 /\n/\n", "COMPDAT" );

    auto result = RigSimulationInputConnectionTools::processCompdatRecord( records[0], "W", settings );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 1u, result->size() );

    const auto& rec = result->at( 0 );
    EXPECT_EQ( 26, rec.getItem( "I" ).get<int>( 0 ) );
    EXPECT_EQ( 36, rec.getItem( "J" ).get<int>( 0 ) );

    const double rw       = 0.108;
    const double expected = 10.0 * ( std::log( 20.0 / rw ) + 1.0 ) / ( std::log( 2.0 / rw ) + 1.0 );
    EXPECT_NEAR( expected, rec.getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).get<double>( 0 ), 1e-9 );
    EXPECT_NEAR( 2.0, rec.getItem( "PR" ).get<double>( 0 ), 1e-9 );
    EXPECT_DOUBLE_EQ( 100.0, rec.getItem( "Kh" ).get<double>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Without PR the Peaceman radii are computed from the cell sizes and permeabilities, also for
/// refinement in only one of the perpendicular directions
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompdatRecord_LateralRefinementCorrectsCFUsingCellProperties )
{
    auto settings = createSectorSettings( cvf::Vec3st( 4, 1, 1 ) );
    auto records  = parseKeywordRecords( "COMPDAT\n 'W' 3 4 2 2 'OPEN' 1* 10.0 0.216 /\n/\n", "COMPDAT" );

    RigSimulationInputConnectionTools::ConnectionCellProperties properties{ cvf::Vec3d( 100.0, 50.0, 5.0 ), cvf::Vec3d( 200.0, 50.0, 10.0 ) };
    auto provider = [&]( const caf::VecIjk0& ) -> std::optional<RigSimulationInputConnectionTools::ConnectionCellProperties>
    { return properties; };

    auto result = RigSimulationInputConnectionTools::processCompdatRecord( records[0], "W", settings, provider );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 1u, result->size() );

    const double rw       = 0.108;
    const double r0Coarse = RigTransmissibilityEquations::peacemanRadius( 200.0, 50.0, 100.0, 50.0 );
    const double r0Fine   = RigTransmissibilityEquations::peacemanRadius( 200.0, 50.0, 25.0, 50.0 );
    const double expected = 10.0 * std::log( r0Coarse / rw ) / std::log( r0Fine / rw );
    EXPECT_NEAR( expected, result->at( 0 ).getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).get<double>( 0 ), 1e-9 );
}

//--------------------------------------------------------------------------------------------------
/// The correction is skipped when the refined Peaceman radius gets smaller than the well radius
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompdatRecord_LateralRefinementTooSmallRadiusKeepsCF )
{
    auto settings = createSectorSettings( cvf::Vec3st( 10, 10, 1 ) );
    auto records  = parseKeywordRecords( "COMPDAT\n 'W' 3 4 2 2 'OPEN' 1* 10.0 0.216 4* 0.5 /\n/\n", "COMPDAT" );

    auto result = RigSimulationInputConnectionTools::processCompdatRecord( records[0], "W", settings );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 1u, result->size() );
    EXPECT_DOUBLE_EQ( 10.0, result->at( 0 ).getItem( "CONNECTION_TRANSMISSIBILITY_FACTOR" ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 0.5, result->at( 0 ).getItem( "PR" ).get<double>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// A well travelling towards decreasing I assigns the MD interval from the highest to the lowest I sub-cell
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompsegsRecord_ReversedTraversalAlongI )
{
    auto settings = createSectorSettings( cvf::Vec3st( 2, 1, 1 ) );

    Opm::Parser parser;
    auto        deck = parser.parseString( "COMPDAT\n 'W' 4 2 1 1 'OPEN' 1* 10.0 0.216 3* 'X' /\n"
                                           " 'W' 3 2 1 1 'OPEN' 1* 10.0 0.216 3* 'X' /\n/\n"
                                           "COMPSEGS\n 'W' /\n 4 2 1 1 100.0 120.0 /\n 3 2 1 1 120.0 140.0 /\n/\n" );

    std::vector<Opm::DeckKeyword> compdatKeywords  = { deck["COMPDAT"].back() };
    std::vector<Opm::DeckKeyword> compsegsKeywords = { deck["COMPSEGS"].back() };

    auto directions    = RigSimulationInputConnectionTools::connectionDirections( compdatKeywords );
    auto reversedCells = RigSimulationInputConnectionTools::reversedSegmentCells( compsegsKeywords, directions );

    EXPECT_TRUE( reversedCells.contains( { "W", caf::VecIjk0( 3, 1, 0 ) } ) );
    EXPECT_TRUE( reversedCells.contains( { "W", caf::VecIjk0( 2, 1, 0 ) } ) );

    const auto& compsegs = deck["COMPSEGS"].back();

    // Original I=4 -> refined I sub-cells 7 and 8, the well enters at 8
    auto result =
        RigSimulationInputConnectionTools::processCompsegsRecord( compsegs.getRecord( 1 ), "W", false, settings, directions, reversedCells );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 2u, result->size() );
    EXPECT_EQ( 8, result->at( 0 ).getItem( 0 ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 100.0, result->at( 0 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 110.0, result->at( 0 ).getItem( 5 ).get<double>( 0 ) );
    EXPECT_EQ( 7, result->at( 1 ).getItem( 0 ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 110.0, result->at( 1 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 120.0, result->at( 1 ).getItem( 5 ).get<double>( 0 ) );

    // Original I=3 is the last cell, direction taken from the previous cell
    result =
        RigSimulationInputConnectionTools::processCompsegsRecord( compsegs.getRecord( 2 ), "W", false, settings, directions, reversedCells );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 2u, result->size() );
    EXPECT_EQ( 6, result->at( 0 ).getItem( 0 ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 120.0, result->at( 0 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_EQ( 5, result->at( 1 ).getItem( 0 ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 140.0, result->at( 1 ).getItem( 5 ).get<double>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// A well travelling towards increasing I keeps the sub-cell order
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ReversedSegmentCells_ForwardTraversalNotReversed )
{
    Opm::Parser parser;
    auto        deck = parser.parseString( "COMPDAT\n 'W' 3 2 1 1 'OPEN' 1* 10.0 0.216 3* 'X' /\n"
                                           " 'W' 4 2 1 1 'OPEN' 1* 10.0 0.216 3* 'X' /\n/\n"
                                           "COMPSEGS\n 'W' /\n 3 2 1 1 100.0 120.0 /\n 4 2 1 1 120.0 140.0 /\n/\n" );

    std::vector<Opm::DeckKeyword> compdatKeywords  = { deck["COMPDAT"].back() };
    std::vector<Opm::DeckKeyword> compsegsKeywords = { deck["COMPSEGS"].back() };

    auto directions    = RigSimulationInputConnectionTools::connectionDirections( compdatKeywords );
    auto reversedCells = RigSimulationInputConnectionTools::reversedSegmentCells( compsegsKeywords, directions );

    EXPECT_TRUE( reversedCells.empty() );
}

//--------------------------------------------------------------------------------------------------
/// DIRECTION without END_IJK is a single cell, so the MD interval is still split between the sub-cells
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompsegsRecord_DirectionWithoutRangeIsSplit )
{
    auto settings = createSectorSettings( cvf::Vec3st( 1, 1, 2 ) );

    Opm::Parser parser;
    auto        deck = parser.parseString( "COMPSEGS\n 'W' /\n 3 4 2 1 100.0 120.0 'K' /\n/\n" );

    const auto& compsegs = deck["COMPSEGS"].back();
    auto        result   = RigSimulationInputConnectionTools::processCompsegsRecord( compsegs.getRecord( 1 ), "W", false, settings, {} );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 2u, result->size() );
    EXPECT_EQ( 3, result->at( 0 ).getItem( 2 ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 110.0, result->at( 0 ).getItem( 5 ).get<double>( 0 ) );
    EXPECT_EQ( 4, result->at( 1 ).getItem( 2 ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 110.0, result->at( 1 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_EQ( "K", result->at( 1 ).getItem( 6 ).get<std::string>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// A defaulted start MD is zero
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompsegsRecord_DefaultedStartMdIsZero )
{
    auto settings = createSectorSettings( cvf::Vec3st( 1, 1, 2 ) );

    Opm::Parser parser;
    auto        deck = parser.parseString( "COMPSEGS\n 'W' /\n 3 4 2 1 1* 20.0 /\n/\n" );

    const auto& compsegs = deck["COMPSEGS"].back();
    auto        result   = RigSimulationInputConnectionTools::processCompsegsRecord( compsegs.getRecord( 1 ), "W", false, settings, {} );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 2u, result->size() );
    EXPECT_DOUBLE_EQ( 0.0, result->at( 0 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 10.0, result->at( 0 ).getItem( 5 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 10.0, result->at( 1 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 20.0, result->at( 1 ).getItem( 5 ).get<double>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Without an end MD every sub-cell still gets a segment record, with the MD items unchanged
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompsegsRecord_NoEndMdCopiedToAllSubcells )
{
    auto settings = createSectorSettings( cvf::Vec3st( 1, 1, 2 ) );

    Opm::Parser parser;
    auto        deck = parser.parseString( "COMPSEGS\n 'W' /\n 3 4 2 1 100.0 1* 'K' /\n/\n" );

    const auto& compsegs = deck["COMPSEGS"].back();
    auto        result   = RigSimulationInputConnectionTools::processCompsegsRecord( compsegs.getRecord( 1 ), "W", false, settings, {} );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 2u, result->size() );
    for ( size_t s = 0; s < 2; ++s )
    {
        EXPECT_EQ( 3 + static_cast<int>( s ), result->at( s ).getItem( 2 ).get<int>( 0 ) );
        EXPECT_DOUBLE_EQ( 100.0, result->at( s ).getItem( 4 ).get<double>( 0 ) );
        EXPECT_FALSE( result->at( s ).getItem( 5 ).hasValue( 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// A cell range (END_IJK) is not split, only the center sub-cell is used
//--------------------------------------------------------------------------------------------------
TEST( RigSimulationInputConnectionTools, ProcessCompsegsRecord_CellRangeUsesCenterSubcell )
{
    auto settings = createSectorSettings( cvf::Vec3st( 1, 1, 2 ) );

    Opm::Parser parser;
    auto        deck = parser.parseString( "COMPSEGS\n 'W' /\n 3 4 2 1 100.0 120.0 'K' 3 /\n/\n" );

    const auto& compsegs = deck["COMPSEGS"].back();
    auto        result   = RigSimulationInputConnectionTools::processCompsegsRecord( compsegs.getRecord( 1 ), "W", false, settings, {} );
    ASSERT_TRUE( result.has_value() ) << result.error().toStdString();
    ASSERT_EQ( 1u, result->size() );
    EXPECT_EQ( 4, result->at( 0 ).getItem( 2 ).get<int>( 0 ) );
    EXPECT_DOUBLE_EQ( 100.0, result->at( 0 ).getItem( 4 ).get<double>( 0 ) );
    EXPECT_DOUBLE_EQ( 120.0, result->at( 0 ).getItem( 5 ).get<double>( 0 ) );
}

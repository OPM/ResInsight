/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "CompletionsMsw/RigMswTableData.h"
#include "CompletionsMsw/RigMswTableRows.h"

#include "RiaDefines.h"

namespace
{

constexpr auto METRIC = RiaDefines::EclipseUnitSystem::UNITS_METRIC;

WelsegsRow makeWelsegsRow( int segNum, int joinSeg = 1 )
{
    WelsegsRow row;
    row.segment1    = segNum;
    row.segment2    = segNum;
    row.branch      = 1;
    row.joinSegment = joinSeg;
    row.length      = 10.0;
    row.depth       = 5.0;
    return row;
}

CompsegsRow makeCompsegsRow( size_t i, size_t j, size_t k, const std::string& gridName = "" )
{
    CompsegsRow row;
    row.i             = i;
    row.j             = j;
    row.k             = k;
    row.branch        = 1;
    row.distanceStart = 0.0;
    row.distanceEnd   = 10.0;
    row.gridName      = gridName;
    return row;
}

} // anonymous namespace

//==================================================================================================
// isEmpty
//==================================================================================================

TEST( RigMswTableData, IsEmpty_NewObject )
{
    RigMswTableData td( "Well_A", METRIC );
    EXPECT_TRUE( td.isEmpty() );
}

TEST( RigMswTableData, IsEmpty_WithWelsegsRow )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addWelsegsRow( makeWelsegsRow( 2 ) );
    EXPECT_FALSE( td.isEmpty() );
}

TEST( RigMswTableData, IsEmpty_WithCompsegsRow )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addCompsegsRow( makeCompsegsRow( 1, 1, 1 ) );
    EXPECT_FALSE( td.isEmpty() );
}

TEST( RigMswTableData, IsEmpty_WithWsegvalvRow )
{
    RigMswTableData td( "Well_A", METRIC );
    WsegvalvRow     wv;
    wv.well          = "Well_A";
    wv.segmentNumber = 2;
    wv.cv            = 0.5;
    wv.area          = 1e-4;
    td.addWsegvalvRow( wv );
    EXPECT_FALSE( td.isEmpty() );
}

TEST( RigMswTableData, IsEmpty_WithWsegaicdRow )
{
    RigMswTableData td( "Well_A", METRIC );
    WsegaicdRow     aicd;
    aicd.well             = "Well_A";
    aicd.segment1         = 2;
    aicd.segment2         = 2;
    aicd.strength         = 1e-5;
    aicd.maxAbsRate       = 1000.0;
    aicd.flowRateExponent = 0.9;
    aicd.viscExponent     = 0.1;
    td.addWsegaicdRow( aicd );
    EXPECT_FALSE( td.isEmpty() );
}

// isEmpty() does not check wsegsicdData — document that behaviour explicitly.
TEST( RigMswTableData, IsEmpty_WsegsicdOnlyIsConsideredEmpty )
{
    RigMswTableData td( "Well_A", METRIC );
    WsegsicdRow     sicd;
    sicd.well       = "Well_A";
    sicd.segment1   = 2;
    sicd.segment2   = 2;
    sicd.strength   = 2e-5;
    sicd.maxAbsRate = 500.0;
    td.addWsegsicdRow( sicd );

    // isEmpty() does not inspect wsegsicdData, so it returns true even though
    // there is a WSEGSICD row present.
    EXPECT_TRUE( td.isEmpty() );
    EXPECT_FALSE( td.wsegsicdData().empty() ); // data IS there
}

//==================================================================================================
// hasLgrData / mainGridCompsegsData / lgrCompsegsData
//==================================================================================================

TEST( RigMswTableData, HasLgrData_NoCompsegs )
{
    RigMswTableData td( "Well_A", METRIC );
    EXPECT_FALSE( td.hasLgrData() );
}

TEST( RigMswTableData, HasLgrData_OnlyMainGrid )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addCompsegsRow( makeCompsegsRow( 1, 1, 1, "" ) );
    td.addCompsegsRow( makeCompsegsRow( 2, 1, 1, "" ) );
    EXPECT_FALSE( td.hasLgrData() );
}

TEST( RigMswTableData, HasLgrData_OneLgrRow )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addCompsegsRow( makeCompsegsRow( 1, 1, 1, "" ) );
    td.addCompsegsRow( makeCompsegsRow( 2, 1, 1, "LGR1" ) );
    EXPECT_TRUE( td.hasLgrData() );
}

TEST( RigMswTableData, HasLgrData_AllLgrRows )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addCompsegsRow( makeCompsegsRow( 1, 1, 1, "LGR1" ) );
    td.addCompsegsRow( makeCompsegsRow( 2, 1, 1, "LGR2" ) );
    EXPECT_TRUE( td.hasLgrData() );
}

TEST( RigMswTableData, MainGridCompsegsData_FiltersCorrectly )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addCompsegsRow( makeCompsegsRow( 1, 1, 1, "" ) );
    td.addCompsegsRow( makeCompsegsRow( 2, 1, 1, "LGR1" ) );
    td.addCompsegsRow( makeCompsegsRow( 3, 1, 1, "" ) );

    auto mainRows = td.mainGridCompsegsData();
    ASSERT_EQ( 2u, mainRows.size() );
    EXPECT_EQ( 1u, mainRows[0].i );
    EXPECT_EQ( 3u, mainRows[1].i );
    for ( const auto& r : mainRows )
        EXPECT_TRUE( r.isMainGrid() );
}

TEST( RigMswTableData, LgrCompsegsData_FiltersCorrectly )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addCompsegsRow( makeCompsegsRow( 1, 1, 1, "" ) );
    td.addCompsegsRow( makeCompsegsRow( 2, 1, 1, "LGR1" ) );
    td.addCompsegsRow( makeCompsegsRow( 3, 1, 1, "LGR2" ) );

    auto lgrRows = td.lgrCompsegsData();
    ASSERT_EQ( 2u, lgrRows.size() );
    EXPECT_EQ( "LGR1", lgrRows[0].gridName );
    EXPECT_EQ( "LGR2", lgrRows[1].gridName );
    for ( const auto& r : lgrRows )
        EXPECT_TRUE( r.isLgrGrid() );
}

TEST( RigMswTableData, MainGridAndLgrCompsegsData_NoOverlap )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addCompsegsRow( makeCompsegsRow( 1, 1, 1, "" ) );
    td.addCompsegsRow( makeCompsegsRow( 2, 1, 1, "LGR1" ) );

    auto mainRows = td.mainGridCompsegsData();
    auto lgrRows  = td.lgrCompsegsData();

    EXPECT_EQ( 1u, mainRows.size() );
    EXPECT_EQ( 1u, lgrRows.size() );
    EXPECT_EQ( mainRows.size() + lgrRows.size(), td.compsegsData().size() );
}

//==================================================================================================
// validationErrors / isValid
//==================================================================================================

TEST( RigMswTableData, ValidationErrors_EmptyWellName )
{
    RigMswTableData td( "", METRIC );
    td.addWelsegsRow( makeWelsegsRow( 2 ) );

    auto errors = td.validationErrors();
    EXPECT_FALSE( errors.empty() );
    bool foundWellNameError = std::any_of( errors.begin(), errors.end(), []( const QString& e ) { return e.contains( "Well name" ); } );
    EXPECT_TRUE( foundWellNameError );
    EXPECT_FALSE( td.isValid() );
}

TEST( RigMswTableData, ValidationErrors_NoWelsegsRows )
{
    RigMswTableData td( "Well_A", METRIC );

    auto errors = td.validationErrors();
    EXPECT_FALSE( errors.empty() );
    bool foundWelsegsError = std::any_of( errors.begin(), errors.end(), []( const QString& e ) { return e.contains( "WELSEGS" ); } );
    EXPECT_TRUE( foundWelsegsError );
    EXPECT_FALSE( td.isValid() );
}

TEST( RigMswTableData, ValidationErrors_DuplicateSegmentNumber )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addWelsegsRow( makeWelsegsRow( 2 ) );
    td.addWelsegsRow( makeWelsegsRow( 3 ) );
    td.addWelsegsRow( makeWelsegsRow( 2 ) ); // duplicate

    auto errors = td.validationErrors();
    EXPECT_FALSE( errors.empty() );
    bool foundDuplicateError =
        std::any_of( errors.begin(), errors.end(), []( const QString& e ) { return e.contains( "Duplicate" ) || e.contains( "2" ); } );
    EXPECT_TRUE( foundDuplicateError );
    EXPECT_FALSE( td.isValid() );
}

TEST( RigMswTableData, ValidationErrors_MultipleErrors )
{
    // Empty well name AND no WELSEGS → two separate errors
    RigMswTableData td( "", METRIC );

    auto errors = td.validationErrors();
    EXPECT_GE( errors.size(), 2u );
    EXPECT_FALSE( td.isValid() );
}

TEST( RigMswTableData, IsValid_ValidData )
{
    RigMswTableData td( "Well_A", METRIC );
    td.addWelsegsRow( makeWelsegsRow( 2, 1 ) );
    td.addWelsegsRow( makeWelsegsRow( 3, 2 ) );

    EXPECT_TRUE( td.validationErrors().empty() );
    EXPECT_TRUE( td.isValid() );
}

//==================================================================================================
// Metadata accessors
//==================================================================================================

TEST( RigMswTableData, WellNameAndUnitSystem )
{
    RigMswTableData td( "MyWell", RiaDefines::EclipseUnitSystem::UNITS_FIELD );
    EXPECT_EQ( "MyWell", td.wellName() );
    EXPECT_EQ( RiaDefines::EclipseUnitSystem::UNITS_FIELD, td.unitSystem() );
}

TEST( RigMswTableData, HasWelsegsData_HasCompsegsData_HasValveData )
{
    RigMswTableData td( "Well_A", METRIC );
    EXPECT_FALSE( td.hasWelsegsData() );
    EXPECT_FALSE( td.hasCompsegsData() );
    EXPECT_FALSE( td.hasWsegvalvData() );
    EXPECT_FALSE( td.hasWsegaicdData() );
    EXPECT_FALSE( td.hasWsegsicdData() );

    td.addWelsegsRow( makeWelsegsRow( 2 ) );
    EXPECT_TRUE( td.hasWelsegsData() );

    td.addCompsegsRow( makeCompsegsRow( 1, 1, 1 ) );
    EXPECT_TRUE( td.hasCompsegsData() );

    WsegvalvRow wv;
    wv.well          = "Well_A";
    wv.segmentNumber = 2;
    wv.cv            = 0.5;
    wv.area          = 1e-4;
    td.addWsegvalvRow( wv );
    EXPECT_TRUE( td.hasWsegvalvData() );
}

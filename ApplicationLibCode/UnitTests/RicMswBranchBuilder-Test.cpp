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

#include "CompletionExportCommands/MswExport/RicMswBranchBuilder.h"

#include "RifReaderMockModel.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

using namespace RicMswBranchBuilder;

//==================================================================================================
// findOutletSegmentForMD tests
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Empty map returns 1 (heel segment).
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_EmptyMap )
{
    std::vector<CellSegmentEntry> map;
    EXPECT_EQ( 1, findOutletSegmentForMD( map, 500.0 ) );
}

//--------------------------------------------------------------------------------------------------
/// MD falls within the first cell's range.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_MDInFirstCell )
{
    std::vector<CellSegmentEntry> map = { { 100.0, 200.0, 5 }, { 200.0, 300.0, 6 }, { 300.0, 400.0, 7 } };
    EXPECT_EQ( 5, findOutletSegmentForMD( map, 150.0 ) );
}

//--------------------------------------------------------------------------------------------------
/// MD falls within the middle cell's range.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_MDInMiddleCell )
{
    std::vector<CellSegmentEntry> map = { { 100.0, 200.0, 5 }, { 200.0, 300.0, 6 }, { 300.0, 400.0, 7 } };
    EXPECT_EQ( 6, findOutletSegmentForMD( map, 250.0 ) );
}

//--------------------------------------------------------------------------------------------------
/// MD falls within the last cell's range.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_MDInLastCell )
{
    std::vector<CellSegmentEntry> map = { { 100.0, 200.0, 5 }, { 200.0, 300.0, 6 }, { 300.0, 400.0, 7 } };
    EXPECT_EQ( 7, findOutletSegmentForMD( map, 350.0 ) );
}

//--------------------------------------------------------------------------------------------------
/// MD exactly at the start of the first cell — shallower than the first midpoint (100),
/// so no midpoint is at or below md; fallback = first (shallowest) segment.
/// MD exactly at the shared boundary (200) — midpoint of first cell (150) is below 200,
/// midpoint of second cell (250) is above 200; closest-below midpoint is 150 → seg 5.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_MDAtCellStart )
{
    std::vector<CellSegmentEntry> map = { { 100.0, 200.0, 5 }, { 200.0, 300.0, 6 } };
    EXPECT_EQ( 5, findOutletSegmentForMD( map, 100.0 ) ); // shallower than midpoint 150 → first seg
    EXPECT_EQ( 5, findOutletSegmentForMD( map, 200.0 ) ); // midpoint 150 ≤ 200 < midpoint 250 → seg 5
}

//--------------------------------------------------------------------------------------------------
/// MD=200 sits between midpoint 150 (seg 5) and midpoint 250 (seg 6).
/// Closest midpoint at-or-below is 150 → seg 5.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_MDAtCellEnd_ExclusiveBoundary )
{
    std::vector<CellSegmentEntry> map = { { 100.0, 200.0, 5 }, { 200.0, 300.0, 6 } };
    EXPECT_EQ( 5, findOutletSegmentForMD( map, 200.0 ) );
}

//--------------------------------------------------------------------------------------------------
/// MD before all segment midpoints — no midpoint is at or below md;
/// fallback = first (shallowest) segment.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_MDBelowAllCells )
{
    std::vector<CellSegmentEntry> map = { { 100.0, 200.0, 5 }, { 200.0, 300.0, 6 } };
    EXPECT_EQ( 5, findOutletSegmentForMD( map, 50.0 ) ); // shallower than all midpoints → first seg
}

//--------------------------------------------------------------------------------------------------
/// MD beyond the last cell returns the last cell's segment number (fallback).
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_MDBeyondAllCells )
{
    std::vector<CellSegmentEntry> map = { { 100.0, 200.0, 5 }, { 200.0, 300.0, 6 }, { 300.0, 400.0, 7 } };
    EXPECT_EQ( 7, findOutletSegmentForMD( map, 999.0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Single-cell map.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, FindOutlet_SingleCell )
{
    std::vector<CellSegmentEntry> map = { { 0.0, 100.0, 3 } };
    EXPECT_EQ( 3, findOutletSegmentForMD( map, 50.0 ) );
    EXPECT_EQ( 3, findOutletSegmentForMD( map, 0.0 ) );
    EXPECT_EQ( 3, findOutletSegmentForMD( map, 200.0 ) ); // beyond → fallback = 3
}

//==================================================================================================
// toMswCellIntersection tests
//==================================================================================================

namespace
{
//--------------------------------------------------------------------------------------------------
/// Creates a minimal 2x2x3 mock grid (12 cells, IJK layout: I fast, then J, then K).
//--------------------------------------------------------------------------------------------------
cvf::ref<RigEclipseCaseData> makeMockGrid()
{
    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );
    cvf::ref<RifReaderMockModel> reader   = new RifReaderMockModel;
    reader->setWorldCoordinates( cvf::Vec3d( 0, 0, 0 ), cvf::Vec3d( 100, 100, 100 ) );
    reader->setCellCounts( cvf::Vec3st( 2, 2, 3 ) );
    reader->enableWellData( false );
    reader->open( "", caseData.p() );
    caseData->mainGrid()->computeCachedData();
    return caseData;
}

//--------------------------------------------------------------------------------------------------
/// Build a minimal WellPathCellIntersectionInfo with only globCellIndex set.
//--------------------------------------------------------------------------------------------------
WellPathCellIntersectionInfo makeCellInfo( size_t globCellIndex )
{
    WellPathCellIntersectionInfo info{};
    info.globCellIndex = globCellIndex;
    return info;
}
} // anonymous namespace

//--------------------------------------------------------------------------------------------------
/// A gap segment (globCellIndex >= totalCellCount) returns nullopt.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, ToMswCellIntersection_GapSegmentReturnsNullopt )
{
    auto         caseData = makeMockGrid();
    RigMainGrid* grid     = caseData->mainGrid();

    // 2x2x3 = 12 cells; index 12 is out of range
    auto result = toMswCellIntersection( makeCellInfo( 12 ), grid, 0.0, 10.0 );
    EXPECT_FALSE( result.has_value() );

    auto result2 = toMswCellIntersection( makeCellInfo( 999 ), grid, 0.0, 10.0 );
    EXPECT_FALSE( result2.has_value() );
}

//--------------------------------------------------------------------------------------------------
/// Cell index 0 is (I=0,J=0,K=0) in 0-based → (1,1,1) in 1-based.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, ToMswCellIntersection_CellZero_OneBasedIJK )
{
    auto         caseData = makeMockGrid();
    RigMainGrid* grid     = caseData->mainGrid();

    auto result = toMswCellIntersection( makeCellInfo( 0 ), grid, 50.0, 60.0 );
    ASSERT_TRUE( result.has_value() );
    EXPECT_EQ( 1u, result->i );
    EXPECT_EQ( 1u, result->j );
    EXPECT_EQ( 1u, result->k );
}

//--------------------------------------------------------------------------------------------------
/// Distance parameters are passed through unchanged.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, ToMswCellIntersection_DistancesPassedThrough )
{
    auto         caseData = makeMockGrid();
    RigMainGrid* grid     = caseData->mainGrid();

    auto result = toMswCellIntersection( makeCellInfo( 0 ), grid, 123.4, 567.8 );
    ASSERT_TRUE( result.has_value() );
    EXPECT_DOUBLE_EQ( 123.4, result->distanceStart );
    EXPECT_DOUBLE_EQ( 567.8, result->distanceEnd );
}

//--------------------------------------------------------------------------------------------------
/// A main-grid cell has an empty gridName.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, ToMswCellIntersection_MainGridHasEmptyGridName )
{
    auto         caseData = makeMockGrid();
    RigMainGrid* grid     = caseData->mainGrid();

    auto result = toMswCellIntersection( makeCellInfo( 0 ), grid, 0.0, 1.0 );
    ASSERT_TRUE( result.has_value() );
    EXPECT_TRUE( result->gridName.empty() );
}

//--------------------------------------------------------------------------------------------------
/// IJK indexing: for a 2x2x3 grid (nI=2, nJ=2, nK=3), globalIdx = i + j*nI + k*nI*nJ.
/// Cell at (1,0,0) has globalIdx = 1 → (2,1,1) in 1-based.
/// Cell at (0,1,0) has globalIdx = 2 → (1,2,1) in 1-based.
/// Cell at (0,0,2) has globalIdx = 8 → (1,1,3) in 1-based.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, ToMswCellIntersection_IJKMapping )
{
    auto         caseData = makeMockGrid();
    RigMainGrid* grid     = caseData->mainGrid();

    // globalIdx=1: i=1,j=0,k=0 → (2,1,1) 1-based
    auto r1 = toMswCellIntersection( makeCellInfo( 1 ), grid, 0.0, 1.0 );
    ASSERT_TRUE( r1.has_value() );
    EXPECT_EQ( 2u, r1->i );
    EXPECT_EQ( 1u, r1->j );
    EXPECT_EQ( 1u, r1->k );

    // globalIdx=2: i=0,j=1,k=0 → (1,2,1) 1-based
    auto r2 = toMswCellIntersection( makeCellInfo( 2 ), grid, 0.0, 1.0 );
    ASSERT_TRUE( r2.has_value() );
    EXPECT_EQ( 1u, r2->i );
    EXPECT_EQ( 2u, r2->j );
    EXPECT_EQ( 1u, r2->k );

    // globalIdx=8: i=0,j=0,k=2 → (1,1,3) 1-based
    auto r8 = toMswCellIntersection( makeCellInfo( 8 ), grid, 0.0, 1.0 );
    ASSERT_TRUE( r8.has_value() );
    EXPECT_EQ( 1u, r8->i );
    EXPECT_EQ( 1u, r8->j );
    EXPECT_EQ( 3u, r8->k );
}

//--------------------------------------------------------------------------------------------------
/// Dual-porosity: K index is shifted up by cellCountK.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, ToMswCellIntersection_DualPorosity_KShifted )
{
    auto         caseData = makeMockGrid();
    RigMainGrid* grid     = caseData->mainGrid();
    grid->setDualPorosity( true );

    const size_t cellCountK = grid->cellCountK(); // 3

    // Cell index 0: (i=0,j=0,k=0) → 1-based k = 1 + cellCountK = 4
    auto result = toMswCellIntersection( makeCellInfo( 0 ), grid, 0.0, 1.0 );
    ASSERT_TRUE( result.has_value() );
    EXPECT_EQ( 1u + cellCountK, result->k );
}

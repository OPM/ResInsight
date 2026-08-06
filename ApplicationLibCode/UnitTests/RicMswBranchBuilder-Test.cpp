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

//==================================================================================================
// applyEffectiveDiameters / applyIcdAreaPerCell tests
//==================================================================================================

namespace
{
//--------------------------------------------------------------------------------------------------
/// Build export data with one branch holding the given segment numbers, each with a WSEGVALV area.
//--------------------------------------------------------------------------------------------------
RigMswWellExportData makeExportData( const std::vector<std::pair<int, double>>& segmentNumberAndArea )
{
    RigMswWellExportData exportData;
    RigMswBranch         branch;
    branch.branchNumber = 1;

    for ( const auto& [segmentNumber, area] : segmentNumberAndArea )
    {
        RigMswSegment segment;
        segment.segmentNumber       = segmentNumber;
        segment.outletSegmentNumber = 1;
        segment.length              = 1.0;
        segment.depth               = 1.0;

        WsegvalvRow row;
        row.segmentNumber    = segmentNumber;
        row.cv               = 1.5;
        row.area             = area;
        segment.wsegvalvData = row;

        branch.segments.push_back( segment );
    }

    exportData.branches.push_back( branch );
    return exportData;
}

double areaOfSegment( const RigMswWellExportData& exportData, int segmentNumber )
{
    for ( const auto& branch : exportData.branches )
    {
        for ( const auto& segment : branch.segments )
        {
            if ( segment.segmentNumber == segmentNumber && segment.wsegvalvData.has_value() ) return segment.wsegvalvData->area;
        }
    }
    return -1.0;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A single ICD sub in a cell keeps its own area.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, IcdAreaPerCell_SingleIcdIsUnchanged )
{
    FishbonesExportContext context;
    context.icdSegments.push_back( { 10, { 100 } } );

    auto exportData = makeExportData( { { 10, 0.002 } } );
    applyIcdAreaPerCell( context, exportData );

    EXPECT_DOUBLE_EQ( 0.002, areaOfSegment( exportData, 10 ) );
}

//--------------------------------------------------------------------------------------------------
/// Two ICD subs connected to the same cell both report the sum of their areas.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, IcdAreaPerCell_SharedCellGetsAreaSum )
{
    FishbonesExportContext context;
    context.icdSegments.push_back( { 10, { 100 } } );
    context.icdSegments.push_back( { 20, { 100 } } );

    auto exportData = makeExportData( { { 10, 0.002 }, { 20, 0.003 } } );
    applyIcdAreaPerCell( context, exportData );

    EXPECT_DOUBLE_EQ( 0.005, areaOfSegment( exportData, 10 ) );
    EXPECT_DOUBLE_EQ( 0.005, areaOfSegment( exportData, 20 ) );
}

//--------------------------------------------------------------------------------------------------
/// ICD subs in different cells are not combined.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, IcdAreaPerCell_SeparateCellsAreIndependent )
{
    FishbonesExportContext context;
    context.icdSegments.push_back( { 10, { 100 } } );
    context.icdSegments.push_back( { 20, { 200 } } );

    auto exportData = makeExportData( { { 10, 0.002 }, { 20, 0.003 } } );
    applyIcdAreaPerCell( context, exportData );

    EXPECT_DOUBLE_EQ( 0.002, areaOfSegment( exportData, 10 ) );
    EXPECT_DOUBLE_EQ( 0.003, areaOfSegment( exportData, 20 ) );
}

//--------------------------------------------------------------------------------------------------
/// An ICD sub reaching into two cells reports the largest sum, computed from the original areas.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, IcdAreaPerCell_MultipleCellsUseLargestSum )
{
    FishbonesExportContext context;
    context.icdSegments.push_back( { 10, { 100, 200 } } );
    context.icdSegments.push_back( { 20, { 100 } } );
    context.icdSegments.push_back( { 30, { 200 } } );
    context.icdSegments.push_back( { 40, { 200 } } );

    auto exportData = makeExportData( { { 10, 0.001 }, { 20, 0.002 }, { 30, 0.003 }, { 40, 0.004 } } );
    applyIcdAreaPerCell( context, exportData );

    // Cell 100 sums to 0.003, cell 200 to 0.008. Segment 10 takes part in both and reports the larger.
    EXPECT_DOUBLE_EQ( 0.008, areaOfSegment( exportData, 10 ) );
    EXPECT_DOUBLE_EQ( 0.003, areaOfSegment( exportData, 20 ) );
    EXPECT_DOUBLE_EQ( 0.008, areaOfSegment( exportData, 30 ) );
    EXPECT_DOUBLE_EQ( 0.008, areaOfSegment( exportData, 40 ) );
}

//--------------------------------------------------------------------------------------------------
/// The result does not depend on the order the ICD subs are recorded in.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, IcdAreaPerCell_IsIndependentOfRecordingOrder )
{
    FishbonesExportContext forward;
    forward.icdSegments.push_back( { 10, { 100, 200 } } );
    forward.icdSegments.push_back( { 20, { 100 } } );
    forward.icdSegments.push_back( { 30, { 200 } } );

    FishbonesExportContext reversed;
    reversed.icdSegments.push_back( { 30, { 200 } } );
    reversed.icdSegments.push_back( { 20, { 100 } } );
    reversed.icdSegments.push_back( { 10, { 200, 100 } } );

    const std::vector<std::pair<int, double>> segments = { { 10, 0.001 }, { 20, 0.002 }, { 30, 0.003 } };

    auto forwardData = makeExportData( segments );
    applyIcdAreaPerCell( forward, forwardData );

    auto reversedData = makeExportData( segments );
    applyIcdAreaPerCell( reversed, reversedData );

    for ( int segmentNumber : { 10, 20, 30 } )
    {
        EXPECT_DOUBLE_EQ( areaOfSegment( forwardData, segmentNumber ), areaOfSegment( reversedData, segmentNumber ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Valve segments that are not fishbones ICD subs are left alone.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, IcdAreaPerCell_NonIcdValveIsUntouched )
{
    FishbonesExportContext context;
    context.icdSegments.push_back( { 10, { 100 } } );
    context.icdSegments.push_back( { 20, { 100 } } );

    auto exportData = makeExportData( { { 10, 0.002 }, { 20, 0.003 }, { 30, 0.007 } } );
    applyIcdAreaPerCell( context, exportData );

    EXPECT_DOUBLE_EQ( 0.007, areaOfSegment( exportData, 30 ) );
}

//--------------------------------------------------------------------------------------------------
/// Three laterals in the same cell give Deff = sqrt(3) * d.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, EffectiveDiameters_SharedCellCombinesDiameters )
{
    FishbonesExportContext context;
    context.laterals.push_back( { { { { 10 }, 100, 0.0096 } } } );
    context.laterals.push_back( { { { { 20 }, 100, 0.0096 } } } );
    context.laterals.push_back( { { { { 30 }, 100, 0.0096 } } } );

    auto exportData = makeExportData( { { 10, 0.0 }, { 20, 0.0 }, { 30, 0.0 } } );
    applyEffectiveDiameters( context, exportData );

    const double expected = std::sqrt( 3.0 ) * 0.0096;
    for ( const auto& segment : exportData.branches[0].segments )
    {
        ASSERT_TRUE( segment.diameter.has_value() );
        EXPECT_NEAR( expected, *segment.diameter, 1.0e-12 );
    }
}

//--------------------------------------------------------------------------------------------------
/// A cell intersection split into several WELSEGS rows contributes once to the sum, and all its rows
/// get the same effective diameter.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, EffectiveDiameters_SplitIntersectionCountsOnce )
{
    FishbonesExportContext context;
    context.laterals.push_back( { { { { 10, 11, 12 }, 100, 0.0096 } } } );
    context.laterals.push_back( { { { { 20 }, 100, 0.0096 } } } );

    auto exportData = makeExportData( { { 10, 0.0 }, { 11, 0.0 }, { 12, 0.0 }, { 20, 0.0 } } );
    applyEffectiveDiameters( context, exportData );

    // Two intersections in cell 100, not four rows.
    const double expected = std::sqrt( 2.0 ) * 0.0096;
    for ( const auto& segment : exportData.branches[0].segments )
    {
        ASSERT_TRUE( segment.diameter.has_value() );
        EXPECT_NEAR( expected, *segment.diameter, 1.0e-12 );
    }
}

//--------------------------------------------------------------------------------------------------
/// The first segment of a lateral inherits the effective diameter of the second segment.
//--------------------------------------------------------------------------------------------------
TEST( RicMswBranchBuilder, EffectiveDiameters_FirstSegmentInheritsSecond )
{
    FishbonesExportContext context;

    // Two laterals start in cell 100, only the first one continues into cell 200.
    context.laterals.push_back( { { { { 10 }, 100, 0.0096 }, { { 11 }, 200, 0.0096 } } } );
    context.laterals.push_back( { { { { 20 }, 100, 0.0096 } } } );

    auto exportData = makeExportData( { { 10, 0.0 }, { 11, 0.0 }, { 20, 0.0 } } );
    applyEffectiveDiameters( context, exportData );

    const auto& segments = exportData.branches[0].segments;

    // Segment 10 is alone with segment 20 in cell 100, but takes the value of segment 11.
    EXPECT_NEAR( 0.0096, *segments[0].diameter, 1.0e-12 );
    EXPECT_NEAR( 0.0096, *segments[1].diameter, 1.0e-12 );

    // The second lateral has a single intersection and keeps the combined value of cell 100.
    EXPECT_NEAR( std::sqrt( 2.0 ) * 0.0096, *segments[2].diameter, 1.0e-12 );
}

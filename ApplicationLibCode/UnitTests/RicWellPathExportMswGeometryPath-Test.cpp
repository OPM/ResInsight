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

#include "CompletionExportCommands/MswExport/RicWellPathExportMswGeometryPath.h"

#include "CompletionsMsw/RigMswSegment.h"
#include "CompletionsMsw/RigMswTableData.h"
#include "CompletionsMsw/RigMswTableRows.h"

#include "RiaDefines.h"

namespace
{

//--------------------------------------------------------------------------------------------------
/// Build a minimal valid RigMswSegment with no intersections or valve data.
//--------------------------------------------------------------------------------------------------
RigMswSegment makeSegment( int segNum, int outletSegNum, double length = 10.0, double depth = 5.0 )
{
    RigMswSegment seg;
    seg.segmentNumber       = segNum;
    seg.outletSegmentNumber = outletSegNum;
    seg.length              = length;
    seg.depth               = depth;
    seg.diameter            = 0.15;
    seg.roughness           = 1.0e-5;
    seg.description         = "test segment";
    seg.sourceWellName      = "TestWell";
    return seg;
}

//--------------------------------------------------------------------------------------------------
/// Build a minimal RigMswBranch with the given branch number and segments.
//--------------------------------------------------------------------------------------------------
RigMswBranch makeBranch( int branchNum, std::vector<RigMswSegment> segs )
{
    return RigMswBranch{ branchNum, std::nullopt, std::move( segs ) };
}

//--------------------------------------------------------------------------------------------------
/// Build a minimal valid WelsegsHeader.
//--------------------------------------------------------------------------------------------------
WelsegsHeader makeHeader( const std::string& wellName = "TestWell" )
{
    WelsegsHeader hdr;
    hdr.well      = wellName;
    hdr.topDepth  = 100.0;
    hdr.topLength = 200.0;
    hdr.infoType  = "INC";
    return hdr;
}

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
/// Empty segment list — returns valid RigMswTableData with no rows; header well name is set.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, EmptySegmentList )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader( "Well_A" );

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    EXPECT_EQ( "Well_A", result.wellName() );
    EXPECT_EQ( "Well_A", result.welsegsHeader().well );
    EXPECT_TRUE( result.welsegsData().empty() );
    EXPECT_TRUE( result.compsegsData().empty() );
    EXPECT_TRUE( result.wsegvalvData().empty() );
    EXPECT_TRUE( result.wsegaicdData().empty() );
    EXPECT_TRUE( result.wsegsicdData().empty() );
    EXPECT_TRUE( result.mswBranches().empty() );
}

//--------------------------------------------------------------------------------------------------
/// Single segment with no intersections and no valves — 1 WELSEGS row, 0 COMPSEGS.
/// Verify all mapped fields.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SingleSegmentNoIntersections )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment seg   = makeSegment( 2, 1, 25.0, 12.5 );
    seg.description     = "main bore";
    seg.sourceWellName  = "WP_1";
    exportData.branches = { makeBranch( 1, { seg } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 1u, result.welsegsData().size() );
    EXPECT_TRUE( result.compsegsData().empty() );
    EXPECT_TRUE( result.wsegvalvData().empty() );

    const WelsegsRow& row = result.welsegsData()[0];
    EXPECT_EQ( 2, row.segment1 );
    EXPECT_EQ( 2, row.segment2 );
    EXPECT_EQ( 1, row.branch );
    EXPECT_EQ( 1, row.joinSegment );
    EXPECT_DOUBLE_EQ( 25.0, row.length );
    EXPECT_DOUBLE_EQ( 12.5, row.depth );
    EXPECT_TRUE( row.diameter.has_value() );
    EXPECT_DOUBLE_EQ( 0.15, *row.diameter );
    EXPECT_TRUE( row.roughness.has_value() );
    EXPECT_DOUBLE_EQ( 1.0e-5, *row.roughness );
    EXPECT_EQ( "main bore", row.description );
    EXPECT_EQ( "WP_1", row.sourceWellName );
}

//--------------------------------------------------------------------------------------------------
/// Single segment with multiple cell intersections — COMPSEGS count equals intersection count.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentWithMultipleIntersections )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment seg = makeSegment( 2, 1 );

    RigMswCellIntersection ci1{ 3, 5, 7, 100.0, 110.0, "" };
    RigMswCellIntersection ci2{ 3, 5, 8, 110.0, 120.0, "" };
    RigMswCellIntersection ci3{ 4, 5, 8, 120.0, 130.0, "" };
    seg.intersections = { ci1, ci2, ci3 };

    exportData.branches = { makeBranch( 1, { seg } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 1u, result.welsegsData().size() );
    ASSERT_EQ( 3u, result.compsegsData().size() );

    const CompsegsRow& r0 = result.compsegsData()[0];
    EXPECT_EQ( 3u, r0.i );
    EXPECT_EQ( 5u, r0.j );
    EXPECT_EQ( 7u, r0.k );
    EXPECT_EQ( 1, r0.branch );
    EXPECT_DOUBLE_EQ( 100.0, r0.distanceStart );
    EXPECT_DOUBLE_EQ( 110.0, r0.distanceEnd );
    EXPECT_TRUE( r0.isMainGrid() );

    const CompsegsRow& r2 = result.compsegsData()[2];
    EXPECT_EQ( 4u, r2.i );
    EXPECT_EQ( 5u, r2.j );
    EXPECT_EQ( 8u, r2.k );
}

//--------------------------------------------------------------------------------------------------
/// Main-grid vs LGR intersection — LGR row has non-empty gridName.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, MainGridAndLgrIntersections )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment seg = makeSegment( 2, 1 );

    RigMswCellIntersection mainGridCell{ 1, 2, 3, 50.0, 60.0, "" };
    RigMswCellIntersection lgrCell{ 4, 5, 6, 60.0, 70.0, "LGR_NEAR_WELL" };
    seg.intersections = { mainGridCell, lgrCell };

    exportData.branches = { makeBranch( 1, { seg } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 2u, result.compsegsData().size() );

    const CompsegsRow& main = result.compsegsData()[0];
    EXPECT_TRUE( main.isMainGrid() );
    EXPECT_TRUE( main.gridName.empty() );

    const CompsegsRow& lgr = result.compsegsData()[1];
    EXPECT_TRUE( lgr.isLgrGrid() );
    EXPECT_EQ( "LGR_NEAR_WELL", lgr.gridName );
    EXPECT_EQ( 4u, lgr.i );
    EXPECT_EQ( 5u, lgr.j );
    EXPECT_EQ( 6u, lgr.k );
}

//--------------------------------------------------------------------------------------------------
/// Segment with wsegvalvData — 1 WSEGVALV row added; no WSEGAICD or WSEGSICD rows.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentWithWsegvalvData )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment seg = makeSegment( 3, 2 );

    WsegvalvRow wv;
    wv.well          = "TestWell";
    wv.segmentNumber = 3;
    wv.cv            = 0.75;
    wv.area          = 1.2e-4;
    wv.status        = "OPEN";
    wv.description   = "ICD valve";
    seg.wsegvalvData = wv;

    exportData.branches = { makeBranch( 2, { seg } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 1u, result.welsegsData().size() );
    ASSERT_EQ( 1u, result.wsegvalvData().size() );
    EXPECT_TRUE( result.wsegaicdData().empty() );
    EXPECT_TRUE( result.wsegsicdData().empty() );

    const WsegvalvRow& row = result.wsegvalvData()[0];
    EXPECT_EQ( "TestWell", row.well );
    EXPECT_EQ( 3, row.segmentNumber );
    EXPECT_DOUBLE_EQ( 0.75, row.cv );
    EXPECT_DOUBLE_EQ( 1.2e-4, row.area );
    ASSERT_TRUE( row.status.has_value() );
    EXPECT_EQ( "OPEN", *row.status );
}

//--------------------------------------------------------------------------------------------------
/// Segment with wsegaicdData — 1 WSEGAICD row added; no WSEGVALV or WSEGSICD rows.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentWithWsegaicdData )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment seg = makeSegment( 4, 2 );

    WsegaicdRow aicd;
    aicd.well             = "TestWell";
    aicd.segment1         = 4;
    aicd.segment2         = 4;
    aicd.strength         = 1.5e-5;
    aicd.maxAbsRate       = 1000.0;
    aicd.flowRateExponent = 0.9;
    aicd.viscExponent     = 0.1;
    seg.wsegaicdData      = aicd;

    exportData.branches = { makeBranch( 2, { seg } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 1u, result.welsegsData().size() );
    EXPECT_TRUE( result.wsegvalvData().empty() );
    ASSERT_EQ( 1u, result.wsegaicdData().size() );
    EXPECT_TRUE( result.wsegsicdData().empty() );

    const WsegaicdRow& row = result.wsegaicdData()[0];
    EXPECT_EQ( "TestWell", row.well );
    EXPECT_EQ( 4, row.segment1 );
    EXPECT_DOUBLE_EQ( 1.5e-5, row.strength );
}

//--------------------------------------------------------------------------------------------------
/// Segment with wsegsicdData — 1 WSEGSICD row added; no WSEGVALV or WSEGAICD rows.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentWithWsegsicdData )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment seg = makeSegment( 5, 3 );

    WsegsicdRow sicd;
    sicd.well        = "TestWell";
    sicd.segment1    = 5;
    sicd.segment2    = 5;
    sicd.strength    = 2.0e-5;
    sicd.maxAbsRate  = 500.0;
    seg.wsegsicdData = sicd;

    exportData.branches = { makeBranch( 3, { seg } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 1u, result.welsegsData().size() );
    EXPECT_TRUE( result.wsegvalvData().empty() );
    EXPECT_TRUE( result.wsegaicdData().empty() );
    ASSERT_EQ( 1u, result.wsegsicdData().size() );

    const WsegsicdRow& row = result.wsegsicdData()[0];
    EXPECT_EQ( "TestWell", row.well );
    EXPECT_EQ( 5, row.segment1 );
    EXPECT_DOUBLE_EQ( 2.0e-5, row.strength );
}

//--------------------------------------------------------------------------------------------------
/// Multiple segments — WELSEGS count equals segment count;
/// COMPSEGS count equals total intersections across all segments.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, MultipleSegments )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment seg1 = makeSegment( 2, 1 );
    seg1.intersections = { RigMswCellIntersection{ 1, 1, 1, 0.0, 10.0, "" }, RigMswCellIntersection{ 1, 1, 2, 10.0, 20.0, "" } };

    RigMswSegment seg2 = makeSegment( 3, 2 );
    seg2.intersections = { RigMswCellIntersection{ 2, 1, 2, 20.0, 30.0, "" } };

    RigMswSegment seg3 = makeSegment( 4, 3 );
    // No intersections

    exportData.branches = { makeBranch( 1, { seg1, seg2, seg3 } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    EXPECT_EQ( 3u, result.welsegsData().size() );
    EXPECT_EQ( 3u, result.compsegsData().size() ); // 2 + 1 + 0
    ASSERT_EQ( 1u, result.mswBranches().size() );
    EXPECT_EQ( 3u, result.mswBranches()[0].segments.size() );
}

//--------------------------------------------------------------------------------------------------
/// Segment numbering and outlet segment — joinSegment in WelsegsRow matches outletSegmentNumber.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, OutletSegmentNumberMapping )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    // Simulate a chain: 2->1 (heel), 3->2, 4->3
    RigMswSegment seg2 = makeSegment( 2, 1 );
    RigMswSegment seg3 = makeSegment( 3, 2 );
    RigMswSegment seg4 = makeSegment( 4, 3 );

    exportData.branches = { makeBranch( 1, { seg2, seg3, seg4 } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 3u, result.welsegsData().size() );
    EXPECT_EQ( 2, result.welsegsData()[0].segment1 );
    EXPECT_EQ( 1, result.welsegsData()[0].joinSegment );

    EXPECT_EQ( 3, result.welsegsData()[1].segment1 );
    EXPECT_EQ( 2, result.welsegsData()[1].joinSegment );

    EXPECT_EQ( 4, result.welsegsData()[2].segment1 );
    EXPECT_EQ( 3, result.welsegsData()[2].joinSegment );
}

//--------------------------------------------------------------------------------------------------
/// segment1 == segment2 in each WELSEGS row (single-segment entries).
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, Segment1EqualsSegment2 )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    exportData.branches = { makeBranch( 1, { makeSegment( 2, 1 ) } ),
                            makeBranch( 2, { makeSegment( 5, 2 ) } ),
                            makeBranch( 3, { makeSegment( 9, 5 ) } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    for ( const auto& row : result.welsegsData() )
    {
        EXPECT_EQ( row.segment1, row.segment2 );
    }
}

//--------------------------------------------------------------------------------------------------
/// COMPSEGS branch number is inherited from the parent segment's branchNumber.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, CompsegsInheritsBranchFromSegment )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment seg   = makeSegment( 3, 2 ); // branch comes from the enclosing RigMswBranch (branchNumber=7)
    seg.intersections   = { RigMswCellIntersection{ 1, 2, 3, 10.0, 20.0, "" }, RigMswCellIntersection{ 1, 2, 4, 20.0, 30.0, "" } };
    exportData.branches = { makeBranch( 7, { seg } ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 2u, result.compsegsData().size() );
    for ( const auto& row : result.compsegsData() )
    {
        EXPECT_EQ( 7, row.branch );
    }
}

//--------------------------------------------------------------------------------------------------
/// Header fields (topDepth, topLength, infoType) are propagated into the result.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, HeaderFieldsPropagated )
{
    WelsegsHeader hdr;
    hdr.well      = "DeepWell";
    hdr.topDepth  = 1234.5;
    hdr.topLength = 2345.6;
    hdr.infoType  = "ABS";

    RigMswWellExportData exportData;
    exportData.header = hdr;

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_FIELD );

    EXPECT_EQ( "DeepWell", result.welsegsHeader().well );
    EXPECT_DOUBLE_EQ( 1234.5, result.welsegsHeader().topDepth );
    EXPECT_DOUBLE_EQ( 2345.6, result.welsegsHeader().topLength );
    EXPECT_EQ( "ABS", result.welsegsHeader().infoType );
    EXPECT_EQ( RiaDefines::EclipseUnitSystem::UNITS_FIELD, result.unitSystem() );
}

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
#include "CompletionExportCommands/RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswDataFormatter.h"
#include "CompletionsMsw/RigMswSegment.h"
#include "CompletionsMsw/RigMswTableData.h"
#include "CompletionsMsw/RigMswTableRows.h"

#include "RifTextDataTableFormatter.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaTestDataDirectory.h"

#include "RimEclipseCase.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimSegmentCollection.h"
#include "RimSegmentInterval.h"
#include "RimWellPath.h"
#include "RimWellPathTieIn.h"

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <optional>
#include <tuple>

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
RigMswBranch makeBranch( int branchNum, std::vector<RigMswSegment> segs, RigMswBranchSource source = RigMswBranchSource::Perforation )
{
    return RigMswBranch{ branchNum, std::nullopt, std::move( segs ), source };
}

//--------------------------------------------------------------------------------------------------
/// Build a branch with a single segment intersecting the given main grid cell.
//--------------------------------------------------------------------------------------------------
RigMswBranch makeBranchIntersectingCell( int branchNum, int segNum, size_t i, size_t j, size_t k, RigMswBranchSource source )
{
    RigMswSegment seg = makeSegment( segNum, 1 );
    seg.intersections = { RigMswCellIntersection{ i, j, k, 100.0, 110.0, "" } };
    return makeBranch( branchNum, { seg }, source );
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

namespace
{
//--------------------------------------------------------------------------------------------------
/// The data rows of a formatted COMPSEGS (or COMPSEGL) table, split into items
//--------------------------------------------------------------------------------------------------
std::vector<QStringList> formattedCompsegsRows( const RigMswTableData& tableData, bool isLgrData )
{
    QString                   text;
    QTextStream               stream( &text );
    RifTextDataTableFormatter formatter( stream );
    RigMswDataFormatter::formatCompsegsTable( formatter, tableData, isLgrData );

    std::vector<QStringList> rows;
    bool                     isWellNameRow = true;
    for ( const QString& line : text.split( '\n' ) )
    {
        QStringList items = line.simplified().split( ' ', Qt::SkipEmptyParts );
        if ( items.isEmpty() || items.front().startsWith( "--" ) || items.front() == "COMPSEGS" || items.front() == "COMPSEGL" ||
             items.front() == "/" )
            continue;

        if ( isWellNameRow )
        {
            isWellNameRow = false;
            continue;
        }
        if ( items.back() == "/" ) items.removeLast();
        rows.push_back( items );
    }
    return rows;
}

//--------------------------------------------------------------------------------------------------
/// Two segments, each intersecting one main grid cell, and one LGR cell on the second segment
//--------------------------------------------------------------------------------------------------
RigMswWellExportData exportDataWithTwoConnectedSegments()
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment first  = makeSegment( 2, 1 );
    first.intersections  = { RigMswCellIntersection{ 3, 5, 7, 100.0, 110.0, "" } };
    RigMswSegment second = makeSegment( 3, 2 );
    second.intersections = { RigMswCellIntersection{ 3, 5, 8, 110.0, 120.0, "" }, RigMswCellIntersection{ 1, 2, 3, 120.0, 125.0, "LGR_1" } };

    exportData.branches = { makeBranch( 1, { first, second } ) };
    return exportData;
}
} // anonymous namespace

//--------------------------------------------------------------------------------------------------
/// By default the segment number is left to the simulator, and COMPSEGS ends at the end length
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, CompsegsSegmentNumber_NotExportedByDefault )
{
    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportDataWithTwoConnectedSegments(),
                                                                      RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 3u, result.compsegsData().size() );
    for ( const auto& row : result.compsegsData() )
    {
        EXPECT_FALSE( row.segmentNumber.has_value() );
    }

    const auto rows = formattedCompsegsRows( result, false );
    ASSERT_EQ( 2u, rows.size() );
    for ( const auto& row : rows )
    {
        EXPECT_EQ( 6, row.size() ) << row.join( ' ' ).toStdString();
    }
}

//--------------------------------------------------------------------------------------------------
/// With exported segment numbers, ISEG is written after four defaulted items
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, CompsegsSegmentNumber_Exported )
{
    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportDataWithTwoConnectedSegments(),
                                                                      RiaDefines::EclipseUnitSystem::UNITS_METRIC,
                                                                      true );

    ASSERT_EQ( 3u, result.compsegsData().size() );
    EXPECT_EQ( std::optional<int>( 2 ), result.compsegsData()[0].segmentNumber );
    EXPECT_EQ( std::optional<int>( 3 ), result.compsegsData()[1].segmentNumber );
    EXPECT_EQ( std::optional<int>( 3 ), result.compsegsData()[2].segmentNumber );

    const auto rows = formattedCompsegsRows( result, false );
    ASSERT_EQ( 2u, rows.size() );
    const std::vector<QString> expectedSegments = { "2", "3" };
    for ( size_t r = 0; r < rows.size(); ++r )
    {
        const auto& row = rows[r];
        ASSERT_EQ( 11, row.size() ) << row.join( ' ' ).toStdString();
        EXPECT_EQ( "3", row[0] );
        EXPECT_EQ( "5", row[1] );
        EXPECT_EQ( "1", row[3] );
        for ( int item = 6; item < 10; ++item )
        {
            EXPECT_EQ( "1*", row[item] );
        }
        EXPECT_EQ( expectedSegments[r], row[10] );
    }

    const auto lgrRows = formattedCompsegsRows( result, true );
    ASSERT_EQ( 1u, lgrRows.size() );
    ASSERT_EQ( 12, lgrRows[0].size() ) << lgrRows[0].join( ' ' ).toStdString();
    EXPECT_EQ( "LGR_1", lgrRows[0][0] );
    EXPECT_EQ( "3", lgrRows[0][11] );
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

//--------------------------------------------------------------------------------------------------
/// A grid cell has a single COMPDAT connection and must be connected to one segment only. When two
/// branches of the same kind intersect the cell, the branch listed first keeps it.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, CompsegsCellIsConnectedOnlyOnce )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    exportData.branches = { makeBranchIntersectingCell( 2, 10, 3, 5, 7, RigMswBranchSource::Fracture ),
                            makeBranchIntersectingCell( 3, 11, 3, 5, 7, RigMswBranchSource::Fracture ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 2u, result.welsegsData().size() );
    ASSERT_EQ( 1u, result.compsegsData().size() );
    EXPECT_EQ( 2, result.compsegsData()[0].branch );
}

//--------------------------------------------------------------------------------------------------
/// The branch carrying most flow claims a shared cell: perforations before fishbones before
/// fractures, regardless of the order the branches are listed in.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, CompsegsCellIsClaimedByTheBranchWithMostFlow )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    exportData.branches = { makeBranchIntersectingCell( 4, 12, 3, 5, 7, RigMswBranchSource::Fracture ),
                            makeBranchIntersectingCell( 3, 11, 3, 5, 7, RigMswBranchSource::Fishbones ),
                            makeBranchIntersectingCell( 2, 10, 3, 5, 7, RigMswBranchSource::Perforation ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 1u, result.compsegsData().size() );
    EXPECT_EQ( 2, result.compsegsData()[0].branch );
}

//--------------------------------------------------------------------------------------------------
/// The same IJK in the main grid and in an LGR are different cells, and both are connected.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, CompsegsSeparatesMainGridAndLgrCells )
{
    RigMswWellExportData exportData;
    exportData.header = makeHeader();

    RigMswSegment lgrSeg = makeSegment( 11, 1 );
    lgrSeg.intersections = { RigMswCellIntersection{ 3, 5, 7, 100.0, 110.0, "LGR_NEAR_WELL" } };

    exportData.branches = { makeBranchIntersectingCell( 2, 10, 3, 5, 7, RigMswBranchSource::Fracture ),
                            makeBranch( 3, { lgrSeg }, RigMswBranchSource::Fracture ) };

    auto result = RicWellPathExportMswGeometryPath::collectTableData( exportData, RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    ASSERT_EQ( 2u, result.compsegsData().size() );
    EXPECT_TRUE( result.compsegsData()[0].isMainGrid() );
    EXPECT_EQ( "LGR_NEAR_WELL", result.compsegsData()[1].gridName );
}

//==================================================================================================
// Branch numbering, using the multiple_laterals test project.
//
// Well-A is a main bore with an ICD-valved perforation interval and two well path laterals tied in
// (Y2 and Y3). The main bore and the laterals must be given the lowest branch numbers, and the
// completion branches must follow.
//==================================================================================================

namespace
{

//--------------------------------------------------------------------------------------------------
/// The single Eclipse case of the multiple_laterals project, and one of its well paths.
//--------------------------------------------------------------------------------------------------
struct MswExportInput
{
    RimEclipseCase* eclipseCase = nullptr;
    RimWellPath*    wellPath    = nullptr;
};

//--------------------------------------------------------------------------------------------------
/// The project is a global object shared by all tests, and must be closed before the test completes.
/// The tests below leave early on a failing assertion, so close from a destructor.
//--------------------------------------------------------------------------------------------------
struct ProjectCloser
{
    ~ProjectCloser() { RiaApplication::instance()->closeProject(); }
};

//--------------------------------------------------------------------------------------------------
/// Load the multiple_laterals project and look up the well path with the given name.
/// Members are left as nullptr if the project, the case or the well path could not be found.
//--------------------------------------------------------------------------------------------------
MswExportInput loadMultipleLateralsProject( const QString& wellPathName )
{
    MswExportInput input;

    QDir projectFolder( TEST_MODEL_DIR );
    if ( !projectFolder.cd( "msw-export/project-files" ) ) return input;

    const QString projectFileName = projectFolder.absoluteFilePath( "multiple_laterals.rsp" );
    if ( !QFile::exists( projectFileName ) ) return input;

    if ( !RiaApplication::instance()->loadProject( projectFileName ) ) return input;

    RimProject* project = RimProject::current();
    if ( !project ) return input;

    const auto eclipseCases = project->eclipseCases();
    if ( !eclipseCases.empty() ) input.eclipseCase = eclipseCases.front();

    for ( auto* wellPath : project->allWellPaths() )
    {
        if ( wellPath && wellPath->name() == wellPathName ) input.wellPath = wellPath;
    }

    return input;
}

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
/// The main bore and the well path laterals occupy branch 1..N, and every completion branch is
/// numbered above them.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, MultipleLaterals_LateralsNumberedBeforeCompletionBranches )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    auto tableData = RicWellPathExportMswTableData::extractSingleWellMswData( input.eclipseCase, input.wellPath );
    ASSERT_TRUE( tableData.has_value() );

    // Branch 1 is the main bore, branches 2 and 3 are the laterals Y2 and Y3. The three ICD
    // completion branches follow as 4, 5 and 6.
    std::map<int, std::string> descriptionOfFirstSegmentInBranch;
    for ( const auto& branch : tableData->mswBranches() )
    {
        if ( !branch.segments.empty() ) descriptionOfFirstSegmentInBranch[branch.branchNumber] = branch.segments.front().description;
    }

    ASSERT_EQ( 6u, descriptionOfFirstSegmentInBranch.size() );
    EXPECT_EQ( "Segments on main bore", descriptionOfFirstSegmentInBranch[1] );
    EXPECT_EQ( "Segments on lateral Well-A Y2", descriptionOfFirstSegmentInBranch[2] );
    EXPECT_EQ( "Segments on lateral Well-A Y3", descriptionOfFirstSegmentInBranch[3] );
    for ( int branchNumber : { 4, 5, 6 } )
    {
        EXPECT_EQ( "1 ICD: 3100 - 3800 #1", descriptionOfFirstSegmentInBranch[branchNumber] );
    }
}

//--------------------------------------------------------------------------------------------------
/// The completion branches of a lateral are listed immediately after that lateral, even though
/// their branch numbers are higher than the branch numbers of the laterals listed later.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, MultipleLaterals_CompletionBranchesListedAfterTheirLateral )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    auto tableData = RicWellPathExportMswTableData::extractSingleWellMswData( input.eclipseCase, input.wellPath );
    ASSERT_TRUE( tableData.has_value() );

    // The branch number of each WELSEGS row, in the order the rows are written to the export file.
    std::vector<int> branchNumbersInListedOrder;
    for ( const auto& row : tableData->welsegsData() )
    {
        if ( branchNumbersInListedOrder.empty() || branchNumbersInListedOrder.back() != row.branch )
            branchNumbersInListedOrder.push_back( row.branch );
    }

    // Main bore, its three ICD branches, then lateral Y2 and lateral Y3.
    const std::vector<int> expected = { 1, 4, 5, 6, 2, 3 };
    EXPECT_EQ( expected, branchNumbersInListedOrder );
}

//--------------------------------------------------------------------------------------------------
/// COMPSEGS is ordered by branch number, unlike WELSEGS.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, MultipleLaterals_CompsegsOrderedByBranchNumber )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    auto tableData = RicWellPathExportMswTableData::extractSingleWellMswData( input.eclipseCase, input.wellPath );
    ASSERT_TRUE( tableData.has_value() );

    ASSERT_FALSE( tableData->compsegsData().empty() );

    std::vector<int> branchNumbers;
    for ( const auto& row : tableData->compsegsData() )
    {
        branchNumbers.push_back( row.branch );
    }

    EXPECT_TRUE( std::is_sorted( branchNumbers.begin(), branchNumbers.end() ) );

    // The lateral Y2 rows come before the ICD rows of the main bore, even though the ICD segments are
    // listed first in WELSEGS.
    EXPECT_LT( branchNumbers.front(), branchNumbers.back() );
}

//==================================================================================================
// Segmentation rules on segment intervals, using lateral Y2 of the multiple_laterals test project.
//==================================================================================================

namespace
{

//--------------------------------------------------------------------------------------------------
/// The segment nodes and COMPSEGS rows of one well path branch in the export
//--------------------------------------------------------------------------------------------------
struct BranchExport
{
    std::vector<double>                              nodeMDs;
    std::vector<std::vector<RigMswCellIntersection>> intersectionsPerSegment;
};

//--------------------------------------------------------------------------------------------------
/// Export Well-A Y1 and pick the branch of the given lateral. The node MD is written directly for
/// ABS, and relative to the outlet node for INC.
//--------------------------------------------------------------------------------------------------
std::optional<BranchExport> exportLateralBranch( const MswExportInput& input, const RimWellPath* lateral )
{
    auto tableData = RicWellPathExportMswTableData::extractSingleWellMswData( input.eclipseCase, input.wellPath );
    if ( !tableData ) return std::nullopt;

    const bool   isIncremental = tableData->welsegsHeader().infoType == "INC";
    const double tieInMD       = lateral->wellPathTieIn()->tieInMeasuredDepth();
    const auto   description   = QString( "Segments on lateral %1" ).arg( lateral->name() ).toStdString();

    for ( const auto& branch : tableData->mswBranches() )
    {
        if ( branch.segments.empty() || branch.segments.front().description != description ) continue;

        BranchExport result;
        double       outletMD = tieInMD;
        for ( const auto& segment : branch.segments )
        {
            const double nodeMD = isIncremental ? outletMD + segment.length : segment.length;
            result.nodeMDs.push_back( nodeMD );
            result.intersectionsPerSegment.push_back( segment.intersections );
            outletMD = nodeMD;
        }
        return result;
    }
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
/// All COMPSEGS rows of the branch as (I, J, K, start, end), in export order
//--------------------------------------------------------------------------------------------------
std::vector<std::tuple<size_t, size_t, size_t, double, double>> compsegsRows( const BranchExport& branch )
{
    std::vector<std::tuple<size_t, size_t, size_t, double, double>> rows;
    for ( const auto& intersections : branch.intersectionsPerSegment )
    {
        for ( const auto& ci : intersections )
        {
            rows.emplace_back( ci.i, ci.j, ci.k, ci.distanceStart, ci.distanceEnd );
        }
    }
    return rows;
}

//--------------------------------------------------------------------------------------------------
/// Every COMPSEGS row sits on the segment whose node is nearest the row centre, as in the simulator
//--------------------------------------------------------------------------------------------------
void expectRowsOnNearestNode( const BranchExport& branch )
{
    for ( size_t segmentIndex = 0; segmentIndex < branch.intersectionsPerSegment.size(); ++segmentIndex )
    {
        for ( const auto& ci : branch.intersectionsPerSegment[segmentIndex] )
        {
            const double centre   = 0.5 * ( ci.distanceStart + ci.distanceEnd );
            const double distance = std::abs( centre - branch.nodeMDs[segmentIndex] );
            for ( size_t other = 0; other < branch.nodeMDs.size(); ++other )
            {
                if ( other < segmentIndex )
                    EXPECT_LT( distance, std::abs( centre - branch.nodeMDs[other] ) );
                else
                    EXPECT_LE( distance, std::abs( centre - branch.nodeMDs[other] ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Add a segment interval covering the whole lateral, with the default diameter and roughness
//--------------------------------------------------------------------------------------------------
RimSegmentInterval* addSegmentIntervalCoveringWell( RimWellPath* wellPath )
{
    auto* segments = wellPath->segmentCollection();
    return segments->createInterval( 0.0, 1.0e5, segments->linerDiameter(), segments->roughnessFactor() );
}

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
/// With a min segment length, segments span several cells. The node spacing is at least the min
/// length, and every perforated cell is still connected exactly once, to its nearest node.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentInterval_MinSegmentLengthMergesCells )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    RimWellPath* lateral = nullptr;
    for ( auto* wellPath : RimProject::current()->allWellPaths() )
    {
        if ( wellPath->name() == "Well-A Y2" ) lateral = wellPath;
    }
    ASSERT_TRUE( lateral != nullptr );

    auto* interval = addSegmentIntervalCoveringWell( lateral );

    const auto baseline = exportLateralBranch( input, lateral );
    ASSERT_TRUE( baseline.has_value() );
    ASSERT_GT( baseline->nodeMDs.size(), 3u );
    ASSERT_FALSE( compsegsRows( *baseline ).empty() );

    const double tieInMD      = lateral->wellPathTieIn()->tieInMeasuredDepth();
    const double branchLength = baseline->nodeMDs.back() - tieInMD;
    const double minLength    = 3.0 * branchLength / baseline->nodeMDs.size();

    interval->setMinSegmentLength( minLength );
    const auto merged = exportLateralBranch( input, lateral );
    ASSERT_TRUE( merged.has_value() );

    EXPECT_LT( merged->nodeMDs.size(), baseline->nodeMDs.size() );

    double previousMD = tieInMD;
    for ( double nodeMD : merged->nodeMDs )
    {
        EXPECT_GE( nodeMD - previousMD, minLength - 1.0e-6 );
        previousMD = nodeMD;
    }

    // Nodes are kept at cell centres
    for ( double nodeMD : merged->nodeMDs )
    {
        EXPECT_TRUE( std::ranges::any_of( baseline->nodeMDs, [nodeMD]( double md ) { return std::abs( md - nodeMD ) < 1.0e-6; } ) );
    }

    EXPECT_EQ( compsegsRows( *baseline ), compsegsRows( *merged ) );

    const size_t segmentsWithSeveralCells =
        std::ranges::count_if( merged->intersectionsPerSegment, []( const auto& intersections ) { return intersections.size() > 1; } );
    EXPECT_GT( segmentsWithSeveralCells, 0u );

    expectRowsOnNearestNode( *merged );
}

//--------------------------------------------------------------------------------------------------
/// With a max segment length, nodes are inserted until the node spacing is at most the max length.
/// The COMPSEGS rows are unchanged, and each sits on its nearest node.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentInterval_MaxSegmentLengthInsertsNodes )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    RimWellPath* lateral = nullptr;
    for ( auto* wellPath : RimProject::current()->allWellPaths() )
    {
        if ( wellPath->name() == "Well-A Y2" ) lateral = wellPath;
    }
    ASSERT_TRUE( lateral != nullptr );

    auto* interval = addSegmentIntervalCoveringWell( lateral );

    const auto baseline = exportLateralBranch( input, lateral );
    ASSERT_TRUE( baseline.has_value() );

    const double tieInMD    = lateral->wellPathTieIn()->tieInMeasuredDepth();
    double       maxSpacing = 0.0;
    double       previousMD = tieInMD;
    for ( double nodeMD : baseline->nodeMDs )
    {
        maxSpacing = std::max( maxSpacing, nodeMD - previousMD );
        previousMD = nodeMD;
    }
    const double maxLength = 0.4 * maxSpacing;

    interval->setMaxSegmentLength( maxLength );
    const auto refined = exportLateralBranch( input, lateral );
    ASSERT_TRUE( refined.has_value() );

    EXPECT_GT( refined->nodeMDs.size(), baseline->nodeMDs.size() );

    previousMD = tieInMD;
    for ( double nodeMD : refined->nodeMDs )
    {
        EXPECT_LE( nodeMD - previousMD, maxLength + 1.0e-6 );
        EXPECT_GT( nodeMD - previousMD, 0.0 );
        previousMD = nodeMD;
    }

    // All cell centre nodes are kept
    for ( double nodeMD : baseline->nodeMDs )
    {
        EXPECT_TRUE( std::ranges::any_of( refined->nodeMDs, [nodeMD]( double md ) { return std::abs( md - nodeMD ) < 1.0e-6; } ) );
    }

    EXPECT_EQ( compsegsRows( *baseline ), compsegsRows( *refined ) );
    expectRowsOnNearestNode( *refined );
}

//--------------------------------------------------------------------------------------------------
/// A segment interval without a segmentation rule leaves the export unchanged
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentInterval_NoRuleGivesUnchangedExport )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    RimWellPath* lateral = nullptr;
    for ( auto* wellPath : RimProject::current()->allWellPaths() )
    {
        if ( wellPath->name() == "Well-A Y2" ) lateral = wellPath;
    }
    ASSERT_TRUE( lateral != nullptr );

    auto*      interval = addSegmentIntervalCoveringWell( lateral );
    const auto before   = exportLateralBranch( input, lateral );
    ASSERT_TRUE( before.has_value() );

    interval->setMinSegmentLength( 50.0 );
    interval->setMinSegmentLength( std::nullopt );
    const auto after = exportLateralBranch( input, lateral );
    ASSERT_TRUE( after.has_value() );

    EXPECT_EQ( before->nodeMDs, after->nodeMDs );
    EXPECT_EQ( compsegsRows( *before ), compsegsRows( *after ) );
}

namespace
{
RimWellPath* findLateralY2()
{
    for ( auto* wellPath : RimProject::current()->allWellPaths() )
    {
        if ( wellPath->name() == "Well-A Y2" ) return wellPath;
    }
    return nullptr;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A fixed segment length gives equally long segments from the interval start, the last one is cut
/// at the end of the branch. Every perforated cell is still connected exactly once.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentInterval_FixedSegmentLength )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    RimWellPath* lateral = findLateralY2();
    ASSERT_TRUE( lateral != nullptr );

    auto* interval = addSegmentIntervalCoveringWell( lateral );

    const auto baseline = exportLateralBranch( input, lateral );
    ASSERT_TRUE( baseline.has_value() );
    ASSERT_GT( baseline->nodeMDs.size(), 3u );

    const double tieInMD      = lateral->wellPathTieIn()->tieInMeasuredDepth();
    const double branchLength = baseline->nodeMDs.back() - tieInMD;
    const double fixedLength  = branchLength / 4.0;

    interval->setFixedSegmentLength( fixedLength );
    const auto fixedExport = exportLateralBranch( input, lateral );
    ASSERT_TRUE( fixedExport.has_value() );

    // Nodes of full segments are one fixed length apart, the first and last segments are cut by the branch ends
    ASSERT_GE( fixedExport->nodeMDs.size(), 4u );
    for ( size_t k = 1; k + 2 < fixedExport->nodeMDs.size(); ++k )
    {
        EXPECT_NEAR( fixedExport->nodeMDs[k + 1] - fixedExport->nodeMDs[k], fixedLength, 1.0e-6 ) << "node " << k;
    }

    EXPECT_EQ( compsegsRows( *baseline ), compsegsRows( *fixedExport ) );
    expectRowsOnNearestNode( *fixedExport );
}

//--------------------------------------------------------------------------------------------------
/// Min and max segment length can be combined. Cells are merged to the min length, then segments longer
/// than the max length are split.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SegmentInterval_MinAndMaxSegmentLengthCombined )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    RimWellPath* lateral = findLateralY2();
    ASSERT_TRUE( lateral != nullptr );

    auto* interval = addSegmentIntervalCoveringWell( lateral );

    const auto baseline = exportLateralBranch( input, lateral );
    ASSERT_TRUE( baseline.has_value() );
    ASSERT_GT( baseline->nodeMDs.size(), 3u );

    const double tieInMD   = lateral->wellPathTieIn()->tieInMeasuredDepth();
    const double minLength = 3.0 * ( baseline->nodeMDs.back() - tieInMD ) / baseline->nodeMDs.size();
    const double maxLength = 3.0 * minLength;

    interval->setMinSegmentLength( minLength );
    interval->setMaxSegmentLength( maxLength );
    ASSERT_EQ( std::optional<double>( minLength ), interval->minSegmentLength() );
    ASSERT_EQ( std::optional<double>( maxLength ), interval->maxSegmentLength() );

    const auto combined = exportLateralBranch( input, lateral );
    ASSERT_TRUE( combined.has_value() );

    double previousMD = tieInMD;
    for ( double nodeMD : combined->nodeMDs )
    {
        EXPECT_GE( nodeMD - previousMD, minLength - 1.0e-6 );
        EXPECT_LE( nodeMD - previousMD, maxLength + 1.0e-6 );
        previousMD = nodeMD;
    }

    EXPECT_EQ( compsegsRows( *baseline ), compsegsRows( *combined ) );
    expectRowsOnNearestNode( *combined );
}

//--------------------------------------------------------------------------------------------------
/// With a single segment before the first and after the last perforation, each region has at most one
/// node. The nodes inside the perforated region are unchanged.
//--------------------------------------------------------------------------------------------------
TEST( RicWellPathExportMswGeometryPath, SingleSegmentOutsidePerforations )
{
    ProjectCloser projectCloser;

    auto input = loadMultipleLateralsProject( "Well-A Y1" );
    ASSERT_TRUE( input.eclipseCase != nullptr );
    ASSERT_TRUE( input.wellPath != nullptr );

    RimWellPath* lateral = findLateralY2();
    ASSERT_TRUE( lateral != nullptr );

    const auto baseline = exportLateralBranch( input, lateral );
    ASSERT_TRUE( baseline.has_value() );

    double firstPerforationMD = std::numeric_limits<double>::infinity();
    double lastPerforationMD  = -std::numeric_limits<double>::infinity();
    for ( const auto* perf : lateral->perforationIntervalCollection()->activePerforations() )
    {
        firstPerforationMD = std::min( firstPerforationMD, perf->startMD() );
        lastPerforationMD  = std::max( lastPerforationMD, perf->endMD() );
    }
    ASSERT_LT( firstPerforationMD, lastPerforationMD );

    const auto countBefore = [&]( const std::vector<double>& nodes )
    { return std::ranges::count_if( nodes, [&]( double md ) { return md < firstPerforationMD; } ); };
    const auto countAfter = [&]( const std::vector<double>& nodes )
    { return std::ranges::count_if( nodes, [&]( double md ) { return md > lastPerforationMD; } ); };

    // The lateral takes the settings from the top level well
    for ( auto* wellPath : { input.wellPath, lateral } )
    {
        wellPath->segmentCollection()->setSingleSegmentBeforeFirstPerforation( true );
        wellPath->segmentCollection()->setSingleSegmentAfterLastPerforation( true );
    }

    const auto collapsed = exportLateralBranch( input, lateral );
    ASSERT_TRUE( collapsed.has_value() );

    EXPECT_LE( countBefore( collapsed->nodeMDs ), 1 );
    EXPECT_LE( countAfter( collapsed->nodeMDs ), 1 );
    EXPECT_LE( collapsed->nodeMDs.size(), baseline->nodeMDs.size() );

    const auto insideCount = [&]( const std::vector<double>& nodes )
    { return std::ranges::count_if( nodes, [&]( double md ) { return md >= firstPerforationMD && md <= lastPerforationMD; } ); };
    EXPECT_EQ( insideCount( baseline->nodeMDs ), insideCount( collapsed->nodeMDs ) );

    EXPECT_EQ( compsegsRows( *baseline ), compsegsRows( *collapsed ) );
}

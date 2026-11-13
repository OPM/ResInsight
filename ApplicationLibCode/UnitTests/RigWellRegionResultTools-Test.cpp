/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiaTestDataDirectory.h"

#include "RifReaderOpmCommon.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigMdRange.h"
#include "RigWellRegionResultTools.h"

#include "Well/RigSimWellData.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include <QDir>

#include <algorithm>
#include <cmath>

namespace
{
const QString resultName = "WELL_REGION";

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigWellRegionResultToolsTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        QDir    baseFolder( TEST_MODEL_DIR );
        QString filename = baseFolder.absoluteFilePath( "TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID" );

        s_caseData = new RigEclipseCaseData( nullptr );
        s_reader   = new RifReaderOpmCommon;
        ASSERT_TRUE( s_reader->open( filename, s_caseData.p() ) );
        s_caseData->mainGrid()->computeCachedData();
        results()->setReaderInterface( s_reader.p() );

        // Same setup as RimEclipseCase, required for correct well path intersections
        std::vector<size_t> activeCellIndices;
        for ( const auto& idx : results()->activeCellInfo()->activeReservoirCellIndices() )
        {
            activeCellIndices.push_back( idx.value() );
        }
        s_caseData->mainGrid()->computeCharacteristicCellSize( activeCellIndices );
        s_caseData->mainGrid()->computeFaceNormalsDirection( activeCellIndices );
    }

    static void TearDownTestSuite()
    {
        s_caseData = nullptr;
        s_reader   = nullptr;
    }

    static RigCaseCellResultsData* results() { return s_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL ); }

    static size_t lastTimeStep() { return results()->maxTimeStepCount() - 1; }

    static size_t reservoirCellIndex( size_t gridIndex, size_t i, size_t j, size_t k )
    {
        const RigGridBase* grid = s_caseData->mainGrid()->gridByIndex( gridIndex );
        return grid->reservoirCellIndex( grid->cellIndexFromIJK( i, j, k ) );
    }

    // Returns the result value of the cell, or -1 for an inactive cell
    static double value( size_t reservoirCellIdx )
    {
        const RigActiveCellInfo* activeCellInfo = results()->activeCellInfo();
        if ( !activeCellInfo->isActive( ReservoirCellIndex( reservoirCellIdx ) ) ) return -1.0;

        const auto& values = results()->cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::GENERATED, resultName ), 0 );
        return values[activeCellInfo->cellResultIndex( ReservoirCellIndex( reservoirCellIdx ) ).value()];
    }

    static double value( size_t gridIndex, size_t i, size_t j, size_t k ) { return value( reservoirCellIndex( gridIndex, i, j, k ) ); }

    static const std::vector<double>& allValues()
    {
        return results()->cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::GENERATED, resultName ), 0 );
    }

    static std::vector<size_t> connectionCells( const QString& wellName )
    {
        std::vector<size_t> cells;
        const auto*         simWell = s_caseData->findSimWellData( wellName );
        for ( const auto& point : simWell->wellResultFrame( lastTimeStep() )->allResultPoints() )
        {
            if ( point.isCell() )
                cells.push_back( s_caseData->mainGrid()->reservoirCellIndexByGridAndGridLocalCellIndex( point.gridIndex(), point.cellIndex() ) );
        }
        return cells;
    }

    // Small open square polygon around the given point
    static std::vector<cvf::Vec3d> squareAround( const cvf::Vec3d& center, double halfSize )
    {
        return { center + cvf::Vec3d( -halfSize, -halfSize, 0 ),
                 center + cvf::Vec3d( halfSize, -halfSize, 0 ),
                 center + cvf::Vec3d( halfSize, halfSize, 0 ),
                 center + cvf::Vec3d( -halfSize, halfSize, 0 ) };
    }

    // A corner shared by the two cells
    static cvf::Vec3d sharedCorner( size_t cellA, size_t cellB )
    {
        const auto cornersB = s_caseData->mainGrid()->cellCornerVertices( cellB );
        for ( const auto& cornerA : s_caseData->mainGrid()->cellCornerVertices( cellA ) )
        {
            for ( const auto& cornerB : cornersB )
            {
                if ( ( cornerA - cornerB ).length() < 1.0e-6 ) return cornerA;
            }
        }
        return cvf::Vec3d::UNDEFINED;
    }

    // Polygon around the LGR1 node shared by cells (5,5) and (6,6), inside main grid host cell (36,61)
    static std::vector<cvf::Vec3d> lgrPolygon()
    {
        return squareAround( sharedCorner( reservoirCellIndex( 1, 5, 5, 14 ), reservoirCellIndex( 1, 6, 6, 14 ) ), 1.0 );
    }

    static std::vector<cvf::Vec3d> polygonOutsideGrid() { return squareAround( cvf::Vec3d( 20000, 20000, 0 ), 100.0 ); }

    static void expectBinaryValues()
    {
        for ( double v : allValues() )
        {
            EXPECT_TRUE( v == 0.0 || v == 1.0 );
        }
    }

    static cvf::ref<RigEclipseCaseData> s_caseData;
    static cvf::ref<RifReaderOpmCommon> s_reader;
};

cvf::ref<RigEclipseCaseData> RigWellRegionResultToolsTest::s_caseData;
cvf::ref<RifReaderOpmCommon> RigWellRegionResultToolsTest::s_reader;

} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, InvalidInput )
{
    const auto polygon = polygonOutsideGrid();

    EXPECT_FALSE( RigWellRegionResultTools::createWellRegionResult( nullptr, polygon, nullptr, std::nullopt, 0, resultName ) );
    EXPECT_FALSE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), { polygon[0], polygon[1] }, nullptr, std::nullopt, 0, resultName ) );
    EXPECT_FALSE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep() + 1, resultName ) );
}

//--------------------------------------------------------------------------------------------------
/// The polygon touches only the first connection cell of GI1. All connection cells of GI1 are marked,
/// also the ones far outside the polygon, while other wells are not selected.
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, SelectsCompleteSimWell )
{
    const size_t firstCell  = reservoirCellIndex( 0, 31, 63, 14 );
    const size_t cellBelow  = reservoirCellIndex( 0, 31, 63, 15 );
    const auto   polygon    = squareAround( sharedCorner( firstCell, cellBelow ), 1.0 );
    const size_t farGi1Cell = reservoirCellIndex( 0, 29, 44, 17 );

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep(), resultName, 0 ) );

    const auto gi1Cells = connectionCells( "GI1" );
    ASSERT_FALSE( gi1Cells.empty() );
    ASSERT_NE( std::find( gi1Cells.begin(), gi1Cells.end(), farGi1Cell ), gi1Cells.end() );

    for ( size_t cellIdx : gi1Cells )
    {
        EXPECT_EQ( 1.0, value( cellIdx ) );
    }

    // Cells inside the polygon are marked even if they are not well cells
    ASSERT_EQ( gi1Cells.end(), std::find( gi1Cells.begin(), gi1Cells.end(), cellBelow ) );
    EXPECT_EQ( 1.0, value( cellBelow ) );

    EXPECT_EQ( 0.0, value( 0, 26, 40, 14 ) ); // GP1
    EXPECT_EQ( 0.0, value( 1, 5, 5, 14 ) ); // GP2
    expectBinaryValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, LgrWellAndGrowth )
{
    const auto   gp2Cells     = connectionCells( "GP2" );
    const size_t hostCell     = reservoirCellIndex( 0, 36, 61, 14 );
    const size_t lgrNeighbor  = reservoirCellIndex( 1, 4, 5, 14 );
    const size_t twoStepsAway = reservoirCellIndex( 1, 3, 5, 14 );

    ASSERT_FALSE( gp2Cells.empty() );
    for ( size_t cellIdx : { hostCell, lgrNeighbor, twoStepsAway } )
    {
        ASSERT_TRUE( results()->activeCellInfo()->isActive( ReservoirCellIndex( cellIdx ) ) );
    }

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), lgrPolygon(), nullptr, std::nullopt, lastTimeStep(), resultName, 0 ) );
    for ( size_t cellIdx : gp2Cells )
    {
        EXPECT_EQ( 1.0, value( cellIdx ) );
    }
    EXPECT_EQ( 0.0, value( hostCell ) );
    EXPECT_EQ( 0.0, value( lgrNeighbor ) );
    EXPECT_EQ( 0.0, value( twoStepsAway ) );
    EXPECT_EQ( 1.0, value( 1, 6, 6, 14 ) ); // Inside the polygon, not a well cell
    expectBinaryValues();

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), lgrPolygon(), nullptr, std::nullopt, lastTimeStep(), resultName, 1 ) );
    for ( size_t cellIdx : gp2Cells )
    {
        EXPECT_EQ( 1.0, value( cellIdx ) );
    }
    EXPECT_EQ( 1.0, value( hostCell ) );
    EXPECT_EQ( 1.0, value( lgrNeighbor ) );
    EXPECT_EQ( 0.0, value( twoStepsAway ) );
    expectBinaryValues();
}

//--------------------------------------------------------------------------------------------------
/// Grown well cells cover the full K range of their columns, and the lateral neighbor columns.
/// Without growth only the connection cells are marked.
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, GrownRegionCoversFullKRange )
{
    const auto polygon = squareAround( sharedCorner( reservoirCellIndex( 0, 31, 63, 14 ), reservoirCellIndex( 0, 31, 63, 15 ) ), 1.0 );

    auto activeCellsInColumn = [&]( size_t i, size_t j )
    {
        std::vector<size_t> cells;
        for ( size_t k = 0; k < s_caseData->mainGrid()->cellCountK(); k++ )
        {
            const size_t cellIdx = reservoirCellIndex( 0, i, j, k );
            if ( results()->activeCellInfo()->isActive( ReservoirCellIndex( cellIdx ) ) ) cells.push_back( cellIdx );
        }
        return cells;
    };

    // Column of a GI1 connection far from the polygon, and a lateral neighbor column
    const auto column         = activeCellsInColumn( 29, 44 );
    const auto neighborColumn = activeCellsInColumn( 28, 44 );
    ASSERT_GT( column.size(), 1u );
    ASSERT_FALSE( neighborColumn.empty() );
    const size_t farTop    = column.front();
    const size_t farBottom = column.back();
    const size_t neighbor  = neighborColumn.front();

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep(), resultName, 0 ) );
    EXPECT_EQ( 0.0, value( farTop ) );
    EXPECT_EQ( 0.0, value( farBottom ) );

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep(), resultName, 1 ) );
    EXPECT_EQ( 1.0, value( farTop ) );
    EXPECT_EQ( 1.0, value( farBottom ) );
    EXPECT_EQ( 1.0, value( neighbor ) );
    expectBinaryValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, GrowthWithoutKFillStaysInPerforatedKRange )
{
    const auto polygon = squareAround( sharedCorner( reservoirCellIndex( 0, 31, 63, 14 ), reservoirCellIndex( 0, 31, 63, 15 ) ), 1.0 );

    // Column of a GI1 connection far from the polygon
    std::vector<size_t> column;
    for ( size_t k = 0; k < s_caseData->mainGrid()->cellCountK(); k++ )
    {
        const size_t cellIdx = reservoirCellIndex( 0, 29, 44, k );
        if ( results()->activeCellInfo()->isActive( ReservoirCellIndex( cellIdx ) ) ) column.push_back( cellIdx );
    }
    ASSERT_GT( column.size(), 1u );

    auto markedCount = [&]()
    {
        size_t count = 0;
        for ( size_t cellIdx : column )
        {
            if ( value( cellIdx ) == 1.0 ) count++;
        }
        return count;
    };

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep(), resultName, 1, true ) );
    EXPECT_EQ( column.size(), markedCount() );

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep(), resultName, 1, false ) );
    EXPECT_GT( markedCount(), 0u );
    EXPECT_LT( markedCount(), column.size() );

    // The polygon does not mark its whole column either
    std::vector<size_t> polygonColumn;
    for ( size_t k = 0; k < s_caseData->mainGrid()->cellCountK(); k++ )
    {
        const size_t cellIdx = reservoirCellIndex( 0, 31, 63, k );
        if ( results()->activeCellInfo()->isActive( ReservoirCellIndex( cellIdx ) ) ) polygonColumn.push_back( cellIdx );
    }
    ASSERT_GT( polygonColumn.size(), 1u );
    const size_t polygonMarked = std::count_if( polygonColumn.begin(), polygonColumn.end(), [&]( size_t c ) { return value( c ) == 1.0; } );
    EXPECT_GT( polygonMarked, 0u );
    EXPECT_LT( polygonMarked, polygonColumn.size() );
    expectBinaryValues();
}

//--------------------------------------------------------------------------------------------------
/// Also verifies that an existing result with the same name is overwritten
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, EmptyPolygonAreaGivesZero )
{
    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), lgrPolygon(), nullptr, std::nullopt, lastTimeStep(), resultName ) );
    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygonOutsideGrid(), nullptr, std::nullopt, lastTimeStep(), resultName ) );

    const auto& values = allValues();
    EXPECT_EQ( results()->activeCellInfo()->reservoirActiveCellCount(), values.size() );
    for ( double v : values )
    {
        EXPECT_EQ( 0.0, v );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, WellPathOutsidePolygon )
{
    const size_t     targetCell = reservoirCellIndex( 0, 26, 40, 14 );
    const cvf::Vec3d center     = s_caseData->mainGrid()->cell( targetCell ).center();

    cvf::ref<RigWellPath> wellPath = new RigWellPath( { cvf::Vec3d( center.x(), center.y(), center.z() + 100.0 ),
                                                        cvf::Vec3d( center.x(), center.y(), center.z() - 100.0 ) },
                                                      { 0.0, 200.0 } );

    ASSERT_TRUE(
        RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygonOutsideGrid(), wellPath.p(), RigMdRange( 0.0, 200.0 ), lastTimeStep(), resultName, 0 ) );

    EXPECT_EQ( 1.0, value( targetCell ) );
    EXPECT_EQ( 0.0, value( 0, 38, 55, 14 ) ); // GP1 is not selected by the polygon
    expectBinaryValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, OpenAndClosedPolygonGiveSameResult )
{
    const auto openPolygon   = lgrPolygon();
    auto       closedPolygon = openPolygon;
    closedPolygon.push_back( closedPolygon.front() );

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), openPolygon, nullptr, std::nullopt, lastTimeStep(), resultName ) );
    const std::vector<double> openValues = allValues();

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), closedPolygon, nullptr, std::nullopt, lastTimeStep(), resultName ) );
    EXPECT_EQ( openValues, allValues() );
    EXPECT_GT( std::count( openValues.begin(), openValues.end(), 1.0 ), 0 );
}

//--------------------------------------------------------------------------------------------------
/// GP1 has no connections in cells (23,48,24) and (31,42,20) (zero based). The well pipe passes through these
/// cells, and they are marked also without growth.
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, CellsWithoutConnectionsAlongSimWellPipe )
{
    const auto   gp1Cells  = connectionCells( "GP1" );
    const size_t firstCell = reservoirCellIndex( 0, 26, 40, 14 );
    const auto   polygon   = squareAround( sharedCorner( firstCell, reservoirCellIndex( 0, 26, 40, 15 ) ), 1.0 );
    const size_t gapCellA  = reservoirCellIndex( 0, 23, 48, 24 );
    const size_t gapCellB  = reservoirCellIndex( 0, 31, 42, 20 );
    for ( size_t cellIdx : { gapCellA, gapCellB } )
    {
        ASSERT_EQ( gp1Cells.end(), std::find( gp1Cells.begin(), gp1Cells.end(), cellIdx ) );
        ASSERT_TRUE( results()->activeCellInfo()->isActive( ReservoirCellIndex( cellIdx ) ) );
    }

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep(), resultName, 0 ) );

    EXPECT_EQ( 1.0, value( gapCellA ) );
    EXPECT_EQ( 1.0, value( gapCellB ) );
    expectBinaryValues();
}

//--------------------------------------------------------------------------------------------------
/// The polygon touches only a gap cell on the GP1 pipe, no GP1 connection cell is inside the polygon.
/// The well is still selected because its pipe intersects cells inside the polygon.
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, WellSelectedByPipeCellInsidePolygon )
{
    const auto   gp1Cells = connectionCells( "GP1" );
    const size_t gapCell  = reservoirCellIndex( 0, 23, 48, 24 );
    const auto   cornersOfGap = s_caseData->mainGrid()->cellCornerVertices( gapCell );
    const auto   polygon      = squareAround( cornersOfGap[0], 1.0 );
    ASSERT_EQ( gp1Cells.end(), std::find( gp1Cells.begin(), gp1Cells.end(), gapCell ) );

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep(), resultName, 0 ) );

    EXPECT_EQ( 1.0, value( gapCell ) );
    for ( size_t cellIdx : gp1Cells )
    {
        EXPECT_EQ( 1.0, value( cellIdx ) );
    }
    expectBinaryValues();
}

//--------------------------------------------------------------------------------------------------
/// Closes the first and last connection of the first branch of GP1 at the given time step for the lifetime of the object
//--------------------------------------------------------------------------------------------------
namespace
{
struct ScopedClosedGp1Ends
{
    ScopedClosedGp1Ends( const RigSimWellData* constSimWell, size_t timeStepIndex )
        : frame( const_cast<RigSimWellData*>( constSimWell )->m_wellCellsTimeSteps[constSimWell->m_resultTimeStepIndexToWellTimeStepIndex[timeStepIndex]] )
        , saved( frame )
    {
        auto branches = frame.wellResultBranches();
        auto points   = branches.front().branchResultPoints();
        points.front().setIsOpen( false );
        points.back().setIsOpen( false );
        branches.front().setBranchResultPoints( points );
        frame.setWellResultBranches( branches );
    }

    ~ScopedClosedGp1Ends() { frame = saved; }

    RigWellResultFrame& frame;
    RigWellResultFrame  saved;
};
} // namespace

//--------------------------------------------------------------------------------------------------
/// Closed connections at the ends of a well are not perforated, and neither is the pipe beyond the
/// last open connection. The cells of the closed connections are still marked if they are inside the polygon.
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, ClosedConnectionsAtWellEndsAreNotPerforated )
{
    const auto*        gp1 = s_caseData->findSimWellData( "GP1" );
    const auto         allCellsBefore = RigWellRegionResultTools::simWellPerforatedCells( s_caseData.p(), gp1, lastTimeStep() );
    ScopedClosedGp1Ends closedEnds( gp1, lastTimeStep() );

    const auto perforated = RigWellRegionResultTools::simWellPerforatedCells( s_caseData.p(), gp1, lastTimeStep() );
    EXPECT_TRUE( std::is_sorted( perforated.begin(), perforated.end() ) );
    EXPECT_LT( perforated.size(), allCellsBefore.size() );

    const auto branchPoints = closedEnds.saved.wellResultBranches().front().branchResultPoints();
    auto       cellOf       = [&]( const RigWellResultPoint& p )
    { return s_caseData->mainGrid()->reservoirCellIndexByGridAndGridLocalCellIndex( p.gridIndex(), p.cellIndex() ); };
    const size_t firstClosed = cellOf( branchPoints.front() );
    const size_t lastClosed  = cellOf( branchPoints.back() );

    auto contains = [&]( size_t cellIdx ) { return std::find( perforated.begin(), perforated.end(), cellIdx ) != perforated.end(); };
    EXPECT_FALSE( contains( firstClosed ) );
    EXPECT_FALSE( contains( lastClosed ) );
    EXPECT_TRUE( contains( cellOf( branchPoints[1] ) ) );
    EXPECT_TRUE( contains( cellOf( branchPoints[branchPoints.size() - 2] ) ) );
}

//--------------------------------------------------------------------------------------------------
/// A well is only selected if a perforated cell is inside the polygon. The polygon around a closed
/// end connection selects the cells inside the polygon, but not the rest of the well.
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, WellNotSelectedByNonPerforatedCellInsidePolygon )
{
    const auto*         gp1 = s_caseData->findSimWellData( "GP1" );
    ScopedClosedGp1Ends closedEnds( gp1, lastTimeStep() );

    const auto branchPoints = closedEnds.saved.wellResultBranches().front().branchResultPoints();
    const auto perforated   = RigWellRegionResultTools::simWellPerforatedCells( s_caseData.p(), gp1, lastTimeStep() );
    ASSERT_FALSE( perforated.empty() );

    // A corner of a closed end cell which is not near any perforated cell in XY. The polygon test is XY only.
    auto isNearPerforatedCell = [&]( const cvf::Vec3d& corner )
    {
        for ( size_t cellIdx : perforated )
        {
            for ( const auto& c : s_caseData->mainGrid()->cellCornerVertices( cellIdx ) )
            {
                if ( std::max( std::abs( c.x() - corner.x() ), std::abs( c.y() - corner.y() ) ) < 2.0 ) return true;
            }
        }
        return false;
    };

    size_t                  closedCell = 0;
    std::vector<cvf::Vec3d> polygon;
    for ( const auto& point : { branchPoints.front(), branchPoints.back() } )
    {
        const size_t cellIdx = s_caseData->mainGrid()->reservoirCellIndexByGridAndGridLocalCellIndex( point.gridIndex(), point.cellIndex() );
        for ( const auto& corner : s_caseData->mainGrid()->cellCornerVertices( cellIdx ) )
        {
            if ( polygon.empty() && !isNearPerforatedCell( corner ) )
            {
                closedCell = cellIdx;
                polygon    = squareAround( corner, 1.0 );
            }
        }
    }
    ASSERT_FALSE( polygon.empty() );

    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygon, nullptr, std::nullopt, lastTimeStep(), resultName, 0 ) );

    EXPECT_EQ( 1.0, value( closedCell ) ); // inside the polygon
    for ( size_t cellIdx : perforated )
    {
        ASSERT_NE( cellIdx, closedCell );
        if ( value( cellIdx ) >= 0.0 ) EXPECT_EQ( 0.0, value( cellIdx ) );
    }
    expectBinaryValues();
}

//--------------------------------------------------------------------------------------------------
/// Only the cells intersected by the perforation MD range are used for the well path
//--------------------------------------------------------------------------------------------------
TEST_F( RigWellRegionResultToolsTest, WellPathUsesPerforatedPartOnly )
{
    const size_t     targetCell = reservoirCellIndex( 0, 26, 40, 14 );
    const cvf::Vec3d center     = s_caseData->mainGrid()->cell( targetCell ).center();

    cvf::ref<RigWellPath> wellPath = new RigWellPath( { cvf::Vec3d( center.x(), center.y(), center.z() + 100.0 ),
                                                        cvf::Vec3d( center.x(), center.y(), center.z() - 100.0 ) },
                                                      { 0.0, 200.0 } );

    const auto allCells    = RigWellRegionResultTools::wellPathPerforatedCells( s_caseData.p(), wellPath.p(), RigMdRange( 0.0, 200.0 ) );
    const auto middleCells = RigWellRegionResultTools::wellPathPerforatedCells( s_caseData.p(), wellPath.p(), RigMdRange( 99.0, 101.0 ) );
    ASSERT_FALSE( middleCells.empty() );
    EXPECT_LT( middleCells.size(), allCells.size() );
    EXPECT_NE( middleCells.end(), std::find( middleCells.begin(), middleCells.end(), targetCell ) );

    // The region contains the perforated cells only
    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(),
                                                                   polygonOutsideGrid(),
                                                                   wellPath.p(),
                                                                   RigMdRange( 99.0, 101.0 ),
                                                                   lastTimeStep(),
                                                                   resultName,
                                                                   0 ) );
    for ( size_t cellIdx : allCells )
    {
        if ( value( cellIdx ) < 0.0 ) continue; // inactive cell

        const bool isPerforated = std::find( middleCells.begin(), middleCells.end(), cellIdx ) != middleCells.end();
        EXPECT_EQ( isPerforated ? 1.0 : 0.0, value( cellIdx ) );
    }

    // No perforation means the well path does not contribute
    ASSERT_TRUE( RigWellRegionResultTools::createWellRegionResult( s_caseData.p(), polygonOutsideGrid(), wellPath.p(), std::nullopt, lastTimeStep(), resultName, 0 ) );
    for ( double v : allValues() )
    {
        EXPECT_EQ( 0.0, v );
    }
}

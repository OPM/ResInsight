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

#include "RigWellRegionResultTools.h"

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigLocalGrid.h"
#include "RigMainGrid.h"

#include "Well/RigEclipseWellLogExtractor.h"
#include "Well/RigSimWellData.h"
#include "Well/RigSimulationWellCenterLineCalculator.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "cvfBoundingBox.h"

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <set>
#include <unordered_map>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Returns a copy of the polygon where the last point equals the first point
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> closedPolygon( const std::vector<cvf::Vec3d>& polygon )
{
    std::vector<cvf::Vec3d> closed = polygon;
    if ( !closed.empty() && closed.front() != closed.back() )
    {
        closed.push_back( closed.front() );
    }
    return closed;
}

//--------------------------------------------------------------------------------------------------
/// A cell is inside if any of its corners is inside the polygon (XY only)
//--------------------------------------------------------------------------------------------------
std::vector<char> cellsInsidePolygon( const RigMainGrid& mainGrid, const std::vector<cvf::Vec3d>& closedPolygon )
{
    const size_t      cellCount = mainGrid.totalCellCount();
    std::vector<char> inside( cellCount, 0 );

    cvf::BoundingBox polygonBox;
    for ( const auto& p : closedPolygon )
    {
        polygonBox.add( cvf::Vec3d( p.x(), p.y(), 0.0 ) );
    }

    auto isInsideBox2D = [&polygonBox]( const cvf::Vec3d& p )
    {
        return p.x() >= polygonBox.min().x() && p.x() <= polygonBox.max().x() && p.y() >= polygonBox.min().y() &&
               p.y() <= polygonBox.max().y();
    };

#pragma omp parallel for
    for ( int cellIdx = 0; cellIdx < static_cast<int>( cellCount ); cellIdx++ )
    {
        if ( mainGrid.cell( cellIdx ).isInvalid() ) continue;

        const std::array<cvf::Vec3d, 8> corners = mainGrid.cellCornerVertices( cellIdx );
        for ( const auto& corner : corners )
        {
            if ( isInsideBox2D( corner ) && RigCellGeometryTools::pointInsidePolygon2D( corner, closedPolygon ) )
            {
                inside[cellIdx] = 1;
                break;
            }
        }
    }

    return inside;
}

//--------------------------------------------------------------------------------------------------
/// Reservoir cell indices of the open connection cells of a well result frame (all branches, all grids)
//--------------------------------------------------------------------------------------------------
std::vector<size_t> openConnectionCells( const RigMainGrid& mainGrid, const RigWellResultFrame& frame )
{
    std::vector<size_t> cells;
    for ( const auto& resultPoint : frame.allResultPoints() )
    {
        if ( !resultPoint.isCell() || !resultPoint.isOpen() || resultPoint.gridIndex() >= mainGrid.gridCount() ) continue;

        const size_t reservoirCellIdx =
            mainGrid.reservoirCellIndexByGridAndGridLocalCellIndex( resultPoint.gridIndex(), resultPoint.cellIndex() );
        if ( reservoirCellIdx < mainGrid.totalCellCount() ) cells.push_back( reservoirCellIdx );
    }
    return cells;
}

//--------------------------------------------------------------------------------------------------
/// Intersected cells with the measured depth interval of each intersection
//--------------------------------------------------------------------------------------------------
struct CellMdInterval
{
    size_t cellIdx;
    double startMD;
    double endMD;
};

std::vector<CellMdInterval> cellMdIntervals( RigEclipseCaseData* eclipseCaseData, const RigWellPath* wellPath )
{
    cvf::ref<RigEclipseWellLogExtractor> extractor = new RigEclipseWellLogExtractor( eclipseCaseData, wellPath, "" );

    const auto& cells = extractor->intersectedCellsGlobIdx();
    const auto& mds   = extractor->cellIntersectionMDs();

    // The extractor stores two entries (enter and leave) for each intersected cell
    std::vector<CellMdInterval> intervals;
    for ( size_t i = 0; i + 1 < cells.size() && i + 1 < mds.size(); i += 2 )
    {
        intervals.push_back( { cells[i], mds[i], mds[i + 1] } );
    }
    return intervals;
}

} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigWellRegionResultTools::wellPathPerforatedCells( RigEclipseCaseData* eclipseCaseData,
                                                                       const RigWellPath*  wellPath,
                                                                       const RigMdRange&   perforation )
{
    std::vector<size_t> cells;
    if ( !eclipseCaseData || !wellPath ) return cells;

    for ( const auto& interval : cellMdIntervals( eclipseCaseData, wellPath ) )
    {
        if ( perforation.overlaps( interval.startMD, interval.endMD ) ) cells.push_back( interval.cellIdx );
    }
    return cells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigWellRegionResultTools::simWellPerforatedCells( RigEclipseCaseData*   eclipseCaseData,
                                                                      const RigSimWellData* simWellData,
                                                                      size_t                timeStepIndex )
{
    std::vector<size_t> cells;
    if ( !eclipseCaseData || !eclipseCaseData->mainGrid() || !simWellData || !simWellData->hasWellResult( timeStepIndex ) ) return cells;

    const RigWellResultFrame* frame = simWellData->wellResultFrame( timeStepIndex );
    if ( !frame ) return cells;

    const std::vector<size_t> openCells = openConnectionCells( *eclipseCaseData->mainGrid(), *frame );
    if ( openCells.empty() ) return cells;

    cells = openCells;

    // The open connections can have gaps (closed connections or cells without connections). The pipe between the
    // first and last open connection of each branch is perforated.
    const bool isAutoDetectBranches = true;
    const bool useAllCellCenters    = false;

    const auto branches = RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineForTimeStep( eclipseCaseData,
                                                                                                         simWellData,
                                                                                                         static_cast<int>( timeStepIndex ),
                                                                                                         isAutoDetectBranches,
                                                                                                         useAllCellCenters );

    const std::set<size_t> openCellSet( openCells.begin(), openCells.end() );

    for ( const auto& [coords, resultPoints] : branches )
    {
        if ( coords.size() < 2 ) continue;

        std::vector<double> measuredDepths = { 0.0 };
        for ( size_t i = 1; i < coords.size(); i++ )
        {
            measuredDepths.push_back( measuredDepths.back() + ( coords[i] - coords[i - 1] ).length() );
        }

        cvf::ref<RigWellPath> pipePath  = new RigWellPath( coords, measuredDepths );
        const auto            intervals = cellMdIntervals( eclipseCaseData, pipePath.p() );

        std::vector<RigMdRange> openRanges;
        for ( const auto& interval : intervals )
        {
            if ( openCellSet.count( interval.cellIdx ) ) openRanges.emplace_back( interval.startMD, interval.endMD );
        }

        const auto perforation = RigMdRange::merged( openRanges );
        if ( !perforation ) continue;

        for ( const auto& interval : intervals )
        {
            if ( perforation->overlaps( interval.startMD, interval.endMD ) ) cells.push_back( interval.cellIdx );
        }
    }

    std::sort( cells.begin(), cells.end() );
    cells.erase( std::unique( cells.begin(), cells.end() ), cells.end() );
    return cells;
}

namespace
{
//--------------------------------------------------------------------------------------------------
/// Neighbors of a cell: lateral face neighbors (I, J +-1) in the same grid, the host cell of an LGR cell,
/// and the LGR cells of a host cell. The K direction is handled by fillMarkedColumns
//--------------------------------------------------------------------------------------------------
std::vector<size_t> neighborCells( const RigMainGrid&                                     mainGrid,
                                   const std::unordered_map<size_t, std::vector<size_t>>& lgrCellsInHostCell,
                                   size_t                                                 reservoirCellIdx )
{
    std::vector<size_t> neighbors;

    const RigCell&     cell = mainGrid.cell( reservoirCellIdx );
    const RigGridBase* grid = cell.hostGrid();
    if ( !grid ) return neighbors;

    size_t i = 0, j = 0, k = 0;
    if ( grid->ijkFromCellIndex( cell.gridLocalCellIndex(), &i, &j, &k ) )
    {
        const std::array<std::array<int, 2>, 4> offsets = { { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } } };
        for ( const auto& [di, dj] : offsets )
        {
            if ( ( di < 0 && i == 0 ) || ( dj < 0 && j == 0 ) ) continue;

            const size_t ni = i + di;
            const size_t nj = j + dj;
            if ( ni >= grid->cellCountI() || nj >= grid->cellCountJ() ) continue;

            neighbors.push_back( grid->reservoirCellIndex( grid->cellIndexFromIJK( ni, nj, k ) ) );
        }
    }

    if ( !grid->isMainGrid() )
    {
        const auto* localGrid = dynamic_cast<const RigLocalGrid*>( grid );
        if ( localGrid && localGrid->parentGrid() )
        {
            neighbors.push_back( localGrid->parentGrid()->reservoirCellIndex( cell.parentCellIndex() ) );
        }
    }

    if ( auto it = lgrCellsInHostCell.find( reservoirCellIdx ); it != lgrCellsInHostCell.end() )
    {
        neighbors.insert( neighbors.end(), it->second.begin(), it->second.end() );
    }

    return neighbors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unordered_map<size_t, std::vector<size_t>> computeLgrCellsInHostCells( const RigMainGrid& mainGrid )
{
    std::unordered_map<size_t, std::vector<size_t>> lgrCellsInHostCell;
    for ( size_t gridIdx = 1; gridIdx < mainGrid.gridCount(); gridIdx++ )
    {
        const auto* localGrid = dynamic_cast<const RigLocalGrid*>( mainGrid.gridByIndex( gridIdx ) );
        if ( !localGrid || !localGrid->parentGrid() ) continue;

        for ( size_t localCellIdx = 0; localCellIdx < localGrid->cellCount(); localCellIdx++ )
        {
            const size_t   reservoirCellIdx = localGrid->reservoirCellIndex( localCellIdx );
            const RigCell& cell             = mainGrid.cell( reservoirCellIdx );
            const size_t   hostCellIdx      = localGrid->parentGrid()->reservoirCellIndex( cell.parentCellIndex() );
            lgrCellsInHostCell[hostCellIdx].push_back( reservoirCellIdx );
        }
    }
    return lgrCellsInHostCell;
}

//--------------------------------------------------------------------------------------------------
/// Mark the full K range of every (I, J) column that has a marked cell, in the grid of the marked cell.
/// The LGR cells of marked host cells are marked as well.
//--------------------------------------------------------------------------------------------------
void fillMarkedColumns( const RigMainGrid&                                     mainGrid,
                        const std::unordered_map<size_t, std::vector<size_t>>& lgrCellsInHostCell,
                        std::vector<char>&                                     marked )
{
    std::set<std::array<size_t, 3>> columns;
    for ( size_t cellIdx = 0; cellIdx < marked.size(); cellIdx++ )
    {
        if ( !marked[cellIdx] ) continue;

        const RigCell&     cell = mainGrid.cell( cellIdx );
        const RigGridBase* grid = cell.hostGrid();
        size_t             i = 0, j = 0, k = 0;
        if ( grid && grid->ijkFromCellIndex( cell.gridLocalCellIndex(), &i, &j, &k ) ) columns.insert( { grid->gridIndex(), i, j } );
    }

    for ( const auto& [gridIdx, i, j] : columns )
    {
        const RigGridBase* grid = mainGrid.gridByIndex( gridIdx );
        for ( size_t k = 0; k < grid->cellCountK(); k++ )
        {
            const size_t cellIdx = grid->reservoirCellIndex( grid->cellIndexFromIJK( i, j, k ) );
            if ( cellIdx >= marked.size() ) continue;

            marked[cellIdx] = 1;
            if ( auto it = lgrCellsInHostCell.find( cellIdx ); it != lgrCellsInHostCell.end() )
            {
                for ( size_t lgrCellIdx : it->second )
                {
                    marked[lgrCellIdx] = 1;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Expand the marked cells by growCellCount layers of lateral neighbor cells, and optionally to the full K range
//--------------------------------------------------------------------------------------------------
void growMarkedCells( const RigMainGrid& mainGrid, int growCellCount, bool fillFullKRange, std::vector<char>& marked )
{
    if ( growCellCount <= 0 ) return;

    const auto lgrCellsInHostCell = computeLgrCellsInHostCells( mainGrid );

    std::vector<size_t> front;
    for ( size_t cellIdx = 0; cellIdx < marked.size(); cellIdx++ )
    {
        if ( marked[cellIdx] ) front.push_back( cellIdx );
    }

    for ( int layer = 0; layer < growCellCount && !front.empty(); layer++ )
    {
        std::vector<size_t> nextFront;
        for ( size_t cellIdx : front )
        {
            for ( size_t neighborIdx : neighborCells( mainGrid, lgrCellsInHostCell, cellIdx ) )
            {
                if ( neighborIdx >= marked.size() || marked[neighborIdx] ) continue;

                marked[neighborIdx] = 1;
                nextFront.push_back( neighborIdx );
            }
        }
        front.swap( nextFront );
    }

    if ( fillFullKRange ) fillMarkedColumns( mainGrid, lgrCellsInHostCell, marked );
}

//--------------------------------------------------------------------------------------------------
/// K index in the main grid of a cell. LGR cells use the K index of their host cell.
//--------------------------------------------------------------------------------------------------
std::optional<size_t> mainGridK( const RigMainGrid& mainGrid, size_t cellIdx )
{
    const RigCell& cell = mainGrid.cell( cellIdx );
    if ( cell.hostGrid() && !cell.hostGrid()->isMainGrid() ) return mainGridK( mainGrid, cell.mainGridCellIndex() );

    size_t i = 0, j = 0, k = 0;
    if ( !mainGrid.ijkFromCellIndex( cell.gridLocalCellIndex(), &i, &j, &k ) ) return std::nullopt;
    return k;
}

//--------------------------------------------------------------------------------------------------
/// Remove cells from insidePolygon which are outside the main grid K range spanned by the marked cells.
/// Nothing is removed if there are no marked cells.
//--------------------------------------------------------------------------------------------------
void limitToKRangeOfMarkedCells( const RigMainGrid& mainGrid, const std::vector<char>& marked, std::vector<char>& insidePolygon )
{
    size_t minK = std::numeric_limits<size_t>::max();
    size_t maxK = 0;
    for ( size_t cellIdx = 0; cellIdx < marked.size(); cellIdx++ )
    {
        if ( !marked[cellIdx] ) continue;
        if ( auto k = mainGridK( mainGrid, cellIdx ) )
        {
            minK = std::min( minK, *k );
            maxK = std::max( maxK, *k );
        }
    }
    if ( minK > maxK ) return;

    for ( size_t cellIdx = 0; cellIdx < insidePolygon.size(); cellIdx++ )
    {
        if ( !insidePolygon[cellIdx] ) continue;
        const auto k = mainGridK( mainGrid, cellIdx );
        if ( !k || *k < minK || *k > maxK ) insidePolygon[cellIdx] = 0;
    }
}

} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellRegionResultTools::createWellRegionResult( RigEclipseCaseData*              eclipseCaseData,
                                                       const std::vector<cvf::Vec3d>&   polygon,
                                                       const RigWellPath*               wellPath,
                                                       const std::optional<RigMdRange>& wellPathPerforation,
                                                       size_t                           timeStepIndex,
                                                       const QString&                   resultName,
                                                       int                              growCellCount,
                                                       bool                             fillFullKRange )
{
    if ( !eclipseCaseData || !eclipseCaseData->mainGrid() || resultName.isEmpty() ) return false;

    const std::vector<cvf::Vec3d> polygonClosed = closedPolygon( polygon );
    if ( polygonClosed.size() < 4 ) return false;

    RigCaseCellResultsData* resultsData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData || !resultsData->activeCellInfo() ) return false;

    const size_t timeStepCount = std::max( size_t( 1 ), resultsData->maxTimeStepCount() );
    if ( timeStepIndex >= timeStepCount ) return false;

    const RigMainGrid& mainGrid  = *eclipseCaseData->mainGrid();
    const size_t       cellCount = mainGrid.totalCellCount();

    std::vector<char> insidePolygon = cellsInsidePolygon( mainGrid, polygonClosed );

    std::vector<char> wellCells( cellCount, 0 );

    for ( const auto& simWellData : eclipseCaseData->wellResults() )
    {
        if ( simWellData.isNull() || !simWellData->hasWellResult( timeStepIndex ) ) continue;

        const std::vector<size_t> perforatedCells = simWellPerforatedCells( eclipseCaseData, simWellData.p(), timeStepIndex );

        const bool isSelected = std::any_of( perforatedCells.begin(),
                                             perforatedCells.end(),
                                             [&insidePolygon, cellCount]( size_t cellIdx )
                                             { return cellIdx < cellCount && insidePolygon[cellIdx]; } );
        if ( !isSelected ) continue;

        for ( size_t cellIdx : perforatedCells )
        {
            if ( cellIdx < cellCount ) wellCells[cellIdx] = 1;
        }
    }

    if ( wellPath && wellPathPerforation )
    {
        for ( size_t cellIdx : wellPathPerforatedCells( eclipseCaseData, wellPath, *wellPathPerforation ) )
        {
            if ( cellIdx < cellCount ) wellCells[cellIdx] = 1;
        }
    }

    growMarkedCells( mainGrid, growCellCount, fillFullKRange, wellCells );

    // Without K filling, the polygon only applies to the K range covered by the well cells
    if ( !fillFullKRange ) limitToKRangeOfMarkedCells( mainGrid, wellCells, insidePolygon );

    const RigActiveCellInfo* activeCellInfo  = resultsData->activeCellInfo();
    const size_t             activeCellCount = activeCellInfo->reservoirActiveCellCount();

    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );
    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::GENERATED, resultName, false, activeCellCount );

    std::vector<double>* resultValues = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    if ( !resultValues ) return false;

    resultValues->assign( activeCellCount, 0.0 );

    for ( size_t cellIdx = 0; cellIdx < cellCount; cellIdx++ )
    {
        if ( !insidePolygon[cellIdx] && !wellCells[cellIdx] ) continue;

        const ReservoirCellIndex reservoirCellIdx( cellIdx );
        if ( !activeCellInfo->isActive( reservoirCellIdx ) ) continue;

        const size_t activeIdx = activeCellInfo->cellResultIndex( reservoirCellIdx ).value();
        if ( activeIdx < activeCellCount ) ( *resultValues )[activeIdx] = 1.0;
    }

    resultsData->recalculateStatistics( resultAddress );

    return true;
}

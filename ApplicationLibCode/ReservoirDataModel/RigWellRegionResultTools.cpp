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
#include "Well/RigWellPath.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "cvfBoundingBox.h"

#include <algorithm>
#include <array>
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
/// Reservoir cell indices of the connection cells of a well result frame (all branches, all grids)
//--------------------------------------------------------------------------------------------------
std::vector<size_t> connectionCells( const RigMainGrid& mainGrid, const RigWellResultFrame& frame )
{
    std::vector<size_t> cells;
    for ( const auto& resultPoint : frame.allResultPoints() )
    {
        if ( !resultPoint.isCell() || resultPoint.gridIndex() >= mainGrid.gridCount() ) continue;

        const size_t reservoirCellIdx =
            mainGrid.reservoirCellIndexByGridAndGridLocalCellIndex( resultPoint.gridIndex(), resultPoint.cellIndex() );
        if ( reservoirCellIdx < mainGrid.totalCellCount() ) cells.push_back( reservoirCellIdx );
    }
    return cells;
}

//--------------------------------------------------------------------------------------------------
/// Neighbors of a cell: face neighbors (I, J, K +-1) in the same grid, the host cell of an LGR cell,
/// and the LGR cells of a host cell
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
        const std::array<std::array<int, 3>, 6> offsets = {
            { { -1, 0, 0 }, { 1, 0, 0 }, { 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 } } };
        for ( const auto& [di, dj, dk] : offsets )
        {
            if ( ( di < 0 && i == 0 ) || ( dj < 0 && j == 0 ) || ( dk < 0 && k == 0 ) ) continue;

            const size_t ni = i + di;
            const size_t nj = j + dj;
            const size_t nk = k + dk;
            if ( ni >= grid->cellCountI() || nj >= grid->cellCountJ() || nk >= grid->cellCountK() ) continue;

            neighbors.push_back( grid->reservoirCellIndex( grid->cellIndexFromIJK( ni, nj, nk ) ) );
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
/// Expand the marked cells by growCellCount layers of neighbor cells
//--------------------------------------------------------------------------------------------------
void growMarkedCells( const RigMainGrid& mainGrid, int growCellCount, std::vector<char>& marked )
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
}

} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellRegionResultTools::createWellRegionResult( RigEclipseCaseData*            eclipseCaseData,
                                                       const std::vector<cvf::Vec3d>& polygon,
                                                       const RigWellPath*             wellPath,
                                                       size_t                         timeStepIndex,
                                                       const QString&                 resultName,
                                                       int                            growCellCount )
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

    const std::vector<char> insidePolygon = cellsInsidePolygon( mainGrid, polygonClosed );

    std::vector<char> wellCells( cellCount, 0 );

    for ( const auto& simWellData : eclipseCaseData->wellResults() )
    {
        if ( simWellData.isNull() || !simWellData->hasWellResult( timeStepIndex ) ) continue;

        const RigWellResultFrame* frame = simWellData->wellResultFrame( timeStepIndex );
        if ( !frame ) continue;

        const std::vector<size_t> cells = connectionCells( mainGrid, *frame );

        const bool isSelected =
            std::any_of( cells.begin(), cells.end(), [&insidePolygon]( size_t cellIdx ) { return insidePolygon[cellIdx]; } );
        if ( !isSelected ) continue;

        for ( size_t cellIdx : cells )
        {
            wellCells[cellIdx] = 1;
        }
    }

    if ( wellPath )
    {
        cvf::ref<RigEclipseWellLogExtractor> extractor = new RigEclipseWellLogExtractor( eclipseCaseData, wellPath, "" );
        for ( size_t cellIdx : extractor->intersectedCellsGlobIdx() )
        {
            if ( cellIdx < cellCount ) wellCells[cellIdx] = 1;
        }
    }

    growMarkedCells( mainGrid, growCellCount, wellCells );

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

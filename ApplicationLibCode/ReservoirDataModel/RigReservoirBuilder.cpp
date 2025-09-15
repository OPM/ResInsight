/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RigReservoirBuilder.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigReservoirBuilder::RigReservoirBuilder()
    : m_minWorldCoordinate( 0.0, 0.0, 0.0 )
    , m_maxWorldCoordinate( 0.0, 0.0, 0.0 )
    , m_ijkCount( 0, 0, 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilder::setWorldCoordinates( cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate )
{
    m_minWorldCoordinate = minWorldCoordinate;
    m_maxWorldCoordinate = maxWorldCoordinate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilder::setIJKCount( const cvf::Vec3st& ijkCount )
{
    m_ijkCount = ijkCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilder::addLocalGridRefinement( const cvf::Vec3st& minCellPosition,
                                                  const cvf::Vec3st& maxCellPosition,
                                                  const cvf::Vec3st& singleCellRefinementFactors )
{
    m_localGridRefinements.push_back( LocalGridRefinement( minCellPosition, maxCellPosition, singleCellRefinementFactors ) );
}

//--------------------------------------------------------------------------------------------------
/// Build the geometry for a grids and cells
/// - create 8 nodes for each cell
/// - create cells by referencing the nodes
/// - optionally create and add LGR grids
/// - set all cells to active
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilder::createGridsAndCells( RigEclipseCaseData* eclipseCase )
{
    std::vector<cvf::Vec3d>& mainGridNodes = eclipseCase->mainGrid()->nodes();
    appendNodes( m_minWorldCoordinate, m_maxWorldCoordinate, ijkCount(), mainGridNodes );
    size_t mainGridNodeCount = mainGridNodes.size();
    size_t mainGridCellCount = mainGridNodeCount / 8;

    // Must create cells in main grid here, as this information is used when creating LGRs
    appendCells( 0, mainGridCellCount, eclipseCase->mainGrid(), eclipseCase->mainGrid()->reservoirCells() );

    size_t totalCellCount = mainGridCellCount;

    for ( size_t lgrIdx = 0; lgrIdx < m_localGridRefinements.size(); lgrIdx++ )
    {
        LocalGridRefinement& lgr = m_localGridRefinements[lgrIdx];

        // Compute all global cell indices to be replaced by local grid refinement
        std::vector<size_t> mainGridIndicesWithSubGrid;
        {
            for ( size_t i = lgr.m_mainGridMinCellPosition.x(); i <= lgr.m_mainGridMaxCellPosition.x(); i++ )
            {
                for ( size_t j = lgr.m_mainGridMinCellPosition.y(); j <= lgr.m_mainGridMaxCellPosition.y(); j++ )
                {
                    for ( size_t k = lgr.m_mainGridMinCellPosition.z(); k <= lgr.m_mainGridMaxCellPosition.z(); k++ )
                    {
                        mainGridIndicesWithSubGrid.push_back( cellIndexFromIJK( i, j, k ) );
                    }
                }
            }
        }

        // Create local grid and set local grid dimensions
        RigLocalGrid* localGrid = new RigLocalGrid( eclipseCase->mainGrid() );
        localGrid->setGridId( 1 );
        localGrid->setGridName( "LGR_1" );
        eclipseCase->mainGrid()->addLocalGrid( localGrid );
        localGrid->setParentGrid( eclipseCase->mainGrid() );

        localGrid->setIndexToStartOfCells( mainGridNodes.size() / 8 );
        cvf::Vec3st cellCounts( lgr.m_singleCellRefinementFactors.x() *
                                    ( lgr.m_mainGridMaxCellPosition.x() - lgr.m_mainGridMinCellPosition.x() ),
                                lgr.m_singleCellRefinementFactors.y() *
                                    ( lgr.m_mainGridMaxCellPosition.y() - lgr.m_mainGridMinCellPosition.y() ),
                                lgr.m_singleCellRefinementFactors.z() *
                                    ( lgr.m_mainGridMaxCellPosition.z() - lgr.m_mainGridMinCellPosition.z() ) );

        localGrid->setCellCounts( cellCounts );

        cvf::BoundingBox bb;
        for ( size_t cellIdx = 0; cellIdx < mainGridIndicesWithSubGrid.size(); cellIdx++ )
        {
            RigCell& cell = eclipseCase->mainGrid()->cell( mainGridIndicesWithSubGrid[cellIdx] );

            std::array<size_t, 8>& indices = cell.cornerIndices();
            for ( int nodeIdx = 0; nodeIdx < 8; nodeIdx++ )
            {
                bb.add( eclipseCase->mainGrid()->nodes()[indices[nodeIdx]] );
            }
            // Deactivate cell in main grid
            cell.setSubGrid( localGrid );
        }

        cvf::Vec3st lgrCellCounts = cellCounts;
        appendNodes( bb.min(), bb.max(), lgrCellCounts, mainGridNodes );

        size_t subGridCellCount = ( mainGridNodes.size() / 8 ) - totalCellCount;
        appendCells( totalCellCount * 8, subGridCellCount, localGrid, eclipseCase->mainGrid()->reservoirCells() );
        totalCellCount += subGridCellCount;
    }

    eclipseCase->mainGrid()->setCellCounts( m_ijkCount );

    // Set all cells active
    RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    activeCellInfo->setReservoirCellCount( eclipseCase->mainGrid()->totalCellCount() );
    for ( size_t i = 0; i < eclipseCase->mainGrid()->totalCellCount(); i++ )
    {
        activeCellInfo->setCellResultIndex( i, i );
    }

    activeCellInfo->setGridCount( 1 );
    activeCellInfo->setGridActiveCellCounts( 0, eclipseCase->mainGrid()->totalCellCount() );
    activeCellInfo->computeDerivedData();

    bool useOptimizedVersion = false; // workaround, optimized version causes assert in debug builds
    eclipseCase->computeActiveCellBoundingBoxes( useOptimizedVersion );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilder::appendCells( size_t nodeStartIndex, size_t cellCount, RigGridBase* hostGrid, std::vector<RigCell>& cells )
{
    size_t cellIndexStart = cells.size();
    cells.resize( cells.size() + cellCount );

#pragma omp parallel for
    for ( long long i = 0; i < static_cast<long long>( cellCount ); i++ )
    {
        RigCell& riCell = cells[cellIndexStart + i];

        riCell.setHostGrid( hostGrid );
        riCell.setGridLocalCellIndex( i );

        riCell.cornerIndices()[0] = nodeStartIndex + i * 8 + 0;
        riCell.cornerIndices()[1] = nodeStartIndex + i * 8 + 1;
        riCell.cornerIndices()[2] = nodeStartIndex + i * 8 + 2;
        riCell.cornerIndices()[3] = nodeStartIndex + i * 8 + 3;
        riCell.cornerIndices()[4] = nodeStartIndex + i * 8 + 4;
        riCell.cornerIndices()[5] = nodeStartIndex + i * 8 + 5;
        riCell.cornerIndices()[6] = nodeStartIndex + i * 8 + 6;
        riCell.cornerIndices()[7] = nodeStartIndex + i * 8 + 7;

        riCell.setParentCellIndex( 0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilder::appendNodes( const cvf::Vec3d& min, const cvf::Vec3d& max, const cvf::Vec3st& cubeDimension, std::vector<cvf::Vec3d>& nodes )
{
    double dx = ( max.x() - min.x() ) / static_cast<double>( cubeDimension.x() );
    double dy = ( max.y() - min.y() ) / static_cast<double>( cubeDimension.y() );
    double dz = ( max.z() - min.z() ) / static_cast<double>( cubeDimension.z() );

    double zPos = min.z();

    for ( size_t k = 0; k < cubeDimension.z(); k++ )
    {
        double yPos = min.y();

        for ( size_t j = 0; j < cubeDimension.y(); j++ )
        {
            double xPos = min.x();

            for ( size_t i = 0; i < cubeDimension.x(); i++ )
            {
                cvf::Vec3d cornerA( xPos, yPos, zPos );
                cvf::Vec3d cornerB( xPos + dx, yPos + dy, zPos + dz );

                appendCubeNodes( cornerA, cornerB, nodes );

                xPos += dx;
            }

            yPos += dy;
        }

        zPos += dz;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilder::appendCubeNodes( const cvf::Vec3d& min, const cvf::Vec3d& max, std::vector<cvf::Vec3d>& nodes )
{
    //
    //     7---------6                Faces:
    //    /|        /|     |k           0 bottom   0, 3, 2, 1
    //   / |       / |     | /j         1 top      4, 5, 6, 7
    //  4---------5  |     |/           2 front    0, 1, 5, 4
    //  |  3------|--2     *---i        3 right    1, 2, 6, 5
    //  | /       | /                   4 back     3, 7, 6, 2
    //  |/        |/                    5 left     0, 4, 7, 3
    //  0---------1

    cvf::Vec3d v0( min.x(), min.y(), min.z() );
    cvf::Vec3d v1( max.x(), min.y(), min.z() );
    cvf::Vec3d v2( max.x(), max.y(), min.z() );
    cvf::Vec3d v3( min.x(), max.y(), min.z() );

    cvf::Vec3d v4( min.x(), min.y(), max.z() );
    cvf::Vec3d v5( max.x(), min.y(), max.z() );
    cvf::Vec3d v6( max.x(), max.y(), max.z() );
    cvf::Vec3d v7( min.x(), max.y(), max.z() );

    nodes.push_back( v0 );
    nodes.push_back( v1 );
    nodes.push_back( v2 );
    nodes.push_back( v3 );
    nodes.push_back( v4 );
    nodes.push_back( v5 );
    nodes.push_back( v6 );
    nodes.push_back( v7 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigReservoirBuilder::cellIndexFromIJK( size_t i, size_t j, size_t k ) const
{
    CVF_TIGHT_ASSERT( i < m_ijkCount.x() );
    CVF_TIGHT_ASSERT( j < m_ijkCount.y() );
    CVF_TIGHT_ASSERT( k < m_ijkCount.z() );

    size_t ci = i + j * m_ijkCount.x() + k * m_ijkCount.x() * m_ijkCount.y();
    return ci;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RigReservoirBuilder::ijkCount() const
{
    return m_ijkCount;
}

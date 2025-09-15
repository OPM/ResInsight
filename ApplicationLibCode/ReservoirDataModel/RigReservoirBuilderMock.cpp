/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigReservoirBuilderMock.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include <QRegularExpression>

/* rand example: guess the number */
#include <cstdio>
#include <cstdlib>
#include <ctime>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigReservoirBuilderMock::RigReservoirBuilderMock()
{
    m_resultCount    = 0;
    m_timeStepCount  = 0;
    m_enableWellData = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::setCellCounts( const cvf::Vec3st& cellCounts )
{
    m_reservoirBuilder.setIJKCount( cellCounts );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::setResultInfo( size_t resultCount, size_t timeStepCount )
{
    m_resultCount   = resultCount;
    m_timeStepCount = timeStepCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::populateReservoir( RigEclipseCaseData* eclipseCase )
{
    m_reservoirBuilder.createGridsAndCells( eclipseCase );

    if ( m_enableWellData )
    {
        addWellData( eclipseCase, eclipseCase->mainGrid() );
    }

    addFaults( eclipseCase );

    // Add grid coarsening for main grid
    //     if ( cellDimension().x() > 4 && cellDimension().y() > 5 && cellDimension().z() > 6 )
    //     {
    //         eclipseCase->mainGrid()->addCoarseningBox( 1, 2, 1, 3, 1, 4 );
    //         eclipseCase->mainGrid()->addCoarseningBox( 3, 4, 4, 5, 5, 6 );
    //     }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::addLocalGridRefinement( const cvf::Vec3st& mainGridStart,
                                                      const cvf::Vec3st& mainGridEnd,
                                                      const cvf::Vec3st& refinementFactors )
{
    m_reservoirBuilder.addLocalGridRefinement( mainGridStart, mainGridEnd, refinementFactors );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::setWorldCoordinates( cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate )
{
    m_reservoirBuilder.setWorldCoordinates( minWorldCoordinate, maxWorldCoordinate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigReservoirBuilderMock::inputProperty( RigEclipseCaseData* eclipseCase, const QString& propertyName, std::vector<double>* values )
{
    size_t k;

    /* initialize random seed: */
    srand( time( nullptr ) );

    /* generate secret number: */
    int iSecret = rand() % 20 + 1;

    for ( k = 0; k < eclipseCase->mainGrid()->totalCellCount(); k++ )
    {
        values->push_back( k * iSecret );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigReservoirBuilderMock::staticResult( RigEclipseCaseData* eclipseCase, const QString& result, std::vector<double>* values )
{
    values->resize( eclipseCase->mainGrid()->totalCellCount() );

#pragma omp parallel for
    for ( long long k = 0; k < static_cast<long long>( eclipseCase->mainGrid()->totalCellCount() ); k++ )
    {
        values->at( k ) = ( k * 2 ) % eclipseCase->mainGrid()->totalCellCount();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigReservoirBuilderMock::dynamicResult( RigEclipseCaseData* eclipseCase, const QString& result, size_t stepIndex, std::vector<double>* values )
{
    int resultIndex = 1;

    QRegularExpression      rx( "[0-9]{1,2}" ); // Find number 0-99
    QRegularExpressionMatch match = rx.match( result );
    if ( match.hasMatch() )
    {
        resultIndex = match.captured( 0 ).toInt() + 1;
    }

    double scaleValue  = 1.0 + resultIndex * 0.1;
    double offsetValue = 100 * resultIndex;

    values->resize( eclipseCase->mainGrid()->totalCellCount() );

#pragma omp parallel for
    for ( long long k = 0; k < static_cast<long long>( eclipseCase->mainGrid()->totalCellCount() ); k++ )
    {
        double val      = offsetValue + scaleValue * ( ( stepIndex * 1000 + k ) % eclipseCase->mainGrid()->totalCellCount() );
        values->at( k ) = val;
    }

    // Set result size to zero for some timesteps
    if ( ( stepIndex + 1 ) % 3 == 0 )
    {
        values->clear();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::addWellData( RigEclipseCaseData* eclipseCase, RigGridBase* grid )
{
    CVF_ASSERT( eclipseCase );
    CVF_ASSERT( grid );

    auto cellCountJ = grid->cellCountJ();
    auto cellCountK = grid->cellCountK();

    cvf::Collection<RigSimWellData> wells;

    int wellIdx;
    for ( wellIdx = 0; wellIdx < 1; wellIdx++ )
    {
        cvf::ref<RigSimWellData> wellCellsTimeHistory = new RigSimWellData;
        wellCellsTimeHistory->m_wellName              = QString( "Well %1" ).arg( wellIdx );

        wellCellsTimeHistory->m_wellCellsTimeSteps.resize( m_timeStepCount );

        size_t timeIdx;
        for ( timeIdx = 0; timeIdx < m_timeStepCount; timeIdx++ )
        {
            RigWellResultFrame& wellCells = wellCellsTimeHistory->m_wellCellsTimeSteps[timeIdx];

            wellCells.setProductionType( RiaDefines::WellProductionType::PRODUCER );
            wellCells.setIsOpen( true );

            auto wellHead = wellCells.wellHead();
            wellHead.setGridIndex( 0 );
            wellHead.setGridCellIndex( grid->cellIndexFromIJK( 1, 0, 0 ) );
            wellCells.setWellHead( wellHead );

            // Connections
            //            int connectionCount = std::min(dim.x(), std::min(dim.y(), dim.z())) - 2;
            size_t connectionCount = cellCountK - 2;
            if ( connectionCount > 0 )
            {
                // Only main grid supported by now. Must be taken care of when LGRs are supported
                auto newWellResultBranches = wellCells.wellResultBranches();
                newWellResultBranches.resize( 1 );
                RigWellResultBranch& wellSegment = newWellResultBranches[0];

                size_t connIdx;
                for ( connIdx = 0; connIdx < connectionCount; connIdx++ )
                {
                    if ( connIdx == (size_t)( connectionCount / 4 ) ) continue;

                    RigWellResultPoint data;
                    data.setGridIndex( 0 );

                    if ( connIdx < cellCountJ - 2 )
                        data.setGridCellIndex( grid->cellIndexFromIJK( 1, 1 + connIdx, 1 + connIdx ) );
                    else
                        data.setGridCellIndex( grid->cellIndexFromIJK( 1, cellCountJ - 2, 1 + connIdx ) );

                    if ( connIdx < connectionCount / 2 )
                    {
                        data.setIsOpen( true );
                    }
                    else
                    {
                        data.setIsOpen( false );
                    }

                    if ( wellSegment.branchResultPoints().empty() || wellSegment.branchResultPoints().back().cellIndex() != data.cellIndex() )
                    {
                        wellSegment.addBranchResultPoint( data );

                        if ( connIdx == connectionCount / 2 )
                        {
                            RigWellResultPoint deadEndData = data;
                            deadEndData.setGridCellIndex( data.cellIndex() + 1 );
                            deadEndData.setIsOpen( true );

                            RigWellResultPoint deadEndData1 = data;
                            deadEndData1.setGridCellIndex( data.cellIndex() + 2 );
                            deadEndData1.setIsOpen( false );

                            wellSegment.addBranchResultPoint( deadEndData );
                            wellSegment.addBranchResultPoint( deadEndData1 );

                            wellSegment.addBranchResultPoint( deadEndData );

                            data.setIsOpen( true );
                            wellSegment.addBranchResultPoint( data );
                        }
                    }

                    if ( connIdx < cellCountJ - 2 )
                    {
                        data.setGridCellIndex( grid->cellIndexFromIJK( 1, 1 + connIdx, 2 + connIdx ) );

                        if ( wellSegment.branchResultPoints().empty() ||
                             wellSegment.branchResultPoints().back().cellIndex() != data.cellIndex() )
                        {
                            wellSegment.addBranchResultPoint( data );
                        }
                    }
                }
                wellCells.setWellResultBranches( newWellResultBranches );
            }
        }

        // Create a mapping from result timestep indices to well timestep indices.
        // Use one-to-one mapping for easy use
        std::vector<size_t> map;
        for ( timeIdx = 0; timeIdx < m_timeStepCount; timeIdx++ )
        {
            map.push_back( timeIdx );
        }
        wellCellsTimeHistory->m_resultTimeStepIndexToWellTimeStepIndex = map;

        wells.push_back( wellCellsTimeHistory.p() );
    }

    eclipseCase->setSimWellData( wells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::addFaults( RigEclipseCaseData* eclipseCase )
{
    if ( !eclipseCase ) return;

    RigMainGrid* grid = eclipseCase->mainGrid();
    if ( !grid ) return;

    cvf::Collection<RigFault> faults;

    auto cellDimension = m_reservoirBuilder.ijkCount();

    {
        cvf::ref<RigFault> fault = new RigFault;
        fault->setName( "Fault A" );

        cvf::Vec3st min = cvf::Vec3st::ZERO;
        cvf::Vec3st max( 0, 0, cellDimension.z() - 2 );

        if ( cellDimension.x() > 5 )
        {
            min.x() = cellDimension.x() / 2;
            max.x() = min.x() + 2;
        }

        if ( cellDimension.y() > 5 )
        {
            min.y() = cellDimension.y() / 2;
            max.y() = cellDimension.y() / 2;
        }

        cvf::CellRange cellRange( min, max );

        fault->addCellRangeForFace( cvf::StructGridInterface::POS_I, cellRange );
        faults.push_back( fault.p() );
    }

    grid->setFaults( faults );

    // NNCs
    RigConnectionContainer nncConnections;
    {
        size_t i1 = 2;
        size_t j1 = 2;
        size_t k1 = 3;

        size_t i2 = 2;
        size_t j2 = 3;
        size_t k2 = 4;

        addNnc( grid, i1, j1, k1, i2, j2, k2, nncConnections );
    }

    {
        size_t i1 = 2;
        size_t j1 = 2;
        size_t k1 = 3;

        size_t i2 = 2;
        size_t j2 = 1;
        size_t k2 = 4;

        addNnc( grid, i1, j1, k1, i2, j2, k2, nncConnections );
    }

    grid->nncData()->setEclipseConnections( nncConnections );

    std::vector<double>& tranVals = grid->nncData()->makeStaticConnectionScalarResult( RiaDefines::propertyNameCombTrans() );
    for ( size_t cIdx = 0; cIdx < tranVals.size(); ++cIdx )
    {
        tranVals[cIdx] = 0.2;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::enableWellData( bool enableWellData )
{
    m_enableWellData = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigReservoirBuilderMock::addNnc( RigMainGrid* grid, size_t i1, size_t j1, size_t k1, size_t i2, size_t j2, size_t k2, RigConnectionContainer& nncConnections )
{
    size_t c1GlobalIndex = grid->cellIndexFromIJK( i1, j1, k1 );
    size_t c2GlobalIndex = grid->cellIndexFromIJK( i2, j2, k2 );

    RigConnection conn( c1GlobalIndex, c2GlobalIndex );

    nncConnections.push_back( conn );
}

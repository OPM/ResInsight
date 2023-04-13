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

#include "RigMswCenterLineCalculator.h"

#include "RiaLogging.h"

#include "RigCell.h"
#include "RigCellFaceGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"
#include "RigWellResultFrame.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"

#include "cvfRay.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SimulationWellCellBranch> RigMswCenterLineCalculator::calculateMswWellPipeGeometry( RimSimWellInView* rimWell )
{
    CVF_ASSERT( rimWell );

    const RigSimWellData* simWellData = rimWell->simWellData();
    if ( !simWellData ) return {};

    RimEclipseView* eclipseView;
    rimWell->firstAncestorOrThisOfType( eclipseView );

    CVF_ASSERT( eclipseView );

    if ( eclipseView->eclipseCase() && eclipseView->eclipseCase()->eclipseCaseData() )
    {
        auto eclipseCaseData = eclipseView->eclipseCase()->eclipseCaseData();
        int  timeStepIndex   = eclipseView->currentTimeStep();

        return calculateMswWellPipeGeometryForTimeStep( eclipseCaseData, simWellData, timeStepIndex );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SimulationWellCellBranch>
    RigMswCenterLineCalculator::calculateMswWellPipeGeometryForTimeStep( const RigEclipseCaseData* eclipseCaseData,
                                                                         const RigSimWellData*     wellResults,
                                                                         int                       timeStepIndex )
{
    if ( timeStepIndex >= 0 && !wellResults->hasAnyValidCells( timeStepIndex ) ) return {};

    const RigWellResultFrame* wellFramePtr = nullptr;

    if ( timeStepIndex < 0 )
    {
        wellFramePtr = wellResults->staticWellResultFrame();
    }
    else
    {
        wellFramePtr = wellResults->wellResultFrame( timeStepIndex );
    }

    const RigWellResultFrame&              wellFrame      = *wellFramePtr;
    const std::vector<RigWellResultBranch> resultBranches = wellFrame.wellResultBranches();

    std::vector<WellBranch> wellBranches = mergeShortBranchesIntoLongBranches( resultBranches );

    // Connect outlet segment of branches to parent branch

    for ( const auto& resultBranch : resultBranches )
    {
        if ( resultBranch.branchResultPoints().empty() ) continue;

        const RigWellResultPoint firstResultPoint = resultBranch.branchResultPoints().front();

        for ( auto& wellBranch : wellBranches )
        {
            if ( wellBranch.m_branchId == resultBranch.ertBranchId() )
            {
                if ( firstResultPoint.branchId() == resultBranch.ertBranchId() )
                {
                    // The first result point is on the same branch, use well head as outlet
                    RigWellResultPoint outletResultPoint = wellFrame.wellHead();

                    auto gridAndCellIndex = std::make_pair( outletResultPoint.gridIndex(), outletResultPoint.cellIndex() );
                    wellBranch.m_segmentsWithGridCells[outletResultPoint.segmentId()].push_back( gridAndCellIndex );
                }
                else
                {
                    // The first result point on a different branch. Find the branch and add the grid cell
                    for ( const auto& candidateResultBranch : wellBranches )
                    {
                        if ( firstResultPoint.branchId() == candidateResultBranch.m_branchId )
                        {
                            std::pair<size_t, size_t> gridAndCellIndexForTieIn;

                            for ( const auto& [segment, gridAndCellIndices] : candidateResultBranch.m_segmentsWithGridCells )
                            {
                                if ( segment > firstResultPoint.segmentId() ) continue;
                                if ( !gridAndCellIndices.empty() )
                                {
                                    gridAndCellIndexForTieIn = gridAndCellIndices.front();
                                }
                            }

                            wellBranch.m_segmentsWithGridCells[firstResultPoint.segmentId()].push_back( gridAndCellIndexForTieIn );
                        }
                    }
                }
            }
        }
    }

    std::vector<SimulationWellCellBranch> simWellBranches;
    for ( const auto& wellBranch : wellBranches )
    {
        std::vector<cvf::Vec3d>         cellCenterCoords;
        std::vector<RigWellResultPoint> cellCenterResultPoints;

        if ( wellBranch.m_branchId == 1 && !wellBranch.m_segmentsWithGridCells.empty() )
        {
            const auto& [firstSegment, gridAndCellIndices] = *wellBranch.m_segmentsWithGridCells.begin();
            if ( !gridAndCellIndices.empty() )
            {
                const auto& [gridIndex, cellIndex] = gridAndCellIndices.front();
                if ( gridIndex < eclipseCaseData->gridCount() && cellIndex < eclipseCaseData->grid( gridIndex )->cellCount() )
                {
                    const RigCell& cell       = eclipseCaseData->grid( gridIndex )->cell( cellIndex );
                    cvf::Vec3d     whStartPos = cell.faceCenter( cvf::StructGridInterface::NEG_K );

                    // Add extra coordinate between cell face and cell center
                    // to make sure the well pipe terminated in a segment parallel to z-axis

                    cvf::Vec3d whIntermediate = whStartPos;
                    whIntermediate.z()        = ( whStartPos.z() + cell.center().z() ) / 2.0;

                    RigWellResultPoint resPoint;
                    for ( const auto& resBranch : resultBranches )
                    {
                        for ( const auto& respoint : resBranch.branchResultPoints() )
                        {
                            if ( respoint.segmentId() == firstSegment )
                            {
                                resPoint = respoint;
                                break;
                            }
                        }
                    }

                    cellCenterCoords.push_back( whStartPos );
                    cellCenterResultPoints.push_back( resPoint );
                    cellCenterCoords.push_back( whIntermediate );
                    cellCenterResultPoints.push_back( resPoint );
                }
            }
        }

        for ( const auto& [segmentId, gridAndCellIndices] : wellBranch.m_segmentsWithGridCells )
        {
            for ( const auto& [gridIndex, cellIndex] : gridAndCellIndices )
            {
                if ( gridIndex < eclipseCaseData->gridCount() && cellIndex < eclipseCaseData->grid( gridIndex )->cellCount() )
                {
                    const RigCell& cell = eclipseCaseData->grid( gridIndex )->cell( cellIndex );
                    cvf::Vec3d     pos  = cell.center();

                    RigWellResultPoint resPoint;

                    // The result point is only used to transport the grid index and cell index
                    // The current implementation will propagate the cell open state to the well segment from one cell to
                    // the next.
                    resPoint.setGridIndex( gridIndex );
                    resPoint.setGridCellIndex( cellIndex );

                    auto gridAndCellIndex = std::make_pair( gridIndex, cellIndex );
                    if ( std::find( wellBranch.m_gridCellsConnectedToValve.begin(),
                                    wellBranch.m_gridCellsConnectedToValve.end(),
                                    gridAndCellIndex ) != wellBranch.m_gridCellsConnectedToValve.end() )
                    {
                        resPoint.setIsConnectedToValve( true );
                    }

                    cellCenterCoords.push_back( pos );
                    cellCenterResultPoints.push_back( resPoint );
                }
            }
        }

        const auto simWellBranch = addCoordsAtCellFaceIntersectionsAndCreateBranch( cellCenterCoords, cellCenterResultPoints, eclipseCaseData );

        simWellBranches.emplace_back( simWellBranch );
    }

    return simWellBranches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SimulationWellCellBranch
    RigMswCenterLineCalculator::addCoordsAtCellFaceIntersectionsAndCreateBranch( const std::vector<cvf::Vec3d>          branchCoords,
                                                                                 const std::vector<RigWellResultPoint>& resultPoints,
                                                                                 const RigEclipseCaseData*              eclipseCaseData )
{
    std::vector<cvf::Vec3d>         adjustedCoords;
    std::vector<RigWellResultPoint> adjustedResPoints;

    RigWellResultPoint previusPoint;

    for ( size_t resPointIdx = 0; resPointIdx < resultPoints.size(); resPointIdx++ )
    {
        if ( resPointIdx >= branchCoords.size() ) continue;

        const auto& currentPoint      = resultPoints[resPointIdx];
        const auto& currentCellCenter = branchCoords[resPointIdx];

        if ( previusPoint.isCell() )
        {
            const RigCell&   prevCell           = eclipseCaseData->cellFromWellResultCell( previusPoint );
            const cvf::Vec3d previousCellCenter = prevCell.center();

            {
                // Insert extra coordinate at the location where the well path is leaving the previous cell

                cvf::Ray rayToThisCell;
                rayToThisCell.setOrigin( previousCellCenter );
                rayToThisCell.setDirection( ( currentCellCenter - previousCellCenter ).getNormalized() );

                cvf::Vec3d outOfPrevCell( previousCellCenter );

                prevCell.firstIntersectionPoint( rayToThisCell, &outOfPrevCell );
                if ( ( currentCellCenter - outOfPrevCell ).lengthSquared() > 1e-3 )
                {
                    adjustedCoords.push_back( outOfPrevCell );
                    adjustedResPoints.push_back( RigWellResultPoint() );
                }
            }

            if ( currentPoint.isCell() )
            {
                // Insert extra coordinate at the location where the well path is entering the current cell

                const RigCell& currentCell = eclipseCaseData->cellFromWellResultCell( currentPoint );

                cvf::Ray rayFromThisCell;
                rayFromThisCell.setOrigin( currentCellCenter );
                rayFromThisCell.setDirection( ( previousCellCenter - currentCellCenter ).getNormalized() );

                cvf::Vec3d outOfCurrentCell( currentCellCenter );

                currentCell.firstIntersectionPoint( rayFromThisCell, &outOfCurrentCell );
                if ( ( currentCellCenter - outOfCurrentCell ).lengthSquared() > 1e-3 )
                {
                    adjustedCoords.push_back( outOfCurrentCell );
                    adjustedResPoints.push_back( currentPoint );
                }
            }
        }

        adjustedResPoints.push_back( currentPoint );
        adjustedCoords.push_back( currentCellCenter );

        previusPoint = currentPoint;
    }

    // Duplicate last coord to make sure we have N+1 coordinates for N result points
    adjustedCoords.push_back( adjustedCoords.back() );

    return { adjustedCoords, adjustedResPoints };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigMswCenterLineCalculator::WellBranch>
    RigMswCenterLineCalculator::mergeShortBranchesIntoLongBranches( const std::vector<RigWellResultBranch>& resBranches )
{
    std::vector<WellBranch> longWellBranches;
    std::vector<WellBranch> shortWellBranches;

    for ( const auto& resultBranch : resBranches )
    {
        WellBranch branch;
        branch.m_branchId = resultBranch.ertBranchId();

        for ( const auto& resPoint : resultBranch.branchResultPoints() )
        {
            size_t gridIndex     = resPoint.gridIndex();
            size_t gridCellIndex = resPoint.cellIndex();

            auto gridAndCellIndex = std::make_pair( gridIndex, gridCellIndex );
            if ( gridIndex != cvf::UNDEFINED_SIZE_T && gridCellIndex != cvf::UNDEFINED_SIZE_T && !branch.containsGridCell( gridAndCellIndex ) )
            {
                OutputSegment outputSegment{ resPoint.outletSegmentId(), resPoint.outletBranchId() };
                branch.m_gridCellsConnectedToSegments[gridAndCellIndex] = outputSegment;
                branch.m_segmentsWithGridCells[resPoint.segmentId()].push_back( gridAndCellIndex );
            }
        }

        const int resultPointThreshold = 3;
        if ( resultBranch.branchResultPoints().size() > resultPointThreshold )
        {
            longWellBranches.push_back( branch );
        }
        else
        {
            shortWellBranches.push_back( branch );
        }
    }

    // Move all grid cells of small branch to the long branch
    for ( const auto& branch : shortWellBranches )
    {
        if ( branch.m_gridCellsConnectedToSegments.empty() ) continue;

        const auto& outputSegment = branch.m_gridCellsConnectedToSegments.begin()->second;
        for ( auto& longBranch : longWellBranches )
        {
            if ( longBranch.m_branchId == outputSegment.outputSegmentBranchId )
            {
                for ( const auto& [gridAndCellIndex, localOutputSegment] : branch.m_gridCellsConnectedToSegments )
                {
                    longBranch.m_segmentsWithGridCells[outputSegment.outputSegmentId].push_back( gridAndCellIndex );
                    longBranch.m_gridCellsConnectedToValve.push_back( gridAndCellIndex );
                }
            }
        }
    }

    return longWellBranches;
}

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

#include "RigFaultDistanceCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigFault.h"
#include "RigMainGrid.h"

#include "cvfBoundingBoxTree.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultDistanceCalculator::computeFaultDistances( const RigMainGrid*                  mainGrid,
                                                        const RigActiveCellInfo*            activeCellInfo,
                                                        const std::vector<const RigFault*>& faultsToInclude,
                                                        std::vector<double>&                resultValues )
{
    if ( !mainGrid || !activeCellInfo ) return;

    const auto activeCells = activeCellInfo->activeReservoirCellIndices();
    if ( activeCells.empty() ) return;

    if ( resultValues.size() < activeCells.size() )
    {
        resultValues.resize( activeCells.size(), std::numeric_limits<double>::infinity() );
    }

    // Collect fault face centers from the requested subset of faults.
    std::vector<cvf::Vec3d> faultFaceCenters;
    for ( const RigFault* fault : faultsToInclude )
    {
        if ( !fault ) continue;
        for ( const RigFault::FaultFace& faultFace : fault->faultFaces() )
        {
            if ( faultFace.m_nativeReservoirCellIndex >= mainGrid->cellCount() ) continue;
            const RigCell& cell = mainGrid->cell( faultFace.m_nativeReservoirCellIndex );
            if ( cell.isInvalid() ) continue;
            faultFaceCenters.push_back( cell.faceCenter( faultFace.m_nativeFace ) );
        }
    }

    if ( faultFaceCenters.empty() ) return;

    // Create bounding box tree for all face centers
    cvf::BoundingBoxTree searchTree;
    {
        std::vector<size_t>           faceIndicesForBoundingBoxes;
        std::vector<cvf::BoundingBox> faceBBs;

        size_t faceCenterIndex = 0;
        for ( const auto& faultFaceCenter : faultFaceCenters )
        {
            cvf::BoundingBox bb;
            bb.add( faultFaceCenter );
            faceBBs.push_back( bb );
            faceIndicesForBoundingBoxes.push_back( faceCenterIndex++ );
        }
        searchTree.buildTreeFromBoundingBoxes( faceBBs, &faceIndicesForBoundingBoxes );
    }

    const auto nodes      = mainGrid->nodes();
    const auto mainGridBB = mainGrid->boundingBox();

#pragma omp parallel for
    for ( int activeIndex = 0; activeIndex < static_cast<int>( activeCells.size() ); activeIndex++ )
    {
        auto cellIdx = activeCells[activeIndex];
        if ( cellIdx.value() == cvf::UNDEFINED_SIZE_T ) continue;

        const RigCell& cell = mainGrid->cell( cellIdx.value() );
        if ( cell.isInvalid() ) continue;

        std::vector<size_t> candidateFaceIndices;
        {
            cvf::BoundingBox bb;
            const auto&      cellIndices = cell.cornerIndices();
            for ( const auto& i : cellIndices )
            {
                bb.add( nodes[i] );
            }

            searchTree.findIntersections( bb, &candidateFaceIndices );

            bool bbIsBelowThreshold = true;
            while ( candidateFaceIndices.empty() && bbIsBelowThreshold )
            {
                if ( bb.extent().x() > mainGridBB.extent().x() * 2 )
                {
                    bbIsBelowThreshold = false;
                    break;
                }
                if ( bb.extent().y() > mainGridBB.extent().y() * 2 )
                {
                    bbIsBelowThreshold = false;
                    break;
                }

                bb.expand( bb.extent().x() );
                searchTree.findIntersections( bb, &candidateFaceIndices );
            }
        }

        // Find closest fault face
        double shortestDistance = std::numeric_limits<double>::infinity();

        for ( const auto& faultFaceIndex : candidateFaceIndices )
        {
            const cvf::Vec3d& faultFaceCenter = faultFaceCenters[faultFaceIndex];
            shortestDistance                  = std::min( cell.center().pointDistance( faultFaceCenter ), shortestDistance );
        }

        resultValues[activeIndex] = shortestDistance;
    }
}

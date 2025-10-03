/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RigFaultDistanceResultCalculator.h"

#include "RiaDefines.h"
#include "RiaResultNames.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"

#include "cvfBoundingBoxTree.h"

//==================================================================================================
///
//==================================================================================================
RigFaultDistanceResultCalculator::RigFaultDistanceResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigFaultDistanceResultCalculator::~RigFaultDistanceResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFaultDistanceResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return resVarAddr.resultName() == RiaResultNames::faultDistanceName() &&
           resVarAddr.resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultDistanceResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    size_t activeCellCount = m_resultsData->activeCellInfo()->reservoirActiveCellCount();
    if ( activeCellCount == 0 ) return;

    size_t resultIndex = m_resultsData->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::faultDistanceName() ) );

    if ( resultIndex == cvf::UNDEFINED_SIZE_T ) return;

    std::vector<std::vector<double>>& result = m_resultsData->m_cellScalarResults[resultIndex];

    if ( result.empty() ) result.resize( 1 );

    bool shouldCompute = false;
    if ( result[0].size() < activeCellCount )
    {
        result[0].resize( activeCellCount, std::numeric_limits<double>::infinity() );
        shouldCompute = true;
    }

    if ( !shouldCompute ) return;

    const auto mainGrid = m_resultsData->m_ownerMainGrid;

    std::vector<cvf::StructGridInterface::FaceType> faceTypes = cvf::StructGridInterface::validFaceTypes();

    // Preprocessing: create vector of all fault face centers.
    std::vector<cvf::Vec3d> faultFaceCenters;
    const auto              activeCells = m_resultsData->activeCellInfo()->activeReservoirCellIndices();
    for ( auto cellIdx : activeCells )
    {
        const RigCell& cell = mainGrid->cell( cellIdx.value() );
        if ( cell.isInvalid() ) continue;
        for ( auto faceType : faceTypes )
        {
            if ( m_resultsData->m_ownerMainGrid->findFaultFromCellIndexAndCellFace( cellIdx.value(), faceType ) )
                faultFaceCenters.push_back( cell.faceCenter( faceType ) );
        }
    }

    if ( faultFaceCenters.empty() ) return;

    // Create bounding box tree for all face centers
    auto searchTree = new cvf::BoundingBoxTree;
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
        searchTree->buildTreeFromBoundingBoxes( faceBBs, &faceIndicesForBoundingBoxes );
    }

    const auto nodes      = m_resultsData->m_ownerMainGrid->nodes();
    const auto mainGridBB = m_resultsData->m_ownerMainGrid->boundingBox();

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

            searchTree->findIntersections( bb, &candidateFaceIndices );

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
                searchTree->findIntersections( bb, &candidateFaceIndices );
            }
        }

        // Find closest fault face
        double shortestDistance = std::numeric_limits<double>::infinity();

        for ( const auto& faultFaceIndex : candidateFaceIndices )
        {
            const cvf::Vec3d& faultFaceCenter = faultFaceCenters[faultFaceIndex];
            shortestDistance                  = std::min( cell.center().pointDistance( faultFaceCenter ), shortestDistance );
        }

        result[0][activeIndex] = shortestDistance;
    }
}

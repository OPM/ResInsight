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
    size_t reservoirCellCount = m_resultsData->activeCellInfo()->reservoirCellCount();
    if ( reservoirCellCount == 0 ) return;

    size_t resultIndex = m_resultsData->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::faultDistanceName() ) );

    if ( resultIndex == cvf::UNDEFINED_SIZE_T ) return;

    std::vector<std::vector<double>>& result = m_resultsData->m_cellScalarResults[resultIndex];

    if ( result.empty() ) result.resize( 1 );

    bool shouldCompute = false;
    if ( result[0].size() < reservoirCellCount )
    {
        result[0].resize( reservoirCellCount, std::numeric_limits<double>::infinity() );
        shouldCompute = true;
    }

    if ( !shouldCompute ) return;

    const std::vector<RigCell>& globalCellArray = m_resultsData->m_ownerMainGrid->globalCellArray();

    long long numCells = static_cast<long long>( globalCellArray.size() );

    std::vector<cvf::StructGridInterface::FaceType> faceTypes = cvf::StructGridInterface::validFaceTypes();

    // Preprocessing: create vector of all fault face centers.
    std::vector<cvf::Vec3d> faultFaceCenters;
    for ( long long cellIdx = 0; cellIdx < numCells; cellIdx++ )
    {
        if ( m_resultsData->activeCellInfo()->isActive( cellIdx ) )
        {
            const RigCell& cell = globalCellArray[cellIdx];
            for ( auto faceType : faceTypes )
            {
                if ( m_resultsData->m_ownerMainGrid->findFaultFromCellIndexAndCellFace( cellIdx, faceType ) )
                    faultFaceCenters.push_back( cell.faceCenter( faceType ) );
            }
        }
    }

#pragma omp parallel for
    for ( long long cellIdx = 0; cellIdx < numCells; cellIdx++ )
    {
        const RigCell& cell = globalCellArray[cellIdx];

        size_t resultIndex = cellIdx;
        if ( resultIndex == cvf::UNDEFINED_SIZE_T || !m_resultsData->activeCellInfo()->isActive( cellIdx ) ) continue;

        // Find closest fault face
        double shortestDistance = std::numeric_limits<double>::infinity();
        for ( const cvf::Vec3d& faultFaceCenter : faultFaceCenters )
        {
            shortestDistance = std::min( cell.center().pointDistance( faultFaceCenter ), shortestDistance );
        }

        result[0][resultIndex] = shortestDistance;
    }
}

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

#include "RigIndexIjkResultCalculator.h"
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
RigIndexIjkResultCalculator::RigIndexIjkResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigIndexIjkResultCalculator::~RigIndexIjkResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigIndexIjkResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return ( resVarAddr.resultName() == RiaResultNames::indexIResultName() || resVarAddr.resultName() == RiaResultNames::indexJResultName() ||
             resVarAddr.resultName() == RiaResultNames::indexKResultName() ) &&
           resVarAddr.resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigIndexIjkResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    size_t reservoirCellCount = m_resultsData->activeCellInfo()->reservoirCellCount();
    if ( reservoirCellCount == 0 ) return;

    size_t iResultIndex = m_resultsData->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::indexIResultName() ) );
    size_t jResultIndex = m_resultsData->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::indexJResultName() ) );
    size_t kResultIndex = m_resultsData->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::indexKResultName() ) );

    if ( iResultIndex == cvf::UNDEFINED_SIZE_T || jResultIndex == cvf::UNDEFINED_SIZE_T || kResultIndex == cvf::UNDEFINED_SIZE_T ) return;

    bool computeIndexI = false;
    bool computeIndexJ = false;
    bool computeIndexK = false;

    std::vector<std::vector<double>>& indexI = m_resultsData->m_cellScalarResults[iResultIndex];
    std::vector<std::vector<double>>& indexJ = m_resultsData->m_cellScalarResults[jResultIndex];
    std::vector<std::vector<double>>& indexK = m_resultsData->m_cellScalarResults[kResultIndex];

    if ( indexI.empty() ) indexI.resize( 1 );
    if ( indexI[0].size() < reservoirCellCount )
    {
        indexI[0].resize( reservoirCellCount, std::numeric_limits<double>::infinity() );
        computeIndexI = true;
    }

    if ( indexJ.empty() ) indexJ.resize( 1 );
    if ( indexJ[0].size() < reservoirCellCount )
    {
        indexJ[0].resize( reservoirCellCount, std::numeric_limits<double>::infinity() );
        computeIndexJ = true;
    }

    if ( indexK.empty() ) indexK.resize( 1 );
    if ( indexK[0].size() < reservoirCellCount )
    {
        indexK[0].resize( reservoirCellCount, std::numeric_limits<double>::infinity() );
        computeIndexK = true;
    }

    if ( !( computeIndexI || computeIndexJ || computeIndexK ) ) return;

    const std::vector<RigCell>& globalCellArray = m_resultsData->m_ownerMainGrid->globalCellArray();
    long long                   numCells        = static_cast<long long>( globalCellArray.size() );
#pragma omp parallel for
    for ( long long cellIdx = 0; cellIdx < numCells; cellIdx++ )
    {
        const RigCell& cell = globalCellArray[cellIdx];

        size_t resultIndex = cellIdx;
        if ( resultIndex == cvf::UNDEFINED_SIZE_T ) continue;

        bool isTemporaryGrid = cell.hostGrid()->isTempGrid();

        size_t       gridLocalNativeCellIndex = cell.gridLocalCellIndex();
        RigGridBase* grid                     = cell.hostGrid();

        size_t i, j, k;
        if ( grid->ijkFromCellIndex( gridLocalNativeCellIndex, &i, &j, &k ) )
        {
            // I/J/K is 1-indexed when shown to user, thus "+ 1"
            if ( computeIndexI || isTemporaryGrid )
            {
                indexI[0][resultIndex] = i + 1;
            }

            if ( computeIndexJ || isTemporaryGrid )
            {
                indexJ[0][resultIndex] = j + 1;
            }

            if ( computeIndexK || isTemporaryGrid )
            {
                indexK[0][resultIndex] = k + 1;
            }
        }
    }
}

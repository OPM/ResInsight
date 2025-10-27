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

#include "RigCellVolumeResultCalculator.h"
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
RigCellVolumeResultCalculator::RigCellVolumeResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigCellVolumeResultCalculator::~RigCellVolumeResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCellVolumeResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return resVarAddr.resultName() == RiaResultNames::riCellVolumeResultName() &&
           resVarAddr.resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCellVolumeResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    if ( m_resultsData->activeCellInfo()->reservoirActiveCellCount() == 0 )
    {
        return;
    }

    size_t cellVolIdx = m_resultsData->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                               RiaResultNames::riCellVolumeResultName() ),
                                                                      false );

    if ( m_resultsData->m_cellScalarResults[cellVolIdx].empty() )
    {
        m_resultsData->m_cellScalarResults[cellVolIdx].resize( 1 );
    }
    std::vector<double>& cellVolumeResults = m_resultsData->m_cellScalarResults[cellVolIdx][0];

    size_t cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();
    cellVolumeResults.resize( cellResultCount, std::numeric_limits<double>::infinity() );

#pragma omp parallel for
    for ( int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>( m_resultsData->m_ownerMainGrid->totalCellCount() );
          nativeResvCellIndex++ )
    {
        size_t resultIndex = m_resultsData->activeCellInfo()->cellResultIndex( nativeResvCellIndex );
        if ( ( resultIndex != cvf::UNDEFINED_SIZE_T ) && ( resultIndex < cellResultCount ) )
        {
            const RigCell& cell = m_resultsData->m_ownerMainGrid->cell( nativeResvCellIndex );
            if ( !cell.subGrid() )
            {
                cellVolumeResults[resultIndex] = cell.volume();
            }
        }
    }

    // Clear oil volume so it will have to be recalculated.
    m_resultsData->clearScalarResult( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riOilVolumeResultName() );
}

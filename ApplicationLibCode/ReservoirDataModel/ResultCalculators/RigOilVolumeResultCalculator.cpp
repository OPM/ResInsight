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

#include "RigOilVolumeResultCalculator.h"
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
RigOilVolumeResultCalculator::RigOilVolumeResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigOilVolumeResultCalculator::~RigOilVolumeResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigOilVolumeResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return resVarAddr.resultName() == RiaResultNames::riOilVolumeResultName() &&
           resVarAddr.resultCatType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigOilVolumeResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    size_t cellVolIdx = m_resultsData->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                               RiaResultNames::riCellVolumeResultName() ),
                                                                      false );
    const std::vector<double>& cellVolumeResults = m_resultsData->m_cellScalarResults[cellVolIdx][0];

    size_t soilIdx = m_resultsData->findOrLoadKnownScalarResult(
        RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ) );
    size_t oilVolIdx = m_resultsData->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                              RiaResultNames::riOilVolumeResultName() ),
                                                                     false );
    m_resultsData->m_cellScalarResults[oilVolIdx].resize( m_resultsData->maxTimeStepCount() );

    size_t cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();
    for ( size_t timeStepIdx = 0; timeStepIdx < m_resultsData->maxTimeStepCount(); timeStepIdx++ )
    {
        const std::vector<double>& soilResults      = m_resultsData->m_cellScalarResults[soilIdx][timeStepIdx];
        std::vector<double>&       oilVolumeResults = m_resultsData->m_cellScalarResults[oilVolIdx][timeStepIdx];
        oilVolumeResults.resize( cellResultCount, 0u );

#pragma omp parallel for
        for ( int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>( m_resultsData->m_ownerMainGrid->globalCellArray().size() );
              nativeResvCellIndex++ )
        {
            size_t resultIndex = m_resultsData->activeCellInfo()->cellResultIndex( nativeResvCellIndex );
            if ( resultIndex != cvf::UNDEFINED_SIZE_T )
            {
                if ( resultIndex < soilResults.size() && resultIndex < cellVolumeResults.size() )
                {
                    CVF_ASSERT( soilResults.at( resultIndex ) <= 1.01 );
                    oilVolumeResults[resultIndex] = std::max( 0.0, soilResults.at( resultIndex ) * cellVolumeResults.at( resultIndex ) );
                }
            }
        }
    }
}

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

#include "RigCellsWithNncsCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"

//==================================================================================================
///
//==================================================================================================
RigCellsWithNncsCalculator::RigCellsWithNncsCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigCellsWithNncsCalculator::~RigCellsWithNncsCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCellsWithNncsCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return ( resVarAddr.resultName() == RiaResultNames::riNncCells() &&
             resVarAddr.resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCellsWithNncsCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    if ( !m_resultsData ) return;
    if ( !m_resultsData->m_ownerCaseData ) return;
    if ( m_resultsData->activeCellInfo()->reservoirActiveCellCount() == 0 ) return;

    auto nncData     = m_resultsData->m_ownerMainGrid->nncData();
    auto connections = nncData->allConnections();

    std::set<size_t> uniqueReservoirIndices;
    for ( size_t i = 0; i < nncData->eclipseConnectionCount(); i++ )
    {
        uniqueReservoirIndices.insert( connections[i].c1GlobIdx() );
        uniqueReservoirIndices.insert( connections[i].c2GlobIdx() );
    }

    size_t scalarResultIndex = m_resultsData->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                                      RiaResultNames::riNncCells() ),
                                                                             false );

    if ( m_resultsData->m_cellScalarResults[scalarResultIndex].empty() )
    {
        m_resultsData->m_cellScalarResults[scalarResultIndex].resize( 1 );
    }
    std::vector<double>& resultValues = m_resultsData->m_cellScalarResults[scalarResultIndex][0];

    size_t cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();
    resultValues.resize( cellResultCount, 0 );

    for ( auto reservoirCellIndex : uniqueReservoirIndices )
    {
        size_t resultIndex        = m_resultsData->activeCellInfo()->cellResultIndex( reservoirCellIndex );
        resultValues[resultIndex] = 1.0;
    }
}

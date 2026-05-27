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
#include "RigEclipseResultInfo.h"
#include "RigFault.h"
#include "RigFaultDistanceCalculator.h"
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

    std::vector<const RigFault*> allFaults;
    for ( size_t i = 0; i < mainGrid->faults().size(); ++i )
    {
        allFaults.push_back( mainGrid->faults().at( i ) );
    }

    RigFaultDistanceCalculator::computeFaultDistances( mainGrid, m_resultsData->activeCellInfo(), allFaults, result[0] );
}

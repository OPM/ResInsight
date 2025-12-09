/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RigSwatResultCalculator.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"

#include "RiaPhaseTools.h"
#include "RiaResultNames.h"

//==================================================================================================
///
//==================================================================================================
RigSwatResultCalculator::RigSwatResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigSwatResultCalculator::~RigSwatResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSwatResultCalculator::checkAndCreatePlaceholderEntry( const RigEclipseResultAddress& resVarAddr )
{
    if ( !isMatching( resVarAddr ) ) return;

    if ( !m_resultsData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ) ) )
    {
        if ( hasOnlyWaterPhase() )
        {
            bool needsToBeStored = false;

            size_t swatIndex = m_resultsData->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                                      RiaResultNames::swat() ),
                                                                             needsToBeStored );
            m_resultsData->setMustBeCalculated( swatIndex );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSwatResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return resVarAddr.resultName() == RiaResultNames::swat() && resVarAddr.resultCatType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSwatResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    if ( !hasOnlyWaterPhase() ) return;

    // Get the number of cells from active cell info
    const RigActiveCellInfo* activeCellInfo = m_resultsData->activeCellInfo();
    if ( !activeCellInfo ) return;

    size_t cellCount = activeCellInfo->reservoirActiveCellCount();
    if ( cellCount == 0 ) return;

    // Make sure memory is allocated for the SWAT results
    size_t swatResultScalarIndex = m_resultsData->findScalarResultIndexFromAddress( resVarAddr );

    size_t timeStepCount = m_resultsData->maxTimeStepCount();
    m_resultsData->m_cellScalarResults[swatResultScalarIndex].resize( timeStepCount );

    if ( !m_resultsData->cellScalarResults( resVarAddr, timeStepIndex ).empty() )
    {
        // Data is already computed and allocated, nothing more to do
        return;
    }

    m_resultsData->m_cellScalarResults[swatResultScalarIndex][timeStepIndex].resize( cellCount );

    // Fill all cells with 1.0 (100% water saturation)
    if ( std::vector<double>* swatForTimeStep = m_resultsData->modifiableCellScalarResult( resVarAddr, timeStepIndex ) )
    {
        std::fill( swatForTimeStep->begin(), swatForTimeStep->end(), 1.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSwatResultCalculator::hasOnlyWaterPhase() const
{
    // Get the available phases from the eclipse case data
    const RigEclipseCaseData* eclipseCaseData = m_resultsData->m_ownerCaseData;
    if ( !eclipseCaseData ) return false;

    std::set<RiaDefines::PhaseType> availablePhases = eclipseCaseData->availablePhases();
    return RiaPhaseTools::isSinglePhaseWater( availablePhases );
}
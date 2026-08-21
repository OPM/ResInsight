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

#include "RigSgasResultCalculator.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"

#include "RiaPhaseTools.h"
#include "RiaResultNames.h"

//==================================================================================================
///
//==================================================================================================
RigSgasResultCalculator::RigSgasResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigSgasResultCalculator::~RigSgasResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSgasResultCalculator::checkAndCreatePlaceholderEntry( const RigEclipseResultAddress& resVarAddr )
{
    if ( !isMatching( resVarAddr ) ) return;

    // Only compute SGAS for two-phase gas/water models, see issue #14565
    if ( !isTwoPhaseGasWater() ) return;

    if ( m_resultsData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) ) )
    {
        return;
    }

    if ( !m_resultsData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ) ) )
    {
        return;
    }

    bool   needsToBeStored = false;
    size_t sgasIndex = m_resultsData->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                              RiaResultNames::sgas() ),
                                                                     needsToBeStored );
    m_resultsData->setMustBeCalculated( sgasIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSgasResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return resVarAddr.resultName() == RiaResultNames::sgas() && resVarAddr.resultCatType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSgasResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    if ( !isTwoPhaseGasWater() ) return;

    RigEclipseResultAddress swatAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() );

    size_t scalarIndexSWAT = m_resultsData->findOrLoadKnownScalarResultForTimeStep( swatAddr, timeStepIndex );
    if ( scalarIndexSWAT == cvf::UNDEFINED_SIZE_T ) return;

    size_t swatTimeStepCount = m_resultsData->infoForEachResultIndex()[scalarIndexSWAT].timeStepInfos().size();
    if ( timeStepIndex >= swatTimeStepCount ) return;

    size_t swatResultValueCount = m_resultsData->cellScalarResults( swatAddr, timeStepIndex ).size();
    if ( swatResultValueCount == 0 ) return;

    // Make sure memory is allocated for the new SGAS results
    size_t sgasResultScalarIndex = m_resultsData->findScalarResultIndexFromAddress( resVarAddr );
    if ( sgasResultScalarIndex == cvf::UNDEFINED_SIZE_T ) return;

    m_resultsData->m_cellScalarResults[sgasResultScalarIndex].resize( swatTimeStepCount );

    if ( !m_resultsData->cellScalarResults( resVarAddr, timeStepIndex ).empty() )
    {
        // Data is computed and allocated, nothing more to do
        return;
    }

    m_resultsData->m_cellScalarResults[sgasResultScalarIndex][timeStepIndex].resize( swatResultValueCount );

    const std::vector<double>& swatForTimeStep = m_resultsData->cellScalarResults( swatAddr, timeStepIndex );
    std::vector<double>*       sgasForTimeStep = m_resultsData->modifiableCellScalarResult( resVarAddr, timeStepIndex );
    if ( !sgasForTimeStep ) return;

#pragma omp parallel for
    for ( int idx = 0; idx < static_cast<int>( swatResultValueCount ); idx++ )
    {
        sgasForTimeStep->at( idx ) = 1.0 - swatForTimeStep[idx];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSgasResultCalculator::isTwoPhaseGasWater() const
{
    const RigEclipseCaseData* eclipseCaseData = m_resultsData->m_ownerCaseData;
    if ( !eclipseCaseData ) return false;

    return RiaPhaseTools::isTwoPhaseGasWater( eclipseCaseData->availablePhases() );
}

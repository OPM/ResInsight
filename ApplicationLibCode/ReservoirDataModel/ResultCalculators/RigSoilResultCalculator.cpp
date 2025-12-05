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

#include "RigSoilResultCalculator.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"

#include "RiaResultNames.h"

//==================================================================================================
///
//==================================================================================================
RigSoilResultCalculator::RigSoilResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigSoilResultCalculator::~RigSoilResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSoilResultCalculator::checkAndCreatePlaceholderEntry( const RigEclipseResultAddress& resVarAddr )
{
    if ( !isMatching( resVarAddr ) ) return;

    // Only create SOIL if oil phase is present
    if ( !hasOilPhase() ) return;

    bool needsToBeStored = false;

    if ( !m_resultsData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ) ) )
    {
        if ( m_resultsData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ) ) ||
             m_resultsData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) ) )
        {
            size_t soilIndex = m_resultsData->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                                      RiaResultNames::soil() ),
                                                                             needsToBeStored );
            m_resultsData->setMustBeCalculated( soilIndex );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSoilResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return resVarAddr.resultName() == RiaResultNames::soil() && resVarAddr.resultCatType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSoilResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    // See similar function in RifReaderOpmRft::values, but the current implementation is not suitable for merging
    // Compute SGAS based on SWAT if the simulation contains no oil
    m_resultsData->testAndComputeSgasForTimeStep( timeStepIndex );

    RigEclipseResultAddress SWATAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() );
    RigEclipseResultAddress SGASAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() );
    RigEclipseResultAddress SSOLAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "SSOL" );

    size_t scalarIndexSWAT =
        m_resultsData->findOrLoadKnownScalarResultForTimeStep( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                        RiaResultNames::swat() ),
                                                               timeStepIndex );
    size_t scalarIndexSGAS =
        m_resultsData->findOrLoadKnownScalarResultForTimeStep( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                        RiaResultNames::sgas() ),
                                                               timeStepIndex );
    size_t scalarIndexSSOL =
        m_resultsData->findOrLoadKnownScalarResultForTimeStep( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "SSOL" ),
                                                               timeStepIndex );

    // Early exit if none of SWAT or SGAS is present
    if ( scalarIndexSWAT == cvf::UNDEFINED_SIZE_T && scalarIndexSGAS == cvf::UNDEFINED_SIZE_T )
    {
        return;
    }

    size_t soilResultValueCount = 0;
    size_t soilTimeStepCount    = 0;

    if ( scalarIndexSWAT != cvf::UNDEFINED_SIZE_T )
    {
        const std::vector<double>& swatForTimeStep = m_resultsData->cellScalarResults( SWATAddr, timeStepIndex );
        if ( !swatForTimeStep.empty() )
        {
            soilResultValueCount = swatForTimeStep.size();
            soilTimeStepCount    = m_resultsData->infoForEachResultIndex()[scalarIndexSWAT].timeStepInfos().size();
        }
    }

    if ( scalarIndexSGAS != cvf::UNDEFINED_SIZE_T )
    {
        const std::vector<double>& sgasForTimeStep = m_resultsData->cellScalarResults( SGASAddr, timeStepIndex );
        if ( !sgasForTimeStep.empty() )
        {
            soilResultValueCount = qMax( soilResultValueCount, sgasForTimeStep.size() );

            size_t sgasTimeStepCount = m_resultsData->infoForEachResultIndex()[scalarIndexSGAS].timeStepInfos().size();
            soilTimeStepCount        = qMax( soilTimeStepCount, sgasTimeStepCount );
        }
    }

    // Make sure memory is allocated for the new SOIL results
    size_t soilResultScalarIndex = m_resultsData->findScalarResultIndexFromAddress( resVarAddr );
    m_resultsData->m_cellScalarResults[soilResultScalarIndex].resize( soilTimeStepCount );

    if ( !m_resultsData->cellScalarResults( resVarAddr, timeStepIndex ).empty() )
    {
        // Data is computed and allocated, nothing more to do
        return;
    }

    m_resultsData->m_cellScalarResults[soilResultScalarIndex][timeStepIndex].resize( soilResultValueCount );

    const std::vector<double>* swatForTimeStep = nullptr;
    const std::vector<double>* sgasForTimeStep = nullptr;
    const std::vector<double>* ssolForTimeStep = nullptr;

    if ( scalarIndexSWAT != cvf::UNDEFINED_SIZE_T )
    {
        swatForTimeStep = &( m_resultsData->cellScalarResults( SWATAddr, timeStepIndex ) );
        if ( swatForTimeStep->empty() )
        {
            swatForTimeStep = nullptr;
        }
    }

    if ( scalarIndexSGAS != cvf::UNDEFINED_SIZE_T )
    {
        sgasForTimeStep = &( m_resultsData->cellScalarResults( SGASAddr, timeStepIndex ) );
        if ( sgasForTimeStep->empty() )
        {
            sgasForTimeStep = nullptr;
        }
    }

    if ( scalarIndexSSOL != cvf::UNDEFINED_SIZE_T )
    {
        ssolForTimeStep = &( m_resultsData->cellScalarResults( SSOLAddr, timeStepIndex ) );
        if ( ssolForTimeStep->empty() )
        {
            ssolForTimeStep = nullptr;
        }
    }

    std::vector<double>* soilForTimeStep = m_resultsData->modifiableCellScalarResult( resVarAddr, timeStepIndex );

#pragma omp parallel for
    for ( int idx = 0; idx < static_cast<int>( soilResultValueCount ); idx++ )
    {
        double soilValue = 1.0;
        if ( sgasForTimeStep )
        {
            soilValue -= sgasForTimeStep->at( idx );
        }

        if ( swatForTimeStep )
        {
            soilValue -= swatForTimeStep->at( idx );
        }

        if ( ssolForTimeStep )
        {
            soilValue -= ssolForTimeStep->at( idx );
        }

        soilForTimeStep->at( idx ) = soilValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSoilResultCalculator::hasOilPhase() const
{
    // Get the available phases from the eclipse case data
    const RigEclipseCaseData* eclipseCaseData = m_resultsData->m_ownerCaseData;
    if ( !eclipseCaseData ) return false;

    std::set<RiaDefines::PhaseType> availablePhases = eclipseCaseData->availablePhases();

    // Consider case with no phase information as having oil phase
    if ( availablePhases.empty() ) return true;

    // Check if oil phase is present
    return availablePhases.count( RiaDefines::PhaseType::OIL_PHASE ) > 0;
}

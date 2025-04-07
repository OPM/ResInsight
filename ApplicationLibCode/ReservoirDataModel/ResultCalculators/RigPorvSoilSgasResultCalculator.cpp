/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RigPorvSoilSgasResultCalculator.h"
#include "RiaDefines.h"
#include "RiaResultNames.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

//==================================================================================================
///
//==================================================================================================
RigPorvSoilSgasResultCalculator::RigPorvSoilSgasResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigPorvSoilSgasResultCalculator::~RigPorvSoilSgasResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigPorvSoilSgasResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return ( ( resVarAddr.resultName() == RiaResultNames::riPorvSoil() || resVarAddr.resultName() == RiaResultNames::riPorvSgas() ||
               resVarAddr.resultName() == RiaResultNames::riPorvSoilSgas() ) &&
             resVarAddr.resultCatType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPorvSoilSgasResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    RigEclipseResultAddress soilAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() );
    RigEclipseResultAddress sgasAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() );
    RigEclipseResultAddress porvAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::porv() );

    if ( resVarAddr.resultName() == RiaResultNames::riPorvSoil() )
    {
        if ( !m_resultsData->hasResultEntry( porvAddress ) || !m_resultsData->hasResultEntry( soilAddress ) ) return;

        calculateProduct( porvAddress, soilAddress, resVarAddr );
    }
    else if ( resVarAddr.resultName() == RiaResultNames::riPorvSgas() )
    {
        if ( !m_resultsData->hasResultEntry( porvAddress ) || !m_resultsData->hasResultEntry( sgasAddress ) ) return;

        calculateProduct( porvAddress, sgasAddress, resVarAddr );
    }
    else if ( resVarAddr.resultName() == RiaResultNames::riPorvSoilSgas() )
    {
        RigEclipseResultAddress soilAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSoil() );
        RigEclipseResultAddress sgasAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSgas() );
        if ( !m_resultsData->hasResultEntry( soilAddress ) || !m_resultsData->hasResultEntry( sgasAddress ) ) return;

        calculateSum( soilAddress, sgasAddress, resVarAddr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPorvSoilSgasResultCalculator::calculateProduct( const RigEclipseResultAddress& in1Addr,
                                                        const RigEclipseResultAddress& in2Addr,
                                                        const RigEclipseResultAddress& outAddr )
{
    calculate( in1Addr, in2Addr, outAddr, std::multiplies<double>{} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPorvSoilSgasResultCalculator::calculateSum( const RigEclipseResultAddress& in1Addr,
                                                    const RigEclipseResultAddress& in2Addr,
                                                    const RigEclipseResultAddress& outAddr )
{
    calculate( in1Addr, in2Addr, outAddr, std::plus<double>{} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigPorvSoilSgasResultCalculator::calculate( const RigEclipseResultAddress&          in1Addr,
                                                 const RigEclipseResultAddress&          in2Addr,
                                                 const RigEclipseResultAddress&          outAddr,
                                                 std::function<double( double, double )> operation )
{
    size_t in1Idx = m_resultsData->findOrLoadKnownScalarResult( in1Addr );
    size_t in2Idx = m_resultsData->findOrLoadKnownScalarResult( in2Addr );

    size_t outIdx = m_resultsData->findOrCreateScalarResultIndex( outAddr, false );
    m_resultsData->m_cellScalarResults[outIdx].resize( m_resultsData->maxTimeStepCount() );

    size_t activeCellCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();
    for ( size_t timeStepIdx = 0; timeStepIdx < m_resultsData->maxTimeStepCount(); timeStepIdx++ )
    {
        size_t                     timeStep1Idx = in1Addr.resultCatType() != RiaDefines::ResultCatType::STATIC_NATIVE ? timeStepIdx : 0;
        const std::vector<double>& in1Results   = m_resultsData->m_cellScalarResults[in1Idx][timeStep1Idx];
        size_t                     timeStep2Idx = in2Addr.resultCatType() != RiaDefines::ResultCatType::STATIC_NATIVE ? timeStepIdx : 0;
        const std::vector<double>& in2Results   = m_resultsData->m_cellScalarResults[in2Idx][timeStep2Idx];

        std::vector<double>& outResults = m_resultsData->m_cellScalarResults[outIdx][timeStepIdx];
        outResults.resize( activeCellCount, 0.0 );

        bool res1ActiveOnly = in1Results.size() == activeCellCount;
        bool res2ActiveOnly = in2Results.size() == activeCellCount;

#pragma omp parallel for
        for ( int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>( m_resultsData->m_ownerMainGrid->totalCellCount() );
              nativeResvCellIndex++ )
        {
            size_t resultIndex = m_resultsData->activeCellInfo()->cellResultIndex( nativeResvCellIndex );
            if ( resultIndex != cvf::UNDEFINED_SIZE_T )
            {
                size_t idx1             = res1ActiveOnly ? resultIndex : nativeResvCellIndex;
                size_t idx2             = res2ActiveOnly ? resultIndex : nativeResvCellIndex;
                outResults[resultIndex] = operation( in1Results[idx1], in2Results[idx2] );
            }
        }
    }
}

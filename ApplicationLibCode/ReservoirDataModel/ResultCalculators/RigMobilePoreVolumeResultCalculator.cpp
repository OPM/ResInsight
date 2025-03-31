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

#include "RigMobilePoreVolumeResultCalculator.h"
#include "RiaDefines.h"
#include "RiaResultNames.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"

#include "RiaLogging.h"

//==================================================================================================
///
//==================================================================================================
RigMobilePoreVolumeResultCalculator::RigMobilePoreVolumeResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//==================================================================================================
///
//==================================================================================================
RigMobilePoreVolumeResultCalculator::~RigMobilePoreVolumeResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMobilePoreVolumeResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return resVarAddr.resultName() == RiaResultNames::mobilePoreVolumeName() &&
           resVarAddr.resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMobilePoreVolumeResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    std::vector<double> porvDataTemp;
    std::vector<double> swcrDataTemp;
    std::vector<double> multpvDataTemp;

    const std::vector<double>* porvResults   = nullptr;
    const std::vector<double>* swcrResults   = nullptr;
    const std::vector<double>* multpvResults = nullptr;

    porvResults = RigCaseCellResultsData::getResultIndexableStaticResult( m_resultsData->activeCellInfo(),
                                                                          m_resultsData,
                                                                          RiaResultNames::porv(),
                                                                          porvDataTemp );
    if ( !porvResults || porvResults->empty() )
    {
        RiaLogging::error( "Assumed PORV, but not data was found." );
        return;
    }

    swcrResults =
        RigCaseCellResultsData::getResultIndexableStaticResult( m_resultsData->activeCellInfo(), m_resultsData, "SWCR", swcrDataTemp );
    multpvResults =
        RigCaseCellResultsData::getResultIndexableStaticResult( m_resultsData->activeCellInfo(), m_resultsData, "MULTPV", multpvDataTemp );

    size_t mobPVIdx = m_resultsData->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                             RiaResultNames::mobilePoreVolumeName() ),
                                                                    false );

    std::vector<double>& mobPVResults = m_resultsData->m_cellScalarResults[mobPVIdx][0];

    // Set up output container to correct number of results
    mobPVResults.resize( porvResults->size() );

    if ( multpvResults && swcrResults )
    {
        for ( size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx )
        {
            mobPVResults[vIdx] = ( *multpvResults )[vIdx] * ( *porvResults )[vIdx] * ( 1.0 - ( *swcrResults )[vIdx] );
        }
    }
    else if ( !multpvResults && swcrResults )
    {
        for ( size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx )
        {
            mobPVResults[vIdx] = ( *porvResults )[vIdx] * ( 1.0 - ( *swcrResults )[vIdx] );
        }
    }
    else if ( !swcrResults && multpvResults )
    {
        for ( size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx )
        {
            mobPVResults[vIdx] = ( *multpvResults )[vIdx] * ( *porvResults )[vIdx];
        }
    }
    else
    {
        mobPVResults.assign( porvResults->begin(), porvResults->end() );
    }
}

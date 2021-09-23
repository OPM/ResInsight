/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigEclipseNativeVisibleCellsStatCalc.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigStatisticsMath.h"
#include "RigWeightedMeanCalc.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseNativeVisibleCellsStatCalc::RigEclipseNativeVisibleCellsStatCalc( RigCaseCellResultsData* cellResultsData,
                                                                            const RigEclipseResultAddress& scalarResultIndex,
                                                                            const cvf::UByteArray* cellVisibilities )
    : m_caseData( cellResultsData )
    , m_resultAddress( scalarResultIndex )
    , m_cellVisibilities( cellVisibilities )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeVisibleCellsStatCalc::minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max )
{
    MinMaxAccumulator acc( min, max );
    traverseCells( acc, timeStepIndex );
    min = acc.min;
    max = acc.max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeVisibleCellsStatCalc::posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg )
{
    PosNegAccumulator acc( pos, neg );
    traverseCells( acc, timeStepIndex );
    pos = acc.pos;
    neg = acc.neg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeVisibleCellsStatCalc::valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount )
{
    SumCountAccumulator acc( valueSum, sampleCount );
    traverseCells( acc, timeStepIndex );
    valueSum    = acc.valueSum;
    sampleCount = acc.sampleCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeVisibleCellsStatCalc::addDataToHistogramCalculator( size_t                  timeStepIndex,
                                                                         RigHistogramCalculator& histogramCalculator )
{
    traverseCells( histogramCalculator, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeVisibleCellsStatCalc::uniqueValues( size_t timeStepIndex, std::set<int>& values )
{
    UniqueValueAccumulator acc;
    traverseCells( acc, timeStepIndex );
    values = acc.uniqueValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigEclipseNativeVisibleCellsStatCalc::timeStepCount()
{
    return m_caseData->timeStepCount( m_resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeVisibleCellsStatCalc::mobileVolumeWeightedMean( size_t timeStepIndex, double& result )
{
    RigEclipseResultAddress mobPorvAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                            RiaResultNames::mobilePoreVolumeName() );

    // For statistics result cases, the pore volume is not available, as
    // RigCaseCellResultsData::createPlaceholderResultEntries has not been executed
    if ( !m_caseData->ensureKnownResultLoaded( mobPorvAddress ) )
    {
        return;
    }

    m_caseData->ensureKnownResultLoaded( mobPorvAddress );

    if ( !m_caseData->hasResultEntry( m_resultAddress ) ) return;
    if ( m_caseData->timeStepCount( m_resultAddress ) == 0 ) return;

    const std::vector<double>& weights = m_caseData->cellScalarResults( mobPorvAddress, 0 );
    const std::vector<double>& values  = m_caseData->cellScalarResults( m_resultAddress, timeStepIndex );

    const RigActiveCellInfo* actCellInfo = m_caseData->activeCellInfo();

    RigWeightedMeanCalc::weightedMeanOverCells( &weights,
                                                &values,
                                                m_cellVisibilities.p(),
                                                true,
                                                actCellInfo,
                                                m_caseData->isUsingGlobalActiveIndex( m_resultAddress ),
                                                &result );
}

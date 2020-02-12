/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigEclipseMultiPropertyStatCalc.h"
#include "RigEclipseNativeStatCalc.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseMultiPropertyStatCalc::RigEclipseMultiPropertyStatCalc()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseMultiPropertyStatCalc::addStatisticsCalculator( RigStatisticsCalculator* statisticsCalculator )
{
    if ( statisticsCalculator )
    {
        m_nativeStatisticsCalculators.push_back( statisticsCalculator );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseMultiPropertyStatCalc::minMaxCellScalarValues( size_t timeStepIndex, double& min, double& max )
{
    for ( size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++ )
    {
        if ( m_nativeStatisticsCalculators.at( i ) )
        {
            m_nativeStatisticsCalculators.at( i )->minMaxCellScalarValues( timeStepIndex, min, max );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseMultiPropertyStatCalc::posNegClosestToZero( size_t timeStepIndex, double& pos, double& neg )
{
    for ( size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++ )
    {
        if ( m_nativeStatisticsCalculators.at( i ) )
        {
            m_nativeStatisticsCalculators.at( i )->posNegClosestToZero( timeStepIndex, pos, neg );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseMultiPropertyStatCalc::valueSumAndSampleCount( size_t timeStepIndex, double& valueSum, size_t& sampleCount )
{
    for ( size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++ )
    {
        if ( m_nativeStatisticsCalculators.at( i ) )
        {
            m_nativeStatisticsCalculators.at( i )->valueSumAndSampleCount( timeStepIndex, valueSum, sampleCount );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseMultiPropertyStatCalc::addDataToHistogramCalculator( size_t                  timeStepIndex,
                                                                    RigHistogramCalculator& histogramCalculator )
{
    for ( size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++ )
    {
        if ( m_nativeStatisticsCalculators.at( i ) )
        {
            m_nativeStatisticsCalculators.at( i )->addDataToHistogramCalculator( timeStepIndex, histogramCalculator );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseMultiPropertyStatCalc::uniqueValues( size_t timeStepIndex, std::set<int>& values )
{
    for ( size_t i = 0; i < m_nativeStatisticsCalculators.size(); i++ )
    {
        if ( m_nativeStatisticsCalculators.at( i ) )
        {
            m_nativeStatisticsCalculators.at( i )->uniqueValues( timeStepIndex, values );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigEclipseMultiPropertyStatCalc::timeStepCount()
{
    if ( m_nativeStatisticsCalculators.size() > 0 )
    {
        return m_nativeStatisticsCalculators[0]->timeStepCount();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseMultiPropertyStatCalc::addNativeStatisticsCalculator( RigCaseCellResultsData*        cellResultsData,
                                                                     const RigEclipseResultAddress& eclipseResultAddress )
{
    if ( eclipseResultAddress.isValid() )
    {
        this->addStatisticsCalculator( new RigEclipseNativeStatCalc( cellResultsData, eclipseResultAddress ) );
    }
}

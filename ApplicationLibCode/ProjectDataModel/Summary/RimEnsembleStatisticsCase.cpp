/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleStatisticsCase.h"

#include "RiaCurveMerger.h"
#include "RiaHashTools.h"
#include "RiaTimeHistoryCurveResampler.h"
#include "Summary/RiaSummaryTools.h"

#include "RigStatisticsMath.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimEnsembleStatisticsCase::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatisticsCase::hasP10Data() const
{
    return !m_p10Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatisticsCase::hasP50Data() const
{
    return !m_p50Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatisticsCase::hasP90Data() const
{
    return !m_p90Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatisticsCase::hasMeanData() const
{
    return !m_meanData.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RimEnsembleStatisticsCase::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    switch ( resultAddress.statisticsType() )
    {
        case RifEclipseSummaryAddressDefines::StatisticsType::P10:
            return { true, m_p10Data };
        case RifEclipseSummaryAddressDefines::StatisticsType::P50:
            return { true, m_p50Data };
        case RifEclipseSummaryAddressDefines::StatisticsType::P90:
            return { true, m_p90Data };
        case RifEclipseSummaryAddressDefines::StatisticsType::MEAN:
            return { true, m_meanData };
        default:
            return { true, {} };
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleStatisticsCase::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_firstSummaryCase && m_firstSummaryCase->summaryReader() )
    {
        return m_firstSummaryCase->summaryReader()->unitName( resultAddress );
    }

    return "Ensemble Statistics Case - Undefined Unit";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleStatisticsCase::caseName() const
{
    return "Ensemble Statistics";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::createSummaryReaderInterface()
{
    // Nothing to do here as RimEnsembleStatisticsCase inherits from RifSummaryReaderInterface
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimEnsembleStatisticsCase::summaryReader()
{
    return this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::calculate( const std::vector<RimSummaryCase*>& summaryCases,
                                           const RifEclipseSummaryAddress&     inputAddress,
                                           bool                                includeIncompleteCurves )
{
    auto hash = RiaHashTools::hash( summaryCases, inputAddress.toEclipseTextAddress(), includeIncompleteCurves );
    if ( hash == m_hash ) return;

    m_hash = hash;

    clearData();

    if ( !inputAddress.isValid() ) return;
    if ( summaryCases.empty() ) return;

    // Use first summary case to get unit system and other meta data
    m_firstSummaryCase = summaryCases.front();

    const auto [minTime, maxTime] = findMinMaxTime( summaryCases, inputAddress );

    // The last time step for the individual realizations in an ensemble is usually identical. Add a small threshold to improve robustness.
    const auto timeThreshold = RiaSummaryTools::calculateTimeThreshold( minTime, maxTime );

    RiaTimeHistoryCurveMerger curveMerger;
    for ( const auto& sumCase : summaryCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddress );
            const auto [isOk, values]            = reader->values( inputAddress );

            if ( values.empty() || timeSteps.empty() ) continue;

            if ( !includeIncompleteCurves && ( timeSteps.back() < timeThreshold ) ) continue;

            curveMerger.addCurveData( timeSteps, values );
        }
    }

    curveMerger.computeInterpolatedValues( includeIncompleteCurves );

    std::vector<std::vector<double>> curveValues;
    for ( size_t i = 0; i < curveMerger.curveCount(); i++ )
    {
        curveValues.push_back( curveMerger.interpolatedYValuesForAllXValues( i ) );
    }

    m_timeSteps = curveMerger.allXValues();

    m_p10Data.reserve( m_timeSteps.size() );
    m_p50Data.reserve( m_timeSteps.size() );
    m_p90Data.reserve( m_timeSteps.size() );
    m_meanData.reserve( m_timeSteps.size() );

    for ( size_t timeStepIndex = 0; timeStepIndex < m_timeSteps.size(); timeStepIndex++ )
    {
        std::vector<double> valuesAtTimeStep;
        valuesAtTimeStep.reserve( curveMerger.curveCount() );

        for ( size_t curveIdx = 0; curveIdx < curveMerger.curveCount(); curveIdx++ )
        {
            valuesAtTimeStep.push_back( curveValues[curveIdx][timeStepIndex] );
        }

        double p10, p50, p90, mean;
        RigStatisticsMath::calculateStatisticsCurves( valuesAtTimeStep, &p10, &p50, &p90, &mean, RigStatisticsMath::PercentileStyle::SWITCHED );
        m_p10Data.push_back( p10 );
        m_p50Data.push_back( p50 );
        m_p90Data.push_back( p90 );
        m_meanData.push_back( mean );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RimEnsembleStatisticsCase::unitSystem() const
{
    if ( m_firstSummaryCase && m_firstSummaryCase->summaryReader() )
    {
        return m_firstSummaryCase->summaryReader()->unitSystem();
    }

    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::clearData()
{
    m_timeSteps.clear();
    m_p10Data.clear();
    m_p50Data.clear();
    m_p90Data.clear();
    m_meanData.clear();
    m_firstSummaryCase = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimEnsembleStatisticsCase::findMinMaxTime( const std::vector<RimSummaryCase*>& sumCases,
                                                                     const RifEclipseSummaryAddress&     inputAddress )
{
    time_t minTime = std::numeric_limits<time_t>::max();
    time_t maxTime = 0;

    for ( const auto& sumCase : sumCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddress );
            if ( !timeSteps.empty() )
            {
                minTime = std::min( timeSteps.front(), minTime );
                maxTime = std::max( timeSteps.back(), maxTime );
            }
        }
    }

    return std::make_pair( minTime, maxTime );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::DateTimePeriod RimEnsembleStatisticsCase::findBestResamplingPeriod( time_t minTimeStep, time_t maxTimeStep )
{
    std::vector<RiaDefines::DateTimePeriod> periods = { RiaDefines::DateTimePeriod::DAY,
                                                        RiaDefines::DateTimePeriod::HOUR,
                                                        RiaDefines::DateTimePeriod::MINUTE };

    for ( auto p : periods )
    {
        size_t numSamples = RiaTimeHistoryCurveResampler::timeStepsFromTimeRange( p, minTimeStep, maxTimeStep ).size();
        // Resampled data should ideally have at least 100 samples to look good.
        if ( numSamples > 100 )
        {
            return p;
        }
    }

    return RiaDefines::DateTimePeriod::DAY;
}

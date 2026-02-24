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
#include "RiaLogging.h"
#include "RiaTimeHistoryCurveResampler.h"
#include "Summary/RiaSummaryTools.h"

#include "RifEclipseSummaryAddress.h"

#include "RigStatisticsMath.h"

#include <algorithm>
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
    return hasPercentileData( 10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatisticsCase::hasP50Data() const
{
    return hasPercentileData( 50 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleStatisticsCase::hasP90Data() const
{
    return hasPercentileData( 90 );
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
bool RimEnsembleStatisticsCase::hasPercentileData( int percentile ) const
{
    if ( percentile < RifEclipseSummaryAddress::MIN_PERCENTILE || percentile > RifEclipseSummaryAddress::MAX_PERCENTILE ) return false;

    auto it = m_percentileData.find( percentile );
    return it != m_percentileData.end() && !it->second.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RimEnsembleStatisticsCase::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( resultAddress.isErrorResult() ) return { true, {} };

    if ( resultAddress.statisticsType() == RifEclipseSummaryAddressDefines::StatisticsType::MEAN )
    {
        return { true, m_meanData };
    }

    int percentile = -1;
    switch ( resultAddress.statisticsType() )
    {
        case RifEclipseSummaryAddressDefines::StatisticsType::P10:
            percentile = 10;
            break;
        case RifEclipseSummaryAddressDefines::StatisticsType::P50:
            percentile = 50;
            break;
        case RifEclipseSummaryAddressDefines::StatisticsType::P90:
            percentile = 90;
            break;
        case RifEclipseSummaryAddressDefines::StatisticsType::CUSTOM:
            percentile = resultAddress.percentile();
            break;
        default:
            return { true, {} };
    }

    if ( percentile >= RifEclipseSummaryAddress::MIN_PERCENTILE && percentile <= RifEclipseSummaryAddress::MAX_PERCENTILE )
    {
        auto it = m_percentileData.find( percentile );
        if ( it != m_percentileData.end() )
        {
            return { true, it->second };
        }
    }

    return { true, {} };
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
                                           bool                                includeIncompleteCurves,
                                           const std::vector<int>&             percentiles )
{
    auto hash = RiaHashTools::hash( summaryCases, inputAddress.toEclipseTextAddress(), includeIncompleteCurves, percentiles );
    if ( hash == m_hash ) return;

    auto startTime = RiaLogging::currentTime();

    m_hash = hash;

    clearData();

    m_requestedPercentiles = percentiles;

    // Always include P10, P50, and P90
    for ( int p : { 10, 50, 90 } )
    {
        if ( std::find( m_requestedPercentiles.begin(), m_requestedPercentiles.end(), p ) == m_requestedPercentiles.end() )
        {
            m_requestedPercentiles.push_back( p );
        }
    }
    std::sort( m_requestedPercentiles.begin(), m_requestedPercentiles.end() );

    if ( !inputAddress.isValid() ) return;
    if ( summaryCases.empty() ) return;

    // Use first summary case to get unit system and other meta data
    m_firstSummaryCase = summaryCases.front();

    const auto [minTime, maxTime] = findMinMaxTime( summaryCases, inputAddress );

    // The last time step for the individual realizations in an ensemble is usually identical. Add a small threshold to improve robustness.
    const auto timeThreshold = RiaSummaryTools::calculateTimeThreshold( minTime, maxTime );

    auto                      interpolationMethod = inputAddress.hasAccumulatedData() ? RiaCurveDefines::InterpolationMethod::LINEAR
                                                                                      : RiaCurveDefines::InterpolationMethod::STEP_RIGHT;
    RiaTimeHistoryCurveMerger curveMerger( interpolationMethod );
    for ( const auto& sumCase : summaryCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( !reader )
        {
            sumCase->setUiToolTip( "No data available for realization" );
            sumCase->setUiReadOnly( true );
        }
        else
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddress );
            const auto [isOk, values]            = reader->values( inputAddress );

            if ( values.empty() || timeSteps.empty() )
            {
                sumCase->setUiToolTip( "No data available for realization" );
                sumCase->setUiReadOnly( true );

                continue;
            }

            if ( !includeIncompleteCurves && ( timeSteps.back() < timeThreshold ) )
            {
                sumCase->setUiToolTip( "Incomplete realization" );
                continue;
            }
            sumCase->setUiToolTip( "" );
            sumCase->setUiReadOnly( false );

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

    m_meanData.reserve( m_timeSteps.size() );

    // Initialize percentile storage
    for ( int p : m_requestedPercentiles )
    {
        m_percentileData[p].reserve( m_timeSteps.size() );
    }

    for ( size_t timeStepIndex = 0; timeStepIndex < m_timeSteps.size(); timeStepIndex++ )
    {
        std::vector<double> valuesAtTimeStep;
        valuesAtTimeStep.reserve( curveMerger.curveCount() );

        for ( size_t curveIdx = 0; curveIdx < curveMerger.curveCount(); curveIdx++ )
        {
            valuesAtTimeStep.push_back( curveValues[curveIdx][timeStepIndex] );
        }

        double mean = RigStatisticsMath::calculateMean( valuesAtTimeStep );
        m_meanData.push_back( mean );

        // Calculate percentiles
        std::vector<double> percentilePositions;
        for ( int p : m_requestedPercentiles )
        {
            percentilePositions.push_back( static_cast<double>( p ) / 100.0 );
        }

        auto percentileValuesResult =
            RigStatisticsMath::calculatePercentiles( valuesAtTimeStep, percentilePositions, RigStatisticsMath::PercentileStyle::SWITCHED );

        if ( percentileValuesResult.has_value() )
        {
            const auto& percentileValues = *percentileValuesResult;
            for ( size_t i = 0; i < m_requestedPercentiles.size(); i++ )
            {
                m_percentileData[m_requestedPercentiles[i]].push_back( percentileValues[i] );
            }
        }
        else
        {
            // If percentiles could not be calculated (e.g. not enough data points or infinity in any of the input values), fill with
            // infinity to indicate missing data.
            for ( size_t i = 0; i < m_requestedPercentiles.size(); i++ )
            {
                m_percentileData[m_requestedPercentiles[i]].push_back( std::numeric_limits<double>::infinity() );
            }
        }
    }

    bool showDebugTiming = false;
    if ( showDebugTiming )
    {
        QString timingText = "RimEnsembleStatisticsCase::calculate" + QString::fromStdString( inputAddress.toEclipseTextAddress() );
        RiaLogging::logElapsedTime( timingText, startTime );
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
    m_meanData.clear();
    m_percentileData.clear();
    m_requestedPercentiles.clear();
    m_firstSummaryCase = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimEnsembleStatisticsCase::keywordCount() const
{
    if ( m_firstSummaryCase && m_firstSummaryCase->summaryReader() )
    {
        m_firstSummaryCase->summaryReader()->createAddressesIfRequired();

        return m_firstSummaryCase->summaryReader()->keywordCount();
    }

    return 0;
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

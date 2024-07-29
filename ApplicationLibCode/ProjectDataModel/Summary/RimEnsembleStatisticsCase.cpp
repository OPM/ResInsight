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

#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RigStatisticsMath.h"

#include "RimEnsembleCurveSet.h"
#include "RimSummaryEnsemble.h"

#include <limits>
#include <set>
#include <vector>

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
    auto quantityName = resultAddress.ensembleStatisticsVectorName();

    if ( quantityName == RifEclipseSummaryAddressDefines::statisticsNameP10() )
        return { true, m_p10Data };
    else if ( quantityName == RifEclipseSummaryAddressDefines::statisticsNameP50() )
        return { true, m_p50Data };
    else if ( quantityName == RifEclipseSummaryAddressDefines::statisticsNameP90() )
        return { true, m_p90Data };
    else if ( quantityName == RifEclipseSummaryAddressDefines::statisticsNameMean() )
        return { true, m_meanData };

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
void RimEnsembleStatisticsCase::calculate( const std::vector<RimSummaryCase*>& sumCases,
                                           const RifEclipseSummaryAddress&     inputAddress,
                                           bool                                includeIncompleteCurves )
{
    clearData();
    if ( !inputAddress.isValid() ) return;
    if ( sumCases.empty() ) return;

    // Use first summary case to get unit system and other meta data
    m_firstSummaryCase = sumCases.front();

    auto [minTimeStep, maxTimeStep]   = findMinMaxTimeStep( sumCases, inputAddress );
    RiaDefines::DateTimePeriod period = findBestResamplingPeriod( minTimeStep, maxTimeStep );

    std::vector<time_t>              allTimeSteps;
    std::vector<std::vector<double>> caseAndTimeStepValues;
    caseAndTimeStepValues.reserve( sumCases.size() );
    for ( const auto& sumCase : sumCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddress );
            auto [isOk, values]                  = reader->values( inputAddress );

            if ( values.empty() ) continue;

            if ( !includeIncompleteCurves && timeSteps.size() != values.size() ) continue;

            auto [resampledTimeSteps, resampledValues] = RiaSummaryTools::resampledValuesForPeriod( inputAddress, timeSteps, values, period );

            if ( allTimeSteps.empty() ) allTimeSteps = resampledTimeSteps;
            caseAndTimeStepValues.push_back( resampledValues );
        }
    }

    m_timeSteps = allTimeSteps;

    for ( size_t timeStepIndex = 0; timeStepIndex < allTimeSteps.size(); timeStepIndex++ )
    {
        std::vector<double> valuesAtTimeStep;
        valuesAtTimeStep.reserve( sumCases.size() );

        for ( const std::vector<double>& caseValues : caseAndTimeStepValues )
        {
            if ( timeStepIndex < caseValues.size() )
            {
                valuesAtTimeStep.push_back( caseValues[timeStepIndex] );
            }
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
std::vector<RimSummaryCase*> RimEnsembleStatisticsCase::validSummaryCases( const std::vector<RimSummaryCase*>& allSumCases,
                                                                           const RifEclipseSummaryAddress&     inputAddress,
                                                                           bool                                includeIncompleteCurves )
{
    std::vector<RimSummaryCase*>                    validCases;
    std::vector<std::pair<RimSummaryCase*, time_t>> times;

    time_t maxTimeStep = 0;

    for ( auto& sumCase : allSumCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddress );
            if ( !timeSteps.empty() )
            {
                time_t lastTimeStep = timeSteps.back();

                maxTimeStep = std::max( lastTimeStep, maxTimeStep );
                times.push_back( std::make_pair( sumCase, lastTimeStep ) );
            }
        }
    }

    for ( const auto& [sumCase, lastTimeStep] : times )
    {
        // Previous versions tested on identical first time step, this test is now removed. For large simulations with
        // numerical issues the first time step can be slightly different
        //
        // https://github.com/OPM/ResInsight/issues/7774
        //
        if ( !includeIncompleteCurves && lastTimeStep != maxTimeStep )
        {
            continue;
        }

        validCases.push_back( sumCase );
    }

    return validCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimEnsembleStatisticsCase::findMinMaxTimeStep( const std::vector<RimSummaryCase*>& sumCases,
                                                                         const RifEclipseSummaryAddress&     inputAddress )
{
    time_t minTimeStep = std::numeric_limits<time_t>::max();
    time_t maxTimeStep = 0;

    for ( const auto& sumCase : sumCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddress );
            if ( !timeSteps.empty() )
            {
                minTimeStep = std::min( timeSteps.front(), minTimeStep );
                maxTimeStep = std::max( timeSteps.back(), maxTimeStep );
            }
        }
    }

    return std::make_pair( minTimeStep, maxTimeStep );
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

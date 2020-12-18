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

#include "RifEnsembleStatisticsReader.h"

#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RigStatisticsMath.h"

#include "RimEnsembleCurveSet.h"

#include <limits>
#include <set>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleStatisticsCase::RimEnsembleStatisticsCase( RimEnsembleCurveSet* curveSet )
{
    m_curveSet = curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimEnsembleStatisticsCase::timeSteps() const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleStatisticsCase::p10() const
{
    return m_p10Data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleStatisticsCase::p50() const
{
    return m_p50Data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleStatisticsCase::p90() const
{
    return m_p90Data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleStatisticsCase::mean() const
{
    return m_meanData;
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
    m_statisticsReader.reset( new RifEnsembleStatisticsReader( this ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimEnsembleStatisticsCase::summaryReader()
{
    if ( !m_statisticsReader )
    {
        createSummaryReaderInterface();
    }
    return m_statisticsReader.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimEnsembleCurveSet* RimEnsembleStatisticsCase::curveSet() const
{
    return m_curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::calculate( const std::vector<RimSummaryCase*>& sumCases, bool includeIncompleteCurves )
{
    auto inputAddress = m_curveSet->summaryAddress();
    if ( m_statisticsReader && inputAddress.isValid() )
    {
        const std::vector<RimSummaryCase*>& validCases =
            validSummaryCases( sumCases, inputAddress, includeIncompleteCurves );

        calculate( validCases, inputAddress, includeIncompleteCurves );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleStatisticsCase::calculate( const std::vector<RimSummaryCase*> sumCases,
                                           const RifEclipseSummaryAddress&    inputAddress,
                                           bool                               includeIncompleteCurves )
{
    std::vector<time_t>              allTimeSteps;
    std::vector<std::vector<double>> caseAndTimeStepValues;

    if ( !inputAddress.isValid() ) return;

    caseAndTimeStepValues.reserve( sumCases.size() );
    for ( const auto& sumCase : sumCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddress );
            std::vector<double>        values;
            reader->values( inputAddress, &values );

            if ( values.empty() ) continue;

            if ( !includeIncompleteCurves && timeSteps.size() != values.size() ) continue;

            RiaTimeHistoryCurveResampler resampler;
            resampler.setCurveData( values, timeSteps );
            if ( RiaSummaryTools::hasAccumulatedData( inputAddress ) )
                resampler.resampleAndComputePeriodEndValues( RiaQDateTimeTools::DateTimePeriod::DAY );
            else
                resampler.resampleAndComputeWeightedMeanValues( RiaQDateTimeTools::DateTimePeriod::DAY );

            if ( allTimeSteps.empty() ) allTimeSteps = resampler.resampledTimeSteps();
            caseAndTimeStepValues.push_back(
                std::vector<double>( resampler.resampledValues().begin(), resampler.resampledValues().end() ) );
        }
    }

    clearData();
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
        RigStatisticsMath::calculateStatisticsCurves( valuesAtTimeStep, &p10, &p50, &p90, &mean );

        if ( p10 != HUGE_VAL ) m_p10Data.push_back( p10 );
        if ( p50 != HUGE_VAL ) m_p50Data.push_back( p50 );
        if ( p90 != HUGE_VAL ) m_p90Data.push_back( p90 );
        if ( mean != HUGE_VAL ) m_meanData.push_back( mean );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RimEnsembleStatisticsCase::unitSystem() const
{
    if ( m_curveSet )
    {
        return m_curveSet->summaryCaseCollection()->unitSystem();
    }
    return RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN;
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimEnsembleStatisticsCase::validSummaryCases( const std::vector<RimSummaryCase*> allSumCases,
                                                                           const RifEclipseSummaryAddress& inputAddress,
                                                                           bool includeIncompleteCurves )
{
    std::vector<RimSummaryCase*>                             validCases;
    std::vector<std::tuple<RimSummaryCase*, time_t, time_t>> times;

    time_t minTimeStep = std::numeric_limits<time_t>::max();
    time_t maxTimeStep = 0;

    for ( auto& sumCase : allSumCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddress );
            if ( !timeSteps.empty() )
            {
                time_t firstTimeStep = timeSteps.front();
                time_t lastTimeStep  = timeSteps.back();

                if ( firstTimeStep < minTimeStep ) minTimeStep = firstTimeStep;
                if ( lastTimeStep > maxTimeStep ) maxTimeStep = lastTimeStep;
                times.push_back( std::make_tuple( sumCase, firstTimeStep, lastTimeStep ) );
            }
        }
    }

    for ( auto& item : times )
    {
        RimSummaryCase* sumCase       = std::get<0>( item );
        time_t          firstTimeStep = std::get<1>( item );
        time_t          lastTimeStep  = std::get<2>( item );

        if ( firstTimeStep == minTimeStep )
        {
            if ( !includeIncompleteCurves && lastTimeStep != maxTimeStep )
            {
                continue;
            }

            validCases.push_back( sumCase );
        }
    }
    return validCases;
}

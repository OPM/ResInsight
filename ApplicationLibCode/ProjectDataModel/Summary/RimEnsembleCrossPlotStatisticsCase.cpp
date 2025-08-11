/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimEnsembleCrossPlotStatisticsCase.h"

#include "RiaLogging.h"
#include "RiaTimeHistoryCurveResampler.h"
#include "Summary/RiaSummaryTools.h"

#include "RifEclipseSummaryAddressDefines.h"

#include "RigStatisticsMath.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleStatisticsCase.h"
#include "RimSummaryEnsemble.h"

#include <limits>
#include <set>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RimEnsembleCrossPlotStatisticsCase::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_adrX.isValid() )
    {
        auto stringToTest = resultAddress.vectorName();
        auto it           = stringToTest.find( m_adrX.vectorName() );
        if ( it != std::string::npos )
        {
            return { true, m_binnedXValues };
        }
    }

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
            break;
    }

    return { true, {} };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleCrossPlotStatisticsCase::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_firstSummaryCase && m_firstSummaryCase->summaryReader() )
    {
        return m_firstSummaryCase->summaryReader()->unitName( resultAddress );
    }

    return "Ensemble Cross Plot - Undefined Unit";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCrossPlotStatisticsCase::caseName() const
{
    return "Ensemble Statistics";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCrossPlotStatisticsCase::createSummaryReaderInterface()
{
    // Nothing to do, as RimEnsembleCrossPlotStatisticsCase inherits RifSummaryReaderInterface
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimEnsembleCrossPlotStatisticsCase::summaryReader()
{
    return this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCrossPlotStatisticsCase::calculate( const std::vector<RimSummaryCase*>& sumCases,
                                                    const RifEclipseSummaryAddress&     inputAddressX,
                                                    const RifEclipseSummaryAddress&     inputAddressY,
                                                    bool                                includeIncompleteCurves,
                                                    int                                 binCount,
                                                    int                                 realizationCountThreshold )
{
    if ( !inputAddressX.isValid() || !inputAddressY.isValid() ) return;
    if ( sumCases.empty() ) return;

    clearData();

    // Use first summary case to get unit system and other meta data
    m_firstSummaryCase = sumCases.front();

    m_adrX = inputAddressX;
    m_adrY = inputAddressY;

    struct SampleData
    {
        double xValue;
        double yValue;
        int    realizationId;
    };

    std::vector<SampleData> sampleData;

    auto [minTimeStep, maxTimeStep]   = RimEnsembleStatisticsCase::findMinMaxTime( sumCases, inputAddressX );
    RiaDefines::DateTimePeriod period = RimEnsembleStatisticsCase::findBestResamplingPeriod( minTimeStep, maxTimeStep );

    for ( const auto& sumCase : sumCases )
    {
        int realizationId = sumCase->caseId();

        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddressX );

            auto [isXOk, valuesX] = reader->values( inputAddressX );
            if ( valuesX.empty() ) continue;

            auto [isYOk, valuesY] = reader->values( inputAddressY );
            if ( valuesY.empty() ) continue;

            if ( !includeIncompleteCurves && timeSteps.size() != valuesX.size() ) continue;
            if ( !includeIncompleteCurves && timeSteps.size() != valuesY.size() ) continue;

            auto [resampledTimeStepsX, resampledValuesX] =
                RiaSummaryTools::resampledValuesForPeriod( inputAddressX, timeSteps, valuesX, period );

            auto [resampledTimeStepsY, resampledValuesY] =
                RiaSummaryTools::resampledValuesForPeriod( inputAddressY, timeSteps, valuesY, period );

            size_t upperLimit = std::min( resampledValuesX.size(), resampledValuesY.size() );

            for ( size_t i = 0; i < upperLimit; i++ )
            {
                sampleData.push_back( { .xValue = resampledValuesX[i], .yValue = resampledValuesY[i], .realizationId = realizationId } );
            }
        }
    }

    if ( sampleData.empty() ) return;

    // Sort on X values
    std::sort( sampleData.begin(), sampleData.end(), []( const auto& lhs, const auto& rhs ) { return lhs.xValue < rhs.xValue; } );

    auto minX        = sampleData.front().xValue;
    auto maxX        = sampleData.back().xValue;
    auto rangeX      = maxX - minX;
    auto deltaRangeX = rangeX / binCount;

    double currentX = minX;

    bool                               anyValidValueForStatistics = false;
    std::map<int, std::vector<double>> yValuesPerRealization;
    for ( auto v : sampleData )
    {
        if ( v.xValue < currentX + deltaRangeX )
        {
            yValuesPerRealization[v.realizationId].emplace_back( v.yValue );
        }
        else
        {
            // Add statistics for current bin if sample count is above threshold
            const bool isRealizationCountOk = static_cast<int>( yValuesPerRealization.size() ) > realizationCountThreshold;
            if ( isRealizationCountOk ) anyValidValueForStatistics = true;

            {
                std::vector<double> meanYPerRealization;

                for ( const auto& [id, values] : yValuesPerRealization )
                {
                    if ( values.empty() ) continue;

                    double sum = 0.0;
                    for ( double value : values )
                    {
                        sum += value;
                    }

                    meanYPerRealization.emplace_back( sum / values.size() );
                }

                double p10  = std::numeric_limits<double>::infinity();
                double p50  = std::numeric_limits<double>::infinity();
                double p90  = std::numeric_limits<double>::infinity();
                double mean = std::numeric_limits<double>::infinity();
                RigStatisticsMath::calculateStatisticsCurves( meanYPerRealization,
                                                              &p10,
                                                              &p50,
                                                              &p90,
                                                              &mean,
                                                              RigStatisticsMath::PercentileStyle::SWITCHED );
                if ( !isRealizationCountOk )
                {
                    p10 = std::numeric_limits<double>::infinity();
                    p50 = std::numeric_limits<double>::infinity();
                    p90 = std::numeric_limits<double>::infinity();
                }

                m_meanData.push_back( mean );
                m_p10Data.push_back( p10 );
                m_p50Data.push_back( p50 );
                m_p90Data.push_back( p90 );

                // Use middle of bin as X value
                m_binnedXValues.emplace_back( currentX + deltaRangeX / 2.0 );
            }

            currentX += deltaRangeX;
            yValuesPerRealization.clear();
        }
    }

    if ( !anyValidValueForStatistics )
    {
        RiaLogging::warning( "Not enough data to compute statistics curves. Consider reducing the realization count threshold." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCrossPlotStatisticsCase::hasP10Data() const
{
    return !m_p10Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCrossPlotStatisticsCase::hasP50Data() const
{
    return !m_p50Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCrossPlotStatisticsCase::hasP90Data() const
{
    return !m_p90Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCrossPlotStatisticsCase::hasMeanData() const
{
    return !m_meanData.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RimEnsembleCrossPlotStatisticsCase::unitSystem() const
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
std::vector<time_t> RimEnsembleCrossPlotStatisticsCase::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_firstSummaryCase && m_firstSummaryCase->summaryReader() )
    {
        return m_firstSummaryCase->summaryReader()->timeSteps( resultAddress );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCrossPlotStatisticsCase::clearData()
{
    m_binnedXValues.clear();
    m_p10Data.clear();
    m_p50Data.clear();
    m_p90Data.clear();
    m_meanData.clear();
    m_firstSummaryCase = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimEnsembleCrossPlotStatisticsCase::keywordCount() const
{
    if ( m_firstSummaryCase && m_firstSummaryCase->summaryReader() )
    {
        return m_firstSummaryCase->summaryReader()->keywordCount();
    }

    return 0;
}

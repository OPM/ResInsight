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

#include "RiaSummaryTools.h"
#include "RiaTimeHistoryCurveResampler.h"

#include "RigStatisticsMath.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleStatisticsCase.h"
#include "RimSummaryCaseCollection.h"

#include <limits>
#include <set>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCrossPlotStatisticsCase::RimEnsembleCrossPlotStatisticsCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCrossPlotStatisticsCase::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    if ( m_adrX.isValid() )
    {
        auto stringToTest = resultAddress.vectorName();
        auto it           = stringToTest.find( m_adrX.vectorName() );
        if ( it != std::string::npos )
        {
            *values = m_binnedXValues;
            return true;
        }
    }

    auto quantityName = resultAddress.ensembleStatisticsVectorName();

    if ( quantityName == ENSEMBLE_STAT_P10_QUANTITY_NAME )
        *values = m_p10Data;
    else if ( quantityName == ENSEMBLE_STAT_P50_QUANTITY_NAME )
        *values = m_p50Data;
    else if ( quantityName == ENSEMBLE_STAT_P90_QUANTITY_NAME )
        *values = m_p90Data;
    else if ( quantityName == ENSEMBLE_STAT_MEAN_QUANTITY_NAME )
        *values = m_meanData;

    return true;
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
                                                    int                                 sampleCountThreshold )
{
    if ( !inputAddressX.isValid() || !inputAddressY.isValid() ) return;
    if ( sumCases.empty() ) return;

    clearData();

    // Use first summary case to get unit system and other meta data
    m_firstSummaryCase = sumCases.front();

    m_adrX = inputAddressX;
    m_adrY = inputAddressY;

    std::vector<std::pair<double, double>> pairs;

    // find resampling period
    // compute resampled values for both X and Y
    // construct xy value pairs

    // sort values on x
    // split X-axis into a number of bins
    // for each bin, compute statistics on Y values in bin

    auto [minTimeStep, maxTimeStep] = RimEnsembleStatisticsCase::findMinMaxTimeStep( sumCases, inputAddressX );

    RiaDefines::DateTimePeriod period = RimEnsembleStatisticsCase::findBestResamplingPeriod( minTimeStep, maxTimeStep );

    for ( const auto& sumCase : sumCases )
    {
        const auto& reader = sumCase->summaryReader();
        if ( reader )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( inputAddressX );

            std::vector<double> valuesX;
            reader->values( inputAddressX, &valuesX );
            if ( valuesX.empty() ) continue;

            std::vector<double> valuesY;
            reader->values( inputAddressY, &valuesY );
            if ( valuesY.empty() ) continue;

            if ( !includeIncompleteCurves && timeSteps.size() != valuesX.size() ) continue;
            if ( !includeIncompleteCurves && timeSteps.size() != valuesY.size() ) continue;

            auto [resampledTimeStepsX, resampledValuesX] =
                RiaSummaryTools::resampledValuesForPeriod( inputAddressX, timeSteps, valuesX, period );

            auto [resampledTimeStepsY, resampledValuesY] =
                RiaSummaryTools::resampledValuesForPeriod( inputAddressY, timeSteps, valuesY, period );

            size_t minimumCount = std::min( resampledValuesX.size(), resampledValuesY.size() );

            for ( size_t i = 0; i < minimumCount; i++ )
            {
                pairs.emplace_back( std::make_pair( resampledValuesX[i], resampledValuesY[i] ) );
            }
        }
    }

    // Sort on X values
    std::sort( pairs.begin(), pairs.end(), []( const auto& lhs, const auto& rhs ) { return lhs.first < rhs.first; } );

    const auto p           = std::minmax_element( pairs.begin(), pairs.end() );
    auto       minX        = p.first->first;
    auto       maxX        = p.second->first;
    auto       rangeX      = maxX - minX;
    auto       deltaRangeX = rangeX / binCount;

    double currentX = minX;

    std::vector<double> binnedYValues;
    for ( auto v : pairs )
    {
        if ( v.first < currentX + deltaRangeX )
        {
            binnedYValues.emplace_back( v.second );
        }
        else
        {
            // Add statistics for current bin if sample count is above threshold
            // TODO: Add option to skip bin if unique realization count is below threshold

            if ( static_cast<int>( binnedYValues.size() ) > sampleCountThreshold )
            {
                double p10, p50, p90, mean;
                RigStatisticsMath::calculateStatisticsCurves( binnedYValues, &p10, &p50, &p90, &mean, RigStatisticsMath::PercentileStyle::SWITCHED );
                m_p10Data.push_back( p10 );
                m_p50Data.push_back( p50 );
                m_p90Data.push_back( p90 );
                m_meanData.push_back( mean );

                m_binnedXValues.emplace_back( currentX );
            }

            currentX += deltaRangeX;
            binnedYValues.clear();
        }
    }
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

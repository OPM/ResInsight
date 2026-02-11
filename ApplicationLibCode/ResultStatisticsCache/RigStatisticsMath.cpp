/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigStatisticsMath.h"

#include "cvfMath.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <expected>
#include <numeric>

namespace
{
bool isValidQuantile( double quantile )
{
    return quantile >= 0.0 && quantile <= 1.0;
}

bool isValidPercentile( double percentile )
{
    return percentile >= 0.0 && percentile <= 100.0;
}

bool areValidQuantiles( const std::vector<double>& quantiles )
{
    return std::all_of( quantiles.begin(), quantiles.end(), isValidQuantile );
}

bool areValidPercentiles( const std::vector<double>& percentiles )
{
    return std::all_of( percentiles.begin(), percentiles.end(), isValidPercentile );
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A function to do basic statistical calculations
///
/// Formulas:
///   mean = sum(x) / n
///
///   Standard deviation (population):
///   stdev = sqrt((n * sum(x^2) - (sum(x))^2)) / n
///
///   Which is equivalent to: sqrt(sum((x - mean)^2) / n)
///
///   range = max - min
///
/// References:
///   Standard deviation: https://en.wikipedia.org/wiki/Standard_deviation
///   Rapid calculation method: https://en.wikipedia.org/wiki/Standard_deviation#Rapid_calculation_methods
//--------------------------------------------------------------------------------------------------

void RigStatisticsMath::calculateBasicStatistics( const std::vector<double>& values,
                                                  double*                    min,
                                                  double*                    max,
                                                  double*                    sum,
                                                  double*                    range,
                                                  double*                    mean,
                                                  double*                    dev )
{
    double m_min( HUGE_VAL );
    double m_max( -HUGE_VAL );
    double m_mean( HUGE_VAL );
    double m_dev( HUGE_VAL );

    double m_sum      = 0.0;
    double sumSquared = 0.0;

    size_t validValueCount = 0;

    for ( size_t i = 0; i < values.size(); i++ )
    {
        double val = values[i];
        if ( RiaStatisticsTools::isInvalidNumber<double>( val ) ) continue;

        validValueCount++;

        if ( val < m_min ) m_min = val;
        if ( val > m_max ) m_max = val;

        m_sum += val;
        sumSquared += ( val * val );
    }

    if ( validValueCount > 0 )
    {
        m_mean = m_sum / validValueCount;

        // http://en.wikipedia.org/wiki/Standard_deviation#Rapid_calculation_methods
        // Running standard deviation

        double s0 = static_cast<double>( validValueCount );
        double s1 = m_sum;
        double s2 = sumSquared;

        m_dev = sqrt( ( s0 * s2 ) - ( s1 * s1 ) ) / s0;
    }

    if ( min ) *min = m_min;
    if ( max ) *max = m_max;
    if ( sum ) *sum = m_sum;
    if ( range ) *range = m_max - m_min;

    if ( mean ) *mean = m_mean;
    if ( dev ) *dev = m_dev;
}

//--------------------------------------------------------------------------------------------------
/// Calculate statistical curves (P10, P50, P90, mean)
///
/// Percentiles (P10, P50, P90) are calculated using linear interpolation:
///   rank = percentile * (n + 1) - 1
///   value = sorted[floor(rank)] + frac(rank) * (sorted[floor(rank)+1] - sorted[floor(rank)])
///
/// Mean is calculated as:
///   mean = sum(x) / n
///
/// References:
///   Percentiles: https://en.wikipedia.org/wiki/Percentile#Third_variant,_C_=_0
///   P10/P50/P90: https://en.wikipedia.org/wiki/Percentile#Definitions
//--------------------------------------------------------------------------------------------------
void RigStatisticsMath::calculateStatisticsCurves( const std::vector<double>& values,
                                                   double*                    p10,
                                                   double*                    p50,
                                                   double*                    p90,
                                                   double*                    mean,
                                                   PercentileStyle            percentileStyle )
{
    CVF_ASSERT( p10 && p50 && p90 && mean );

    if ( values.empty() ) return;

    // Use the vector-based implementation
    std::vector<double> percentiles = { 0.1, 0.5, 0.9 };
    auto                results     = calculatePercentiles( values, percentiles, percentileStyle );

    if ( results.has_value() && results->size() == 3 )
    {
        *p10 = ( *results )[0];
        *p50 = ( *results )[1];
        *p90 = ( *results )[2];
    }
    else
    {
        *p10 = HUGE_VAL;
        *p50 = HUGE_VAL;
        *p90 = HUGE_VAL;
    }

    *mean = calculateMean( values );
}

//--------------------------------------------------------------------------------------------------
/// Calculate percentiles using linear interpolation method
///
/// Formula:
///   rank = percentile * (n + 1) - 1
///
///   If rank is not an integer:
///     value = sorted[floor(rank)] + frac(rank) * (sorted[floor(rank)+1] - sorted[floor(rank)])
///
///   Where frac(rank) is the fractional part of rank
///
///   Valid for percentiles in range [1/(n+1), n/(n+1)]
///
/// References:
///   https://en.wikipedia.org/wiki/Percentile
///   https://en.wikipedia.org/wiki/Percentile#Third_variant,_C_=_0
//--------------------------------------------------------------------------------------------------
std::expected<std::vector<double>, std::string> RigStatisticsMath::calculatePercentiles( const std::vector<double>& values,
                                                                                         const std::vector<double>& quantiles,
                                                                                         PercentileStyle            percentileStyle )
{
    if ( !areValidQuantiles( quantiles ) )
    {
        return std::unexpected( "Quantiles must be in range [0-1]" );
    }

    if ( values.empty() )
    {
        return std::unexpected( "Input values are empty" );
    }

    if ( quantiles.empty() )
    {
        return std::unexpected( "Quantiles are empty" );
    }

    std::vector<double> sortedValues = values;

    sortedValues.erase( std::remove_if( sortedValues.begin(),
                                        sortedValues.end(),
                                        []( double x ) { return !RiaStatisticsTools::isValidNumber( x ); } ),
                        sortedValues.end() );

    if ( sortedValues.empty() )
    {
        return std::unexpected( "No valid values in input" );
    }

    std::sort( sortedValues.begin(), sortedValues.end() );

    int                 valueCount = (int)sortedValues.size();
    std::vector<double> resultValues;
    resultValues.reserve( quantiles.size() );

    for ( size_t i = 0; i < quantiles.size(); ++i )
    {
        double quantile = quantiles[i];

        if ( percentileStyle == PercentileStyle::SWITCHED )
        {
            quantile = 1.0 - quantile;
        }

        double value = HUGE_VAL;

        // Check valid params
        if ( quantile >= 1.0 / ( static_cast<double>( valueCount ) + 1 ) &&
             quantile <= static_cast<double>( valueCount ) / ( static_cast<double>( valueCount ) + 1 ) )
        {
            double rank = quantile * ( valueCount + 1 ) - 1;
            double rankRem;
            double rankFrac = std::modf( rank, &rankRem );
            int    rankInt  = static_cast<int>( rankRem );

            if ( rankInt < valueCount - 1 )
            {
                value = sortedValues[rankInt] + rankFrac * ( sortedValues[rankInt + 1] - sortedValues[rankInt] );
            }
            else
            {
                value = sortedValues.back();
            }
        }

        resultValues.push_back( value );
    }

    return resultValues;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the percentiles of /a inputValues at the pValPosition percentages using the "Nearest Rank"
/// method. This method treats HUGE_VAL as "undefined" values, and ignores these. Will return HUGE_VAL if
/// the inputValues does not contain any valid values
///
/// Formula (Nearest Rank Method):
///   index = floor(n * percentile)
///   value = sorted[index]
///
///   Note: pValPositions are expected as percentages (0-100), converted to fraction (0-1) internally
///
/// References:
///   https://en.wikipedia.org/wiki/Percentile#The_nearest-rank_method
///   https://en.wikipedia.org/wiki/Percentile#First_variant,_C_=_1/2
//--------------------------------------------------------------------------------------------------

std::expected<std::vector<double>, std::string>
    RigStatisticsMath::calculateNearestRankPercentiles( const std::vector<double>&         inputValues,
                                                        const std::vector<double>&         percentiles,
                                                        RigStatisticsMath::PercentileStyle percentileStyle )
{
    if ( !areValidPercentiles( percentiles ) )
    {
        return std::unexpected( "Percentiles must be in range [0-100]" );
    }

    if ( inputValues.empty() )
    {
        return std::unexpected( "Input values are empty" );
    }

    if ( percentiles.empty() )
    {
        return std::unexpected( "Percentiles are empty" );
    }

    std::vector<double> sortedValues;
    sortedValues.reserve( inputValues.size() );

    for ( size_t i = 0; i < inputValues.size(); ++i )
    {
        if ( RiaStatisticsTools::isValidNumber<double>( inputValues[i] ) )
        {
            sortedValues.push_back( inputValues[i] );
        }
    }

    if ( sortedValues.empty() )
    {
        return std::unexpected( "No valid values in input" );
    }

    std::sort( sortedValues.begin(), sortedValues.end() );

    std::vector<double> resultValues( percentiles.size(), HUGE_VAL );
    for ( size_t i = 0; i < percentiles.size(); ++i )
    {
        double quantile = cvf::Math::abs( percentiles[i] ) / 100;
        if ( percentileStyle == RigStatisticsMath::PercentileStyle::SWITCHED ) quantile = 1.0 - quantile;

        size_t index = static_cast<size_t>( sortedValues.size() * quantile );

        if ( index >= sortedValues.size() ) index = sortedValues.size() - 1;

        auto value      = sortedValues[index];
        resultValues[i] = value;
    }

    return resultValues;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the percentiles of /a inputValues at the pValPosition percentages by interpolating input values.
/// This method treats HUGE_VAL as "undefined" values, and ignores these. Will return HUGE_VAL if
/// the inputValues does not contain any valid values
///
/// Formula (Linear Interpolation Method):
///   doubleIndex = (n - 1) * percentile
///   lowerIndex = floor(doubleIndex)
///   upperIndex = lowerIndex + 1
///   weight = doubleIndex - lowerIndex
///
///   value = (1 - weight) * sorted[lowerIndex] + weight * sorted[upperIndex]
///
///   Note: pValPositions are expected as percentages (0-100), convert to fraction (0-1) internally
///
/// References:
///   https://en.wikipedia.org/wiki/Percentile#The_linear_interpolation_between_closest_ranks_method
///   https://en.wikipedia.org/wiki/Percentile#Second_variant,_C_=_1
//--------------------------------------------------------------------------------------------------
std::expected<std::vector<double>, std::string>
    RigStatisticsMath::calculateInterpolatedPercentiles( const std::vector<double>&         inputValues,
                                                         const std::vector<double>&         percentiles,
                                                         RigStatisticsMath::PercentileStyle percentileStyle )
{
    if ( !areValidPercentiles( percentiles ) )
    {
        return std::unexpected( "Percentiles must be in range [0-100]" );
    }

    if ( inputValues.empty() )
    {
        return std::unexpected( "Input values are empty" );
    }

    if ( percentiles.empty() )
    {
        return std::unexpected( "Percentiles are empty" );
    }

    std::vector<double> sortedValues;
    sortedValues.reserve( inputValues.size() );

    for ( size_t i = 0; i < inputValues.size(); ++i )
    {
        if ( RiaStatisticsTools::isValidNumber<double>( inputValues[i] ) )
        {
            sortedValues.push_back( inputValues[i] );
        }
    }

    if ( sortedValues.empty() )
    {
        return std::unexpected( "No valid values in input" );
    }

    std::sort( sortedValues.begin(), sortedValues.end() );

    std::vector<double> resultValues( percentiles.size(), HUGE_VAL );
    for ( size_t i = 0; i < percentiles.size(); ++i )
    {
        double value = HUGE_VAL;

        double quantile = cvf::Math::abs( percentiles[i] ) / 100.0;
        if ( percentileStyle == RigStatisticsMath::PercentileStyle::SWITCHED ) quantile = 1.0 - quantile;

        double doubleIndex = ( sortedValues.size() - 1 ) * quantile;

        size_t lowerValueIndex = static_cast<size_t>( floor( doubleIndex ) );
        size_t upperValueIndex = lowerValueIndex + 1;

        double upperValueWeight = doubleIndex - lowerValueIndex;
        assert( upperValueWeight < 1.0 );

        if ( upperValueIndex < sortedValues.size() )
        {
            value = ( 1.0 - upperValueWeight ) * sortedValues[lowerValueIndex] + upperValueWeight * sortedValues[upperValueIndex];
        }
        else
        {
            value = sortedValues[lowerValueIndex];
        }
        resultValues[i] = value;
    }

    return resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigStatisticsMath::calculateMean( const std::vector<double>& values )
{
    std::vector<double> validValues = values;
    validValues.erase( std::remove_if( validValues.begin(),
                                       validValues.end(),
                                       []( double x ) { return !RiaStatisticsTools::isValidNumber( x ); } ),
                       validValues.end() );

    if ( !validValues.empty() )
    {
        double valueSum = std::accumulate( validValues.begin(), validValues.end(), 0.0 );
        return valueSum / validValues.size();
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramCalculator::RigHistogramCalculator( double min, double max, size_t nBins, std::vector<size_t>* histogram )
{
    assert( histogram );
    assert( nBins > 0 );

    if ( max == min )
    {
        nBins = 1;
    } // Avoid dividing on 0 range

    m_histogram        = histogram;
    m_min              = min;
    m_observationCount = 0;

    // Initialize bins
    m_histogram->resize( nBins );
    for ( size_t i = 0; i < m_histogram->size(); ++i )
        ( *m_histogram )[i] = 0;

    m_range    = max - min;
    m_maxIndex = nBins - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigHistogramCalculator::addValue( double value )
{
    if ( RiaStatisticsTools::isInvalidNumber<double>( value ) ) return;

    size_t index = 0;

    if ( m_maxIndex > 0 ) index = (size_t)( m_maxIndex * ( value - m_min ) / m_range );

    if ( index < m_histogram->size() ) // Just clip to the max min range (-index will overflow to positive )
    {
        ( *m_histogram )[index]++;
        m_observationCount++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigHistogramCalculator::addData( const std::vector<double>& data )
{
    assert( m_histogram );
    for ( size_t i = 0; i < data.size(); ++i )
    {
        addValue( data[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigHistogramCalculator::addData( const std::vector<float>& data )
{
    assert( m_histogram );
    for ( size_t i = 0; i < data.size(); ++i )
    {
        addValue( data[i] );
    }
}

//--------------------------------------------------------------------------------------------------
/// Calculate percentile from histogram data
///
/// Formula:
///   1. Find cumulative count up to target: targetCount = percentile * totalObservations
///   2. Find bin where cumulative count >= targetCount
///   3. Interpolate within bin:
///      unusedFraction = (cumulativeCount - targetCount) / binCount
///      value = binEndValue - unusedFraction * binWidth
///
///   Where:
///     binWidth = (max - min) / numberOfBins
///     binEndValue = min + (binIndex + 1) * binWidth
///
/// References:
///   https://en.wikipedia.org/wiki/Histogram
///   https://en.wikipedia.org/wiki/Percentile#Estimating_percentiles_from_a_histogram
//--------------------------------------------------------------------------------------------------
double RigHistogramCalculator::calculatePercentil( double pVal, RigStatisticsMath::PercentileStyle percentileStyle )
{
    assert( m_histogram );
    assert( m_histogram->size() );
    auto pValClamped = cvf::Math::clamp( pVal, 0.0, 1.0 );
    assert( 0.0 <= pValClamped && pValClamped <= 1.0 );
    if ( percentileStyle == RigStatisticsMath::PercentileStyle::SWITCHED )
    {
        pValClamped = 1.0 - pValClamped;
    }

    double pValObservationCount = pValClamped * m_observationCount;
    if ( pValObservationCount == 0.0 ) return m_min;

    size_t accObsCount = 0;
    double binWidth    = m_range / m_histogram->size();
    for ( size_t binIdx = 0; binIdx < m_histogram->size(); ++binIdx )
    {
        size_t binObsCount = ( *m_histogram )[binIdx];

        accObsCount += binObsCount;
        if ( accObsCount >= pValObservationCount )
        {
            double domainValueAtEndOfBin   = m_min + ( binIdx + 1 ) * binWidth;
            double unusedFractionOfLastBin = static_cast<double>( accObsCount - pValObservationCount ) / binObsCount;

            double histogramBasedEstimate = domainValueAtEndOfBin - unusedFractionOfLastBin * binWidth;

            // See https://resinsight.org/docs/casegroupsandstatistics/#percentile-methods for details

            return histogramBasedEstimate;
        }
    }
    assert( false );

    return HUGE_VAL;
}

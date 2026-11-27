/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cafAssert.h"

#include <algorithm>
#include <cmath> // Needed for HUGE_VAL on Linux
#include <iterator>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
bool XValueComparator<XValueType>::operator()( const XValueType& lhs, const XValueType& rhs ) const
{
    if ( XValueComparator<XValueType>::equals( lhs, rhs ) )
    {
        return false;
    }
    return lhs < rhs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
double XValueComparator<XValueType>::diff( const XValueType& lhs, const XValueType& rhs )
{
    return lhs - rhs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
bool XValueComparator<XValueType>::equals( const XValueType& lhs, const XValueType& rhs )
{
    return lhs == rhs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
RiaCurveMerger<XValueType>::RiaCurveMerger( RiaCurveDefines::InterpolationMethod method )
    : m_isXValuesSharedBetweenCurves( false )
    , m_interpolationMethod( method )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
void RiaCurveMerger<XValueType>::addCurveData( const std::vector<XValueType>& xValues, const std::vector<double>& yValues )
{
    // X and Y can have different sizes when the data is read from file. This is a legitimate run-time condition, e.g. for an
    // ongoing simulation where the last report step is not complete, or when depth values and result values come from different
    // sources. computeInterpolatedValues() assumes that the two vectors have the same number of samples, so discard the trailing
    // samples that have no counterpart. See https://github.com/OPM/ResInsight/issues/12810
    const size_t sampleCount = std::min( xValues.size(), yValues.size() );
    if ( sampleCount == 0 ) return;

    std::vector<XValueType> x( xValues.begin(), xValues.begin() + sampleCount );
    std::vector<double>     y( yValues.begin(), yValues.begin() + sampleCount );

    if ( m_originalValues.empty() )
    {
        m_isXValuesSharedBetweenCurves = true;
    }
    else
    {
        const auto& firstXValues = m_originalValues.front().first;
        m_isXValuesSharedBetweenCurves &= std::equal( firstXValues.begin(), firstXValues.end(), x.begin(), x.end() );
    }

    // Evaluate once per curve, as interpolatedYValue is called for every merged X value
    m_isCurveXValuesMonotonicallyIncreasing.push_back( isMonotonicallyIncreasing( x ) );
    m_originalValues.push_back( std::make_pair( std::move( x ), std::move( y ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
size_t RiaCurveMerger<XValueType>::curveCount() const
{
    return m_interpolatedValuesForAllCurves.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
RiaCurveDataTools::CurveIntervals RiaCurveMerger<XValueType>::validIntervalsForAllXValues() const
{
    return m_validIntervalsForAllXValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
const std::vector<XValueType>& RiaCurveMerger<XValueType>::allXValues() const
{
    return m_allXValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
const std::vector<double>& RiaCurveMerger<XValueType>::interpolatedYValuesForAllXValues( size_t curveIdx ) const
{
    CAF_ASSERT( curveIdx < m_interpolatedValuesForAllCurves.size() );

    return m_interpolatedValuesForAllCurves[curveIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
void RiaCurveMerger<XValueType>::computeInterpolatedValues( bool includeValuesFromPartialCurves )
{
    m_validIntervalsForAllXValues.clear();
    m_allXValues.clear();
    m_interpolatedValuesForAllCurves.clear();

    const size_t curveCount = m_originalValues.size();
    if ( curveCount == 0 )
    {
        return;
    }

    computeUnionOfXValues( includeValuesFromPartialCurves );

    const size_t dataValueCount = m_allXValues.size();
    if ( dataValueCount == 0 )
    {
        return;
    }

    m_interpolatedValuesForAllCurves.resize( curveCount );

    std::vector<double> accumulatedValidValues( dataValueCount, 1.0 );

    if ( m_isXValuesSharedBetweenCurves )
    {
        // All curves share the X values of the merged list, so no interpolation is required
        for ( size_t curveIdx = 0; curveIdx < curveCount; curveIdx++ )
        {
            const std::vector<double>& originalValues = m_originalValues[curveIdx].second;
            m_interpolatedValuesForAllCurves[curveIdx].assign( originalValues.begin(), originalValues.begin() + dataValueCount );
        }

#pragma omp parallel for
        for ( int valueIndex = 0; valueIndex < static_cast<int>( dataValueCount ); valueIndex++ )
        {
            for ( size_t curveIdx = 0; curveIdx < curveCount; curveIdx++ )
            {
                if ( !RiaCurveDataTools::isValidValue( m_interpolatedValuesForAllCurves[curveIdx][valueIndex], false ) )
                {
                    accumulatedValidValues[valueIndex] = HUGE_VAL;
                    break;
                }
            }
        }
    }
    else
    {
        for ( auto& curveValues : m_interpolatedValuesForAllCurves )
        {
            curveValues.resize( dataValueCount );
        }

        // Process blocks of consecutive merged X values in parallel. The merged X values are sorted, so each curve can be searched
        // by moving a cursor forward through the block. Each iteration writes only to its own elements in every curve and in
        // accumulatedValidValues, so no synchronization is required
        constexpr int blockSize  = 4096;
        const int     valueCount = static_cast<int>( dataValueCount );
        const int     blockCount = ( valueCount + blockSize - 1 ) / blockSize;

#pragma omp parallel for
        for ( int blockIndex = 0; blockIndex < blockCount; blockIndex++ )
        {
            const int blockStart = blockIndex * blockSize;
            const int blockEnd   = std::min( valueCount, blockStart + blockSize );

            for ( size_t curveIdx = 0; curveIdx < curveCount; curveIdx++ )
            {
                size_t cursor = std::numeric_limits<size_t>::max();

                for ( int valueIndex = blockStart; valueIndex < blockEnd; valueIndex++ )
                {
                    const double interpolValue = interpolatedYValue( m_allXValues[valueIndex],
                                                                     m_originalValues[curveIdx].first,
                                                                     m_originalValues[curveIdx].second,
                                                                     m_interpolationMethod,
                                                                     m_isCurveXValuesMonotonicallyIncreasing[curveIdx],
                                                                     &cursor );

                    if ( !RiaCurveDataTools::isValidValue( interpolValue, false ) )
                    {
                        accumulatedValidValues[valueIndex] = HUGE_VAL;
                    }

                    m_interpolatedValuesForAllCurves[curveIdx][valueIndex] = interpolValue;
                }
            }
        }
    }

    m_validIntervalsForAllXValues = RiaCurveDataTools::calculateIntervalsOfValidValues( accumulatedValidValues, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
void RiaCurveMerger<XValueType>::computeUnionOfXValues( bool includeValuesForPartialCurves )
{
    m_allXValues.clear();

    if ( m_isXValuesSharedBetweenCurves && !m_originalValues.empty() )
    {
        // If all curves have the same X values, use the X values from the first curve.
        m_allXValues = m_originalValues.front().first;
        return;
    }

    // The curves are merged one at a time into a sorted list without duplicates. Sorted curves, which is the common case, are
    // merged in linear time.
    XValueType lowerBound = std::numeric_limits<XValueType>::lowest();
    XValueType upperBound = std::numeric_limits<XValueType>::max();

    if ( !includeValuesForPartialCurves )
    {
        // Values outside the range of any of the curves are removed
        for ( size_t curveIdx = 0; curveIdx < m_originalValues.size(); curveIdx++ )
        {
            const auto& xValues = m_originalValues[curveIdx].first;
            if ( xValues.empty() ) continue;

            if ( m_isCurveXValuesMonotonicallyIncreasing[curveIdx] )
            {
                lowerBound = std::max( lowerBound, xValues.front() );
                upperBound = std::min( upperBound, xValues.back() );
            }
            else
            {
                auto minmax_it = std::minmax_element( xValues.begin(), xValues.end() );
                lowerBound     = std::max( lowerBound, *minmax_it.first );
                upperBound     = std::min( upperBound, *minmax_it.second );
            }
        }
    }

    // Create one sorted run without duplicates for each curve
    std::vector<std::vector<XValueType>> runs;
    runs.reserve( m_originalValues.size() );
    for ( size_t curveIdx = 0; curveIdx < m_originalValues.size(); curveIdx++ )
    {
        const auto& xValues = m_originalValues[curveIdx].first;

        std::vector<XValueType> run;
        run.reserve( xValues.size() );
        for ( const auto& x : xValues )
        {
            if ( x >= lowerBound && x <= upperBound ) run.push_back( x );
        }
        if ( run.empty() ) continue;

        if ( !m_isCurveXValuesMonotonicallyIncreasing[curveIdx] ) std::sort( run.begin(), run.end() );

        run.erase( std::unique( run.begin(),
                                run.end(),
                                []( const XValueType& lhs, const XValueType& rhs ) { return XComparator::equals( lhs, rhs ); } ),
                   run.end() );
        runs.push_back( std::move( run ) );
    }

    // Merge neighbouring runs pairwise until a single run remains. This is linear in the number of values for each level, giving
    // O(N log(curveCount)) in total, instead of repeatedly merging into one growing list. set_union removes values that are equal
    // according to XComparator.
    while ( runs.size() > 1 )
    {
        std::vector<std::vector<XValueType>> mergedRuns( ( runs.size() + 1 ) / 2 );

#pragma omp parallel for
        for ( int mergeIndex = 0; mergeIndex < static_cast<int>( mergedRuns.size() ); mergeIndex++ )
        {
            auto& first  = runs[2 * mergeIndex];
            auto& target = mergedRuns[mergeIndex];

            if ( static_cast<size_t>( 2 * mergeIndex + 1 ) < runs.size() )
            {
                const auto& second = runs[2 * mergeIndex + 1];
                target.reserve( first.size() + second.size() );
                std::set_union( first.begin(), first.end(), second.begin(), second.end(), std::back_inserter( target ), XComparator() );
            }
            else
            {
                target = std::move( first );
            }
        }

        runs = std::move( mergedRuns );
    }

    if ( !runs.empty() ) m_allXValues = std::move( runs.front() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
void RiaCurveMerger<XValueType>::removeValuesForPartialCurves( std::set<XValueType, XComparator>&                    unionOfXValues,
                                                               const std::vector<std::pair<XValueType, XValueType>>& originalXBounds )
{
    for ( auto it = unionOfXValues.begin(); it != unionOfXValues.end(); )
    {
        bool outsideBounds = false;
        for ( const auto& curveXBounds : originalXBounds )
        {
            if ( *it < curveXBounds.first || *it > curveXBounds.second )
            {
                outsideBounds = true;
                break;
            }
        }
        if ( outsideBounds )
        {
            it = unionOfXValues.erase( it );
        }
        else
        {
            ++it;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
double RiaCurveMerger<XValueType>::interpolatedYValue( const XValueType&                    interpolationXValue,
                                                       const std::vector<XValueType>&       xValues,
                                                       const std::vector<double>&           yValues,
                                                       RiaCurveDefines::InterpolationMethod interpolationMethod )
{
    if ( xValues.empty() ) return HUGE_VAL;
    if ( yValues.size() != xValues.size() ) return HUGE_VAL;

    return interpolatedYValue( interpolationXValue, xValues, yValues, interpolationMethod, isMonotonicallyIncreasing( xValues ), nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
double RiaCurveMerger<XValueType>::interpolatedYValue( const XValueType&                    interpolationXValue,
                                                       const std::vector<XValueType>&       xValues,
                                                       const std::vector<double>&           yValues,
                                                       RiaCurveDefines::InterpolationMethod interpolationMethod,
                                                       bool                                 isXValuesMonotonicallyIncreasing,
                                                       size_t*                              sortedSearchCursor )
{
    if ( xValues.empty() ) return HUGE_VAL;
    if ( yValues.size() != xValues.size() ) return HUGE_VAL;

    size_t startIndex = 0;

    if ( isXValuesMonotonicallyIncreasing )
    {
        // Use lower_bound to find the first element that is not less than the interpolation value using a threshold that is larger than
        // the threshold used in XComparator::equals
        //
        // Using this method will improve the performance significantly for large datasets, as std::lower_bound is much faster than the
        // search in the loop below. One relevant use case is computation of delta summary values for large datasets. Here the time
        // steps are specified in increasing order
        //
        XValueType threshold = 1.0e-6 * xValues.back();
        XValueType limit     = interpolationXValue - threshold;

        size_t lowerBoundIndex = 0;
        if ( sortedSearchCursor && *sortedSearchCursor != std::numeric_limits<size_t>::max() )
        {
            // When called with increasing interpolation values, the result can be found by moving forward from the previous result
            lowerBoundIndex = *sortedSearchCursor;
            while ( lowerBoundIndex < xValues.size() && xValues[lowerBoundIndex] < limit )
            {
                lowerBoundIndex++;
            }
        }
        else
        {
            lowerBoundIndex = std::lower_bound( xValues.begin(), xValues.end(), limit ) - xValues.begin();
        }

        if ( sortedSearchCursor ) *sortedSearchCursor = lowerBoundIndex;
        if ( lowerBoundIndex == xValues.size() ) return HUGE_VAL;

        startIndex = lowerBoundIndex;
        if ( startIndex > 0 ) startIndex--;
    }

    const size_t xCount                   = xValues.size();
    const bool   removeInterpolatedValues = false;

    // The second element of a pass becomes the first element of the next pass, and has already been compared for equality
    bool isFirstKnownNotEqual = false;

    for ( size_t firstI = startIndex; firstI < xCount; firstI++ )
    {
        const XValueType firstX = xValues[firstI];

        if ( !isFirstKnownNotEqual && XComparator::equals( firstX, interpolationXValue ) )
        {
            const double& firstValue = yValues[firstI];
            if ( !RiaCurveDataTools::isValidValue( firstValue, removeInterpolatedValues ) )
            {
                return HUGE_VAL;
            }

            return firstValue;
        }

        // With sorted X values, no later element can match or bracket the interpolation value
        if ( isXValuesMonotonicallyIncreasing && firstX > interpolationXValue ) return HUGE_VAL;

        size_t secondI = firstI + 1;

        if ( secondI < xCount )
        {
            const XValueType secondX = xValues[secondI];

            if ( XComparator::equals( secondX, interpolationXValue ) )
            {
                const double& secondValue = yValues[secondI];
                if ( !RiaCurveDataTools::isValidValue( secondValue, removeInterpolatedValues ) )
                {
                    return HUGE_VAL;
                }

                return secondValue;
            }

            if ( firstX < interpolationXValue && secondX > interpolationXValue )
            {
                const double& firstValue  = yValues[firstI];
                const double& secondValue = yValues[secondI];

                bool isFirstValid = RiaCurveDataTools::isValidValue( firstValue, removeInterpolatedValues );
                if ( !isFirstValid ) return HUGE_VAL;

                bool isSecondValid = RiaCurveDataTools::isValidValue( secondValue, removeInterpolatedValues );
                if ( !isSecondValid ) return HUGE_VAL;

                if ( interpolationMethod == RiaCurveDefines::InterpolationMethod::STEP_RIGHT )
                {
                    return secondValue;
                }

                if ( interpolationMethod == RiaCurveDefines::InterpolationMethod::STEP_LEFT )
                {
                    return firstValue;
                }

                const double firstDiff  = fabs( XComparator::diff( interpolationXValue, firstX ) );
                const double secondDiff = fabs( XComparator::diff( secondX, interpolationXValue ) );
                const double totalDiff  = firstDiff + secondDiff;

                double val;
                if ( totalDiff < std::numeric_limits<double>::epsilon() )
                {
                    // Points are essentially identical - use either value
                    val = firstValue; // or secondValue, or average: (firstValue + secondValue) * 0.5
                }
                else
                {
                    const double firstWeight  = secondDiff / totalDiff;
                    const double secondWeight = firstDiff / totalDiff;
                    val                       = ( firstValue * firstWeight ) + ( secondValue * secondWeight );
                }

                return val;
            }

            isFirstKnownNotEqual = true;
        }
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
bool RiaCurveMerger<XValueType>::isMonotonicallyIncreasing( const std::vector<XValueType>& container )
{
    return std::adjacent_find( container.begin(),
                               container.end(),
                               []( const auto& a, const auto& b )
                               {
                                   return b < a; // Returns true if not monotonically increasing
                               } ) == container.end();
}

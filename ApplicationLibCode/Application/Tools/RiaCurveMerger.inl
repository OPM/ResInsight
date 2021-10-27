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

#include <algorithm>
#include <cmath> // Needed for HUGE_VAL on Linux

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
RiaCurveMerger<XValueType>::RiaCurveMerger()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
void RiaCurveMerger<XValueType>::addCurveData( const std::vector<XValueType>& xValues, const std::vector<double>& yValues )
{
    CVF_ASSERT( xValues.size() == yValues.size() );

    if ( !xValues.empty() )
    {
        m_originalValues.push_back( std::make_pair( xValues, yValues ) );
    }
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
    CVF_ASSERT( curveIdx < m_interpolatedValuesForAllCurves.size() );

    return m_interpolatedValuesForAllCurves[curveIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
std::vector<double>& RiaCurveMerger<XValueType>::interpolatedYValuesForAllXValues( size_t curveIdx )
{
    CVF_ASSERT( curveIdx < m_interpolatedValuesForAllCurves.size() );

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

    computeUnionOfXValues( includeValuesFromPartialCurves );

    const size_t curveCount = m_originalValues.size();
    if ( curveCount == 0 )
    {
        return;
    }

    const size_t dataValueCount = m_allXValues.size();
    if ( dataValueCount == 0 )
    {
        return;
    }

    m_interpolatedValuesForAllCurves.resize( curveCount );

    std::vector<double> accumulatedValidValues( dataValueCount, 1.0 );

    for ( size_t curveIdx = 0; curveIdx < curveCount; curveIdx++ )
    {
        std::vector<double>& curveValues = m_interpolatedValuesForAllCurves[curveIdx];
        curveValues.resize( dataValueCount );

        for ( size_t valueIndex = 0; valueIndex < dataValueCount; valueIndex++ )
        {
            double interpolValue = interpolatedYValue( m_allXValues[valueIndex],
                                                       m_originalValues[curveIdx].first,
                                                       m_originalValues[curveIdx].second );
            if ( !RiaCurveDataTools::isValidValue( interpolValue, false ) )
            {
                accumulatedValidValues[valueIndex] = HUGE_VAL;
            }

            curveValues[valueIndex] = interpolValue;
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

    std::set<XValueType, XComparator> unionOfXValues;

    std::vector<std::pair<XValueType, XValueType>> originalXBounds;
    for ( const auto& curveData : m_originalValues )
    {
        if ( curveData.first.empty() ) continue;

        for ( const auto& x : curveData.first )
        {
            unionOfXValues.insert( x );
        }
        auto minmax_it = std::minmax_element( curveData.first.begin(), curveData.first.end() );
        originalXBounds.push_back( std::make_pair( *( minmax_it.first ), *( minmax_it.second ) ) );
    }

    if ( !includeValuesForPartialCurves ) removeValuesForPartialCurves( unionOfXValues, originalXBounds );

    m_allXValues = std::vector<XValueType>( unionOfXValues.begin(), unionOfXValues.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XValueType>
void RiaCurveMerger<XValueType>::removeValuesForPartialCurves( std::set<XValueType, XComparator>& unionOfXValues,
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
double RiaCurveMerger<XValueType>::interpolatedYValue( const XValueType&              interpolationXValue,
                                                       const std::vector<XValueType>& xValues,
                                                       const std::vector<double>&     yValues )
{
    if ( yValues.size() != xValues.size() ) return HUGE_VAL;

    const bool removeInterpolatedValues = false;

    for ( size_t firstI = 0; firstI < xValues.size(); firstI++ )
    {
        if ( XComparator::equals( xValues.at( firstI ), interpolationXValue ) )
        {
            const double& firstValue = yValues.at( firstI );
            if ( !RiaCurveDataTools::isValidValue( firstValue, removeInterpolatedValues ) )
            {
                return HUGE_VAL;
            }

            return firstValue;
        }

        size_t secondI = firstI + 1;

        if ( secondI < xValues.size() )
        {
            if ( XComparator::equals( xValues.at( secondI ), interpolationXValue ) )
            {
                const double& secondValue = yValues.at( secondI );
                if ( !RiaCurveDataTools::isValidValue( secondValue, removeInterpolatedValues ) )
                {
                    return HUGE_VAL;
                }

                return secondValue;
            }

            if ( xValues.at( firstI ) < interpolationXValue && xValues.at( secondI ) > interpolationXValue )
            {
                const double& firstValue  = yValues.at( firstI );
                const double& secondValue = yValues.at( secondI );

                bool isFirstValid = RiaCurveDataTools::isValidValue( firstValue, removeInterpolatedValues );
                if ( !isFirstValid ) return HUGE_VAL;

                bool isSecondValid = RiaCurveDataTools::isValidValue( secondValue, removeInterpolatedValues );
                if ( !isSecondValid ) return HUGE_VAL;

                double firstDiff  = fabs( XComparator::diff( interpolationXValue, xValues.at( firstI ) ) );
                double secondDiff = fabs( XComparator::diff( xValues.at( secondI ), interpolationXValue ) );

                double firstWeight  = secondDiff / ( firstDiff + secondDiff );
                double secondWeight = firstDiff / ( firstDiff + secondDiff );

                double val = ( firstValue * firstWeight ) + ( secondValue * secondWeight );

                CVF_ASSERT( RiaCurveDataTools::isValidValue( val, removeInterpolatedValues ) );

                return val;
            }
        }
    }

    return HUGE_VAL;
}

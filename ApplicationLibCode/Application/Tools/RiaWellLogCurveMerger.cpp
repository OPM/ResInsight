/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RiaWellLogCurveMerger.h"

#include "RiaCurveMerger.h"

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaWellLogCurveMerger::RiaWellLogCurveMerger()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaWellLogCurveMerger::addCurveData( const std::vector<double>& xValues, const std::vector<double>& yValues )
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
size_t RiaWellLogCurveMerger::curveCount() const
{
    return m_lookupValuesForAllCurves.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCurveDataTools::CurveIntervals RiaWellLogCurveMerger::validIntervalsForAllXValues() const
{
    return m_validIntervalsForAllXValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RiaWellLogCurveMerger::allXValues() const
{
    return m_allXValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RiaWellLogCurveMerger::lookupYValuesForAllXValues( size_t curveIdx ) const
{
    CVF_ASSERT( curveIdx < m_lookupValuesForAllCurves.size() );

    return m_lookupValuesForAllCurves[curveIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaWellLogCurveMerger::computeLookupValues( bool includeValuesFromPartialCurves )
{
    m_validIntervalsForAllXValues.clear();
    m_allXValues.clear();
    m_lookupValuesForAllCurves.clear();

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

    m_lookupValuesForAllCurves.resize( curveCount );

    std::vector<double> accumulatedValidValues( dataValueCount, 1.0 );

    for ( size_t curveIdx = 0; curveIdx < curveCount; curveIdx++ )
    {
        std::vector<double>& curveValues = m_lookupValuesForAllCurves[curveIdx];
        curveValues.resize( dataValueCount );

        for ( size_t valueIndex = 0; valueIndex < dataValueCount; valueIndex++ )
        {
            double interpolValue = lookupYValue( m_allXValues[valueIndex],
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

void RiaWellLogCurveMerger::computeUnionOfXValues( bool includeValuesForPartialCurves )
{
    m_allXValues.clear();

    std::set<double, XValueComparator<double>> unionOfXValues;

    std::vector<std::pair<double, double>> originalXBounds;
    for ( const auto& curveData : m_originalValues )
    {
        if ( curveData.first.empty() ) continue;

        // Well log data has top and bottom depth for each zone
        // Find the mid point in the zone to
        const std::vector<double>& xValues = curveData.first;
        for ( size_t i = 0; i < xValues.size(); i += 2 )
        {
            if ( i + 1 < xValues.size() )
            {
                double top    = xValues.at( i );
                double bottom = xValues.at( i + 1 );
                double mid    = ( top + bottom ) / 2.0;
                unionOfXValues.insert( mid );
            }
        }
        auto minmax_it = std::minmax_element( curveData.first.begin(), curveData.first.end() );
        originalXBounds.push_back( std::make_pair( *( minmax_it.first ), *( minmax_it.second ) ) );
    }

    if ( !includeValuesForPartialCurves )
        RiaCurveMerger<double>::removeValuesForPartialCurves( unionOfXValues, originalXBounds );

    m_allXValues = std::vector<double>( unionOfXValues.begin(), unionOfXValues.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaWellLogCurveMerger::lookupYValue( const double&              interpolationXValue,
                                            const std::vector<double>& xValues,
                                            const std::vector<double>& yValues )
{
    if ( yValues.size() != xValues.size() ) return HUGE_VAL;

    const bool removeInterpolatedValues = false;

    if ( interpolationXValue < xValues[0] ) return HUGE_VAL;

    for ( size_t firstI = 0; firstI < xValues.size(); firstI++ )
    {
        if ( xValues.at( firstI ) >= interpolationXValue )
        {
            double firstValue = yValues.at( firstI );
            if ( !RiaCurveDataTools::isValidValue( firstValue, removeInterpolatedValues ) )
            {
                return HUGE_VAL;
            }

            return firstValue;
        }
    }

    return HUGE_VAL;
}

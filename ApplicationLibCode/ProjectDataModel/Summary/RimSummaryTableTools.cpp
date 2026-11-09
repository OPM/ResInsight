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

#include "RimSummaryTableTools.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryTableTools::isValueMatchingFilter( double value, const ValueFilter& filter )
{
    switch ( filter.type )
    {
        case ValueFilterType::ABOVE:
            return value > filter.lowerThreshold;
        case ValueFilterType::BELOW:
            return value < filter.upperThreshold;
        case ValueFilterType::BETWEEN:
            return value >= filter.lowerThreshold && value <= filter.upperThreshold;
        case ValueFilterType::OUTSIDE:
            return value < filter.lowerThreshold || value > filter.upperThreshold;
        case ValueFilterType::NONE:
        default:
            return value > 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryTableTools::hasValueMatchingFilter( const std::vector<double>& values, const ValueFilter& filter )
{
    return std::any_of( values.begin(), values.end(), [&]( double value ) { return isValueMatchingFilter( value, filter ); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryTableTools::VectorData>
    RimSummaryTableTools::vectorsMatchingFilter( const std::vector<VectorData>& vectorDataCollection, const ValueFilter& filter )
{
    std::vector<VectorData> vectorsMatchingFilter;
    for ( const auto& vectorData : vectorDataCollection )
    {
        if ( hasValueMatchingFilter( vectorData.values, filter ) )
        {
            vectorsMatchingFilter.push_back( vectorData );
        }
    }
    return vectorsMatchingFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableTools::sortVectorDataOnDate( TableData& rTableData )
{
    // Sort vector data on date (vectors with no values matching the value filter placed last). Ties are sorted on category name, as
    // the incoming order depends on string pool indices and is not deterministic.
    std::sort( rTableData.vectorDataCollection.begin(),
               rTableData.vectorDataCollection.end(),
               [&rTableData]( const VectorData& v1, const VectorData& v2 )
               {
                   const bool firstHasNoValueMatchingFilter  = !hasValueMatchingFilter( v1.values, rTableData.valueFilter );
                   const bool secondHasNoValueMatchingFilter = !hasValueMatchingFilter( v2.values, rTableData.valueFilter );
                   return std::tie( firstHasNoValueMatchingFilter, v1.firstTimeStep, v1.lastTimeStep, v1.category ) <
                          std::tie( secondHasNoValueMatchingFilter, v2.firstTimeStep, v2.lastTimeStep, v2.category );
               } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimSummaryTableTools::categoryNames( const std::vector<VectorData>& vectorDataCollection )
{
    if ( vectorDataCollection.empty() ) return {};

    std::vector<QString> categoryNames;
    for ( const auto& vectorData : vectorDataCollection )
    {
        categoryNames.push_back( vectorData.category );
    }
    return categoryNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryTableTools::valuesForTimeSteps( const VectorData& vectorData, const std::set<time_t>& timeSteps )
{
    std::vector<double> values;
    values.reserve( timeSteps.size() );

    const size_t valueCount = std::min( vectorData.timeSteps.size(), vectorData.values.size() );
    for ( const auto timeStep : timeSteps )
    {
        const auto end = vectorData.timeSteps.begin() + valueCount;
        const auto it  = std::lower_bound( vectorData.timeSteps.begin(), end, timeStep );
        values.push_back( ( it != end && *it == timeStep ) ? vectorData.values[it - vectorData.timeSteps.begin()]
                                                           : std::numeric_limits<double>::quiet_NaN() );
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTableTools::ValueFilter RimSummaryTableTools::valueFilterFromLegacyThreshold( double thresholdValue )
{
    // ABOVE is exclusive, so the largest value below the threshold is used to keep values equal to the threshold
    return { .type           = ValueFilterType::ABOVE,
             .lowerThreshold = std::nextafter( thresholdValue, -std::numeric_limits<double>::infinity() ),
             .upperThreshold = 0.0 };
}

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

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryTableTools::hasValueAboveThreshold( const std::vector<double>& values, double thresholdValue )
{
    // Detect if values contain at least one value above threshold
    const auto valueAboveThresholdItr = std::find_if( values.begin(), values.end(), [&]( double value ) { return value > thresholdValue; } );
    return valueAboveThresholdItr != values.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryTableTools::VectorData>
    RimSummaryTableTools::vectorsAboveThreshold( const std::vector<VectorData>& vectorDataCollection, double thresholdValue )
{
    std::vector<VectorData> vectorsAboveThreshold;
    for ( const auto& vectorData : vectorDataCollection )
    {
        if ( hasValueAboveThreshold( vectorData.values, thresholdValue ) )
        {
            vectorsAboveThreshold.push_back( vectorData );
        }
    }
    return vectorsAboveThreshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableTools::sortVectorDataOnDate( TableData& rTableData )
{
    // Sort vector data on date (vectors with no values above threshold placed last):
    std::sort( rTableData.vectorDataCollection.begin(),
               rTableData.vectorDataCollection.end(),
               [&rTableData]( const VectorData& v1, const VectorData& v2 )
               {
                   const bool firstHasValueAboveThreshold  = hasValueAboveThreshold( v1.values, rTableData.thresholdValue );
                   const bool secondHasValueAboveThreshold = hasValueAboveThreshold( v2.values, rTableData.thresholdValue );
                   if ( !firstHasValueAboveThreshold ) return false;
                   if ( firstHasValueAboveThreshold && !secondHasValueAboveThreshold ) return true;
                   if ( v1.firstTimeStep < v2.firstTimeStep ) return true;
                   if ( v1.firstTimeStep == v2.firstTimeStep && v1.lastTimeStep < v2.lastTimeStep ) return true;
                   return false;
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

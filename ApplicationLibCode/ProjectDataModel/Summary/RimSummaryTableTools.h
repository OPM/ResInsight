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

#pragma once

#include <QString>

#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace RimSummaryTableTools
{
// Struct for storing vector data
struct VectorData
{
    QString             category;
    QString             name;
    std::vector<double> values;
    time_t              firstTimeStep;
    time_t              lastTimeStep;
    std::vector<time_t> timeSteps; // Time step for each value
};

// Value filter for table rows. Rows are kept if they have at least one value matching the filter.
enum class ValueFilterType
{
    NONE,
    ABOVE,
    BELOW,
    BETWEEN,
    OUTSIDE
};

struct ValueFilter
{
    ValueFilterType type           = ValueFilterType::NONE;
    double          lowerThreshold = 0.0;
    double          upperThreshold = 0.0;
};

// Struct for storing table data
struct TableData
{
    std::vector<VectorData> vectorDataCollection;
    std::set<time_t>        timeStepsUnion;
    double                  maxValue = 0.0;
    double                  minValue = 0.0;
    ValueFilter             valueFilter;
    // Value range of all rows before the value filter is applied
    std::optional<std::pair<double, double>> unfilteredMinMax;
    QString                                  unitName;
};

// ABOVE:   value > lowerThreshold
// BELOW:   value < upperThreshold
// BETWEEN: lowerThreshold <= value <= upperThreshold
// OUTSIDE: value < lowerThreshold or value > upperThreshold, used to exclude values in a range
// NONE:    value > 0
bool                    isValueMatchingFilter( double value, const ValueFilter& filter );
bool                    hasValueMatchingFilter( const std::vector<double>& values, const ValueFilter& filter );
std::vector<VectorData> vectorsMatchingFilter( const std::vector<VectorData>& vectorDataCollection, const ValueFilter& filter );
void                    sortVectorDataOnDate( TableData& rTableData );
std::vector<QString>    categoryNames( const std::vector<VectorData>& vectorDataCollection );

// Values of the vector for each of the given time steps. Time steps without a value are set to NaN.
std::vector<double> valuesForTimeSteps( const VectorData& vectorData, const std::set<time_t>& timeSteps );

// Value filter equivalent to the threshold in older project files, which kept rows with at least one value equal to or above
// the threshold
ValueFilter valueFilterFromLegacyThreshold( double thresholdValue );
} // namespace RimSummaryTableTools

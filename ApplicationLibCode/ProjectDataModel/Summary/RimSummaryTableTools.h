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

#include <set>
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
};

// Struct for storing table data
struct TableData
{
    std::vector<VectorData> vectorDataCollection;
    std::set<time_t>        timeStepsUnion;
    double                  maxValue       = 0.0;
    double                  minValue       = 0.0;
    double                  thresholdValue = 0.0;
    QString                 unitName;
};

bool                    hasValueAboveThreshold( const std::vector<double>& values, double thresholdValue );
std::vector<VectorData> vectorsAboveThreshold( const std::vector<VectorData>& vectorDataCollection, double thresholdValue );
void                    sortVectorDataOnDate( TableData& rTableData );
std::vector<QString>    categoryNames( const std::vector<VectorData>& vectorDataCollection );
} // namespace RimSummaryTableTools

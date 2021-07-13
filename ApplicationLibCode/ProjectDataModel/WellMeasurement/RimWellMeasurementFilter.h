/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include <vector>

class RimWellPath;
class RimWellMeasurement;
class RimWellPathCollection;

class QString;

class RimWellMeasurementFilter
{
public:
    static std::vector<RimWellMeasurement*> filterMeasurements( const std::vector<RimWellMeasurement*>& measurements,
                                                                const RimWellPathCollection& wellPathCollection,
                                                                const RimWellPath&           rimWellPath,
                                                                const std::vector<QString>&  measurementKinds );

    static std::vector<RimWellMeasurement*> filterMeasurements( const std::vector<RimWellMeasurement*>& measurements,
                                                                const RimWellPathCollection& wellPathCollection,
                                                                const RimWellPath&           rimWellPath,
                                                                const std::vector<QString>&  measurementKinds,
                                                                double                       lowerBound,
                                                                double                       upperBound,
                                                                const std::vector<int>&      qualityFilter );

    static std::vector<RimWellMeasurement*> filterMeasurements( const std::vector<RimWellMeasurement*>& measurements,
                                                                const std::vector<QString>& measurementKinds );

private:
    RimWellMeasurementFilter();

    static bool isInsideRange( double value, double lowerBound, double upperBound );

    static bool hasQuality( int quality, const std::vector<int>& qualityFilter );
};

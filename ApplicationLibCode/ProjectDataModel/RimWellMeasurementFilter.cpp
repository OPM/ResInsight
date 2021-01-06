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
#include "RimWellMeasurementFilter.h"

#include "cvfMath.h"

#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellMeasurement*>
    RimWellMeasurementFilter::filterMeasurements( const std::vector<RimWellMeasurement*>& measurements,
                                                  const RimWellPathCollection&            wellPathCollection,
                                                  const RimWellPath&                      wellPath,
                                                  const std::vector<QString>&             measurementKinds )
{
    std::vector<RimWellMeasurement*> filteredMeasurementsByKinds = filterMeasurements( measurements, measurementKinds );

    std::vector<RimWellMeasurement*> filteredMeasurements;
    for ( auto& measurement : filteredMeasurementsByKinds )
    {
        RimWellPath* matchedWellPath = wellPathCollection.tryFindMatchingWellPath( measurement->wellName() );
        if ( matchedWellPath && matchedWellPath == &wellPath )
        {
            filteredMeasurements.push_back( measurement );
        }
    }

    return filteredMeasurements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellMeasurement*>
    RimWellMeasurementFilter::filterMeasurements( const std::vector<RimWellMeasurement*>& measurements,
                                                  const RimWellPathCollection&            wellPathCollection,
                                                  const RimWellPath&                      wellPath,
                                                  const std::vector<QString>&             measurementKinds,
                                                  double                                  lowerBound,
                                                  double                                  upperBound,
                                                  const std::vector<int>&                 qualityFilter )
{
    std::vector<RimWellMeasurement*> filteredMeasurementsByKindsAndWellPath =
        filterMeasurements( measurements, wellPathCollection, wellPath, measurementKinds );

    std::vector<RimWellMeasurement*> filteredMeasurements;
    for ( auto& measurement : filteredMeasurementsByKindsAndWellPath )
    {
        if ( RimWellMeasurementFilter::isInsideRange( measurement->value(), lowerBound, upperBound ) &&
             RimWellMeasurementFilter::hasQuality( measurement->quality(), qualityFilter ) )
        {
            filteredMeasurements.push_back( measurement );
        }
    }

    return filteredMeasurements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellMeasurement*>
    RimWellMeasurementFilter::filterMeasurements( const std::vector<RimWellMeasurement*>& measurements,
                                                  const std::vector<QString>&             measurementKinds )
{
    std::vector<RimWellMeasurement*> filteredMeasurements;

    for ( auto& measurement : measurements )
    {
        if ( std::find( measurementKinds.begin(), measurementKinds.end(), measurement->kind() ) != measurementKinds.end() )
        {
            filteredMeasurements.push_back( measurement );
        }
    }

    return filteredMeasurements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellMeasurementFilter::isInsideRange( double value, double lowerBound, double upperBound )
{
    // Invalid range: everything is inside
    if ( lowerBound == cvf::UNDEFINED_DOUBLE || cvf::UNDEFINED_DOUBLE == upperBound )
    {
        return true;
    }

    if ( lowerBound <= value && value <= upperBound )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellMeasurementFilter::hasQuality( int quality, const std::vector<int>& qualityFilter )
{
    return std::find( qualityFilter.begin(), qualityFilter.end(), quality ) != qualityFilter.end();
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RigSurfaceStatisticsCalculator.h"

#include "RigStatisticsMath.h"

#include "cafAppEnum.h"
#include "cafAssert.h"

#include "cvfObject.h"

#include <limits>

namespace caf
{
template <>
void caf::AppEnum<RigSurfaceStatisticsCalculator::StatisticsType>::setUp()
{
    addItem( RigSurfaceStatisticsCalculator::StatisticsType::MEAN, "MEAN", "Mean" );
    addItem( RigSurfaceStatisticsCalculator::StatisticsType::MIN, "MIN", "Minimum" );
    addItem( RigSurfaceStatisticsCalculator::StatisticsType::MAX, "MAX", "Maximum" );
    addItem( RigSurfaceStatisticsCalculator::StatisticsType::P10, "P10", "P10" );
    addItem( RigSurfaceStatisticsCalculator::StatisticsType::P50, "P50", "P50" );
    addItem( RigSurfaceStatisticsCalculator::StatisticsType::P90, "P90", "P90" );
    setDefault( RigSurfaceStatisticsCalculator::StatisticsType::MEAN );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigSurface> RigSurfaceStatisticsCalculator::computeStatistics( const std::vector<cvf::ref<RigSurface>>& surfaces )
{
    // Need at least one surface to generate statistics
    if ( surfaces.empty() ) return nullptr;

    // Check that the size of all the surfaces are the same
    if ( !areSurfacesSameSize( surfaces ) ) return nullptr;

    size_t vecSize = surfaces[0]->vertices().size();

    std::vector<float> meanValues( vecSize, std::numeric_limits<double>::infinity() );
    std::vector<float> minValues( vecSize, std::numeric_limits<double>::infinity() );
    std::vector<float> maxValues( vecSize, std::numeric_limits<double>::infinity() );
    std::vector<float> p10Values( vecSize, std::numeric_limits<double>::infinity() );
    std::vector<float> p50Values( vecSize, std::numeric_limits<double>::infinity() );
    std::vector<float> p90Values( vecSize, std::numeric_limits<double>::infinity() );

    for ( size_t i = 0; i < vecSize; i++ )
    {
        std::vector<double> samples;
        for ( auto surface : surfaces )
        {
            double z = surface->vertices()[i].z();
            samples.push_back( z );
        }

        double min;
        double max;
        double sum;
        double range;
        double mean;
        double dev;
        RigStatisticsMath::calculateBasicStatistics( samples, &min, &max, &sum, &range, &mean, &dev );

        double p10;
        double p50;
        double p90;
        RigStatisticsMath::calculateStatisticsCurves( samples,
                                                      &p10,
                                                      &p50,
                                                      &p90,
                                                      &mean,
                                                      RigStatisticsMath::PercentileStyle::SWITCHED );

        // TODO: improve handling of these cases
        auto makeValid = []( double val ) {
            if ( std::isinf( val ) || std::isnan( val ) ) return 0.0;
            return val;
        };

        meanValues[i] = makeValid( mean );
        minValues[i]  = makeValid( min );
        maxValues[i]  = makeValid( max );
        p10Values[i]  = makeValid( p10 );
        p50Values[i]  = makeValid( p50 );
        p90Values[i]  = makeValid( p90 );
    }

    cvf::ref<RigSurface> statSurface = cvf::make_ref<RigSurface>();
    statSurface->setTriangleData( surfaces[0]->triangleIndices(), surfaces[0]->vertices() );

    auto enumToText = []( auto statisticsType ) { return caf::AppEnum<StatisticsType>::text( statisticsType ); };

    statSurface->addVerticeResult( enumToText( StatisticsType::MEAN ), meanValues );
    statSurface->addVerticeResult( enumToText( StatisticsType::MIN ), minValues );
    statSurface->addVerticeResult( enumToText( StatisticsType::MAX ), maxValues );
    statSurface->addVerticeResult( enumToText( StatisticsType::P10 ), p10Values );
    statSurface->addVerticeResult( enumToText( StatisticsType::P50 ), p50Values );
    statSurface->addVerticeResult( enumToText( StatisticsType::P90 ), p90Values );

    return statSurface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSurfaceStatisticsCalculator::areSurfacesSameSize( const std::vector<cvf::ref<RigSurface>>& surfaces )
{
    CAF_ASSERT( !surfaces.empty() );

    size_t vecSize = surfaces[0]->vertices().size();
    for ( auto surface : surfaces )
    {
        if ( vecSize != surface->vertices().size() ) return false;
    }
    return true;
}

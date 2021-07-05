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

#include "gtest/gtest.h"

#include "RigSurface.h"
#include "RigSurfaceStatisticsCalculator.h"

#include "cvfObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigSurfaceStatisticsTests, computeStatistics )
{
    std::vector<cvf::ref<RigSurface>> surfaces;
    for ( size_t i = 0; i < 100; i++ )
    {
        cvf::ref<RigSurface>      surface  = cvf::make_ref<RigSurface>();
        std::vector<unsigned int> indices  = { 2, 1, 0 };
        std::vector<cvf::Vec3d>   vertices = { cvf::Vec3d( -1.0, -1.0, i ),
                                             cvf::Vec3d( 1.0, -1.0, i ),
                                             cvf::Vec3d( -1.0, 1.0, i ) };

        surface->setTriangleData( indices, vertices );
        surfaces.push_back( surface );
    }

    cvf::ref<RigSurface> statSurface = RigSurfaceStatisticsCalculator::computeStatistics( surfaces );
    ASSERT_TRUE( statSurface.notNull() );

    ASSERT_EQ( statSurface->triangleIndices().size(), surfaces[0]->triangleIndices().size() );

    const std::vector<QString>& propertyNames = statSurface->propertyNames();
    ASSERT_TRUE( std::find( propertyNames.begin(), propertyNames.end(), "MEAN" ) != propertyNames.end() );
    ASSERT_TRUE( std::find( propertyNames.begin(), propertyNames.end(), "MIN" ) != propertyNames.end() );
    ASSERT_TRUE( std::find( propertyNames.begin(), propertyNames.end(), "MAX" ) != propertyNames.end() );
    ASSERT_TRUE( std::find( propertyNames.begin(), propertyNames.end(), "P10" ) != propertyNames.end() );
    ASSERT_TRUE( std::find( propertyNames.begin(), propertyNames.end(), "P50" ) != propertyNames.end() );
    ASSERT_TRUE( std::find( propertyNames.begin(), propertyNames.end(), "P90" ) != propertyNames.end() );

    const std::vector<float>& meanValues = statSurface->propertyValues( "MEAN" );
    const std::vector<float>& minValues  = statSurface->propertyValues( "MIN" );
    const std::vector<float>& maxValues  = statSurface->propertyValues( "MAX" );
    const std::vector<float>& p10Values  = statSurface->propertyValues( "P10" );
    const std::vector<float>& p50Values  = statSurface->propertyValues( "P50" );
    const std::vector<float>& p90Values  = statSurface->propertyValues( "P90" );

    // One value per vertex
    ASSERT_EQ( 3u, meanValues.size() );
    ASSERT_EQ( 3u, minValues.size() );
    ASSERT_EQ( 3u, maxValues.size() );
    ASSERT_EQ( 3u, p10Values.size() );
    ASSERT_EQ( 3u, p50Values.size() );
    ASSERT_EQ( 3u, p90Values.size() );

    for ( size_t i = 0; i < 3u; i++ )
    {
        ASSERT_EQ( 49.5, meanValues[i] );
        ASSERT_EQ( 0, minValues[i] );
        ASSERT_EQ( 99, maxValues[i] );
        ASSERT_NEAR( 9.1, p10Values[i], 0.0001 );
        ASSERT_NEAR( 49.5, p50Values[i], 0.0001 );
        ASSERT_NEAR( 89.9, p90Values[i], 0.0001 );
    }
}

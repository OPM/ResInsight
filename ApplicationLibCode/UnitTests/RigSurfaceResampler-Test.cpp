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

#include "RigSurfaceResampler.h"
#include "cvfObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigSurfaceResamplerTests, sameFlatGeometry )
{
    cvf::ref<RigSurface> targetSurface = cvf::make_ref<RigSurface>();
    cvf::ref<RigSurface> surface       = cvf::make_ref<RigSurface>();

    // Make single triangle, and assign to both surfaces
    double                  z        = 2.0;
    std::vector<cvf::Vec3d> vertices = { cvf::Vec3d( -1.0, -1.0, z ), cvf::Vec3d( 1.0, -1.0, z ), cvf::Vec3d( -1.0, 1.0, z ) };
    std::vector<unsigned int> indices = { 2, 1, 0 };

    targetSurface->setTriangleData( indices, vertices );
    surface->setTriangleData( indices, vertices );

    cvf::ref<RigSurface> resampledSurface = RigSurfaceResampler::resampleSurface( targetSurface, surface );

    ASSERT_EQ( resampledSurface->triangleIndices().size(), targetSurface->triangleIndices().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigSurfaceResamplerTests, flatGeometryLargerSource )
{
    cvf::ref<RigSurface> targetSurface = cvf::make_ref<RigSurface>();
    cvf::ref<RigSurface> surface       = cvf::make_ref<RigSurface>();

    // Make two triangle: target smaller than the surface
    std::vector<unsigned int> indices = { 2, 1, 0 };

    std::vector<cvf::Vec3d> targetVertices = { cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                               cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                               cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    targetSurface->setTriangleData( indices, targetVertices );

    std::vector<cvf::Vec3d> surfaceVertices = { cvf::Vec3d( -2.0, -2.0, 2.0 ),
                                                cvf::Vec3d( 2.0, -2.0, 2.0 ),
                                                cvf::Vec3d( -2.0, 2.0, 2.0 ) };

    surface->setTriangleData( indices, surfaceVertices );

    cvf::ref<RigSurface> resampledSurface = RigSurfaceResampler::resampleSurface( targetSurface, surface );

    ASSERT_EQ( resampledSurface->triangleIndices().size(), targetSurface->triangleIndices().size() );
    cvf::Vec3d r1 = resampledSurface->vertices()[0];
    ASSERT_EQ( r1.x(), -1.0 );
    ASSERT_EQ( r1.y(), -1.0 );
    ASSERT_EQ( r1.z(), 2.0 );

    cvf::Vec3d r2 = resampledSurface->vertices()[1];
    ASSERT_EQ( r2.x(), 1.0 );
    ASSERT_EQ( r2.y(), -1.0 );
    ASSERT_EQ( r2.z(), 2.0 );

    cvf::Vec3d r3 = resampledSurface->vertices()[2];
    ASSERT_EQ( r3.x(), -1.0 );
    ASSERT_EQ( r3.y(), 1.0 );
    ASSERT_EQ( r3.z(), 2.0 );
}

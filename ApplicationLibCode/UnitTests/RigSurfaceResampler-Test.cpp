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

#include "Surface/RigSurface.h"
#include "Surface/RigSurfaceResampler.h"

#include "cvfObject.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigSurfaceResamplerTests, sameFlatGeometry )
{
    cvf::ref<RigSurface> targetSurface = cvf::make_ref<RigSurface>();
    cvf::ref<RigSurface> surface       = cvf::make_ref<RigSurface>();

    // Make single triangle, and assign to both surfaces
    double                    z        = 2.0;
    std::vector<cvf::Vec3d>   vertices = { cvf::Vec3d( -1.0, -1.0, z ), cvf::Vec3d( 1.0, -1.0, z ), cvf::Vec3d( -1.0, 1.0, z ) };
    std::vector<unsigned int> indices  = { 2, 1, 0 };

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

    std::vector<cvf::Vec3d> targetVertices = { cvf::Vec3d( -1.0, -1.0, 1.0 ), cvf::Vec3d( 1.0, -1.0, 1.0 ), cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    targetSurface->setTriangleData( indices, targetVertices );

    std::vector<cvf::Vec3d> surfaceVertices = { cvf::Vec3d( -2.0, -2.0, 2.0 ), cvf::Vec3d( 2.0, -2.0, 2.0 ), cvf::Vec3d( -2.0, 2.0, 2.0 ) };

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

//--------------------------------------------------------------------------------------------------
/// A flat surface at z=-100 spanning [0,4]x[0,4].
/// Resample onto a 3x3 grid with origin (1,1) and increment 1.
/// All 9 sample points lie inside the mesh, so all values should be -100.
//--------------------------------------------------------------------------------------------------
TEST( RigSurfaceResamplerTests, resampleToRegularGrid_flatSurface )
{
    // Two triangles covering [0,4] x [0,4] at z = -100
    const double              z        = -100.0;
    std::vector<cvf::Vec3d>   vertices = { cvf::Vec3d( 0.0, 0.0, z ),
                                           cvf::Vec3d( 4.0, 0.0, z ),
                                           cvf::Vec3d( 4.0, 4.0, z ),
                                           cvf::Vec3d( 0.0, 4.0, z ) };
    std::vector<unsigned int> indices  = { 0, 1, 2, 0, 2, 3 };

    cvf::ref<RigSurface> surface = cvf::make_ref<RigSurface>();
    surface->setTriangleData( indices, vertices );

    // 3x3 grid at origin (1,1), increment 1 → points at x={1,2,3}, y={1,2,3}; all inside mesh
    auto values = RigSurfaceResampler::resampleToRegularGrid( surface.p(), 3, 3, 1.0, 1.0, 1.0, 1.0, 0.0 );

    ASSERT_EQ( values.size(), 9u );
    for ( auto v : values )
    {
        EXPECT_FALSE( std::isnan( v ) );
        EXPECT_FLOAT_EQ( v, static_cast<float>( z ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Grid points outside the mesh extent should be NaN.
//--------------------------------------------------------------------------------------------------
TEST( RigSurfaceResamplerTests, resampleToRegularGrid_outsidePointsAreNaN )
{
    // Single triangle in the first quadrant
    const double              z        = -50.0;
    std::vector<cvf::Vec3d>   vertices = { cvf::Vec3d( 0.0, 0.0, z ), cvf::Vec3d( 10.0, 0.0, z ), cvf::Vec3d( 0.0, 10.0, z ) };
    std::vector<unsigned int> indices  = { 0, 1, 2 };

    cvf::ref<RigSurface> surface = cvf::make_ref<RigSurface>();
    surface->setTriangleData( indices, vertices );

    // 3x3 grid with origin at (8,8) — all points are outside the triangle
    auto values = RigSurfaceResampler::resampleToRegularGrid( surface.p(), 3, 3, 8.0, 8.0, 1.0, 1.0, 0.0 );

    ASSERT_EQ( values.size(), 9u );
    for ( auto v : values )
    {
        EXPECT_TRUE( std::isnan( v ) );
    }
}

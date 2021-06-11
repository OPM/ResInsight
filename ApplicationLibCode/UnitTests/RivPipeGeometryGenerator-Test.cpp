/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "RivPipeGeometryGenerator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Vec3dArray> buildPipeCoords()
{
    cvf::ref<cvf::Vec3dArray> coords = new cvf::Vec3dArray;
    coords->resize( 10 );

    // Two identical at start
    coords->set( 0, cvf::Vec3d( 10, 10, 10 ) );
    coords->set( 1, cvf::Vec3d( 10, 10, 10 ) );

    // 180 degree
    coords->set( 2, cvf::Vec3d( 12, 20, 10 ) );
    coords->set( 3, cvf::Vec3d( 13, 20, 20 ) );
    coords->set( 4, cvf::Vec3d( 13, 20, 20 ) );
    coords->set( 5, cvf::Vec3d( 12, 20, 10 ) );

    // coords->set(5, cvf::Vec3d(15, 10, 10));
    coords->set( 6, cvf::Vec3d( 16, 10, 10 ) );
    coords->set( 7, cvf::Vec3d( 17, 10, 10 ) );

    // Two identical at end
    coords->set( 8, cvf::Vec3d( 18, 10, 10 ) );
    coords->set( 9, cvf::Vec3d( 18, 10, 10 ) );

    return coords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ModelVisualizationTest, AllPartsInputTest )
{
    cvf::ref<cvf::Vec3dArray> coords = new cvf::Vec3dArray;
    RivPipeGeometryGenerator  gen;

    // Set empty coords array
    gen.setPipeCenterCoords( coords.p() );
    {
        cvf::ref<cvf::DrawableGeo> geo = gen.createPipeSurface();
        EXPECT_TRUE( geo.isNull() );
    }

    coords->resize( 2 );
    coords->set( 0, cvf::Vec3d( 10, 10, 10 ) );
    coords->set( 1, cvf::Vec3d( 10, 10, 10 ) );

    gen.setPipeCenterCoords( coords.p() );
    {
        cvf::ref<cvf::DrawableGeo> geo = gen.createPipeSurface();
        EXPECT_TRUE( geo.isNull() );
    }

    coords->set( 1, cvf::Vec3d( 10, 10, 20 ) );
    gen.setPipeCenterCoords( coords.p() );
    {
        cvf::ref<cvf::DrawableGeo> geo = gen.createPipeSurface();
        EXPECT_TRUE( geo.notNull() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ModelVisualizationTest, PipeGeometryTest )
{
    cvf::ref<cvf::Vec3dArray> coords = buildPipeCoords();

    RivPipeGeometryGenerator gen;
    gen.setPipeCenterCoords( coords.p() );
    gen.createPipeSurface();
}

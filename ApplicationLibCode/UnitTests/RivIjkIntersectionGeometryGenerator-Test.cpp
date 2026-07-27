/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilder.h"

#include "RivEclipseIntersectionGrid.h"
#include "RivIjkIntersectionGeometryGenerator.h"

#include "RimIjkIntersection.h"

#include "cvfDrawableGeo.h"
#include "cvfVector3.h"

#include <cmath>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Build a regular ni x nj x nk box grid in memory (no file, no view)
//--------------------------------------------------------------------------------------------------
cvf::ref<RigEclipseCaseData> buildBoxGrid( int ni, int nj, int nk )
{
    RigReservoirBuilder builder;
    builder.setIJKCount( cvf::Vec3st( ni, nj, nk ) );
    builder.setWorldCoordinates( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( ni, nj, -nk ) );

    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );
    builder.createGridsAndCells( caseData.p() );
    caseData->mainGrid()->computeCachedData();

    return caseData;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// A full-range K slice produces one cell face (two triangles) per cell in the K layer
//--------------------------------------------------------------------------------------------------
TEST( RivIjkIntersectionGeometryGeneratorTest, FullRangeKSliceTriangleCount )
{
    const int ni = 4;
    const int nj = 3;
    const int nk = 5;

    cvf::ref<RigEclipseCaseData> caseData = buildBoxGrid( ni, nj, nk );
    RigMainGrid*                 mainGrid = caseData->mainGrid();

    cvf::ref<RivEclipseIntersectionGrid> hexGrid = new RivEclipseIntersectionGrid( mainGrid, nullptr, true );

    RimIjkIntersection def;
    def.setAxis( RimIjkIntersection::GridAxis::AXIS_K );
    def.setFixedIndex( 2 );
    def.setIjkRange( { caf::VecIjk0( 0, 0, 0 ), caf::VecIjk0( ni - 1, nj - 1, nk - 1 ) } );

    cvf::ref<RivIjkIntersectionGeometryGenerator> generator = new RivIjkIntersectionGeometryGenerator( &def, hexGrid.p(), mainGrid );
    generator->generateSurface( nullptr );

    EXPECT_TRUE( generator->isAnyGeometryPresent() );

    // 2 triangles per cell face, 3 vertices per triangle
    EXPECT_EQ( static_cast<size_t>( 2 * ni * nj * 3 ), generator->triangleVxes()->size() );
    EXPECT_EQ( static_cast<size_t>( 2 * ni * nj ), generator->triangleToCellIndex().size() );
    EXPECT_EQ( generator->triangleVxes()->size(), generator->triangleVxToCellCornerInterpolationWeights().size() );
}

//--------------------------------------------------------------------------------------------------
/// All vertices of a K slice through a regular grid lie at the same depth
//--------------------------------------------------------------------------------------------------
TEST( RivIjkIntersectionGeometryGeneratorTest, KSliceVerticesAreCoplanar )
{
    const int ni = 3;
    const int nj = 3;
    const int nk = 4;

    cvf::ref<RigEclipseCaseData> caseData = buildBoxGrid( ni, nj, nk );
    RigMainGrid*                 mainGrid = caseData->mainGrid();

    cvf::ref<RivEclipseIntersectionGrid> hexGrid = new RivEclipseIntersectionGrid( mainGrid, nullptr, true );

    RimIjkIntersection def;
    def.setAxis( RimIjkIntersection::GridAxis::AXIS_K );
    def.setFixedIndex( 1 );
    def.setIjkRange( { caf::VecIjk0( 0, 0, 0 ), caf::VecIjk0( ni - 1, nj - 1, nk - 1 ) } );

    cvf::ref<RivIjkIntersectionGeometryGenerator> generator = new RivIjkIntersectionGeometryGenerator( &def, hexGrid.p(), mainGrid );
    generator->generateSurface( nullptr );

    ASSERT_TRUE( generator->isAnyGeometryPresent() );

    const cvf::Vec3fArray* vxes = generator->triangleVxes();
    float                  z0   = ( *vxes )[0].z();
    for ( size_t i = 0; i < vxes->size(); ++i )
    {
        EXPECT_NEAR( z0, ( *vxes )[i].z(), 1.0e-4f );
    }
}

//--------------------------------------------------------------------------------------------------
/// A narrowed perpendicular range yields exactly the requested number of cell faces
//--------------------------------------------------------------------------------------------------
TEST( RivIjkIntersectionGeometryGeneratorTest, NarrowedRangeISlice )
{
    const int ni = 5;
    const int nj = 4;
    const int nk = 6;

    cvf::ref<RigEclipseCaseData> caseData = buildBoxGrid( ni, nj, nk );
    RigMainGrid*                 mainGrid = caseData->mainGrid();

    cvf::ref<RivEclipseIntersectionGrid> hexGrid = new RivEclipseIntersectionGrid( mainGrid, nullptr, true );

    // Fixed I = 2, J in [1,1] (single column), K in [0,2] (3 layers) => 1 * 3 = 3 cell faces
    RimIjkIntersection def;
    def.setAxis( RimIjkIntersection::GridAxis::AXIS_I );
    def.setFixedIndex( 2 );
    def.setIjkRange( { caf::VecIjk0( 0, 1, 0 ), caf::VecIjk0( ni - 1, 1, 2 ) } );

    cvf::ref<RivIjkIntersectionGeometryGenerator> generator = new RivIjkIntersectionGeometryGenerator( &def, hexGrid.p(), mainGrid );
    generator->generateSurface( nullptr );

    EXPECT_EQ( static_cast<size_t>( 2 * 1 * 3 ), generator->triangleToCellIndex().size() );
}

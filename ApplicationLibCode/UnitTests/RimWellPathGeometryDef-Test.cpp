/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "Well/RigWellPath.h"

//--------------------------------------------------------------------------------------------------
/// A lateral gets its well path geometry from the parent well. The geometry is then long enough to be
/// visualized, even if the lateral has no active well targets. Visualization code must not assume that
/// a well path with geometry has at least one active well target.
//--------------------------------------------------------------------------------------------------
TEST( RimWellPathGeometryDefTest, GeometryFromParentWellWithoutActiveTargets )
{
    RimModeledWellPath wellPath;

    RimWellPathGeometryDef* geoDef = wellPath.geometryDefinition();
    ASSERT_TRUE( geoDef != nullptr );

    // Mimic a lateral tied in to a parent well, see RimModeledWellPath::updateTieInLocationFromParentWell()
    geoDef->setIsAttachedToParentWell( true );
    geoDef->setFixedWellPathPoints( { cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( 0.0, 0.0, -100.0 ) } );
    geoDef->setFixedMeasuredDepths( { 0.0, 100.0 } );

    EXPECT_TRUE( geoDef->activeWellTargets().empty() );
    EXPECT_TRUE( geoDef->showSpheres() );

    wellPath.createWellPathGeometry();

    RigWellPath* wellPathGeometry = wellPath.wellPathGeometry();
    ASSERT_TRUE( wellPathGeometry != nullptr );
    EXPECT_GE( wellPathGeometry->wellPathPoints().size(), size_t( 2 ) );
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "ContourMap/RigContourMapGrid.h"

//--------------------------------------------------------------------------------------------------
/// The map size of a cached ensemble contour map is stored in the project file, and is used directly instead of being
/// recomputed from the extent of the expanded bounding box, which is stored with limited precision. The expanded
/// bounding box must still end up as an exact multiple of the sample spacing.
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapGrid, ExplicitMapSizeIsUsedWhenRestoringGrid )
{
    const double sampleSpacing = 134.078604562017;

    const cvf::BoundingBox originalBoundingBox( cvf::Vec3d( 455778.046191406, 5926228.75742188, -2013.02879333496 ),
                                                cvf::Vec3d( 467774.981152344, 5939762.94570312, -1549.74769592285 ) );

    const RigContourMapGrid computedGrid( originalBoundingBox, sampleSpacing );
    EXPECT_EQ( 94u, computedGrid.mapSize().x() );
    EXPECT_EQ( 105u, computedGrid.mapSize().y() );

    // The expanded bounding box as read back from a project file, stored with 15 significant digits
    const cvf::BoundingBox restoredExpandedBoundingBox( cvf::Vec3d( 455509.888982282, 5925960.60021275, -2013.02879333496 ),
                                                        cvf::Vec3d( 468113.277811112, 5940038.85369176, -1549.74769592285 ) );

    const RigContourMapGrid restoredGrid( originalBoundingBox, restoredExpandedBoundingBox, sampleSpacing, computedGrid.mapSize() );
    EXPECT_EQ( computedGrid.mapSize().x(), restoredGrid.mapSize().x() );
    EXPECT_EQ( computedGrid.mapSize().y(), restoredGrid.mapSize().y() );

    const cvf::Vec3d extent = restoredGrid.expandedBoundingBox().extent();
    EXPECT_NEAR( restoredGrid.mapSize().x() * sampleSpacing, extent.x(), 1.0e-9 );
    EXPECT_NEAR( restoredGrid.mapSize().y() * sampleSpacing, extent.y(), 1.0e-9 );
}

//--------------------------------------------------------------------------------------------------
/// A bounding box extent that is clearly larger than a whole number of cells must add a cell
//--------------------------------------------------------------------------------------------------
TEST( RigContourMapGrid, MapSizeIsRoundedUpForPartialCell )
{
    const double sampleSpacing = 100.0;

    // The bounding box is expanded by two cells in each direction, giving an extent of 4.5 cells in x and 4 cells in y
    const cvf::BoundingBox originalBoundingBox( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( 50.0, 0.0, 10.0 ) );

    const RigContourMapGrid grid( originalBoundingBox, sampleSpacing );
    EXPECT_EQ( 5u, grid.mapSize().x() );
    EXPECT_EQ( 4u, grid.mapSize().y() );
}

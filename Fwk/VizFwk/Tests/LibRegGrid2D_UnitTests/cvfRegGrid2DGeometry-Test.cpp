//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfRegGrid2DGeometry.h"
#include "cvfRegGrid2D.h"
#include "cvfDrawableGeo.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2DGeometryTest, SurfaceConstruction)
{
    ref<RegGrid2D> regGrid = new RegGrid2D;
    ASSERT_TRUE(regGrid.notNull());

    int gridCountI = 2;
    int gridCountJ = 3;
    Vec2d spacing(1, 1);

    regGrid->setSpacing(spacing);
    regGrid->allocateGrid(gridCountI, gridCountJ);

    DoubleArray values;
    values.resize(2 * 3);
    values.setAll(5);
    regGrid->setElevations(values);

    {
        ref<RegGrid2DGeometry> geoBuilder = new RegGrid2DGeometry(regGrid.p());

        ref<DrawableGeo> geo = geoBuilder->generateSurface();
        ASSERT_TRUE(geo.notNull());

        EXPECT_DOUBLE_EQ(0.0, geo->boundingBox().min().x());
        EXPECT_DOUBLE_EQ(0.0, geo->boundingBox().min().y());
        EXPECT_DOUBLE_EQ(5.0, geo->boundingBox().min().z());

        EXPECT_DOUBLE_EQ(1.0, geo->boundingBox().max().x());
        EXPECT_DOUBLE_EQ(2.0, geo->boundingBox().max().y());
        EXPECT_DOUBLE_EQ(5.0, geo->boundingBox().max().z());
    }

    {
        ref<RegGrid2DGeometry> geoBuilder = new RegGrid2DGeometry(regGrid.p());
        geoBuilder->setTranslation(Vec3d(10, 20, 0));

        ref<DrawableGeo> geo = geoBuilder->generateSurface();
        ASSERT_TRUE(geo.notNull());

        EXPECT_DOUBLE_EQ(10.0, geo->boundingBox().min().x());
        EXPECT_DOUBLE_EQ(20.0, geo->boundingBox().min().y());
        EXPECT_DOUBLE_EQ(5.0, geo->boundingBox().min().z());

        EXPECT_DOUBLE_EQ(11.0, geo->boundingBox().max().x());
        EXPECT_DOUBLE_EQ(22.0, geo->boundingBox().max().y());
        EXPECT_DOUBLE_EQ(5.0, geo->boundingBox().max().z());
    }

    {
        ref<RegGrid2DGeometry> geoBuilder = new RegGrid2DGeometry(regGrid.p());
        geoBuilder->setTranslation(Vec3d(10, 20, 2.0));
        geoBuilder->setElevationScaleFactor(-2.0);

        ref<DrawableGeo> geo = geoBuilder->generateSurface();
        ASSERT_TRUE(geo.notNull());

        EXPECT_DOUBLE_EQ(10.0, geo->boundingBox().min().x());
        EXPECT_DOUBLE_EQ(20.0, geo->boundingBox().min().y());
        EXPECT_DOUBLE_EQ(-8.0, geo->boundingBox().min().z());

        EXPECT_DOUBLE_EQ(11.0, geo->boundingBox().max().x());
        EXPECT_DOUBLE_EQ(22.0, geo->boundingBox().max().y());
        EXPECT_DOUBLE_EQ(-8.0, geo->boundingBox().max().z());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RegGrid2DGeometryTest, ClosedVolume)
{
    ref<RegGrid2D> regGrid2D = new RegGrid2D;
    EXPECT_TRUE(regGrid2D.notNull());

    regGrid2D->allocateGrid(10, 20);
    {
        ref<RegGrid2DGeometry> regGrid2DGeometryNoElevations = new RegGrid2DGeometry(regGrid2D.p());
        ref<DrawableGeo> closedVolumeGeo = regGrid2DGeometryNoElevations->generateClosedVolume(10);
        ASSERT_FALSE(closedVolumeGeo.notNull());

        // It is allowed to create a surface when elevations are undefined. Should this be allowed?
        ref<DrawableGeo> surface = regGrid2DGeometryNoElevations->generateSurface();
        ASSERT_TRUE(surface.notNull());
    }

    DoubleArray values;
    values.resize(10 * 20);
    values.setAll(5);

    regGrid2D->setElevations(values);

    ref<RegGrid2DGeometry> regGrid2DGeometry = new RegGrid2DGeometry(regGrid2D.p());

    {
        ref<DrawableGeo> closedVolumeGeo = regGrid2DGeometry->generateClosedVolume(10);
        ASSERT_FALSE(closedVolumeGeo.notNull());
    }

    {
        ref<DrawableGeo> closedVolumeGeo = regGrid2DGeometry->generateClosedVolume(-10);
        ASSERT_TRUE(closedVolumeGeo.notNull());
    }
}


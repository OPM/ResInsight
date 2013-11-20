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
#include "cvfBoxGenerator.h"

#include "gtest/gtest.h"

using namespace cvf;

#include "utestGeometryBuilders.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoxGeneratorTest, GenerateSimple)
{
    // From min/max
    {
        BoxGenerator gen;
        gen.setMinMax(Vec3d(10, 20, 30), Vec3d(20, 40, 60));

        BuilderTrisQuads builder;
        gen.generate(&builder);
        BoundingBox bb = builder.vertexBoundingBox();
        EXPECT_DOUBLE_EQ(10, bb.min().x());
        EXPECT_DOUBLE_EQ(20, bb.min().y());
        EXPECT_DOUBLE_EQ(30, bb.min().z());
        EXPECT_DOUBLE_EQ(20, bb.max().x());
        EXPECT_DOUBLE_EQ(40, bb.max().y());
        EXPECT_DOUBLE_EQ(60, bb.max().z());

        EXPECT_EQ(0, builder.triCount());
        EXPECT_EQ(6, builder.quadCount());
    }

    // From origin and extent
    {
        BoxGenerator gen;
        gen.setOriginAndExtent(Vec3d(10, 20, 30), Vec3d(100, 200, 300));

        BuilderTrisQuads builder;
        gen.generate(&builder);
        BoundingBox bb = builder.vertexBoundingBox();
        EXPECT_DOUBLE_EQ(10,  bb.min().x());
        EXPECT_DOUBLE_EQ(20,  bb.min().y());
        EXPECT_DOUBLE_EQ(30,  bb.min().z());
        EXPECT_DOUBLE_EQ(110, bb.max().x());
        EXPECT_DOUBLE_EQ(220, bb.max().y());
        EXPECT_DOUBLE_EQ(330, bb.max().z());
    }

    // From center and extent
    {
        BoxGenerator gen;
        gen.setCenterAndExtent(Vec3d(100, 200, 300), Vec3d(10, 20, 30));

        BuilderTrisQuads builder;
        gen.generate(&builder);
        BoundingBox bb = builder.vertexBoundingBox();
        EXPECT_DOUBLE_EQ(95,  bb.min().x());
        EXPECT_DOUBLE_EQ(190, bb.min().y());
        EXPECT_DOUBLE_EQ(285,  bb.min().z());
        EXPECT_DOUBLE_EQ(105, bb.max().x());
        EXPECT_DOUBLE_EQ(210, bb.max().y());
        EXPECT_DOUBLE_EQ(315, bb.max().z());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoxGeneratorTest, GenerateWithSubdivisions)
{
    BoxGenerator gen;
    gen.setMinMax(Vec3d(10, 20, 30), Vec3d(20, 40, 60));
    gen.setSubdivisions(2, 4, 6);

    BuilderTrisQuads builder;
    gen.generate(&builder);
    BoundingBox bb = builder.vertexBoundingBox();
    EXPECT_DOUBLE_EQ(10, bb.min().x());
    EXPECT_DOUBLE_EQ(20, bb.min().y());
    EXPECT_DOUBLE_EQ(30, bb.min().z());
    EXPECT_DOUBLE_EQ(20, bb.max().x());
    EXPECT_DOUBLE_EQ(40, bb.max().y());
    EXPECT_DOUBLE_EQ(60, bb.max().z());

    EXPECT_EQ(0, builder.triCount());

    // top/bot:    2*(2*4) = 16
    // front/back: 2*(2*6) = 24
    // left/right: 2*(4*6) = 48
    EXPECT_EQ(88, builder.quadCount());
}


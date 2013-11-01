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
#include "cvfPatchGenerator.h"

#include "gtest/gtest.h"

using namespace cvf;

#include "utestGeometryBuilders.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PatchGeneratorTest, Generate)
{
    PatchGenerator gen;
    gen.setOrigin(Vec3d(1, 2, 3));
    gen.setExtent(10, 20);
    gen.setSubdivisions(3, 4);

    {
        BuilderTrisQuads builder;
        gen.generate(&builder);
        BoundingBox bb = builder.vertexBoundingBox();
        EXPECT_DOUBLE_EQ(1,  bb.min().x());
        EXPECT_DOUBLE_EQ(2,  bb.min().y());
        EXPECT_DOUBLE_EQ(3,  bb.min().z());
        EXPECT_DOUBLE_EQ(11, bb.max().x());
        EXPECT_DOUBLE_EQ(22, bb.max().y());
        EXPECT_DOUBLE_EQ(3,  bb.max().z());

        EXPECT_EQ(0, builder.triCount());
        EXPECT_EQ(12, builder.quadCount());
    }

    gen.setQuads(false);

    {
        BuilderTrisQuads builder;
        gen.generate(&builder);
        BoundingBox bb = builder.vertexBoundingBox();
        EXPECT_DOUBLE_EQ(1,  bb.min().x());
        EXPECT_DOUBLE_EQ(2,  bb.min().y());
        EXPECT_DOUBLE_EQ(3,  bb.min().z());
        EXPECT_DOUBLE_EQ(11, bb.max().x());
        EXPECT_DOUBLE_EQ(22, bb.max().y());
        EXPECT_DOUBLE_EQ(3,  bb.max().z());

        EXPECT_EQ(24, builder.triCount());
        EXPECT_EQ(0, builder.quadCount());
    }
}



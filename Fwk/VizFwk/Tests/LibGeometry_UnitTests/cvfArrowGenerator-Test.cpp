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
#include "cvfArrowGenerator.h"

#include "gtest/gtest.h"

using namespace cvf;

#include "utestGeometryBuilders.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrowGeneratorTest, DefaultArrow)
{
    ArrowGenerator gen;

    BuilderTrisQuads builder;
    gen.generate(&builder);
    BoundingBox bb = builder.vertexBoundingBox();
    EXPECT_DOUBLE_EQ(-0.085f,  bb.min().x());
    EXPECT_DOUBLE_EQ(-0.085f,  bb.min().y());
    EXPECT_DOUBLE_EQ(0.0f,  bb.min().z());
    EXPECT_DOUBLE_EQ(0.085f, bb.max().x());
    EXPECT_DOUBLE_EQ(0.085f, bb.max().y());
    EXPECT_DOUBLE_EQ(1.0f,  bb.max().z());

    // Default num slices is 20
    EXPECT_EQ(60, builder.triCount());      // 20 + 20 in cone and 20 in bottom of cylinder
    EXPECT_EQ(20, builder.quadCount());     // 20 on sides of cylinder
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrowGeneratorTest, CustomArrow)
{
    ArrowGenerator gen;

    gen.setShaftRelativeRadius(0.4f);
    gen.setHeadRelativeRadius(0.3f);
    gen.setHeadRelativeLength(0.5f);
    gen.setNumSlices(12);

    BuilderTrisQuads builder;
    gen.generate(&builder);
    BoundingBox bb = builder.vertexBoundingBox();
    EXPECT_DOUBLE_EQ(-0.4f,  bb.min().x());
    EXPECT_DOUBLE_EQ(-0.4f,  bb.min().y());
    EXPECT_DOUBLE_EQ(0.0f,  bb.min().z());
    EXPECT_DOUBLE_EQ(0.4f, bb.max().x());
    EXPECT_DOUBLE_EQ(0.4f, bb.max().y());
    EXPECT_DOUBLE_EQ(1.0f,  bb.max().z());

    EXPECT_EQ(36, builder.triCount());
    EXPECT_EQ(12, builder.quadCount());
}

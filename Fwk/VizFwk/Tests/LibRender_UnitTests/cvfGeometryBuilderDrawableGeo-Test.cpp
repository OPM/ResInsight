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
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfPrimitiveSet.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderDrawableGeo, BasicConstruction)
{
    GeometryBuilderDrawableGeo b;
    ref<DrawableGeo> geo = b.drawableGeo();

    EXPECT_EQ(0u, geo->vertexCount());
    EXPECT_EQ(0u, geo->faceCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderDrawableGeo, AddVertices)
{
    Vec3fArray inputVertexArr;
    inputVertexArr.reserve(3);
    inputVertexArr.add(Vec3f::X_AXIS);
    inputVertexArr.add(Vec3f::Y_AXIS);
    inputVertexArr.add(Vec3f::Z_AXIS);

    GeometryBuilderDrawableGeo b;
    int indexOfFirstVertex = b.addVertices(inputVertexArr);
    EXPECT_EQ(0, indexOfFirstVertex);
    EXPECT_EQ(3u, b.drawableGeo()->vertexCount());

    indexOfFirstVertex = b.addVertices(inputVertexArr);
    EXPECT_EQ(3, indexOfFirstVertex);
    
    ref<DrawableGeo> geo = b.drawableGeo();
    const Vec3fArray* va = geo->vertexArray();
    EXPECT_EQ(6u, va->size());

    EXPECT_TRUE(va->get(0) == Vec3f::X_AXIS);
    EXPECT_TRUE(va->get(1) == Vec3f::Y_AXIS);
    EXPECT_TRUE(va->get(2) == Vec3f::Z_AXIS);
    EXPECT_TRUE(va->get(3) == Vec3f::X_AXIS);
    EXPECT_TRUE(va->get(4) == Vec3f::Y_AXIS);
    EXPECT_TRUE(va->get(5) == Vec3f::Z_AXIS);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderDrawableGeo, AddTriangle)
{
    GeometryBuilderDrawableGeo b;
    EXPECT_EQ(0u, b.drawableGeo()->faceCount());

    b.addTriangle(10, 11, 12);

    ref<DrawableGeo> geo = b.drawableGeo();

    EXPECT_EQ(1u, geo->faceCount());
    ASSERT_EQ(1, geo->primitiveSetCount());

    const PrimitiveSet* prim = geo->primitiveSet(0);
    
    ASSERT_EQ(1, prim->faceCount());
    ASSERT_EQ(1, prim->triangleCount());
    ASSERT_EQ(3, prim->indexCount());
    EXPECT_EQ(10, prim->index(0));
    EXPECT_EQ(11, prim->index(1));
    EXPECT_EQ(12, prim->index(2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderDrawableGeo, AddQuad)
{
    GeometryBuilderDrawableGeo b;
    EXPECT_EQ(0u, b.drawableGeo()->faceCount());

    b.addQuad(10, 11, 12, 13);

    ref<DrawableGeo> geo = b.drawableGeo();

    EXPECT_EQ(2u, geo->faceCount());
    ASSERT_EQ(1, geo->primitiveSetCount());


    const PrimitiveSet* prim = geo->primitiveSet(0);

    ASSERT_EQ(2, prim->faceCount());
    ASSERT_EQ(2, prim->triangleCount());
    ASSERT_EQ(6, prim->indexCount());
    EXPECT_EQ(10, prim->index(0));
    EXPECT_EQ(11, prim->index(1));
    EXPECT_EQ(12, prim->index(2));
    EXPECT_EQ(10, prim->index(3));
	EXPECT_EQ(12, prim->index(4));
	EXPECT_EQ(13, prim->index(5));
}


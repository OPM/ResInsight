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
#include "cvfGeometryBuilderTriangles.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTrianglesTest, BasicConstruction)
{
    GeometryBuilderTriangles b;
    ref<Vec3fArray> vertices = b.vertices();
    ref<UIntArray> faceList = b.triangles();
    EXPECT_EQ(0u, vertices->size());
    EXPECT_EQ(0u, faceList->size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTrianglesTest, AddVertices)
{
    Vec3fArray inputVertexArr;
    inputVertexArr.reserve(3);
    inputVertexArr.add(Vec3f::X_AXIS);
    inputVertexArr.add(Vec3f::Y_AXIS);
    inputVertexArr.add(Vec3f::Z_AXIS);

    GeometryBuilderTriangles b;
    int indexOfFirstVertex = b.addVertices(inputVertexArr);
    EXPECT_EQ(0, indexOfFirstVertex);
    EXPECT_EQ(3u, b.vertices()->size());

    indexOfFirstVertex = b.addVertices(inputVertexArr);
    EXPECT_EQ(3, indexOfFirstVertex);
    
    ref<Vec3fArray> va = b.vertices();
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
TEST(GeometryBuilderTrianglesTest, AddTriangle)
{
    GeometryBuilderTriangles b;
    ASSERT_EQ(0u, b.triangles()->size());

    b.addTriangle(10, 11, 12);
    ref<UIntArray> tris = b.triangles();

    ASSERT_EQ(3u, tris->size());
    EXPECT_EQ(10, tris->get(0));
    EXPECT_EQ(11, tris->get(1));
    EXPECT_EQ(12, tris->get(2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTrianglesTest, AddQuad)
{
    GeometryBuilderTriangles b;
    ASSERT_EQ(0u, b.triangles()->size());

    b.addQuad(10, 11, 12, 13);
    ref<UIntArray> tris = b.triangles();

    ASSERT_EQ(6u, tris->size());
    EXPECT_EQ(10, tris->get(0));
    EXPECT_EQ(11, tris->get(1));
    EXPECT_EQ(12, tris->get(2));
    EXPECT_EQ(10, tris->get(3));
    EXPECT_EQ(12, tris->get(4));
    EXPECT_EQ(13, tris->get(5));
}


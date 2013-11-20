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
#include "cvfGeometryBuilder.h"

#include "gtest/gtest.h"

using namespace cvf;

#include "utestGeometryBuilders.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, CheckImplOfPureVirtualMembers)
{
    Vec3fArray vertexArr;
    vertexArr.reserve(3);
    vertexArr.add(Vec3f::X_AXIS);
    vertexArr.add(Vec3f::Y_AXIS);
    vertexArr.add(Vec3f::Z_AXIS);

    BuilderTris b;
    int indexOfFirstVertex = b.addVertices(vertexArr);
    EXPECT_EQ(0, indexOfFirstVertex);
    EXPECT_EQ(3u, b.vertexArr.size());
    EXPECT_EQ(3, b.vertexCount());

    indexOfFirstVertex = b.addVertices(vertexArr);
    EXPECT_EQ(3, indexOfFirstVertex);
    EXPECT_EQ(6u, b.vertexArr.size());


    ASSERT_EQ(0u, b.triArr.size());
    b.addTriangle(10, 11, 12);
    ASSERT_EQ(3u, b.triArr.size());
    EXPECT_EQ(10, b.triArr[0]);
    EXPECT_EQ(11, b.triArr[1]);
    EXPECT_EQ(12, b.triArr[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddSingleTriangleFromVertices)
{
    const Vec3f v0(0, 0, 0);
    const Vec3f v1(1, 0, 0);
    const Vec3f v2(2, 0, 0);

    BuilderTris b;
    b.addTriangleByVertices(v0, v1, v2);
    EXPECT_EQ(3u, b.vertexArr.size());
    ASSERT_EQ(3u,  b.triArr.size());

    EXPECT_EQ(0,  b.triArr[0]);
    EXPECT_EQ(1,  b.triArr[1]);
    EXPECT_EQ(2,  b.triArr[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddMultipleTriangles)
{
    IntArray indices;
    indices.reserve(6);
    indices.add(0);  indices.add(1);  indices.add(2);
    indices.add(10); indices.add(11); indices.add(12);

    BuilderTris b;
    b.addTriangles(indices);
    ASSERT_EQ(6u,  b.triArr.size());
    EXPECT_EQ(0,  b.triArr[0]);
    EXPECT_EQ(1,  b.triArr[1]);
    EXPECT_EQ(2,  b.triArr[2]);
    EXPECT_EQ(10, b.triArr[3]);
    EXPECT_EQ(11, b.triArr[4]);
    EXPECT_EQ(12, b.triArr[5]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddTriangleFan)
{
    UIntArray indices;
    indices.reserve(5);
    indices.add(0);  
    indices.add(1);  
    indices.add(2);
    indices.add(3);
    indices.add(4);

    BuilderTris b;
    b.addTriangleFan(indices);
    ASSERT_EQ(9u, b.triArr.size());

    EXPECT_EQ(0, b.triArr[0]);
    EXPECT_EQ(1, b.triArr[1]);
    EXPECT_EQ(2, b.triArr[2]);

    EXPECT_EQ(0, b.triArr[3]);
    EXPECT_EQ(2, b.triArr[4]);
    EXPECT_EQ(3, b.triArr[5]);

    EXPECT_EQ(0, b.triArr[6]);
    EXPECT_EQ(3, b.triArr[7]);
    EXPECT_EQ(4, b.triArr[8]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddTriangleStrip)
{
    UIntArray indices;
    indices.reserve(6);
    indices.add(0);  
    indices.add(1);  
    indices.add(2);
    indices.add(3);
    indices.add(4);
    indices.add(5);

    BuilderTris b;
    b.addTriangleStrip(indices);
    ASSERT_EQ(12u, b.triArr.size());

    EXPECT_EQ(0, b.triArr[0]);
    EXPECT_EQ(1, b.triArr[1]);
    EXPECT_EQ(2, b.triArr[2]);

    EXPECT_EQ(2, b.triArr[3]);
    EXPECT_EQ(1, b.triArr[4]);
    EXPECT_EQ(3, b.triArr[5]);

    EXPECT_EQ(2, b.triArr[6]);
    EXPECT_EQ(3, b.triArr[7]);
    EXPECT_EQ(4, b.triArr[8]);

    EXPECT_EQ(4, b.triArr[9]);
    EXPECT_EQ(3, b.triArr[10]);
    EXPECT_EQ(5, b.triArr[11]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddSingleQuad)
{
    BuilderTris b;
    b.addQuad(10, 11, 12, 13);
    ASSERT_EQ(6u,  b.triArr.size());

    EXPECT_EQ(10,  b.triArr[0]);
    EXPECT_EQ(11,  b.triArr[1]);
    EXPECT_EQ(12,  b.triArr[2]);
    EXPECT_EQ(10,  b.triArr[3]);
    EXPECT_EQ(12,  b.triArr[4]);
    EXPECT_EQ(13,  b.triArr[5]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddMultipleQuads)
{
    IntArray indices;
    indices.reserve(8);
    indices.add(0);  indices.add(1);  indices.add(2);  indices.add(3);
    indices.add(10); indices.add(11); indices.add(12); indices.add(13);

    BuilderTris b;
    b.addQuads(indices);
    ASSERT_EQ(12u,  b.triArr.size());

    EXPECT_EQ(0,  b.triArr[0]);
    EXPECT_EQ(1,  b.triArr[1]);
    EXPECT_EQ(2,  b.triArr[2]);
    EXPECT_EQ(0,  b.triArr[3]);
    EXPECT_EQ(2,  b.triArr[4]);
    EXPECT_EQ(3,  b.triArr[5]);

    EXPECT_EQ(10, b.triArr[6]);
    EXPECT_EQ(11, b.triArr[7]);
    EXPECT_EQ(12, b.triArr[8]);
    EXPECT_EQ(10, b.triArr[9]);
    EXPECT_EQ(12, b.triArr[10]);
    EXPECT_EQ(13, b.triArr[11]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddQuadStrip)
{
    //   v0    v2    v4    v6   Resulting quads:
    //   *-----*-----*-----*       q1: v0, v1, v3, v2
    //   |     |     |     |       q2: v2, v3, v5, v4
    //   |     |     |     |       q3: v4, v5, v7, v6
    //   |     |     |     |       
    //   *-----*-----*-----*
    //   v1    v3    v5    v7 

    UIntArray indices;
    indices.reserve(8);
    indices.add(0); 
    indices.add(1); 
    indices.add(2); 
    indices.add(3);
    indices.add(4); 
    indices.add(5); 
    indices.add(6); 
    indices.add(7);

    BuilderTris b;
    b.addQuadStrip(indices);
    ASSERT_EQ(18u,  b.triArr.size());

    EXPECT_EQ(0, b.triArr[0]);
    EXPECT_EQ(1, b.triArr[1]);
    EXPECT_EQ(3, b.triArr[2]);
    EXPECT_EQ(0, b.triArr[3]);
    EXPECT_EQ(3, b.triArr[4]);
    EXPECT_EQ(2, b.triArr[5]);

    EXPECT_EQ(2, b.triArr[6]);
    EXPECT_EQ(3, b.triArr[7]);
    EXPECT_EQ(5, b.triArr[8]);
    EXPECT_EQ(2, b.triArr[9]);
    EXPECT_EQ(5, b.triArr[10]);
    EXPECT_EQ(4, b.triArr[11]);

    EXPECT_EQ(4, b.triArr[12]);
    EXPECT_EQ(5, b.triArr[13]);
    EXPECT_EQ(7, b.triArr[14]);
    EXPECT_EQ(4, b.triArr[15]);
    EXPECT_EQ(7, b.triArr[16]);
    EXPECT_EQ(6, b.triArr[17]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddSingleQuadFromVertices)
{
    const Vec3f v0(0, 0, 0);
    const Vec3f v1(1, 0, 0);
    const Vec3f v2(2, 0, 0);
    const Vec3f v3(3, 0, 0);

    BuilderTris b;
    b.addQuadByVertices(v0, v1, v2, v3);
    EXPECT_EQ(4u, b.vertexArr.size());
    ASSERT_EQ(6u, b.triArr.size());

    EXPECT_EQ(0, b.triArr[0]);
    EXPECT_EQ(1, b.triArr[1]);
    EXPECT_EQ(2, b.triArr[2]);
    EXPECT_EQ(0, b.triArr[3]);
    EXPECT_EQ(2, b.triArr[4]);
    EXPECT_EQ(3, b.triArr[5]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryBuilderTest, AddFace)
{
    BuilderTris b;

    {
        UIntArray indices;
        indices.reserve(3);
        indices.add(0);  
        indices.add(1);  
        indices.add(2);
        b.addFace(indices);
    }

    {
        UIntArray indices;
        indices.reserve(4);
        indices.add(10);  
        indices.add(11);  
        indices.add(12);
        indices.add(13);
        b.addFace(indices);
    }

    {
        UIntArray indices;
        indices.reserve(5);
        indices.add(20);  
        indices.add(21);  
        indices.add(22);
        indices.add(23);
        indices.add(24);
        b.addFace(indices);
    }

    ASSERT_EQ(18u, b.triArr.size());

    EXPECT_EQ(0, b.triArr[0]);
    EXPECT_EQ(1, b.triArr[1]);
    EXPECT_EQ(2, b.triArr[2]);

    EXPECT_EQ(10, b.triArr[3]);
    EXPECT_EQ(11, b.triArr[4]);
    EXPECT_EQ(12, b.triArr[5]);
    EXPECT_EQ(10, b.triArr[6]);
    EXPECT_EQ(12, b.triArr[7]);
    EXPECT_EQ(13, b.triArr[8]);

    EXPECT_EQ(20, b.triArr[9]);
    EXPECT_EQ(21, b.triArr[10]);
    EXPECT_EQ(22, b.triArr[11]);
    EXPECT_EQ(20, b.triArr[12]);
    EXPECT_EQ(22, b.triArr[13]);
    EXPECT_EQ(23, b.triArr[14]);
    EXPECT_EQ(20, b.triArr[15]);
    EXPECT_EQ(23, b.triArr[16]);
    EXPECT_EQ(24, b.triArr[17]);
}



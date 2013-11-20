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
#include "cvfTriangleVertexSplitter.h"
#include "cvfMath.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleVertexSplitterTest, Constructor)
{
    ref<Vec3fArray> va = new Vec3fArray;
    ref<UIntArray> ta = new UIntArray;
    TriangleVertexSplitter splitter(cvf::Math::toRadians(60.0), *ta, *va);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleVertexSplitterTest, EmptyInputs)
{
    ref<Vec3fArray> orgVertices = new Vec3fArray;
    ref<UIntArray> orgIndices = new UIntArray;
    TriangleVertexSplitter splitter(cvf::Math::toRadians(60.0), *orgIndices, *orgVertices);

    ref<UIntArray>  indices = splitter.triangleIndices();
    ref<Vec3fArray> va = splitter.vertexArray();
    ref<UIntArray>  vertexSourceIndices = splitter.perVertexOriginalIndices();

    ASSERT_TRUE(indices.notNull());
    ASSERT_TRUE(va.notNull());
    ASSERT_TRUE(vertexSourceIndices.notNull());

    EXPECT_EQ(0, indices->size());
    EXPECT_EQ(0, va->size());
    EXPECT_EQ(0, vertexSourceIndices->size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleVertexSplitterTest, TwoTriangelsAngleBelowThreshold)
{
    //
    //            2  
    //           /|\       |z    
    //          / | \      | /y               
    //         /  |  \     |/                 
    //        0---1---3    *---x                

    Vec3f v0(0, 0, 0);
    Vec3f v1(1, 0, 0);
    Vec3f v2(1, 0, 1);
    Vec3f v3(2, 1, 0);

    Vec3fArray orgVertices;
    orgVertices.reserve(4);
    orgVertices.add(v0);
    orgVertices.add(v1);
    orgVertices.add(v2);
    orgVertices.add(v3);

    UIntArray orgIndices;
    orgIndices.reserve(6);
    orgIndices.add(0);
    orgIndices.add(1);
    orgIndices.add(2);
    orgIndices.add(1);
    orgIndices.add(3);
    orgIndices.add(2);

    TriangleVertexSplitter splitter(cvf::Math::toRadians(60.0), orgIndices, orgVertices);

    ref<UIntArray>  indices = splitter.triangleIndices();
    ref<Vec3fArray> va = splitter.vertexArray();
    ref<UIntArray>  vertexSourceIndices = splitter.perVertexOriginalIndices();

    ASSERT_TRUE(indices.notNull());
    ASSERT_TRUE(va.notNull());
    ASSERT_TRUE(vertexSourceIndices.notNull());

    ASSERT_EQ(6, indices->size());
    ASSERT_EQ(4, va->size());
    ASSERT_EQ(4, vertexSourceIndices->size());

    EXPECT_EQ(0, indices->get(0));
    EXPECT_EQ(1, indices->get(1));
    EXPECT_EQ(2, indices->get(2));

    EXPECT_EQ(1, indices->get(3));
    EXPECT_EQ(3, indices->get(4));
    EXPECT_EQ(2, indices->get(5));

    EXPECT_EQ(0, vertexSourceIndices->get(0));
    EXPECT_EQ(1, vertexSourceIndices->get(1));
    EXPECT_EQ(2, vertexSourceIndices->get(2));
    EXPECT_EQ(3, vertexSourceIndices->get(3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleVertexSplitterTest, TwoTriangels)
{
    //
    //            2  
    //           /|\      |z    
    //          / | /3    | /y               
    //         /  |/      |/                 
    //        0---1       *---x                
                                
    Vec3f v0(0, 0, 0);
    Vec3f v1(1, 0, 0);
    Vec3f v2(1, 0, 1);
    Vec3f v3(1, 1, 0);

    Vec3fArray orgVertices;
    orgVertices.reserve(4);
    orgVertices.add(v0);
    orgVertices.add(v1);
    orgVertices.add(v2);
    orgVertices.add(v3);

    UIntArray orgIndices;
    orgIndices.reserve(6);
    orgIndices.add(0);
    orgIndices.add(1);
    orgIndices.add(2);
    orgIndices.add(1);
    orgIndices.add(3);
    orgIndices.add(2);

    TriangleVertexSplitter splitter(cvf::Math::toRadians(60.0), orgIndices, orgVertices);

    ref<UIntArray>  indices = splitter.triangleIndices();
    ref<Vec3fArray> va = splitter.vertexArray();
    ref<UIntArray>  vertexSourceIndices = splitter.perVertexOriginalIndices();

    ASSERT_TRUE(indices.notNull());
    ASSERT_TRUE(va.notNull());
    ASSERT_TRUE(vertexSourceIndices.notNull());

    ASSERT_EQ(6, indices->size());
    ASSERT_EQ(6, va->size());
    ASSERT_EQ(6, vertexSourceIndices->size());

    EXPECT_EQ(0, indices->get(0));
    EXPECT_EQ(1, indices->get(1));
    EXPECT_EQ(2, indices->get(2));

    EXPECT_EQ(3, indices->get(3));
    EXPECT_EQ(4, indices->get(4));
    EXPECT_EQ(5, indices->get(5));

    EXPECT_EQ(0, vertexSourceIndices->get(0));
    EXPECT_EQ(1, vertexSourceIndices->get(1));
    EXPECT_EQ(2, vertexSourceIndices->get(2));
    EXPECT_EQ(1, vertexSourceIndices->get(3));
    EXPECT_EQ(3, vertexSourceIndices->get(4));
    EXPECT_EQ(2, vertexSourceIndices->get(5));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleVertexSplitterTest, ThreeTriangels)
{
    //
    //        4---2  
    //        |  /|\      |z    
    //        | / | /3    | /y               
    //        |/  |/      |/                 
    //        0---1       *---x                

    Vec3f v0(0, 0, 0);
    Vec3f v1(1, 0, 0);
    Vec3f v2(1, 0, 1);
    Vec3f v3(1, 1, 0);
    Vec3f v4(0, 0, 1);

    Vec3fArray orgVertices;
    orgVertices.reserve(5);
    orgVertices.add(v0);
    orgVertices.add(v1);
    orgVertices.add(v2);
    orgVertices.add(v3);
    orgVertices.add(v4);

    UIntArray orgIndices;
    orgIndices.reserve(9);
    orgIndices.add(0);
    orgIndices.add(1);
    orgIndices.add(2);
    orgIndices.add(1);
    orgIndices.add(3);
    orgIndices.add(2);
    orgIndices.add(0);
    orgIndices.add(2);
    orgIndices.add(4);

    TriangleVertexSplitter splitter(cvf::Math::toRadians(60.0), orgIndices, orgVertices);

    ref<UIntArray>  indices = splitter.triangleIndices();
    ref<Vec3fArray> va = splitter.vertexArray();
    ref<UIntArray>  vertexSourceIndices = splitter.perVertexOriginalIndices();

    ASSERT_TRUE(indices.notNull());
    ASSERT_TRUE(va.notNull());
    ASSERT_TRUE(vertexSourceIndices.notNull());

    ASSERT_EQ(9, indices->size());
    ASSERT_EQ(7, va->size());
    ASSERT_EQ(7, vertexSourceIndices->size());

    EXPECT_EQ(0, indices->get(0));
    EXPECT_EQ(1, indices->get(1));
    EXPECT_EQ(2, indices->get(2));

    EXPECT_EQ(3, indices->get(3));
    EXPECT_EQ(4, indices->get(4));
    EXPECT_EQ(5, indices->get(5));

    EXPECT_EQ(0, indices->get(6));
    EXPECT_EQ(2, indices->get(7));
    EXPECT_EQ(6, indices->get(8));

    EXPECT_EQ(0, vertexSourceIndices->get(0));
    EXPECT_EQ(1, vertexSourceIndices->get(1));
    EXPECT_EQ(2, vertexSourceIndices->get(2));
    EXPECT_EQ(1, vertexSourceIndices->get(3));
    EXPECT_EQ(3, vertexSourceIndices->get(4));
    EXPECT_EQ(2, vertexSourceIndices->get(5));
    EXPECT_EQ(4, vertexSourceIndices->get(6));
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleVertexSplitterTest, PartialBoxTest)
{
    //      6 ------ 5                Faces:
    //    /   \     /|     |z           0 bottom   0, 3, 2, 1
    //   /      \  / |     | /y         1 top      4, 5, 6, 7
    //  3---------4  |     |/           2 front    4, 0, 1, 5
    //  |       / | \2     *---x        3 right    5, 1, 2, 6
    //  |    /    | /                   4 back     6, 2, 3, 7
    //  | /       |/                    5 left     7, 3, 0, 4
    //  0---------1                     

    Vec3f v0(0, 0, 0);
    Vec3f v1(1, 0, 0);
    Vec3f v2(1, 1, 0);
    Vec3f v3(0, 0, 1);
    Vec3f v4(1, 0, 1);
    Vec3f v5(1, 1, 1);
    Vec3f v6(0, 1, 1);

    Vec3fArray orgVertices;
    orgVertices.reserve(7);
    orgVertices.add(v0);
    orgVertices.add(v1);
    orgVertices.add(v2);
    orgVertices.add(v3);
    orgVertices.add(v4);
    orgVertices.add(v5);
    orgVertices.add(v6);

    const cvf::uint conn[] = 
    { 
        0, 1, 4, 
        0, 4, 3, 
        1, 2, 4, 
        2, 5, 4, 
        3, 4, 6, 
        4, 5, 6, 
    };

    UIntArray orgIndices(conn, sizeof(conn)/sizeof(cvf::uint));

    TriangleVertexSplitter splitter(cvf::Math::toRadians(60.0), orgIndices, orgVertices);

    ref<UIntArray>  indices = splitter.triangleIndices();
    ref<Vec3fArray> va = splitter.vertexArray();
    ref<UIntArray>  vertexSourceIndices = splitter.perVertexOriginalIndices();

    ASSERT_TRUE(indices.notNull());
    ASSERT_TRUE(va.notNull());
    ASSERT_TRUE(vertexSourceIndices.notNull());

    EXPECT_EQ(18, indices->size());
    EXPECT_EQ(12, va->size());
    EXPECT_EQ(12, vertexSourceIndices->size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TriangleVertexSplitterTest, BoxTest)
{
    // The ordering of the faces is consistent with GLviewAPI's hexahedron element.
    // Note that the vertex ordering within a face is not consistent 
    //
    //     7---------6                Faces:
    //    /|        /|     |z           0 bottom   0, 3, 2, 1
    //   / |       / |     | /y         1 top      4, 5, 6, 7
    //  4---------5  |     |/           2 front    4, 0, 1, 5
    //  |  3------|--2     *---x        3 right    5, 1, 2, 6
    //  | /       | /                   4 back     6, 2, 3, 7
    //  |/        |/                    5 left     7, 3, 0, 4
    //  0---------1                     

    Vec3f min(0,0,0);
    Vec3f max(1,1,1);

    ref<Vec3fArray> orgVertices = new Vec3fArray;
    orgVertices->reserve(8);

    orgVertices->add(Vec3f(min.x(), min.y(), min.z()));
    orgVertices->add(Vec3f(max.x(), min.y(), min.z()));
    orgVertices->add(Vec3f(max.x(), max.y(), min.z()));
    orgVertices->add(Vec3f(min.x(), max.y(), min.z()));
                                                     
    orgVertices->add(Vec3f(min.x(), min.y(), max.z()));
    orgVertices->add(Vec3f(max.x(), min.y(), max.z()));
    orgVertices->add(Vec3f(max.x(), max.y(), max.z()));
    orgVertices->add(Vec3f(min.x(), max.y(), max.z()));

    const cvf::uint conn[] = 
    { 
        0, 1, 5, 0, 5, 4,   // Front
        1, 2, 6, 1, 6, 5,   // Right
        2, 3, 7, 2, 7, 6,   // Back
        0, 4, 7, 0, 7, 3,   // Left
        0, 3, 2, 0, 2, 1,   // Bottom
        4, 5, 6, 4, 6, 7    // Top
    };

    ref<UIntArray> orgIndices = new UIntArray(conn, sizeof(conn)/sizeof(cvf::uint));

    ASSERT_EQ(8, orgVertices->size());
    ASSERT_EQ(12*3, orgIndices->size());

    TriangleVertexSplitter splitter(cvf::Math::toRadians(60.0), *orgIndices, *orgVertices);

    ref<UIntArray>  indices = splitter.triangleIndices();
    ref<Vec3fArray> va = splitter.vertexArray();
    ref<UIntArray>  vertexSourceIndices = splitter.perVertexOriginalIndices();

    ASSERT_TRUE(indices.notNull());
    ASSERT_TRUE(va.notNull());
    ASSERT_TRUE(vertexSourceIndices.notNull());

    // Expecting same triangle count, but 4 separate vertices per box side -> 24 nodes
    
    // HACK - these will fail until this is properly implemented.
    EXPECT_EQ(12*3, indices->size());
    EXPECT_EQ(6*4, va->size());
    EXPECT_EQ(6*4, vertexSourceIndices->size());
}

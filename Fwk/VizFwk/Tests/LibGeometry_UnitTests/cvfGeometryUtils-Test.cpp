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
#include "cvfGeometryUtils.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfBoundingBox.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryUtilsTest, CreatePatch)
{
    //         v8-----v9----v10----v11   
    //         |      |      |      |    
    //  origin |      |      |      |    
    // + vunit v4-----v5-----v6-----v7   
    //         |      |      |      |    
    //         |      |      |      |    
    //         v0-----v1-----v2-----v3   
    //     origin    origin              
    //              + uUnit   

    GeometryBuilderFaceList builder;

    const Vec3f origin(10, 20, 2);
    const Vec3f u(2, 0, 0);
    const Vec3f v(0, 1, 0);
    const int uCount = 3;
    const int vCount = 2;
    GeometryUtils::createPatch(origin, u, v, uCount, vCount, &builder);

    ref<Vec3fArray> vertices = builder.vertices();
    ref<UIntArray> faceList = builder.faceList();

    ASSERT_EQ(12, vertices->size());
    ASSERT_EQ(6*5, faceList->size());

    BoundingBox bb;
    bb.add(*vertices);
    const Vec3d& bbMin = bb.min();
    const Vec3d& bbMax = bb.max();
    EXPECT_DOUBLE_EQ(10, bbMin.x());
    EXPECT_DOUBLE_EQ(20, bbMin.y());
    EXPECT_DOUBLE_EQ(2,  bbMin.z());
    EXPECT_DOUBLE_EQ(16, bbMax.x());
    EXPECT_DOUBLE_EQ(22, bbMax.y());
    EXPECT_DOUBLE_EQ(2,  bbMax.z());

    // v0
    {
        const Vec3f& v = vertices->get(0);
        EXPECT_FLOAT_EQ(10, v.x());
        EXPECT_FLOAT_EQ(20, v.y());
    }

    // v3
    {
        const Vec3f& v = vertices->get(3);
        EXPECT_FLOAT_EQ(16, v.x());
        EXPECT_FLOAT_EQ(20, v.y());
    }

    // v11
    {
        const Vec3f& v = vertices->get(11);
        EXPECT_FLOAT_EQ(16, v.x());
        EXPECT_FLOAT_EQ(22, v.y());
    }


    // First quad
    int idx = 0*5;
    ASSERT_EQ(4, faceList->get(idx + 0));
    ASSERT_EQ(4, faceList->get(idx + 1));
    ASSERT_EQ(0, faceList->get(idx + 2));
    ASSERT_EQ(1, faceList->get(idx + 3));
    ASSERT_EQ(5, faceList->get(idx + 4));

    // Third quad
    idx = 2*5;
    ASSERT_EQ(4, faceList->get(idx + 0));
    ASSERT_EQ(6, faceList->get(idx + 1));
    ASSERT_EQ(2, faceList->get(idx + 2));
    ASSERT_EQ(3, faceList->get(idx + 3));
    ASSERT_EQ(7, faceList->get(idx + 4));

    // Last quad
    idx = 5*5;
    ASSERT_EQ(4,  faceList->get(idx + 0));
    ASSERT_EQ(10, faceList->get(idx + 1));
    ASSERT_EQ(6,  faceList->get(idx + 2));
    ASSERT_EQ(7,  faceList->get(idx + 3));
    ASSERT_EQ(11, faceList->get(idx + 4));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryUtilsTest, CreateSphere)
{
    GeometryBuilderFaceList builder;


    //
    //      1 .-------. 3
    //      /\         /\      3 triangles seen from
    //     /   \     /   \     the top of the sphere
    //    |      \0/      | 
    //    |       *       |
    //     \      |      /
    //      \     |     / 
    //        `'-----'´ 
    //            2
    // 
    // 
    //           0
    //       ..---|---..
    //     1/     |     \3
    //     /------|2---- \    Quadstrip seen from the side 
    //   4|______5|_______|6
    //    |       |       |
    //     \------|8-----/
    //    7 \     |     /9
    //        `'--|--'´
    //          10
    GeometryUtils::createSphere(1, 3, 4, &builder);
    
    ref<Vec3fArray> vertices = builder.vertices();
    ref<UIntArray> faceList = builder.faceList();

    ASSERT_EQ(11u, vertices->size());
    ASSERT_EQ(42u + 12u, faceList->size());

    int idx = 0;

    // First row (Triangles)
    ASSERT_EQ(3, faceList->get(idx++));
    ASSERT_EQ(0, faceList->get(idx++));
    ASSERT_EQ(1, faceList->get(idx++));
    ASSERT_EQ(2, faceList->get(idx++));

    ASSERT_EQ(3, faceList->get(idx++));
    ASSERT_EQ(0, faceList->get(idx++));
    ASSERT_EQ(2, faceList->get(idx++));
    ASSERT_EQ(3, faceList->get(idx++));

    ASSERT_EQ(3, faceList->get(idx++));
    ASSERT_EQ(0, faceList->get(idx++));
    ASSERT_EQ(3, faceList->get(idx++));
    ASSERT_EQ(1, faceList->get(idx++));

    // Second row (Quads)
    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(1, faceList->get(idx++));
    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(5, faceList->get(idx++));
    ASSERT_EQ(2, faceList->get(idx++));

    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(2, faceList->get(idx++));
    ASSERT_EQ(5, faceList->get(idx++));
    ASSERT_EQ(6, faceList->get(idx++));
    ASSERT_EQ(3, faceList->get(idx++));

    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(3, faceList->get(idx++));
    ASSERT_EQ(6, faceList->get(idx++));
    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(1, faceList->get(idx++));

    // Third row (Quads)
    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(7, faceList->get(idx++));
    ASSERT_EQ(8, faceList->get(idx++));
    ASSERT_EQ(5, faceList->get(idx++));

    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(5, faceList->get(idx++));
    ASSERT_EQ(8, faceList->get(idx++));
    ASSERT_EQ(9, faceList->get(idx++));
    ASSERT_EQ(6, faceList->get(idx++));

    ASSERT_EQ(4, faceList->get(idx++));
    ASSERT_EQ(6, faceList->get(idx++));
    ASSERT_EQ(9, faceList->get(idx++));
    ASSERT_EQ(7, faceList->get(idx++));
    ASSERT_EQ(4, faceList->get(idx++));

    // Last row (Triangles)
    ASSERT_EQ(3, faceList->get(idx++));
    ASSERT_EQ(10,faceList->get(idx++));
    ASSERT_EQ(9, faceList->get(idx++));
    ASSERT_EQ(8, faceList->get(idx++));

    ASSERT_EQ(3, faceList->get(idx++));
    ASSERT_EQ(10,faceList->get(idx++));
    ASSERT_EQ(8, faceList->get(idx++));
    ASSERT_EQ(7, faceList->get(idx++));

    ASSERT_EQ(3, faceList->get(idx++));
    ASSERT_EQ(10,faceList->get(idx++));
    ASSERT_EQ(7, faceList->get(idx++));
    ASSERT_EQ(9, faceList->get(idx++));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryUtilsTest, IsConvexQuad)
{
    {
        // a------d
        // |      |
        // |      |
        // b------c
        Vec3f a(-2,  2, 0);
        Vec3f b(-2, -2, 0);
        Vec3f c( 2, -2, 0);
        Vec3f d( 2,  2, 0);

        EXPECT_TRUE(GeometryUtils::isConvexQuad(a, b, c, d));
    }

    {
        // Arrow pointing left
        //        d
        //    
        // a    c   
        //    
        //        b
        Vec3f a(-2,  0, 0);
        Vec3f b( 2, -2, 0);
        Vec3f c( 1,  0, 0);
        Vec3f d( 2,  2, 0);

        EXPECT_FALSE(GeometryUtils::isConvexQuad(a, b, c, d));
    }

    {
        // Arrow pointing down
        // a     c
        //    d
        // 
        //    b
        Vec3f a(-2,  2, 0);
        Vec3f b( 0, -2, 0);
        Vec3f c( 2,  2, 0);
        Vec3f d( 0,  1, 0);

        EXPECT_FALSE(GeometryUtils::isConvexQuad(a, b, c, d));
    }

    {
        // Arrow pointing right
        // a 
        //    
        //   b    d   
        //    
        // c 
        Vec3f a(-2,  2, 0);
        Vec3f b(-1,  0, 0);
        Vec3f c(-2, -2, 0);
        Vec3f d( 2,  0, 0);

        EXPECT_FALSE(GeometryUtils::isConvexQuad(a, b, c, d));
    }

    {
        // Arrow pointing up
        //    a    
        //    
        //    c
        // b     d
        Vec3f a( 0,  2, 0);
        Vec3f b(-2, -2, 0);
        Vec3f c( 0, -1, 0);
        Vec3f d( 2, -2, 0);

        EXPECT_FALSE(GeometryUtils::isConvexQuad(a, b, c, d));
    }

    {
        // a---d
        //  \ /
        //   X 
        //  / \   Bowtie
        // c---b  
        Vec3f a(-2,  2, 0);
        Vec3f b( 2, -2, 0);
        Vec3f c(-2, -2, 0);
        Vec3f d( 2,  2, 0);

        EXPECT_FALSE(GeometryUtils::isConvexQuad(a, b, c, d));
    }

    {
        // a--b--c--d
        Vec3f a(-2,  0, 0);
        Vec3f b(-1,  0, 0);
        Vec3f c( 1,  0, 0);
        Vec3f d( 2,  0, 0);

        EXPECT_FALSE(GeometryUtils::isConvexQuad(a, b, c, d));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryUtilsTest, QuadNormal)
{
    Vec3fArray va;
    va.reserve(4);
    va.add(Vec3f(10, 20, 3));
    va.add(Vec3f(11, 20, 3));
    va.add(Vec3f(11, 22, 3));
    va.add(Vec3f(10, 22, 3));

    {
        Vec3f n = GeometryUtils::quadNormal(va[0], va[1], va[2], va[3]);
        EXPECT_NEAR(0, n.x(), 1e-10);
        EXPECT_NEAR(0, n.y(), 1e-10);
        EXPECT_NEAR(1, n.z(), 1e-10);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryUtilsTest, PolygonNormal)
{
    Vec3fArray va;
    va.reserve(5);
    va.add(Vec3f(10, 20, 3));
    va.add(Vec3f(11, 20, 3));
    va.add(Vec3f(11, 22, 3));
    va.add(Vec3f(10, 22, 3));
    va.add(Vec3f(10, 21, 3));

    {
        cvf::uint indices[] = { 0, 1, 2, 3, 4 };
        Vec3f n = GeometryUtils::polygonNormal(va, indices, 5);
        EXPECT_NEAR(0, n.x(), 1e-10);
        EXPECT_NEAR(0, n.y(), 1e-10);
        EXPECT_NEAR(1, n.z(), 1e-10);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(GeometryUtilsTest, RemoveUnusedVertices)
{
    {
        UIntArray indices;
        indices.reserve(9);
        indices.add(2);
        indices.add(1);
        indices.add(0);
        indices.add(1);
        indices.add(2);
        indices.add(3);
        indices.add(6);
        indices.add(7);
        indices.add(8);

        UIntArray newIndices, newToOldVertexMapping;
        GeometryUtils::removeUnusedVertices(indices, &newIndices, &newToOldVertexMapping, 10);
        EXPECT_EQ(9, newIndices.size());
        EXPECT_EQ(7, newToOldVertexMapping.size());

        EXPECT_EQ(0, newIndices[0]);
        EXPECT_EQ(1, newIndices[1]);
        EXPECT_EQ(2, newIndices[2]);
        EXPECT_EQ(1, newIndices[3]);
        EXPECT_EQ(0, newIndices[4]);
        EXPECT_EQ(3, newIndices[5]);
        EXPECT_EQ(4, newIndices[6]);
        EXPECT_EQ(5, newIndices[7]);
        EXPECT_EQ(6, newIndices[8]);
    }

    // Empty source arrays
    {
        UIntArray indices;

        UIntArray newIndices, newToOldVertexMapping;
        GeometryUtils::removeUnusedVertices(indices, &newIndices, &newToOldVertexMapping, 10);
        size_t zero = 0;
        EXPECT_EQ(zero, newIndices.size());
        EXPECT_EQ(zero, newToOldVertexMapping.size());

        // Test if NULL
        GeometryUtils::removeUnusedVertices(indices, NULL, &newToOldVertexMapping, 10);
        GeometryUtils::removeUnusedVertices(indices, &newIndices, NULL, 10);
    }

    // All nodes are used, source and compact are equal
    {
        UIntArray indices;
        indices.reserve(9);
        indices.add(0);
        indices.add(1);
        indices.add(2);
        indices.add(1);
        indices.add(2);
        indices.add(3);
        indices.add(1);
        indices.add(2);
        indices.add(3);

        UIntArray newIndices, newToOldVertexMapping;
        GeometryUtils::removeUnusedVertices(indices, &newIndices, &newToOldVertexMapping, 10);
        EXPECT_EQ(9, newIndices.size());
        EXPECT_EQ(4, newToOldVertexMapping.size());

        EXPECT_EQ(indices[0], newIndices[0]);
        EXPECT_EQ(indices[1], newIndices[1]);
        EXPECT_EQ(indices[2], newIndices[2]);
        EXPECT_EQ(indices[3], newIndices[3]);
        EXPECT_EQ(indices[4], newIndices[4]);
        EXPECT_EQ(indices[5], newIndices[5]);
        EXPECT_EQ(indices[6], newIndices[6]);
        EXPECT_EQ(indices[7], newIndices[7]);
        EXPECT_EQ(indices[8], newIndices[8]);
    }
}

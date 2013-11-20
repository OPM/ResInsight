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
#include "cvfVertexCompactor.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexCompactorTest, Constructor)
{
    ref<Vec3fArray> orgVertices = new Vec3fArray;
    ref<UIntArray> orgIndices = new UIntArray;
    VertexCompactor vc(*orgIndices, *orgVertices);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexCompactorTest, EmptyInputs)
{
    ref<Vec3fArray> orgVertices = new Vec3fArray;
    ref<UIntArray> orgIndices = new UIntArray;
    VertexCompactor vc(*orgIndices, *orgVertices);

    ref<UIntArray>  indices = vc.indices();
    ref<Vec3fArray> va = vc.vertexArray();
    ref<UIntArray>  vertexSourceIndices = vc.perVertexOriginalIndices();

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
TEST(VertexCompactorTest, Quads)
{
    // Vertices, three possible quads 
    // 4------5------6------7
    // |  Q1  |  Q2  |  Q3  | 
    // |      |      |      | 
    // 0------1------2------3

    Vec3fArray orgVertices;
    orgVertices.reserve(8);
    orgVertices.add(Vec3f(0, 0, 0));
    orgVertices.add(Vec3f(1, 0, 0));
    orgVertices.add(Vec3f(2, 0, 0));
    orgVertices.add(Vec3f(3, 0, 0));
    orgVertices.add(Vec3f(0, 1, 0));
    orgVertices.add(Vec3f(1, 1, 0));
    orgVertices.add(Vec3f(2, 1, 0));
    orgVertices.add(Vec3f(3, 1, 0));

    const cvf::uint connQ1[4] = { 0, 1, 5, 4 };
    const cvf::uint connQ2[4] = { 1, 2, 6, 5 };
    //const cvf::uint connQ3[4] = { 2, 3, 7, 6 };

    // Q2
    {
        UIntArray orgConn(connQ2, 4);
        VertexCompactor vc(orgConn, orgVertices);

        ref<UIntArray> indices = vc.indices();
        ASSERT_EQ(4, indices->size());
        EXPECT_EQ(0, indices->get(0));
        EXPECT_EQ(1, indices->get(1));
        EXPECT_EQ(2, indices->get(2));
        EXPECT_EQ(3, indices->get(3));

        ref<Vec3fArray> va = vc.vertexArray();
        ASSERT_EQ(4, va->size());
        EXPECT_TRUE(Vec3f(1, 0, 0) == va->get(0));
        EXPECT_TRUE(Vec3f(2, 0, 0) == va->get(1));
        EXPECT_TRUE(Vec3f(2, 1, 0) == va->get(2));
        EXPECT_TRUE(Vec3f(1, 1, 0) == va->get(3));

        ref<UIntArray> vertexSourceIndices = vc.perVertexOriginalIndices();
        ASSERT_EQ(4, vertexSourceIndices->size());
        EXPECT_EQ(1, vertexSourceIndices->get(0));
        EXPECT_EQ(2, vertexSourceIndices->get(1));
        EXPECT_EQ(6, vertexSourceIndices->get(2));
        EXPECT_EQ(5, vertexSourceIndices->get(3));
    }

    // Q1 + Q2
    {
        UIntArray orgConn(8);
        orgConn.copyData(connQ1, 4, 0);
        orgConn.copyData(connQ2, 4, 4);

        VertexCompactor vc(orgConn, orgVertices);

        ref<UIntArray> indices = vc.indices();
        ASSERT_EQ(8, indices->size());
        EXPECT_EQ(0, indices->get(0));
        EXPECT_EQ(1, indices->get(1));
        EXPECT_EQ(2, indices->get(2));
        EXPECT_EQ(3, indices->get(3));
        EXPECT_EQ(1, indices->get(4));
        EXPECT_EQ(4, indices->get(5));
        EXPECT_EQ(5, indices->get(6));
        EXPECT_EQ(2, indices->get(7));

        ref<Vec3fArray> va = vc.vertexArray();
        ASSERT_EQ(6, va->size());
        EXPECT_TRUE(Vec3f(0, 0, 0) == va->get(0));
        EXPECT_TRUE(Vec3f(1, 0, 0) == va->get(1));
        EXPECT_TRUE(Vec3f(1, 1, 0) == va->get(2));
        EXPECT_TRUE(Vec3f(0, 1, 0) == va->get(3));
        EXPECT_TRUE(Vec3f(2, 0, 0) == va->get(4));
        EXPECT_TRUE(Vec3f(2, 1, 0) == va->get(5));

        ref<UIntArray> vertexSourceIndices = vc.perVertexOriginalIndices();
        ASSERT_EQ(6, vertexSourceIndices->size());
        EXPECT_EQ(0, vertexSourceIndices->get(0));
        EXPECT_EQ(1, vertexSourceIndices->get(1));
        EXPECT_EQ(5, vertexSourceIndices->get(2));
        EXPECT_EQ(4, vertexSourceIndices->get(3));
        EXPECT_EQ(2, vertexSourceIndices->get(4));
        EXPECT_EQ(6, vertexSourceIndices->get(5));
    }
}



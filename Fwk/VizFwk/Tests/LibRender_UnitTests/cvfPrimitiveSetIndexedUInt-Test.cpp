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
#include "cvfPrimitiveSetIndexedUInt.h"

#include "gtest/gtest.h"

#include "cvfOpenGL.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, CheckPrimitiveTypeMapping)
{
    // This is really a test of functionality in the base!

    PrimitiveSetIndexedUInt p1(PT_POINTS);
    PrimitiveSetIndexedUInt p2(PT_LINES);
    PrimitiveSetIndexedUInt p3(PT_LINE_LOOP);
    PrimitiveSetIndexedUInt p4(PT_LINE_STRIP);
    PrimitiveSetIndexedUInt p5(PT_TRIANGLES);
    PrimitiveSetIndexedUInt p6(PT_TRIANGLE_STRIP);
    PrimitiveSetIndexedUInt p7(PT_TRIANGLE_FAN);

    EXPECT_EQ(GL_POINTS,            static_cast<int>(p1.primitiveTypeOpenGL()));
    EXPECT_EQ(GL_LINES,             static_cast<int>(p2.primitiveTypeOpenGL()));
    EXPECT_EQ(GL_LINE_LOOP,         static_cast<int>(p3.primitiveTypeOpenGL()));
    EXPECT_EQ(GL_LINE_STRIP,        static_cast<int>(p4.primitiveTypeOpenGL()));
    EXPECT_EQ(GL_TRIANGLES,         static_cast<int>(p5.primitiveTypeOpenGL()));
    EXPECT_EQ(GL_TRIANGLE_STRIP,    static_cast<int>(p6.primitiveTypeOpenGL()));
    EXPECT_EQ(GL_TRIANGLE_FAN,      static_cast<int>(p7.primitiveTypeOpenGL()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, BasicConstructionAndEmptyObject)
{
    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_POINTS);

    EXPECT_EQ(0u, myPrim->indexCount());

    myPrim->setIndices(NULL);
    EXPECT_EQ(0u, myPrim->indexCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(PrimitiveSetIndexedUIntDeathTest, AccessOutOfBounds)
{
    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_POINTS);
    EXPECT_EQ(0u, myPrim->indexCount());

    EXPECT_DEATH(myPrim->index(0), "Assertion");

    ref<UIntArray> myIndices = new UIntArray;
    myIndices->reserve(2);
    myIndices->add(10);
    myIndices->add(11);

    myPrim->setIndices(myIndices.p());
    ASSERT_EQ(2u, myPrim->indexCount());
    EXPECT_EQ(10, myPrim->index(0));
    EXPECT_EQ(11, myPrim->index(1));

    EXPECT_DEATH(myPrim->index(2), "Assertion");
}
#endif



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, SettingAndGettingIndices)
{
    ref<UIntArray> myIndices = new UIntArray;
    EXPECT_EQ(1, myIndices->refCount());
    myIndices->reserve(3);
    myIndices->add(10);
    myIndices->add(11);
    myIndices->add(12);

    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_POINTS);
    EXPECT_EQ(0u, myPrim->indexCount());

    myPrim->setIndices(myIndices.p());
    EXPECT_EQ(2, myIndices->refCount());

    ASSERT_EQ(3u, myPrim->indexCount());
    EXPECT_EQ(10u, myPrim->index(0));
    EXPECT_EQ(11u, myPrim->index(1));
    EXPECT_EQ(12u, myPrim->index(2));

    EXPECT_EQ(10u, myPrim->minIndex());
    EXPECT_EQ(12u, myPrim->maxIndex());

    const UIntArray* indices = myPrim->indices();
    ASSERT_TRUE(indices != NULL);
    ASSERT_EQ(3u, indices->size());
    EXPECT_EQ(10u, indices->get(0));
    EXPECT_EQ(11u, indices->get(1));
    EXPECT_EQ(12u, indices->get(2));

    myPrim->setIndices(NULL);
    EXPECT_EQ(1, myIndices->refCount());
    EXPECT_EQ(-1, myPrim->minIndex());
    EXPECT_EQ(-1, myPrim->maxIndex());
    EXPECT_EQ(UNDEFINED_UINT, myPrim->minIndex());
    EXPECT_EQ(UNDEFINED_UINT, myPrim->maxIndex());

    ASSERT_EQ(0u, myPrim->indexCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, GetOpenGLPrimitive_FromPoints)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(3);
    indices->add(0);  
    indices->add(1);  
    indices->add(2);

    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_POINTS);
    myPrim->setIndices(indices.p());

    ASSERT_EQ(3u, myPrim->faceCount());

    UIntArray conn;

    myPrim->getFaceIndices(0, &conn);
    ASSERT_EQ(1u, conn.size());
    EXPECT_EQ(0u, conn.get(0));

    myPrim->getFaceIndices(1, &conn);
    ASSERT_EQ(1u, conn.size());
    EXPECT_EQ(1u, conn.get(0));

    myPrim->getFaceIndices(2, &conn);
    ASSERT_EQ(1u, conn.size());
    EXPECT_EQ(2u, conn.get(0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, GetOpenGLPrimitive_FromLines)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(4);
    indices->add(0);  
    indices->add(1);  
    indices->add(2);
    indices->add(3);

    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_LINES);
    myPrim->setIndices(indices.p());

    ASSERT_EQ(2u, myPrim->faceCount());

    UIntArray conn;

    myPrim->getFaceIndices(0, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(0u, conn.get(0));
    EXPECT_EQ(1u, conn.get(1));

    myPrim->getFaceIndices(1, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(2u, conn.get(0));
    EXPECT_EQ(3u, conn.get(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, GetOpenGLPrimitive_FromLineLoop)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(4);
    indices->add(0);  
    indices->add(1);  
    indices->add(2);
    indices->add(3);

    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_LINE_LOOP);
    myPrim->setIndices(indices.p());

    ASSERT_EQ(4u, myPrim->faceCount());

    UIntArray conn;

    myPrim->getFaceIndices(0, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(0u, conn.get(0));
    EXPECT_EQ(1u, conn.get(1));

    myPrim->getFaceIndices(1, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(1u, conn.get(0));
    EXPECT_EQ(2u, conn.get(1));

    myPrim->getFaceIndices(2, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(2u, conn.get(0));
    EXPECT_EQ(3u, conn.get(1));

    myPrim->getFaceIndices(3, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(3u, conn.get(0));
    EXPECT_EQ(0u, conn.get(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, GetOpenGLPrimitive_FromLineStrip)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(4);
    indices->add(0);  
    indices->add(1);  
    indices->add(2);
    indices->add(3);

    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_LINE_STRIP);
    myPrim->setIndices(indices.p());

    ASSERT_EQ(3u, myPrim->faceCount());

    UIntArray conn;

    myPrim->getFaceIndices(0, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(0u, conn.get(0));
    EXPECT_EQ(1u, conn.get(1));

    myPrim->getFaceIndices(1, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(1u, conn.get(0));
    EXPECT_EQ(2u, conn.get(1));

    myPrim->getFaceIndices(2, &conn);
    ASSERT_EQ(2u, conn.size());
    EXPECT_EQ(2u, conn.get(0));
    EXPECT_EQ(3u, conn.get(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, GetOpenGLPrimitive_FromTriangles)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(6);
    indices->add(0);  
    indices->add(1);  
    indices->add(2);
    indices->add(3);
    indices->add(4);
    indices->add(5);

    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
    myPrim->setIndices(indices.p());

    ASSERT_EQ(2u, myPrim->faceCount());

    UIntArray conn;
    
    myPrim->getFaceIndices(0, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(0u, conn.get(0));
    EXPECT_EQ(1u, conn.get(1));
    EXPECT_EQ(2u, conn.get(2));

    myPrim->getFaceIndices(1, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(3u, conn.get(0));
    EXPECT_EQ(4u, conn.get(1));
    EXPECT_EQ(5u, conn.get(2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, GetOpenGLPrimitive_FromTriFan)
{
    // See TEST(GeometryBuilderTest, AddTriangleFan)
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(5);
    indices->add(0);  
    indices->add(1);  
    indices->add(2);
    indices->add(3);
    indices->add(4);

    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_TRIANGLE_FAN);
    myPrim->setIndices(indices.p());

    ASSERT_EQ(3u, myPrim->faceCount());

    UIntArray conn;

    myPrim->getFaceIndices(0, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(0u, conn.get(0));
    EXPECT_EQ(1u, conn.get(1));
    EXPECT_EQ(2u, conn.get(2));

    myPrim->getFaceIndices(1, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(0u, conn.get(0));
    EXPECT_EQ(2u, conn.get(1));
    EXPECT_EQ(3u, conn.get(2));

    myPrim->getFaceIndices(2, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(0u, conn.get(0));
    EXPECT_EQ(3u, conn.get(1));
    EXPECT_EQ(4u, conn.get(2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUInt, GetOpenGLPrimitive_FromTriStrip)
{
    // See TEST(GeometryBuilderTest, AddTriangleStrip)
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(6);
    indices->add(0);  
    indices->add(1);  
    indices->add(2);
    indices->add(3);
    indices->add(4);
    indices->add(5);

    ref<PrimitiveSetIndexedUInt> myPrim = new PrimitiveSetIndexedUInt(PT_TRIANGLE_STRIP);
    myPrim->setIndices(indices.p());

    ASSERT_EQ(4u, myPrim->faceCount());

    UIntArray conn;
    
    myPrim->getFaceIndices(0, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(0u, conn.get(0));
    EXPECT_EQ(1u, conn.get(1));
    EXPECT_EQ(2u, conn.get(2));

    myPrim->getFaceIndices(1, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(2u, conn.get(0));
    EXPECT_EQ(1u, conn.get(1));
    EXPECT_EQ(3u, conn.get(2));

    myPrim->getFaceIndices(2, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(2u, conn.get(0));
    EXPECT_EQ(3u, conn.get(1));
    EXPECT_EQ(4u, conn.get(2));

    myPrim->getFaceIndices(3, &conn);
    ASSERT_EQ(3u, conn.size());
    EXPECT_EQ(4u, conn.get(0));
    EXPECT_EQ(3u, conn.get(1));
    EXPECT_EQ(5u, conn.get(2));
}

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
#include "cvfVertexBundle.h"
#include "cvfVertexAttribute.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexBundleTest, BasicConstruction)
{
    VertexBundle vb;
    EXPECT_EQ(0, vb.vertexCount());
    EXPECT_EQ(NULL, vb.vertexArray());
    EXPECT_EQ(NULL, vb.normalArray());
    EXPECT_EQ(0, vb.genericAttributeCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexBundleTest, SetGetArrays)
{
    ref<Vec3fArray> va = new Vec3fArray(1);
    ref<Vec3fArray> na = new Vec3fArray(1);
    ref<Vec2fArray> ta = new Vec2fArray(1);
    ref<Color3ubArray> ca = new Color3ubArray(1);

    VertexBundle vb;
    vb.setVertexArray(va.p());
    vb.setNormalArray(na.p());
    vb.setTextureCoordArray(ta.p());
    vb.setColorArray(ca.p());
    ASSERT_EQ(2, va->refCount());
    ASSERT_EQ(2, na->refCount());
    ASSERT_EQ(2, ta->refCount());
    ASSERT_EQ(2, ca->refCount());

    EXPECT_EQ(va.p(), vb.vertexArray());
    EXPECT_EQ(na.p(), vb.normalArray());
    EXPECT_EQ(ta.p(), vb.textureCoordArray());
    EXPECT_EQ(ca.p(), vb.colorArray());

    vb.setVertexArray(NULL);
    vb.setNormalArray(NULL);
    vb.setTextureCoordArray(NULL);
    vb.setColorArray(NULL);

    ASSERT_EQ(1, va->refCount());
    ASSERT_EQ(1, na->refCount());
    ASSERT_EQ(1, ta->refCount());
    ASSERT_EQ(1, ca->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexBundleTest, ShallowCopy)
{
    ref<Vec3fArray> va = new Vec3fArray(1);
    ref<Vec3fArray> na = new Vec3fArray(1);
    ref<Vec2fArray> ta = new Vec2fArray(1);
    ref<Color3ubArray> ca = new Color3ubArray(1);

    ref<VertexAttribute> a1 = new FloatVertexAttribute("attr1", new FloatArray);
    ref<VertexAttribute> a2 = new IntVertexAttributePure("attr2", new IntArray);

    VertexBundle vb;
    vb.setVertexArray(va.p());
    vb.setNormalArray(na.p());
    vb.setTextureCoordArray(ta.p());
    vb.setColorArray(ca.p());
    vb.setGenericAttribute(a1.p());
    vb.setGenericAttribute(a2.p());

    ref<VertexBundle> cpy = vb.shallowCopy();
    EXPECT_EQ(vb.vertexArray(),       cpy->vertexArray());
    EXPECT_EQ(vb.normalArray(),       cpy->normalArray());
    EXPECT_EQ(vb.textureCoordArray(), cpy->textureCoordArray());
    EXPECT_EQ(vb.colorArray(),        cpy->colorArray());

    ASSERT_EQ(vb.genericAttributeCount(), cpy->genericAttributeCount());
    for (size_t i = 0; i < vb.genericAttributeCount(); i++)
    {
        VertexAttribute* a = vb.genericAttribute(i);
        VertexAttribute* b = cpy->genericAttribute(i);
        EXPECT_EQ(a, b);
    }

    vb.clear();

    EXPECT_EQ(va.p(), cpy->vertexArray());
    EXPECT_EQ(na.p(), cpy->normalArray());
    EXPECT_EQ(ta.p(), cpy->textureCoordArray());
    EXPECT_EQ(ca.p(), cpy->colorArray());

    EXPECT_EQ(a1.p(), cpy->genericAttribute(0));
    EXPECT_EQ(a2.p(), cpy->genericAttribute(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexBundleTest, SetGenericAttribute)
{
    VertexBundle vb;

    ref<VertexAttribute> va1 = new FloatVertexAttribute("va1", new FloatArray);
    vb.setGenericAttribute(va1.p());
    ASSERT_EQ(1, vb.genericAttributeCount());
    EXPECT_EQ(va1.p(), vb.genericAttribute(0));

    ref<VertexAttribute> va2 = new FloatVertexAttribute("va2", new FloatArray);
    vb.setGenericAttribute(va2.p());
    ASSERT_EQ(2, vb.genericAttributeCount());
    EXPECT_EQ(va2.p(), vb.genericAttribute(1));

    // Add the va1 again
    vb.setGenericAttribute(va1.p());
    ASSERT_EQ(2, vb.genericAttributeCount());
    EXPECT_EQ(va1.p(), vb.genericAttribute(0));

    // Add an attribute with the same name
    ref<VertexAttribute> va1_2 = new FloatVertexAttribute("va1", new FloatArray);
    vb.setGenericAttribute(va1_2.p());
    ASSERT_EQ(2, vb.genericAttributeCount());
    EXPECT_EQ(va1_2.p(), vb.genericAttribute(0));
    EXPECT_EQ(va2.p(), vb.genericAttribute(1));

    // Then another new one
    ref<VertexAttribute> va3 = new FloatVertexAttribute("va3", new FloatArray);
    vb.setGenericAttribute(va3.p());
    ASSERT_EQ(3, vb.genericAttributeCount());
    EXPECT_EQ(va3.p(), vb.genericAttribute(2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(VertexBundleDeathTest, SetGenericAttributeWithNullPtr)
{
    VertexBundle vb;
    EXPECT_DEATH(vb.setGenericAttribute(NULL), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(VertexBundleDeathTest, AccessEmptyGenericAttribute)
{
    VertexBundle vb;
    ASSERT_EQ(0, vb.genericAttributeCount());

    EXPECT_DEATH(vb.genericAttribute(0), "Assertion");
    EXPECT_DEATH(vb.genericAttribute(1), "Assertion");

    vb.setGenericAttribute(new FloatVertexAttribute("dummy", new FloatArray));
    EXPECT_DEATH(vb.genericAttribute(1), "Assertion");
}
#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VertexBundleTest, RemoveGenericAttribute)
{
    VertexBundle vb;

    ref<VertexAttribute> va1 = new FloatVertexAttribute("va1", new FloatArray);
    ref<VertexAttribute> va2 = new FloatVertexAttribute("va2", new FloatArray);
    ref<VertexAttribute> va3 = new FloatVertexAttribute("va3", new FloatArray);
    ASSERT_EQ(1, va1->refCount());
    ASSERT_EQ(1, va1->refCount());
    ASSERT_EQ(1, va1->refCount());

    vb.setGenericAttribute(va1.p());
    vb.setGenericAttribute(va2.p());
    vb.setGenericAttribute(va3.p());
    ASSERT_EQ(3, vb.genericAttributeCount());
    ASSERT_EQ(2, va1->refCount());
    ASSERT_EQ(2, va1->refCount());
    ASSERT_EQ(2, va1->refCount());

    vb.removeGenericAttribute(va2.p());
    ASSERT_EQ(2, vb.genericAttributeCount());
    ASSERT_EQ(1, va2->refCount());

    // Everything in place?
    EXPECT_EQ(va1.p(), vb.genericAttribute(0));
    EXPECT_EQ(va3.p(), vb.genericAttribute(1));

    // Remove again
    vb.removeGenericAttribute(va2.p());
    ASSERT_EQ(2, vb.genericAttributeCount());
    ASSERT_EQ(1, va2->refCount());

    // Remove with NULL
    vb.removeGenericAttribute(NULL);
    ASSERT_EQ(2, vb.genericAttributeCount());

    vb.removeGenericAttribute(va1.p());
    vb.removeGenericAttribute(va3.p());
    ASSERT_EQ(0, vb.genericAttributeCount());
    ASSERT_EQ(1, va1->refCount());
    ASSERT_EQ(1, va2->refCount());
    ASSERT_EQ(1, va3->refCount());
}


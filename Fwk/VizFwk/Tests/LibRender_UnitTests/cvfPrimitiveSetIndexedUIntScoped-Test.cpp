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
#include "cvfPrimitiveSetIndexedUIntScoped.h"

#include "gtest/gtest.h"

#include "cvfOpenGL.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUIntScoped, BasicConstructionAndEmptyObject)
{
    PrimitiveSetIndexedUIntScoped ps(PT_POINTS);
    EXPECT_EQ(0u, ps.indexCount());

    ps.setIndices(NULL, 0, 0);
    EXPECT_EQ(0u, ps.indexCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUIntScoped, SettingFullScope)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(2);
    indices->add(10);
    indices->add(11);

    PrimitiveSetIndexedUIntScoped ps(PT_POINTS);
    ps.setIndices(indices.p(), 0, 2);
    ASSERT_EQ(2, ps.indexCount());
    EXPECT_EQ(10, ps.index(0));
    EXPECT_EQ(11, ps.index(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUIntScoped, SettingScoped)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(3);
    indices->add(10);
    indices->add(11);
    indices->add(12);

    PrimitiveSetIndexedUIntScoped ps(PT_POINTS);
    ps.setIndices(indices.p(), 0, 0);
    ASSERT_EQ(0, ps.indexCount());

    ps.setIndices(indices.p(), 0, 2);
    ASSERT_EQ(2, ps.indexCount());
    EXPECT_EQ(10, ps.index(0));
    EXPECT_EQ(11, ps.index(1));

    ps.setIndices(indices.p(), 1, 2);
    ASSERT_EQ(2, ps.indexCount());
    EXPECT_EQ(11, ps.index(0));
    EXPECT_EQ(12, ps.index(1));
    EXPECT_EQ(1, ps.scopeFirstElement());
    EXPECT_EQ(2, ps.scopeElementCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(PrimitiveSetIndexedUIntScopedDeathTest, IllegalSettingOfIndices)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(2);
    indices->add(10);
    indices->add(11);

    PrimitiveSetIndexedUIntScoped ps(PT_POINTS);
    EXPECT_DEATH(ps.setIndices(indices.p(), 0, 3), "Assertion");
    EXPECT_DEATH(ps.setIndices(indices.p(), 1, 2), "Assertion");
    EXPECT_DEATH(ps.setIndices(indices.p(), 2, 3), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(PrimitiveSetIndexedUIntScopedDeathTest, IllegalSettingOfNullIndices)
{
    PrimitiveSetIndexedUIntScoped ps(PT_POINTS);
    EXPECT_DEATH(ps.setIndices(NULL, 1, 2), "Assertion");
    EXPECT_DEATH(ps.setIndices(NULL, 0, 3), "Assertion");
    EXPECT_DEATH(ps.setIndices(NULL, 3, 0), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveSetIndexedUIntScoped, SettingScopeOnly)
{
    ref<UIntArray> indices = new UIntArray;
    indices->reserve(3);
    indices->add(10);
    indices->add(11);
    indices->add(12);

    PrimitiveSetIndexedUIntScoped ps(PT_POINTS);
    ps.setIndices(indices.p(), 0, 3);
    ASSERT_EQ(3, ps.indexCount());

    ps.setScope(0, 2);
    ASSERT_EQ(2, ps.indexCount());
    EXPECT_EQ(10, ps.index(0));
    EXPECT_EQ(11, ps.index(1));

    ps.setScope(1, 2);
    ASSERT_EQ(2, ps.indexCount());
    EXPECT_EQ(11, ps.index(0));
    EXPECT_EQ(12, ps.index(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(PrimitiveSetIndexedUIntScopedDeathTest, AccessOutOfBounds)
{
    PrimitiveSetIndexedUIntScoped ps(PT_POINTS);
    EXPECT_EQ(0, ps.indexCount());
    EXPECT_DEATH(ps.index(0), "Assertion");

    ref<UIntArray> indices = new UIntArray;
    indices->reserve(2);
    indices->add(10);
    indices->add(11);

    ps.setIndices(indices.p(), 0, 2);
    EXPECT_DEATH(ps.index(2), "Assertion");
}
#endif




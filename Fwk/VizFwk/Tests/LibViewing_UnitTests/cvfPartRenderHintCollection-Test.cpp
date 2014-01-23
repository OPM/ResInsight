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
#include "cvfPartRenderHintCollection.h"
#include "cvfPart.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartRenderHintCollectionTest, BasicConstruction)
{
    ref<PartRenderHintCollection> collection = new PartRenderHintCollection;

    ASSERT_EQ(0u, collection->count());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartRenderHintCollectionTest, SimpleLifeCycle)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;
    ref<Part> p3 = new Part;

    PartRenderHintCollection coll;
    ASSERT_EQ(0, coll.count());

    coll.add(p1.p());
    coll.add(p2.p());
    coll.add(p3.p());
    ASSERT_EQ(3, coll.count());

    coll.setCountZero();
    ASSERT_EQ(0, coll.count());

    coll.add(p3.p());
    coll.add(p2.p());
    coll.add(p1.p());
    ASSERT_EQ(3, coll.count());

    coll.removeAll();
    ASSERT_EQ(0, coll.count());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartRenderHintCollectionTest, AddPartOnly)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;
    ref<Part> p3 = new Part;

    PartRenderHintCollection coll;
    ASSERT_EQ(0, coll.count());

    coll.add(p1.p());
    coll.add(p2.p());
    coll.add(p3.p());
    ASSERT_EQ(3, coll.count());

    EXPECT_EQ(p1.p(), coll.part(0));
    EXPECT_EQ(p2.p(), coll.part(1));
    EXPECT_EQ(p3.p(), coll.part(2));

    EXPECT_EQ(NULL, coll.renderHint(0));
    EXPECT_EQ(NULL, coll.renderHint(1));
    EXPECT_EQ(NULL, coll.renderHint(2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartRenderHintCollectionTest, AddPartAndHint)
{
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;
    ref<Part> p3 = new Part;

    PartRenderHintCollection coll;
    ASSERT_EQ(0, coll.count());

    coll.add(p1.p(), 1.0f, 11.0f);
    coll.add(p2.p(), 2.0f, 12.0f);
    coll.add(p3.p(), 3.0f, 13.0f);
    ASSERT_EQ(3, coll.count());

    EXPECT_EQ(p1.p(), coll.part(0));
    EXPECT_EQ(p2.p(), coll.part(1));
    EXPECT_EQ(p3.p(), coll.part(2));

    EXPECT_FLOAT_EQ(1.0,  coll.renderHint(0)->projectedAreaPixels());
    EXPECT_FLOAT_EQ(2.0,  coll.renderHint(1)->projectedAreaPixels());
    EXPECT_FLOAT_EQ(3.0,  coll.renderHint(2)->projectedAreaPixels());
    EXPECT_FLOAT_EQ(11.0, coll.renderHint(0)->centerDistance());
    EXPECT_FLOAT_EQ(12.0, coll.renderHint(1)->centerDistance());
    EXPECT_FLOAT_EQ(13.0, coll.renderHint(2)->centerDistance());

    coll.setCountZero();
    EXPECT_EQ(0, coll.count());

    coll.add(p2.p(), 20.0, 120);
    coll.add(p3.p(), 30.0, 130);
    ASSERT_EQ(2, coll.count());

    EXPECT_EQ(p2.p(), coll.part(0));
    EXPECT_EQ(p3.p(), coll.part(1));

    EXPECT_FLOAT_EQ(20.0,  coll.renderHint(0)->projectedAreaPixels());
    EXPECT_FLOAT_EQ(30.0,  coll.renderHint(1)->projectedAreaPixels());
    EXPECT_FLOAT_EQ(120.0, coll.renderHint(0)->centerDistance());
    EXPECT_FLOAT_EQ(130.0, coll.renderHint(1)->centerDistance());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(PartRenderHintCollectionDeathTest, AddWithNullPart)
{
    PartRenderHintCollection coll;

    EXPECT_DEATH(coll.add(NULL), "Assertion");
    EXPECT_DEATH(coll.add(NULL, 1.0f, 2.0f), "Assertion");
}
#endif




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(PartRenderHintCollectionDeathTest, IllegalAccess)
{
    PartRenderHintCollection coll;

    EXPECT_DEATH(coll.part(0), "Assertion");
    EXPECT_DEATH(coll.renderHint(0), "Assertion");

    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;
    ref<Part> p3 = new Part;

    coll.add(p1.p(), 1.0f, 11.0f);
    coll.add(p2.p(), 2.0f, 12.0f);
    coll.add(p3.p(), 3.0f, 13.0f);
    ASSERT_EQ(3, coll.count());
    EXPECT_DEATH(coll.part(3), "Assertion");
    EXPECT_DEATH(coll.renderHint(3), "Assertion");

    coll.setCountZero();
    EXPECT_DEATH(coll.part(0), "Assertion");
    EXPECT_DEATH(coll.renderHint(0), "Assertion");
}
#endif




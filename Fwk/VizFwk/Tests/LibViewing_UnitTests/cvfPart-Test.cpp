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
#include "cvfPart.h"
#include "cvfArray.h"
#include "cvfDrawableGeo.h"
#include "cvfEffect.h"


#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, BasicConstruction)
{
    Part part;
    ASSERT_EQ(0, part.refCount());

    ref<Part> part2 = new Part;
    ASSERT_EQ(1, part2->refCount());

    // Check defaults
    EXPECT_STREQ("", part2->name().toAscii().ptr());
    EXPECT_EQ(-1, part2->id());
    EXPECT_EQ(0xffffffff, part2->enableMask());

    Part p2(123, "test");
    EXPECT_STREQ("test", p2.name().toAscii().ptr());
    EXPECT_EQ(123, p2.id());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, SetGetId)
{
    ref<Part> myPart1 = new Part;
    EXPECT_EQ(1, myPart1->refCount());
    EXPECT_EQ(-1, myPart1->id());
    myPart1->setId(1);
    EXPECT_EQ(1, myPart1->id());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, SetGetName)
{
    ref<Part> myPart1 = new Part;
    EXPECT_EQ(1, myPart1->refCount());
    EXPECT_STREQ("", myPart1->name().toAscii().ptr());
    myPart1->setName("Part1");
    EXPECT_STREQ("Part1", myPart1->name().toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, EnableMask)
{
    const cvf::uint visibleBit  = 0x00000001;
    const cvf::uint selectedBit = 0x00000002;


    ref<Part> myPart = new Part;
    EXPECT_EQ(0xffffffff, myPart->enableMask());

    EXPECT_NE(0u, myPart->enableMask() & visibleBit);
    EXPECT_NE(0u, myPart->enableMask() & selectedBit);

    myPart->setEnableMask(0x00000000);
    EXPECT_EQ(0u, myPart->enableMask() & visibleBit);
    EXPECT_EQ(0u, myPart->enableMask() & selectedBit);

    myPart->setEnableMask(visibleBit);
    EXPECT_NE(0u, myPart->enableMask() & visibleBit);
    EXPECT_EQ(0u, myPart->enableMask() & selectedBit);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, Priority)
{
    Part myPart;
    EXPECT_EQ(0, myPart.priority());

    myPart.setPriority(5);
    EXPECT_EQ(5, myPart.priority());

    myPart.setPriority(-2);
    EXPECT_EQ(-2, myPart.priority());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, TwoPartsSharingGeometry)
{
    ref<Vec3fArray> verts = new Vec3fArray;
    verts->reserve(3);
    verts->add(Vec3f(0, 0, 0));
    verts->add(Vec3f(1, 0, 0));
    verts->add(Vec3f(1, 1, 0));

    ref<Vec3fArray> norms = new Vec3fArray;
    norms->resize(3);
    norms->set(0, Vec3f::Z_AXIS);
    norms->set(1, Vec3f::Z_AXIS);
    norms->set(2, Vec3f::Z_AXIS);

    ASSERT_EQ(1, verts->refCount());
    ASSERT_EQ(1, norms->refCount());

    ref<DrawableGeo> myGeo = new DrawableGeo;
    ASSERT_EQ(1, myGeo->refCount());

    myGeo->setFromTriangleVertexArray(verts.p());
    myGeo->setNormalArray(norms.p());
    ASSERT_EQ(2, verts->refCount());
    ASSERT_EQ(2, norms->refCount());

    {
        Part part1;
        part1.setDrawable(myGeo.p());
        ASSERT_EQ(2, myGeo->refCount());

        Part part2;
        part2.setDrawable(myGeo.p());
        ASSERT_EQ(3, myGeo->refCount());
    }

    ASSERT_EQ(1, myGeo->refCount());

    myGeo = NULL;

    ASSERT_EQ(1, verts->refCount());
    ASSERT_EQ(1, norms->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, SetGetDrawable)
{
    ref<Drawable> drawable1 = new DrawableGeo;
    ref<Drawable> drawable2 = new DrawableGeo;
    ref<Part> myPart = new Part;
    
    myPart->setDrawable(drawable1.p());
    ASSERT_EQ(drawable1.p(), myPart->drawable());
    ASSERT_EQ(2, drawable1->refCount());

    myPart->setDrawable(drawable2.p());
    ASSERT_EQ(1, drawable1->refCount());
    ASSERT_EQ(2, drawable2->refCount());
    ASSERT_EQ(drawable2.p(), myPart->drawable());

    myPart->setDrawable(NULL);
    ASSERT_EQ(1, drawable1->refCount());
    ASSERT_EQ(1, drawable2->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, VerifyEmptyEffectsForAllLods)
{
    ref<Part> myPart = new Part;

    int i;
    for (i = 0; i < Part::MAX_NUM_LOD_LEVELS; i++)
    {
        EXPECT_EQ(NULL, myPart->effect(i));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, SetGetEffectLod0Only)
{
    ref<Part> myPart = new Part;
    EXPECT_EQ(NULL, myPart->effect());
    EXPECT_EQ(NULL, myPart->effect(0));

    ref<Effect> e1 = new Effect;
    EXPECT_EQ(1, e1->refCount());

    myPart->setEffect(e1.p());
    EXPECT_EQ(2, e1->refCount());
    EXPECT_EQ(e1.p(), myPart->effect());
    EXPECT_EQ(e1.p(), myPart->effect(0));

    ref<Effect> e2 = new Effect;
    myPart->setEffect(e2.p());
    EXPECT_EQ(1, e1->refCount());
    EXPECT_EQ(2, e2->refCount());
    EXPECT_EQ(e2.p(), myPart->effect());
    EXPECT_EQ(e2.p(), myPart->effect(0));

    myPart->setEffect(NULL);
    EXPECT_EQ(1, e1->refCount());
    EXPECT_EQ(1, e2->refCount());
    EXPECT_EQ(NULL, myPart->effect());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartTest, SetGetEffectLods)
{
    ref<Effect> e0 = new Effect;
    ref<Effect> e1 = new Effect;

    ref<Part> myPart = new Part;
    myPart->setEffect(0, e0.p());
    myPart->setEffect(1, e1.p());
    EXPECT_EQ(2, e0->refCount());
    EXPECT_EQ(2, e1->refCount());

    EXPECT_EQ(e0.p(), myPart->effect(0));
    EXPECT_EQ(e0.p(), myPart->effect());
    
    EXPECT_EQ(e1, myPart->effect(1));
}



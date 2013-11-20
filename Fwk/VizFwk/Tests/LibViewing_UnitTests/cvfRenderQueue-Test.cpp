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
#include "cvfRenderQueue.h"
#include "cvfRenderQueueBuilder.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfDrawableGeo.h"
#include "cvfEffect.h"
#include "cvfPart.h"
#include "cvfRenderState_FF.h"
#include "cvfCamera.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderQueueTest, BasicConstruction)
{
    ref<RenderQueue> q = new RenderQueue;
    ASSERT_EQ(1, q->refCount());

    ASSERT_EQ(0u, q->count());
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
TEST(RenderQueueTest, SimpleLifeCycle)
{
    ref<RenderQueue> q = new RenderQueue;
    
    ref<Part> p1 = new Part;
    ref<Part> p2 = new Part;
    q->add(p1.p(), NULL, NULL, 0, 0);
    q->add(p2.p(), NULL, NULL, 0, 0);

    ASSERT_EQ(2, q->count());
    EXPECT_EQ(p1.p(), q->item(0)->part());
    ASSERT_EQ(p2.p(), q->item(1)->part());

    q->removeAll();
    ASSERT_EQ(0, q->count());
}


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(RenderQueueDeathTest, AssertOnIllegalIndex)
{
    ref<RenderQueue> q = new RenderQueue;
    EXPECT_DEATH(q->item(0), "Assertion");

    q->add(NULL, NULL, NULL, 0, 0);
    EXPECT_DEATH(q->item(1), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
TEST(RenderQueueBuilderTest, DISABLED_Build)
{
    /*
    ref<DrawableGeo> g1 = new DrawableGeo;
    ref<DrawableGeo> g2 = new DrawableGeo;

    ref<Effect> e1 = new Effect;
    ref<Effect> e2 = new Effect;
    e1->setRenderState(new Material_FF(Color3f(1, 0, 0)));
    e2->setRenderState(new Material_FF(Color3f(0, 1, 0)));

    ref<Part> p1 = new Part;
    p1->setDrawable(g1.p());
    p1->setEffect(e1.p());

    ref<Part> p2 = new Part;
    p2->setDrawable(g2.p());
    p2->setEffect(e2.p());

    ref<PartRenderHintCollection> pc = new PartRenderHintCollection;
    pc->add(p1.p());
    pc->add(p2.p());

    Camera cam;
    RenderQueueBuilder builder(pc.p());
    RenderQueue rq;
    builder.populateRenderQueue(&rq, cam);
    ASSERT_EQ(2u, rq.count());

    RenderItem* r1 = rq.item(0);
    ASSERT_TRUE(r1 != NULL);
    EXPECT_EQ(g1.p(), r1->drawable());
    EXPECT_EQ(e1.p(), r1->effect());

    RenderItem* r2 = rq.item(1);
    ASSERT_TRUE(r2 != NULL);
    EXPECT_EQ(g2.p(), r2->drawable());
    EXPECT_EQ(e2.p(), r2->effect());
    */
}


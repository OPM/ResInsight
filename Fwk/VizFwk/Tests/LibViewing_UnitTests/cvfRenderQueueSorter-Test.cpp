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
#include "cvfRenderQueueSorter.h"
#include "cvfRenderQueue.h"
#include "cvfEffect.h"
#include "cvfPart.h"
#include "cvfRenderState_FF.h"

#include "gtest/gtest.h"
#include <algorithm>

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderQueueSorterBasicTest, StrategyMinimal)
{
    ref<RenderQueueSorterBasic> sorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::MINIMAL);
    ref<RenderQueue> q = new RenderQueue;

    ref<Effect> e = new Effect;
    ref<Part> part1 = new Part;
    ref<Part> part2 = new Part;
    ref<Part> part3 = new Part;

    part1->setPriority(30);
    part2->setPriority(20);
    part3->setPriority(10);

    q->add(part1.p(), NULL, e.p(), 0, 0);
    q->add(part2.p(), NULL, e.p(), 0, 0);
    q->add(part3.p(), NULL, e.p(), 0, 0);

    sorter->sort(q.p());

    ASSERT_EQ(3, q->count());
    EXPECT_EQ(part3.p(), q->item(0)->part());
    EXPECT_EQ(part2.p(), q->item(1)->part());
    EXPECT_EQ(part1.p(), q->item(2)->part());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderQueueSorterBasicTest, StrategyEffectOnlyNoPriorities)
{
    ref<RenderQueueSorterBasic> sorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::EFFECT_ONLY);
    ref<RenderQueue> q = new RenderQueue;

    ref<Effect> e1 = new Effect;
    ref<Effect> e2 = new Effect;
    e1->setRenderState(new RenderStateMaterial_FF(Color3f(1,0,1)));
    e2->setRenderState(new RenderStateMaterial_FF(Color3f(0,1,0)));

    if (e1.p() > e2.p()) e1.swap(e2);
    EXPECT_LT(e1.p(), e2.p());

    ref<Part> dummyPart = new Part;
    q->add(dummyPart.p(), NULL, e2.p(), 0, 0);
    q->add(dummyPart.p(), NULL, e1.p(), 0, 0);
    q->add(dummyPart.p(), NULL, e2.p(), 0, 0);
    q->add(dummyPart.p(), NULL, e1.p(), 0, 0);

    sorter->sort(q.p());
    ASSERT_EQ(4, q->count());
    EXPECT_EQ(e1.p(), q->item(0)->effect());
    EXPECT_EQ(e1.p(), q->item(1)->effect());
    EXPECT_EQ(e2.p(), q->item(2)->effect());
    EXPECT_EQ(e2.p(), q->item(3)->effect());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderQueueSorterBasicTest, StrategyEffectOnlyWithPriorities)
{
    ref<RenderQueueSorterBasic> sorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::EFFECT_ONLY);
    ref<RenderQueue> q = new RenderQueue;

    ref<Effect> e1 = new Effect;
    ref<Effect> e2 = new Effect;
    e1->setRenderState(new RenderStateMaterial_FF(Color3f(1,0,1)));
    e2->setRenderState(new RenderStateMaterial_FF(Color3f(0,1,0)));

    if (e1.p() > e2.p()) e1.swap(e2);
    EXPECT_LT(e1.p(), e2.p());

    ref<Part> part1 = new Part;
    ref<Part> part2 = new Part;
    ref<Part> part3 = new Part;
    ref<Part> part4 = new Part;
    q->add(part1.p(), NULL, e2.p(), 0, 0);
    q->add(part2.p(), NULL, e1.p(), 0, 0);
    q->add(part3.p(), NULL, e2.p(), 0, 0);
    q->add(part4.p(), NULL, e1.p(), 0, 0);

    part1->setPriority(40);
    part2->setPriority(30);
    part3->setPriority(20);
    part4->setPriority(10);

    sorter->sort(q.p());
    EXPECT_EQ(part4.p(), q->item(0)->part());
    EXPECT_EQ(part3.p(), q->item(1)->part());
    EXPECT_EQ(part2.p(), q->item(2)->part());
    EXPECT_EQ(part1.p(), q->item(3)->part());

    // Check that effects get sorted correct within priorities
    part1->setPriority(10);
    part2->setPriority(10);
    part3->setPriority(20);
    part4->setPriority(20);

    sorter->sort(q.p());
    EXPECT_EQ(part2.p(), q->item(0)->part());
    EXPECT_EQ(part1.p(), q->item(1)->part());
    EXPECT_EQ(part4.p(), q->item(2)->part());
    EXPECT_EQ(part3.p(), q->item(3)->part());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderQueueSorterBasicTest, StrategyStandardAllPartsDifferentPriority)
{
    ref<RenderQueueSorterBasic> sorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::EFFECT_ONLY);
    ref<RenderQueue> q = new RenderQueue;

    ref<Effect> e1 = new Effect;
    ref<Effect> e2 = new Effect;
    e1->setRenderState(new RenderStateMaterial_FF(Color3f(1,0,1)));
    e2->setRenderState(new RenderStateMaterial_FF(Color3f(0,1,0)));

    if (e1.p() > e2.p()) e1.swap(e2);
    EXPECT_LT(e1.p(), e2.p());

    ref<Part> part1 = new Part;
    ref<Part> part2 = new Part;
    ref<Part> part3 = new Part;
    ref<Part> part4 = new Part;
    q->add(part1.p(), NULL, e2.p(), 0, 0);
    q->add(part2.p(), NULL, e1.p(), 0, 0);
    q->add(part3.p(), NULL, e2.p(), 0, 0);
    q->add(part4.p(), NULL, e1.p(), 0, 0);

    part1->setPriority(40);
    part2->setPriority(30);
    part3->setPriority(20);
    part4->setPriority(10);

    sorter->sort(q.p());

    sorter->sort(q.p());
    EXPECT_EQ(part4.p(), q->item(0)->part());
    EXPECT_EQ(part3.p(), q->item(1)->part());
    EXPECT_EQ(part2.p(), q->item(2)->part());
    EXPECT_EQ(part1.p(), q->item(3)->part());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderQueueSorterBasicTest, StrategyStandard)
{
    // TODO Must be replaced by a proper test!
    ref<RenderQueueSorterBasic> sorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::STANDARD);
    ref<RenderQueue> q = new RenderQueue;

    ref<Effect> e1 = new Effect;
    ref<Effect> e2 = new Effect;
    e1->setRenderState(new RenderStateMaterial_FF(Color3f(1,0,1)));
    e2->setRenderState(new RenderStateMaterial_FF(Color3f(0,1,0)));

    ref<Part> dummyPart = new Part;

    q->add(dummyPart.p(), NULL, e1.p(), 0, 0);
    q->add(dummyPart.p(), NULL, e2.p(), 0, 0);
    q->add(dummyPart.p(), NULL, e1.p(), 0, 0);

    sorter->sort(q.p());


    size_t numEntries = q->count();
    EXPECT_EQ(3, numEntries);

    std::vector<const Effect*> effectsFound;

    size_t i;
    for (i = 0; i < numEntries; i++)
    {
        RenderItem* ri = q->item(i);
        const Effect* e = ri->effect();

        bool foundIt = (std::find(effectsFound.begin(), effectsFound.end(), e) != effectsFound.end());

        if (!foundIt)
        {
            // Not found, add it
            effectsFound.push_back(e);
        }
        else 
        {
            // It is the last item, we're OK
            if (e != effectsFound.back())
            {
                FAIL() << "Unexpected sort order.";
            }
        }
    }
}

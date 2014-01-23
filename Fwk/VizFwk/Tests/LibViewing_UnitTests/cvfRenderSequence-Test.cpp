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
#include "cvfRenderSequence.h"
#include "cvfRendering.h"
#include "cvfModelBasicList.h"
#include "cvfDrawableGeo.h"
#include "cvfCamera.h"
#include "cvfPartRenderHintCollection.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderSequenceTest, BasicConstruction)
{
    RenderSequence seq;
    ASSERT_EQ(0, seq.refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderSequenceTest, TestSimpleLifeCycle)
{
    ref<RenderSequence> mySeq = new RenderSequence;
    ASSERT_EQ(0, mySeq->renderingCount());

    ref<Rendering> r1 = new Rendering;
    ref<Rendering> r2 = new Rendering;
    ref<Rendering> r3 = new Rendering;
    mySeq->addRendering(r1.p());
    mySeq->addRendering(r2.p());
    mySeq->addRendering(r3.p());
    ASSERT_EQ(3, mySeq->renderingCount());
    EXPECT_EQ(2, r1->refCount());
    EXPECT_EQ(2, r2->refCount());
    EXPECT_EQ(2, r3->refCount());

    EXPECT_EQ(r1.p(), mySeq->rendering(0));
    EXPECT_EQ(r2.p(), mySeq->rendering(1));
    EXPECT_EQ(r3.p(), mySeq->rendering(2));

    mySeq->removeRendering(r2.p());
    EXPECT_EQ(2, mySeq->renderingCount());
    EXPECT_EQ(2, r1->refCount());
    EXPECT_EQ(1, r2->refCount());
    EXPECT_EQ(2, r3->refCount());

    mySeq->removeAllRenderings();
    EXPECT_EQ(0, mySeq->renderingCount());
    EXPECT_EQ(1, r1->refCount());
    EXPECT_EQ(1, r2->refCount());
    EXPECT_EQ(1, r3->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderSequenceTest, GetFirstRendering)
{
    ref<RenderSequence> mySeq = new RenderSequence;
    EXPECT_EQ(0, mySeq->renderingCount());
    EXPECT_EQ(NULL, mySeq->firstRendering());

    ref<Rendering> r1 = new Rendering;
    mySeq->addRendering(r1.p());
    EXPECT_EQ(r1.p(), mySeq->rendering(0));
    EXPECT_EQ(r1.p(), mySeq->firstRendering());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RenderSequenceTest, InsertRendering)
{
    ref<RenderSequence> mySeq = new RenderSequence;

    // Insert in empty sequence
    {
        ref<Rendering> r = new Rendering;
        mySeq->insertRendering(NULL, r.p());

        ASSERT_EQ(1, mySeq->renderingCount());
        EXPECT_EQ(r.p(),  mySeq->rendering(0));
        mySeq->removeRendering(r.p());
    }

    ref<Rendering> r1 = new Rendering;
    mySeq->addRendering(r1.p());
    ref<Rendering> r2 = new Rendering;
    mySeq->addRendering(r2.p());
    ASSERT_EQ(2, mySeq->renderingCount());
    EXPECT_EQ(r1.p(), mySeq->rendering(0));
    EXPECT_EQ(r2.p(), mySeq->rendering(1));

    // Insert with NULL ptr - should end up last
    {
        ref<Rendering> r = new Rendering;
        mySeq->insertRendering(NULL, r.p());

        ASSERT_EQ(3, mySeq->renderingCount());
        EXPECT_EQ(r1.p(), mySeq->rendering(0));
        EXPECT_EQ(r2.p(), mySeq->rendering(1));
        EXPECT_EQ(r.p(),  mySeq->rendering(2));
        mySeq->removeRendering(r.p());
    }

    // Insert before a rendering that is not in sequence - should end up last
    {
        ref<Rendering> renderingNotInSeq = new Rendering;

        ref<Rendering> r = new Rendering;
        mySeq->insertRendering(renderingNotInSeq.p(), r.p());

        ASSERT_EQ(3, mySeq->renderingCount());
        EXPECT_EQ(r1.p(), mySeq->rendering(0));
        EXPECT_EQ(r2.p(), mySeq->rendering(1));
        EXPECT_EQ(r.p(),  mySeq->rendering(2));
        mySeq->removeRendering(r.p());
    }

    {
        ref<Rendering> r = new Rendering;
        mySeq->insertRendering(r1.p(), r.p());

        ASSERT_EQ(3, mySeq->renderingCount());
        EXPECT_EQ(r.p(),  mySeq->rendering(0));
        EXPECT_EQ(r1.p(), mySeq->rendering(1));
        EXPECT_EQ(r2.p(), mySeq->rendering(2));
        mySeq->removeRendering(r.p());
    }

    {
        ref<Rendering> r = new Rendering;
        mySeq->insertRendering(r2.p(), r.p());

        ASSERT_EQ(3, mySeq->renderingCount());
        EXPECT_EQ(r1.p(), mySeq->rendering(0));
        EXPECT_EQ(r.p(),  mySeq->rendering(1));
        EXPECT_EQ(r2.p(), mySeq->rendering(2));
        mySeq->removeRendering(r.p());
    }
}


//------------------------------------------------------------------------------------------------
/// 
//------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(RenderSequenceDeathTest, IllegalRenderingIndex)
{
    ref<RenderSequence> mySeq = new RenderSequence;
    EXPECT_DEATH(mySeq->rendering(0), "Assertion");

    ref<Rendering> r = new Rendering;
    mySeq->addRendering(r.p());
    EXPECT_DEATH(mySeq->rendering(1), "Assertion");
}
#endif


//------------------------------------------------------------------------------------------------
/// 
//------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(RenderSequenceDeathTest, IllegalAdd)
{
    ref<RenderSequence> mySeq = new RenderSequence;
    EXPECT_DEATH(mySeq->addRendering(NULL), "Assertion");
}
#endif


//------------------------------------------------------------------------------------------------
/// 
//------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(RenderSequenceDeathTest, IllegalRemoveRendering)
{
    ref<RenderSequence> mySeq = new RenderSequence;
    EXPECT_DEATH(mySeq->removeRendering(NULL), "Assertion");
}
#endif


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
#include "cvfOpenGLContextGroup.h"
#include "cvfOpenGLContext.h"

#include "gtest/gtest.h"

using namespace cvf;

class MockContext : public OpenGLContext
{
public:
    MockContext(OpenGLContextGroup* contextGroup) : OpenGLContext(contextGroup) {}
    void    makeCurrent() {}
    bool    isCurrent() const {return true; }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OpenGLContextGroupTest, Defaults)
{
    OpenGLContextGroup grp;
    EXPECT_TRUE(grp.resourceManager() != NULL);
    EXPECT_TRUE(grp.logger() != NULL);

    EXPECT_FALSE(grp.isContextGroupInitialized());
    
    EXPECT_TRUE(grp.capabilities() != NULL);

    // Cannot do this - causes assert
    //EXPECT_TRUE(grp.glewContextStruct() != NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OpenGLContextGroupTest, LifeCycle)
{
    ref<OpenGLContextGroup> grp = new OpenGLContextGroup;

    ref<OpenGLContext> ctx1 = new MockContext(grp.p());
    EXPECT_TRUE(grp->containsContext(ctx1.p()));
    EXPECT_EQ(grp.p(), ctx1->group());
    EXPECT_EQ(2, ctx1->refCount());

    ref<OpenGLContext> ctx2 = new MockContext(grp.p());
    ref<OpenGLContext> ctx3 = new MockContext(grp.p());
    EXPECT_TRUE(grp->containsContext(ctx2.p()));
    EXPECT_TRUE(grp->containsContext(ctx3.p()));
    EXPECT_EQ(grp.p(), ctx2->group());
    EXPECT_EQ(grp.p(), ctx3->group());
    EXPECT_EQ(2, ctx2->refCount());
    EXPECT_EQ(2, ctx3->refCount());

    ctx1->shutdownContext();
    EXPECT_EQ(1, ctx1->refCount());
    EXPECT_TRUE(ctx1->group() == NULL);
    EXPECT_FALSE(grp->containsContext(ctx1.p()));

    ctx2 = NULL;

    grp = NULL;
    EXPECT_EQ(1, ctx3->refCount());
    EXPECT_TRUE(ctx3->group() == NULL);
}



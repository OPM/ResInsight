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
#include "cvfViewport.h"
#include "cvfMath.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ViewportTest, DefaultValues)
{
    Viewport vp;

    EXPECT_EQ(0, vp.x());
    EXPECT_EQ(0, vp.y());
    EXPECT_EQ(0, vp.width());
    EXPECT_EQ(0, vp.height());
    EXPECT_EQ(1.0, vp.aspectRatio());

    EXPECT_EQ(0.69f, vp.clearColor().r());
    EXPECT_EQ(0.77f, vp.clearColor().g());
    EXPECT_EQ(0.87f, vp.clearColor().b());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ViewportTest, SettersAndGetters)
{
    Viewport vp;

    vp.set(1,2,3,4);
    EXPECT_EQ(1, vp.x());
    EXPECT_EQ(2, vp.y());
    EXPECT_EQ(3, vp.width());
    EXPECT_EQ(4, vp.height());
    EXPECT_DOUBLE_EQ(3.0/4.0, vp.aspectRatio());

    vp.setClearColor(Color4f(1,0,1,0));
    EXPECT_TRUE(vp.clearColor() == Color4f(1,0,1,0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ViewportTest, ZeroSizeViewport)
{
    Viewport vp;

    vp.set(0, 0, 0, 0);
    EXPECT_EQ(0, vp.width());
    EXPECT_EQ(0, vp.height());
    EXPECT_EQ(1.0, vp.aspectRatio());
}

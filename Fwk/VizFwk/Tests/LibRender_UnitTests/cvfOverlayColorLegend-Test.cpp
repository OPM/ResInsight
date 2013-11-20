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
#include "cvfOverlayColorLegend.h"
#include "cvfFixedAtlasFont.h"

#include "gtest/gtest.h"

using namespace cvf;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayColorLegendTest, Defaults)
{
    ref<Font> myFont = new FixedAtlasFont(FixedAtlasFont::STANDARD);
    OverlayColorLegend cl(myFont.p());

    EXPECT_EQ(200, cl.sizeHint().x());
    EXPECT_EQ(200, cl.sizeHint().y());

    EXPECT_STREQ("", cl.title().toAscii().ptr());
    EXPECT_TRUE(Color3f(Color3::BLACK) == cl.color());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayColorLegendTest, SizeHint)
{
    ref<Font> myFont = new FixedAtlasFont(FixedAtlasFont::STANDARD);
    OverlayColorLegend cl(myFont.p());
    cl.setSizeHint(Vec2ui(500,100));

    EXPECT_EQ(500, cl.sizeHint().x());
    EXPECT_EQ(100, cl.sizeHint().y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayColorLegendTest, GettersAndSetters)
{
    ref<Font> myFont = new FixedAtlasFont(FixedAtlasFont::STANDARD);
    OverlayColorLegend cl(myFont.p());

    cl.setColor(Color3::BROWN);
    EXPECT_TRUE(Color3f(Color3::BROWN)   == cl.color());

    cl.setTitle("Testing");
    EXPECT_STREQ("Testing", cl.title().toAscii().ptr());

    cl.setTitle("Testing\nSecondLine\nThird");
    EXPECT_STREQ("Testing\nSecondLine\nThird", cl.title().toAscii().ptr());
}



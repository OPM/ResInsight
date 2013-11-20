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
#include "cvfOverlayTextBox.h"
#include "cvfFixedAtlasFont.h"

#include "gtest/gtest.h"

using namespace cvf;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayTextBoxTest, Defaults)
{
    ref<Font> myFont = new FixedAtlasFont(FixedAtlasFont::STANDARD);
    OverlayTextBox tb(myFont.p());

    EXPECT_EQ(200, tb.sizeHint().x());
    EXPECT_EQ( 50, tb.sizeHint().y());

    EXPECT_STREQ("", tb.text().toAscii().ptr());
    EXPECT_TRUE(Color3f(Color3::WHITE) == tb.textColor());
    EXPECT_TRUE(Color3f(0.2f, 0.2f, 1.0f) == tb.backgroundColor());
    EXPECT_TRUE(Color3f(0.6f, 0.6f, 1.0f) == tb.borderColor());
    EXPECT_TRUE(tb.drawBackground());
    EXPECT_TRUE(tb.drawBorder());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayTextBoxTest, SizeHint)
{
    ref<Font> myFont = new FixedAtlasFont(FixedAtlasFont::STANDARD);
    OverlayTextBox tb(myFont.p());
    tb.setPixelSize(Vec2ui(500,100));

    EXPECT_EQ(500, tb.sizeHint().x());
    EXPECT_EQ(100, tb.sizeHint().y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayTextBoxTest, GettersAndSetters)
{
    ref<Font> myFont = new FixedAtlasFont(FixedAtlasFont::STANDARD);
    OverlayTextBox tb(myFont.p());

    tb.setText("Testing");
    tb.setTextColor(Color3::BROWN);
    tb.setBackgroundColor(Color3::MAGENTA);
    tb.setBorderColor(Color3::PURPLE);
    tb.setDrawBackground(true);
    tb.setDrawBorder(false);
    
    EXPECT_STREQ("Testing", tb.text().toAscii().ptr());
    EXPECT_TRUE(Color3f(Color3::BROWN)   == tb.textColor());
    EXPECT_TRUE(Color3f(Color3::MAGENTA) == tb.backgroundColor());
    EXPECT_TRUE(Color3f(Color3::PURPLE)  == tb.borderColor());
    EXPECT_TRUE(tb.drawBackground());
    EXPECT_FALSE(tb.drawBorder());

    tb.setDrawBackground(false);
    tb.setDrawBorder(true);

    EXPECT_FALSE(tb.drawBackground());
    EXPECT_TRUE(tb.drawBorder());
}



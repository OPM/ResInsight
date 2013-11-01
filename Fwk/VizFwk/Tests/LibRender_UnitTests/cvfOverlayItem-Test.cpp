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
#include "cvfOverlayItem.h"

#include "gtest/gtest.h"

using namespace cvf;


class MyItem : public OverlayItem
{
public:
    MyItem()                                                                    { m_size.set(0, 0); }
    void            setSize(Vec2ui size)                                        { m_size = size; }
    virtual Vec2ui  sizeHint()                                                  { return m_size; }
    virtual void    render(OpenGLContext*, const Vec2i&, const Vec2ui&)         { }
    virtual void    renderSoftware(OpenGLContext*, const Vec2i&, const Vec2ui&) { }

private:
    Vec2ui m_size;
};



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayItem, BasicConstruction)
{
    ref<MyItem> item = new MyItem;

    EXPECT_EQ(Vec2ui(0, 0), item->sizeHint());
    EXPECT_EQ(OverlayItem::HORIZONTAL, item->layoutScheme());
    EXPECT_EQ(OverlayItem::BOTTOM_LEFT, item->anchorCorner());
    EXPECT_EQ(Vec2i(0, 0), item->fixedPosition());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayItem, setLayout)
{
    ref<MyItem> item = new MyItem;

    item->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_RIGHT);
    EXPECT_EQ(OverlayItem::VERTICAL, item->layoutScheme());
    EXPECT_EQ(OverlayItem::TOP_RIGHT, item->anchorCorner());
    EXPECT_EQ(Vec2i(0, 0), item->fixedPosition());

    item->setLayout(OverlayItem::HORIZONTAL, OverlayItem::TOP_LEFT);
    EXPECT_EQ(OverlayItem::HORIZONTAL, item->layoutScheme());
    EXPECT_EQ(OverlayItem::TOP_LEFT, item->anchorCorner());
    EXPECT_EQ(Vec2i(0, 0), item->fixedPosition());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayItem, setLayoutFixedPosition)
{
    ref<MyItem> item = new MyItem;

    item->setLayoutFixedPosition(Vec2i(1, 2));
    EXPECT_EQ(OverlayItem::FIXED_POSITION, item->layoutScheme());
    EXPECT_EQ(OverlayItem::BOTTOM_LEFT, item->anchorCorner());
    EXPECT_EQ(Vec2i(1, 2), item->fixedPosition());

    item->setLayout(OverlayItem::HORIZONTAL, OverlayItem::TOP_LEFT);
    EXPECT_EQ(OverlayItem::HORIZONTAL, item->layoutScheme());
    EXPECT_EQ(OverlayItem::TOP_LEFT, item->anchorCorner());
    EXPECT_EQ(Vec2i(0, 0), item->fixedPosition());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OverlayItem, pick)
{
    ref<MyItem> item = new MyItem;

    EXPECT_TRUE(item->pick(0, 0, Vec2i(0, 0), Vec2ui(1, 1)));
    EXPECT_FALSE(item->pick(0, 0, Vec2i(0, 0), Vec2ui(0, 0)));

    EXPECT_TRUE(item->pick(10, 10, Vec2i(10, 10), Vec2ui(2, 2)));
    EXPECT_TRUE(item->pick(11, 12, Vec2i(10, 10), Vec2ui(2, 2)));
    EXPECT_FALSE(item->pick( 9, 10, Vec2i(10, 10), Vec2ui(2, 2)));
    EXPECT_FALSE(item->pick(11,  9, Vec2i(10, 10), Vec2ui(2, 2)));
    EXPECT_FALSE(item->pick(13, 10, Vec2i(10, 10), Vec2ui(2, 2)));
    EXPECT_FALSE(item->pick(10, 14, Vec2i(10, 10), Vec2ui(2, 2)));

    EXPECT_TRUE(item->pick(10, 10, Vec2i(10, 10), Vec2ui(1, 1)));
    EXPECT_FALSE(item->pick(10, 10, Vec2i(10, 10), Vec2ui(0, 0)));
    EXPECT_FALSE(item->pick(10, 10, Vec2i(10, 10), Vec2ui(1, 0)));
    EXPECT_FALSE(item->pick(10, 10, Vec2i(10, 10), Vec2ui(0, 1)));
}



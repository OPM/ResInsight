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
#include "cvfPrimitiveTests.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PrimitiveTestsTest, intersectLines)
{
    {
        Vec2d p1(0, 0);   Vec2d p2(2, 0);
        Vec2d p3(1, -1);  Vec2d p4(1, 1);
        Vec2d isect(0, 0);
        EXPECT_TRUE(PrimitiveTests::intersectLines(p1, p2, p3, p4, &isect));
        EXPECT_DOUBLE_EQ(1, isect.x());
        EXPECT_DOUBLE_EQ(0, isect.y());
    }

    {
        Vec2d p1(0, 0);   Vec2d p2(2, 0);
        Vec2d p3(1, 2);  Vec2d p4(1, 1);
        Vec2d isect(0, 0);
        EXPECT_TRUE(PrimitiveTests::intersectLines(p1, p2, p3, p4, &isect));
        EXPECT_DOUBLE_EQ(1, isect.x());
        EXPECT_DOUBLE_EQ(0, isect.y());
    }

    // Incident
    {
        Vec2d p1(1, 0);  Vec2d p2(3, 0);
        Vec2d p3(2, 0);  Vec2d p4(4, 0);
        Vec2d isect(0, 0);
        EXPECT_TRUE(PrimitiveTests::intersectLines(p1, p2, p3, p4, &isect));
        EXPECT_DOUBLE_EQ(2, isect.x());
        EXPECT_DOUBLE_EQ(0, isect.y());
    }

    // Parallell
    {
        Vec2d p1(0, 0);  Vec2d p2(2, 0);
        Vec2d p3(0, 2);  Vec2d p4(2, 2);
        Vec2d isect(0, 0);
        EXPECT_FALSE(PrimitiveTests::intersectLines(p1, p2, p3, p4, &isect));
        EXPECT_DOUBLE_EQ(0, isect.x());
        EXPECT_DOUBLE_EQ(0, isect.y());
    }
}


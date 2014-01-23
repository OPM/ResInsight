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
#include "cvfRect.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, Construction)
{
    {
        Rectd rect;
        EXPECT_DOUBLE_EQ(0.0, rect.min().x());
        EXPECT_DOUBLE_EQ(0.0, rect.min().y());
        EXPECT_DOUBLE_EQ(0.0, rect.width());
        EXPECT_DOUBLE_EQ(0.0, rect.height());
 
        EXPECT_FALSE(rect.isValid());
    }

    {
        Rectd rect(-1, -2, 5, 10);
        EXPECT_DOUBLE_EQ(-1, rect.min().x());
        EXPECT_DOUBLE_EQ(-2, rect.min().y());
        EXPECT_DOUBLE_EQ(5, rect.width());
        EXPECT_DOUBLE_EQ(10, rect.height());

        EXPECT_TRUE(rect.isValid());
    }

    {
        Vec2d min(-2, -3);
        Rectd rect(min, 5, 10);
        EXPECT_DOUBLE_EQ(min.x(), rect.min().x());
        EXPECT_DOUBLE_EQ(min.y(), rect.min().y());
        EXPECT_DOUBLE_EQ(5, rect.width());
        EXPECT_DOUBLE_EQ(10, rect.height());

        EXPECT_TRUE(rect.isValid());
    }

    {
        Vec2d min(-2, -3);
        Rectd rect(min, 5, 10);
        EXPECT_DOUBLE_EQ(min.x(), rect.min().x());
        EXPECT_DOUBLE_EQ(min.y(), rect.min().y());
        EXPECT_DOUBLE_EQ(5, rect.width());
        EXPECT_DOUBLE_EQ(10, rect.height());

        EXPECT_TRUE(rect.isValid());

        Rectd otherRect(rect);
        EXPECT_DOUBLE_EQ(rect.min().x(),    otherRect.min().x());
        EXPECT_DOUBLE_EQ(rect.min().y(),    otherRect.min().y());
        EXPECT_DOUBLE_EQ(rect.width(),      otherRect.width());
        EXPECT_DOUBLE_EQ(rect.height(),     otherRect.height());

        EXPECT_TRUE(otherRect.isValid());
    }

    {
        Vec2d min(-2, -3);
        Vec2d max(5, 6);
        Rectd rect = Rectd::fromMinMax(min, max);

        EXPECT_TRUE(rect.isValid());
        EXPECT_DOUBLE_EQ(min.x(), rect.min().x());
        EXPECT_DOUBLE_EQ(min.y(), rect.min().y());
        EXPECT_DOUBLE_EQ(7, rect.width());
        EXPECT_DOUBLE_EQ(9, rect.height());

    }

    {
        Vec2d min(-2, -3);
        Rectd rectOriginal(min, 5, 10);
        Rectd rect;
        EXPECT_FALSE(rect.isValid());
        
        rect = rectOriginal;

        EXPECT_TRUE(rect.isValid());
        EXPECT_DOUBLE_EQ(-2, rect.min().x());
        EXPECT_DOUBLE_EQ(-3, rect.min().y());
        EXPECT_DOUBLE_EQ(3, rect.max().x());
        EXPECT_DOUBLE_EQ(7, rect.max().y());

    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, RectTypes)
{
    // Rectf
    {
        Rectf rect;
        EXPECT_FLOAT_EQ(0.0f, rect.min().x());
        EXPECT_FLOAT_EQ(0.0f, rect.min().y());
        EXPECT_FLOAT_EQ(0.0f, rect.width());
        EXPECT_FLOAT_EQ(0.0f, rect.height());

        EXPECT_FALSE(rect.isValid());
    }

    {
        Rectf rect(-1.0f, -2.0f, 5.0f, 10.0f);
        EXPECT_FLOAT_EQ(-1.0f, rect.min().x());
        EXPECT_FLOAT_EQ(-2.0f, rect.min().y());
        EXPECT_FLOAT_EQ(5.0f, rect.width());
        EXPECT_FLOAT_EQ(10.0f, rect.height());

        EXPECT_TRUE(rect.isValid());
    }

    // Recti
    {
        Recti rect;
        EXPECT_EQ(0, rect.min().x());
        EXPECT_EQ(0, rect.min().y());
        EXPECT_EQ(0, rect.width());
        EXPECT_EQ(0, rect.height());

        EXPECT_FALSE(rect.isValid());
    }

    {
        Recti rect(-1, -2, 5, 10);
        EXPECT_EQ(-1, rect.min().x());
        EXPECT_EQ(-2, rect.min().y());
        EXPECT_EQ(5, rect.width());
        EXPECT_EQ(10, rect.height());

        EXPECT_TRUE(rect.isValid());
    }

    // Rectui
    {
        Rectui rect;
        EXPECT_EQ(0, rect.min().x());
        EXPECT_EQ(0, rect.min().y());
        EXPECT_EQ(0, rect.width());
        EXPECT_EQ(0, rect.height());

        EXPECT_FALSE(rect.isValid());
    }

    {
        Rectui rect(1, 2, 5, 10);
        EXPECT_EQ(1, rect.min().x());
        EXPECT_EQ(2, rect.min().y());
        EXPECT_EQ(5, rect.width());
        EXPECT_EQ(10, rect.height());

        EXPECT_TRUE(rect.isValid());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, CenterAndSetters)
{
    Rectd rect(0, 0, 5, 10);
    EXPECT_DOUBLE_EQ(2.5, rect.center().x());
    EXPECT_DOUBLE_EQ(5, rect.center().y());

    Vec2d otherMin(-2, -4);
    rect.setMin(otherMin);
    EXPECT_DOUBLE_EQ(0.5, rect.center().x());
    EXPECT_DOUBLE_EQ(1, rect.center().y());

    rect.setWidth(20);
    EXPECT_DOUBLE_EQ(8, rect.center().x());
    EXPECT_DOUBLE_EQ(1, rect.center().y());

    rect.setHeight(30);
    EXPECT_DOUBLE_EQ(8, rect.center().x());
    EXPECT_DOUBLE_EQ(11, rect.center().y());

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, Normalize)
{
    {
        Rectd r(0, 0, -5, 10);
        EXPECT_FALSE(r.isValid());
        EXPECT_DOUBLE_EQ(-5, r.width());

        r.normalize();
        EXPECT_TRUE(r.isValid());
        EXPECT_DOUBLE_EQ(-5, r.min().x());
        EXPECT_DOUBLE_EQ(5, r.width());
    }

    {
        Recti r(0, 0, -5, 10);
        EXPECT_FALSE(r.isValid());
        EXPECT_DOUBLE_EQ(-5, r.width());

        r.normalize();
        EXPECT_TRUE(r.isValid());
        EXPECT_DOUBLE_EQ(-5, r.min().x());
        EXPECT_DOUBLE_EQ(5, r.width());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, Include)
{
    {
        Rectd rect;
        Vec2d pt1(10, 20);
        rect.include(pt1);

        EXPECT_DOUBLE_EQ(0.0, rect.min().x());
        EXPECT_DOUBLE_EQ(0.0, rect.min().y());
        EXPECT_DOUBLE_EQ(10, rect.width());
        EXPECT_DOUBLE_EQ(20, rect.height());

        Vec2d pt2(20, 30);
        rect.include(pt2);

        EXPECT_DOUBLE_EQ(0, rect.min().x());
        EXPECT_DOUBLE_EQ(0, rect.min().y());
        EXPECT_DOUBLE_EQ(20, rect.width());
        EXPECT_DOUBLE_EQ(30, rect.height());
    }

    {
        Rectd rect;
        Vec2d pt1(10, 20);
        rect.setMin(pt1);

        EXPECT_DOUBLE_EQ(10, rect.min().x());
        EXPECT_DOUBLE_EQ(20, rect.min().y());
        EXPECT_DOUBLE_EQ(0, rect.width());
        EXPECT_DOUBLE_EQ(0, rect.height());

        Vec2d pt2(20, 30);
        rect.include(pt2);

        EXPECT_DOUBLE_EQ(10, rect.min().x());
        EXPECT_DOUBLE_EQ(20, rect.min().y());
        EXPECT_DOUBLE_EQ(10, rect.width());
        EXPECT_DOUBLE_EQ(10, rect.height());

        Vec2d pt3(-20, -30);
        rect.include(pt3);

        EXPECT_DOUBLE_EQ(-20, rect.min().x());
        EXPECT_DOUBLE_EQ(-30, rect.min().y());
        EXPECT_DOUBLE_EQ(40, rect.width());
        EXPECT_DOUBLE_EQ(60, rect.height());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, IncludeRect)
{
    Rectd rect;
    {
        Rectd otherRect(10, 20, 5, 15);
        rect.include(otherRect);

        EXPECT_DOUBLE_EQ(0.0, rect.min().x());
        EXPECT_DOUBLE_EQ(0.0, rect.min().y());
        EXPECT_DOUBLE_EQ(15, rect.width());
        EXPECT_DOUBLE_EQ(35, rect.height());
    }

    {
        Rectd otherRect(-10, -20, 5, 5);
        rect.include(otherRect);

        EXPECT_DOUBLE_EQ(-10, rect.min().x());
        EXPECT_DOUBLE_EQ(-20, rect.min().y());
        EXPECT_DOUBLE_EQ(25, rect.width());
        EXPECT_DOUBLE_EQ(55, rect.height());
    }

    {
        Rectd otherRect(-100, -200, 200, 300);
        rect.include(otherRect);

        EXPECT_DOUBLE_EQ(-100, rect.min().x());
        EXPECT_DOUBLE_EQ(-200, rect.min().y());
        EXPECT_DOUBLE_EQ(200, rect.width());
        EXPECT_DOUBLE_EQ(300, rect.height());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, Contains_Rectd)
{
    Rectd rect;

    {
        const Vec2d pt(10, 20);
        EXPECT_FALSE(rect.contains(pt));
     
        rect.setWidth(10);
        EXPECT_FALSE(rect.contains(pt));

        rect.setHeight(10);
        EXPECT_FALSE(rect.contains(pt));

        rect.setHeight(20);
        EXPECT_TRUE(rect.contains(pt));
    }

    {
        const Vec2d pt(0, 0);
        EXPECT_TRUE(rect.contains(pt));
    }

    {
        const Vec2d pt(-0.5, 0);
        EXPECT_FALSE(rect.contains(pt));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, Contains_Rectui)
{
    // minx, miny, width, height
    Rectui rect(10, 20, 5, 10);

    // Points inside
    EXPECT_TRUE(rect.contains(Vec2ui(11, 21)));
    EXPECT_TRUE(rect.contains(Vec2ui(14, 29)));

    // Points on the edge
    EXPECT_TRUE(rect.contains(Vec2ui(10, 20)));
    EXPECT_TRUE(rect.contains(Vec2ui(10, 21)));
    EXPECT_TRUE(rect.contains(Vec2ui(15, 30)));
    EXPECT_TRUE(rect.contains(Vec2ui(15, 29)));

    // Points outside
    EXPECT_FALSE(rect.contains(Vec2ui(0, 0)));
    EXPECT_FALSE(rect.contains(Vec2ui(26, 31)));
    EXPECT_FALSE(rect.contains(Vec2ui(9, 21)));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, Intersects)
{
    {
        Rectd rect;
        Rectd otherRect(10, 20, 5, 15);

        EXPECT_FALSE(rect.intersects(otherRect));
    }

    {
        Rectd rect(5, 0, 10, 20);
        Rectd otherRect(0, 2, 10, 2);

        EXPECT_TRUE(rect.intersects(otherRect));
    }

    {
        Rectd rect;

        Rectd otherRect(10, 20, 5, 15);
        EXPECT_FALSE(rect.intersects(otherRect));

        rect.setWidth(5);
        EXPECT_FALSE(rect.intersects(otherRect));

        rect.setHeight(5);
        EXPECT_FALSE(rect.intersects(otherRect));

        rect.setWidth(10);
        rect.setHeight(20);
        EXPECT_FALSE(rect.intersects(otherRect));

        rect.setWidth(10.1);
        rect.setHeight(20.1);
        EXPECT_TRUE(rect.intersects(otherRect));
    }

    {
        Rectd rect(10, 20, 5, 15);
        Rectd otherRect(0, 0, 11, 21);

        EXPECT_TRUE(rect.intersects(otherRect));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, Translate)
{
    {
        Rectd rect(0, 5, 2, 2);

        rect.translate(Vec2d(10, 10));
        EXPECT_DOUBLE_EQ(10, rect.min().x());
        EXPECT_DOUBLE_EQ(15, rect.min().y());

        rect.translate(Vec2d(-20, -20));
        EXPECT_DOUBLE_EQ(-10, rect.min().x());
        EXPECT_DOUBLE_EQ(-5, rect.min().y());
   }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, FromMinMax)
{
    {
        Rectd rect = Rectd::fromMinMax(Vec2d(-1, 2), Vec2d(10, 12));
        EXPECT_TRUE(rect.isValid());
        EXPECT_DOUBLE_EQ(-1, rect.min().x());
        EXPECT_DOUBLE_EQ(2,  rect.min().y());
        EXPECT_DOUBLE_EQ(10, rect.max().x());
        EXPECT_DOUBLE_EQ(12, rect.max().y());
        EXPECT_DOUBLE_EQ(11, rect.width());
        EXPECT_DOUBLE_EQ(10, rect.height());
    }

    {
        Recti rect = Recti::fromMinMax(Vec2i(-1, 2), Vec2i(10, 12));
        EXPECT_TRUE(rect.isValid());
        EXPECT_EQ(-1, rect.min().x());
        EXPECT_EQ( 2, rect.min().y());
        EXPECT_EQ(10, rect.max().x());
        EXPECT_EQ(12, rect.max().y());
        EXPECT_EQ(11, rect.width());
        EXPECT_EQ(10, rect.height());
    }

    {
        Rectui rect = Rectui::fromMinMax(Vec2ui(1, 2), Vec2ui(11, 22));
        EXPECT_TRUE(rect.isValid());
        EXPECT_EQ( 1, rect.min().x());
        EXPECT_EQ( 2, rect.min().y());
        EXPECT_EQ(11, rect.max().x());
        EXPECT_EQ(22, rect.max().y());
        EXPECT_EQ(10, rect.width());
        EXPECT_EQ(20, rect.height());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RectTest, LineSegmentIntersect)
{
    // p1 outside, p2 inside
    {
        Rectd rect = Rectd::fromMinMax(Vec2d(5, 7), Vec2d(10, 20));

        Vec2d p1(3, 15);
        Vec2d p2(13, 15);
        Vec2d intersect1;
        Vec2d intersect2;
        EXPECT_TRUE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_DOUBLE_EQ(5, intersect1.x());
        EXPECT_DOUBLE_EQ(15, intersect1.y());
        EXPECT_DOUBLE_EQ(10, intersect2.x());
        EXPECT_DOUBLE_EQ(15, intersect2.y());
    }

    // p1 inside, p2 outside
    {
        Rectd rect = Rectd::fromMinMax(Vec2d(5, 7), Vec2d(10, 20));

        Vec2d p1(7, 15);
        Vec2d p2(13, 15);
        Vec2d intersect1;
        Vec2d intersect2;
        EXPECT_TRUE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_DOUBLE_EQ(7, intersect1.x());
        EXPECT_DOUBLE_EQ(15, intersect1.y());
        EXPECT_DOUBLE_EQ(10, intersect2.x());
        EXPECT_DOUBLE_EQ(15, intersect2.y());
    }

    // p1 inside, p2 inside
    {
        Rectd rect = Rectd::fromMinMax(Vec2d(5, 7), Vec2d(10, 20));

        Vec2d p1(7, 15);
        Vec2d p2(8, 15);
        Vec2d intersect1;
        Vec2d intersect2;
        EXPECT_TRUE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_DOUBLE_EQ(7, intersect1.x());
        EXPECT_DOUBLE_EQ(15, intersect1.y());
        EXPECT_DOUBLE_EQ(8, intersect2.x());
        EXPECT_DOUBLE_EQ(15, intersect2.y());
    }

    // p1 inside, p2 outside
    {
        Rectd rect = Rectd::fromMinMax(Vec2d(5, 7), Vec2d(10, 20));

        Vec2d p1(7, 15);
        Vec2d p2(13, 15);
        Vec2d intersect1;
        Vec2d intersect2;
        EXPECT_TRUE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_DOUBLE_EQ(7, intersect1.x());
        EXPECT_DOUBLE_EQ(15, intersect1.y());
        EXPECT_DOUBLE_EQ(10, intersect2.x());
        EXPECT_DOUBLE_EQ(15, intersect2.y());
    }

    // p1 inside, p2 outside
    {
        Rectd rect = Rectd::fromMinMax(Vec2d(5, 7), Vec2d(10, 20));

        Vec2d p1(7, 15);
        Vec2d p2(7, 25);
        Vec2d intersect1;
        Vec2d intersect2;
        EXPECT_TRUE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_DOUBLE_EQ(7, intersect1.x());
        EXPECT_DOUBLE_EQ(15, intersect1.y());
        EXPECT_DOUBLE_EQ(7, intersect2.x());
        EXPECT_DOUBLE_EQ(20, intersect2.y());
    }

    {
        Rectd rect = Rectd::fromMinMax(Vec2d(5, 7), Vec2d(10, 20));

        Vec2d p1(3, 15);
        Vec2d p2(10, 25);
        Vec2d intersect1;
        Vec2d intersect2;
        EXPECT_TRUE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_DOUBLE_EQ(5, intersect1.x());
        EXPECT_NEAR(17.85, intersect1.y(), 0.1);
        EXPECT_DOUBLE_EQ(6.5, intersect2.x());
        EXPECT_DOUBLE_EQ(20, intersect2.y());
    }


    // No intersection
    {
        Rectd rect = Rectd::fromMinMax(Vec2d(5, 7), Vec2d(10, 20));

        Vec2d p1(3, 15);
        Vec2d p2(4, 15);
        Vec2d intersect1 = Vec2d::UNDEFINED;
        Vec2d intersect2 = Vec2d::UNDEFINED;
        EXPECT_FALSE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_TRUE(intersect1.isUndefined());
        EXPECT_TRUE(intersect2.isUndefined());
    }

    // Invalid rect
    {
        Rectd rect(Vec2d(5, 7), -10, -20);

        Vec2d p1(3, 15);
        Vec2d p2(4, 15);
        Vec2d intersect1 = Vec2d::UNDEFINED;
        Vec2d intersect2 = Vec2d::UNDEFINED;
        EXPECT_FALSE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_TRUE(intersect1.isUndefined());
        EXPECT_TRUE(intersect2.isUndefined());
    }

    // p1 and p2 equal
    {
        Rectd rect = Rectd::fromMinMax(Vec2d(5, 7), Vec2d(10, 20));

        Vec2d p1(7, 15);
        Vec2d p2(7, 15);
        Vec2d intersect1;
        Vec2d intersect2;
        EXPECT_TRUE(rect.segmentIntersect(p1, p2, &intersect1, &intersect2));
        EXPECT_DOUBLE_EQ(7, intersect1.x());
        EXPECT_DOUBLE_EQ(15, intersect1.y());
        EXPECT_DOUBLE_EQ(7, intersect2.x());
        EXPECT_DOUBLE_EQ(15, intersect2.y());
    }

}


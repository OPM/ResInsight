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
#include "cvfBoundingBox.h"

#include <limits>

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, BasicConstruction)
{
    BoundingBox bb;

    bb.add(Vec3f(10, 20, 30));
    EXPECT_EQ(10, bb.min().x());
    EXPECT_EQ(20, bb.min().y());
    EXPECT_EQ(30, bb.min().z());
    EXPECT_EQ(10, bb.max().x());
    EXPECT_EQ(20, bb.max().y());
    EXPECT_EQ(30, bb.max().z());

    bb.add(Vec3f(1, 2, 3));
    EXPECT_EQ(1, bb.min().x());
    EXPECT_EQ(2, bb.min().y());
    EXPECT_EQ(3, bb.min().z());
    EXPECT_EQ(10, bb.max().x());
    EXPECT_EQ(20, bb.max().y());
    EXPECT_EQ(30, bb.max().z());

    bb.add(Vec3f(-1, -2, -3));
    EXPECT_EQ(-1, bb.min().x());
    EXPECT_EQ(-2, bb.min().y());
    EXPECT_EQ(-3, bb.min().z());
    EXPECT_EQ(10, bb.max().x());
    EXPECT_EQ(20, bb.max().y());
    EXPECT_EQ(30, bb.max().z());


    // Constructor with two input points
    Vec3d vec1(-100, -200, -300);
    Vec3d vec2(10, 20, 30);

    BoundingBox bb2(vec1, vec2);
    EXPECT_TRUE(vec1 == bb2.min());
    EXPECT_TRUE(vec2 == bb2.max());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, CopyConstructor)
{
    {
        BoundingBox bb1;
        bb1.add(Vec3d::ZERO);
        EXPECT_TRUE(bb1.isValid());
        EXPECT_DOUBLE_EQ(0, bb1.radius());

        BoundingBox bb2(bb1);
        EXPECT_TRUE(bb2.isValid());
        EXPECT_DOUBLE_EQ(0, bb2.radius());
    }

    {
        const BoundingBox bb1;
        EXPECT_FALSE(bb1.isValid());

        BoundingBox bb2(bb1);
        EXPECT_FALSE(bb2.isValid());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, Assignment)
{
    {
        BoundingBox bb1;
        bb1.add(Vec3d::ZERO);
        EXPECT_TRUE(bb1.isValid());
        EXPECT_DOUBLE_EQ(0, bb1.radius());

        BoundingBox bb2;
        bb2 = bb1;
        EXPECT_TRUE(bb2.isValid());
        EXPECT_DOUBLE_EQ(0, bb2.radius());
    }

    {
        const BoundingBox bb1;
        EXPECT_FALSE(bb1.isValid());

        BoundingBox bb2;
        bb2 = bb1;
        EXPECT_FALSE(bb2.isValid());
    }
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, State)
{
    BoundingBox bb;
    EXPECT_FALSE(bb.isValid());

    bb.add(Vec3f(10, 20, 30));

    EXPECT_TRUE(bb.isValid());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, AddBB)
{
    BoundingBox bb;
    bb.add(Vec3f(10, 20, 30));
    bb.add(Vec3f(100, 200, 300));

    BoundingBox bb2;
    bb2.add(bb);

    EXPECT_TRUE(bb.min() == bb2.min());
    EXPECT_TRUE(bb.max() == bb2.max());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, AddPoints)
{
    Vec3dArray vA;
    vA.resize(4);
    ASSERT_EQ(4u, vA.size());

    vA[0] = Vec3d(1,2,3);
    vA[1] = Vec3d(1.1f, 2.2f, 3.3f);
    vA[2] = Vec3d(0,0,0);
    vA[3] = Vec3d(-4,-5,-6);

    BoundingBox bb;
    bb.add(vA);

    EXPECT_TRUE(bb.min() == vA[3]);
    EXPECT_TRUE(bb.max() == vA[1]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, Computations)
{
    BoundingBox bb;
    Vec3d v1 = Vec3d(2, 4, 6);
    Vec3d v2 = Vec3d(4, 8, 12);
    bb.add(v1);
    bb.add(v2);

    Vec3d expectedCenter(3, 6, 9);
    EXPECT_TRUE(expectedCenter == bb.center());

    Vec3d expectedExtent = v2 - v1;
    EXPECT_TRUE(expectedExtent == bb.extent());
    
    const double expectedRadius = (v1 - v2).length() / 2.0;
    EXPECT_EQ(expectedRadius, bb.radius());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(BoundingBoxDeathTest, AccessInvalidBox)
{
    BoundingBox bb;
    EXPECT_DEATH(bb.min(), "Assertion");
    EXPECT_DEATH(bb.max(), "Assertion");
    EXPECT_DEATH(bb.center(), "Assertion");
    EXPECT_DEATH(bb.extent(), "Assertion");
    EXPECT_DEATH(bb.radius(), "Assertion");

    bb.reset();
    EXPECT_DEATH(bb.min(), "Assertion");
    EXPECT_DEATH(bb.max(), "Assertion");
    EXPECT_DEATH(bb.center(), "Assertion");
    EXPECT_DEATH(bb.extent(), "Assertion");
    EXPECT_DEATH(bb.radius(), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, ContainsPoint)
{
    const BoundingBox bb(Vec3d(1.0, 1.0, 1.0), Vec3d(3.0, 3.0, 3.0));

    EXPECT_TRUE(bb.contains(Vec3d(1.0, 1.0, 1.0)));
    EXPECT_TRUE(bb.contains(Vec3d(3.0, 3.0, 3.0)));
    EXPECT_TRUE(bb.contains(Vec3d(1.0, 2.0, 3.0)));
    EXPECT_TRUE(bb.contains(Vec3d(1.1, 2.0, 2.9)));

    EXPECT_FALSE(bb.contains(Vec3d( 0.0,  0.0,  0.0)));
    EXPECT_FALSE(bb.contains(Vec3d(-1.0, -2.0, -3.0)));
    EXPECT_FALSE(bb.contains(Vec3d( 1.0,  2.0,  4.0)));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, Intersects)
{
    const BoundingBox bb0(Vec3d(1.0, 1.0, 1.0), Vec3d(4.0, 4.0, 4.0));

    {
        BoundingBox bb(Vec3d(2.0, 2.0, 2.0), Vec3d(3.0, 3.0, 3.0));
        EXPECT_TRUE(bb0.intersects(bb));
        EXPECT_TRUE(bb.intersects(bb0));
    }

    {
        BoundingBox bb(Vec3d(0.0, 0.0, 0.0), Vec3d(1.0, 1.0, 1.0));
        EXPECT_TRUE(bb0.intersects(bb));
        EXPECT_TRUE(bb.intersects(bb0));
    }

    {
        BoundingBox bb(Vec3d(0.0, 0.0, 0.0), Vec3d(0.5, 0.5, 0.5));
        EXPECT_FALSE(bb0.intersects(bb));
        EXPECT_FALSE(bb.intersects(bb0));
    }

    {
        BoundingBox bb(Vec3d(5.0, 6.0, 7.0), Vec3d(5.0, 6.0, 7.0));
        EXPECT_FALSE(bb0.intersects(bb));
        EXPECT_FALSE(bb.intersects(bb0));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, CornerPoints)
{
    const BoundingBox bb(Vec3d(1.0, 2.0, 3.0), Vec3d(4.0, 5.0, 6.0));

    //        7---------6                
    //       /|        /|     |z       
    //      / |       / |     | / y    
    //     4---------5  |     |/       
    //     |  3------|--2     *---x    
    //     | /       | /           
    //     |/        |/            
    //     0---------1  

    Vec3d c[8];
    bb.cornerVertices(c);

    EXPECT_TRUE(Vec3d(1, 2, 3) == c[0]);
    EXPECT_TRUE(Vec3d(4, 2, 3) == c[1]);
    EXPECT_TRUE(Vec3d(4, 5, 3) == c[2]);
    EXPECT_TRUE(Vec3d(1, 5, 3) == c[3]);

    EXPECT_TRUE(Vec3d(1, 2, 6) == c[4]);
    EXPECT_TRUE(Vec3d(4, 2, 6) == c[5]);
    EXPECT_TRUE(Vec3d(4, 5, 6) == c[6]);
    EXPECT_TRUE(Vec3d(1, 5, 6) == c[7]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, Transform)
{
    BoundingBox bb;
    Vec3d v1 = Vec3d(2, 4, 6);
    Vec3d v2 = Vec3d(4, 8, 12);
    bb.add(v1);
    bb.add(v2);

    Mat4d m;
    Vec3d t(1,2,3);
    m.setTranslation(t);

    bb.transform(m);

    EXPECT_EQ(3, bb.min().x());
    EXPECT_EQ(6, bb.min().y());
    EXPECT_EQ(9, bb.min().z());
    EXPECT_EQ(5, bb.max().x());
    EXPECT_EQ(10, bb.max().y());
    EXPECT_EQ(15, bb.max().z());

    BoundingBox b2 = bb.getTransformed(m);

    EXPECT_EQ(3, bb.min().x());
    EXPECT_EQ(6, bb.min().y());
    EXPECT_EQ(9, bb.min().z());
    EXPECT_EQ(5, bb.max().x());
    EXPECT_EQ(10, bb.max().y());
    EXPECT_EQ(15, bb.max().z());

    EXPECT_EQ(4, b2.min().x());
    EXPECT_EQ(8, b2.min().y());
    EXPECT_EQ(12, b2.min().z());
    EXPECT_EQ(6, b2.max().x());
    EXPECT_EQ(12, b2.max().y());
    EXPECT_EQ(18, b2.max().z());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BoundingBoxTest, Expand)
{
    BoundingBox bb;
    Vec3d v1 = Vec3d(1, 2, 3);
    Vec3d v2 = Vec3d(3, 6, 9);
    bb.add(v1);
    bb.add(v2);

    Vec3d extent1 = bb.extent();
    ASSERT_DOUBLE_EQ(2, extent1.x());
    ASSERT_DOUBLE_EQ(4, extent1.y());
    ASSERT_DOUBLE_EQ(6, extent1.z());
    
    bb.expand(3);
    Vec3d extent2 = bb.extent();
    EXPECT_DOUBLE_EQ(5, extent2.x());
    EXPECT_DOUBLE_EQ(7, extent2.y());
    EXPECT_DOUBLE_EQ(9, extent2.z());
}



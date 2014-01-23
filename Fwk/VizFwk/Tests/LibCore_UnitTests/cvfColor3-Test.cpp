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
#include "cvfColor3.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, Constructors)
{
    // Default constructor
    Color3f c0;
    EXPECT_FLOAT_EQ(0.0f, c0.r());
    EXPECT_FLOAT_EQ(0.0f, c0.g());
    EXPECT_FLOAT_EQ(0.0f, c0.b());

    // Constructor with component init
    Color3f c1(0.1f, 0.2f, 0.3f);
    EXPECT_FLOAT_EQ(0.1f, c1.r());
    EXPECT_FLOAT_EQ(0.2f, c1.g());
    EXPECT_FLOAT_EQ(0.3f, c1.b());

    // Copy constructor
    Color3f c2(c1);
    EXPECT_FLOAT_EQ(0.1f, c2.r());
    EXPECT_FLOAT_EQ(0.2f, c2.g());
    EXPECT_FLOAT_EQ(0.3f, c2.b());

    Color3f c3 = c1;
    EXPECT_FLOAT_EQ(0.1f, c3.r());
    EXPECT_FLOAT_EQ(0.2f, c3.g());
    EXPECT_FLOAT_EQ(0.3f, c3.b());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, ConstructionFromColorIdent)
{
    {
        Color3f c(Color3f::RED);
        EXPECT_FLOAT_EQ(1.0f, c.r());
        EXPECT_FLOAT_EQ(0.0f, c.g());
        EXPECT_FLOAT_EQ(0.0f, c.b());
    }

    {
        Color3f c(Color3f::GREEN);
        EXPECT_FLOAT_EQ(0.0f, c.r());
        EXPECT_FLOAT_EQ(1.0f, c.g());
        EXPECT_FLOAT_EQ(0.0f, c.b());
    }

    {
        Color3f c(Color3f::BLUE);
        EXPECT_FLOAT_EQ(0.0f, c.r());
        EXPECT_FLOAT_EQ(0.0f, c.g());
        EXPECT_FLOAT_EQ(1.0f, c.b());
    }

    {
        Color3f c(Color3f::GRAY);
        EXPECT_NEAR(0.50196f, c.r(), 1e-5);
        EXPECT_NEAR(0.50196f, c.g(), 1e-5);
        EXPECT_NEAR(0.50196f, c.b(), 1e-5);
    }

    {
        Color3f c(Color3f::ORANGE);
        EXPECT_NEAR(1.00000f, c.r(), 1e-5);
        EXPECT_NEAR(0.64706f, c.g(), 1e-5);
        EXPECT_NEAR(0.00000f, c.b(), 1e-5);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, ConstructionFromByteColor)
{
    const Color3ub cub0(0, 0, 0);
    const Color3ub cub1(255, 255, 255);
    const Color3ub cub2(51, 102, 153);

    {
        Color3f c(cub0);
        EXPECT_FLOAT_EQ(0.0f, c.r());
        EXPECT_FLOAT_EQ(0.0f, c.g());
        EXPECT_FLOAT_EQ(0.0f, c.b());
    }

    {
        Color3f c(cub1);
        EXPECT_FLOAT_EQ(1.0f, c.r());
        EXPECT_FLOAT_EQ(1.0f, c.g());
        EXPECT_FLOAT_EQ(1.0f, c.b());
    }

    {
        Color3f c(cub2);
        EXPECT_FLOAT_EQ(0.2f, c.r());
        EXPECT_FLOAT_EQ(0.4f, c.g());
        EXPECT_FLOAT_EQ(0.6f, c.b());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, AssignmentOperator)
{
    Color3f c0(0.1f, 0.2f, 0.3f);

    Color3f c1;
    c1 = c0;
    EXPECT_FLOAT_EQ(0.1f, c1.r());
    EXPECT_FLOAT_EQ(0.2f, c1.g());
    EXPECT_FLOAT_EQ(0.3f, c1.b());

    Color3f c;
    c = Color3f::RED;
    EXPECT_FLOAT_EQ(1.0f, c.r());
    EXPECT_FLOAT_EQ(0.0f, c.g());
    EXPECT_FLOAT_EQ(0.0f, c.b());

    c = Color3f::GREEN;
    EXPECT_FLOAT_EQ(0.0f, c.r());
    EXPECT_FLOAT_EQ(1.0f, c.g());
    EXPECT_FLOAT_EQ(0.0f, c.b());

    c = Color3f::BLUE;
    EXPECT_FLOAT_EQ(0.0f, c.r());
    EXPECT_FLOAT_EQ(0.0f, c.g());
    EXPECT_FLOAT_EQ(1.0f, c.b());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, Comparison)
{
    const Color3f c0(0.1f, 0.2f, 0.3f);
    const Color3f c1(0.5f, 0.6f, 0.7f);
    const Color3f c2(0.5f, 0.6f, 0.7f);

    EXPECT_TRUE(c0 == c0);
    EXPECT_TRUE(c1 == c1);
    EXPECT_TRUE(c1 == c2);
    EXPECT_FALSE(c0 == c1);

    EXPECT_FALSE(c0 != c0);
    EXPECT_FALSE(c1 != c1);
    EXPECT_FALSE(c1 != c2);
    EXPECT_TRUE(c0 != c1);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, SettersAndGetters)
{
    // Component getters
    {
        const Color3f c(0.1f, 0.2f, 0.3f);
        EXPECT_FLOAT_EQ(0.1f, c.r());
        EXPECT_FLOAT_EQ(0.2f, c.g());
        EXPECT_FLOAT_EQ(0.3f, c.b());
    }

    // Component setters
    {
        Color3f c;
        c.r() = 0.5f;
        c.g() = 0.6f;
        c.b() = 0.7f;
        EXPECT_FLOAT_EQ(0.5f, c.r());
        EXPECT_FLOAT_EQ(0.6f, c.g());
        EXPECT_FLOAT_EQ(0.7f, c.b());
    }

    {
        Color3f c;
        c.set(0.1f, 0.2f, 0.3f);
        EXPECT_FLOAT_EQ(0.1f, c.r());
        EXPECT_FLOAT_EQ(0.2f, c.g());
        EXPECT_FLOAT_EQ(0.3f, c.b());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, GetNativePointer)
{
    Color3f c(0.1f, 0.2f, 0.3f);
    EXPECT_FLOAT_EQ(0.1f, c.r());
    EXPECT_FLOAT_EQ(0.2f, c.g());
    EXPECT_FLOAT_EQ(0.3f, c.b());

    const float* p = c.ptr();
    EXPECT_FLOAT_EQ(0.1f, p[0]);
    EXPECT_FLOAT_EQ(0.2f, p[1]);
    EXPECT_FLOAT_EQ(0.3f, p[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, Validation)
{
    EXPECT_TRUE(Color3f(0.0f, 0.0f, 0.0f).isValid());
    EXPECT_TRUE(Color3f(1.0f, 1.0f, 1.0f).isValid());
    EXPECT_TRUE(Color3f(0.5f, 0.5f, 0.5f).isValid());
    EXPECT_TRUE(Color3f(0.0f, 1.0f, 0.0f).isValid());
    EXPECT_TRUE(Color3f(0.1f, 0.2f, 0.3f).isValid());

    EXPECT_FALSE(Color3f( 2.0f,  0.5f,  0.5f).isValid());
    EXPECT_FALSE(Color3f( 0.5f,  2.0f,  0.5f).isValid());
    EXPECT_FALSE(Color3f( 0.5f,  0.5f,  2.0f).isValid());
    EXPECT_FALSE(Color3f(-0.5f,  0.5f,  0.5f).isValid());
    EXPECT_FALSE(Color3f( 0.5f, -0.5f,  0.5f).isValid());
    EXPECT_FALSE(Color3f(00.5f,  0.5f, -0.5f).isValid());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, CreateFromByteColor)
{
    Color3f c = Color3f::fromByteColor(0, 0, 0);
    EXPECT_FLOAT_EQ(0.0f, c.r());
    EXPECT_FLOAT_EQ(0.0f, c.g());
    EXPECT_FLOAT_EQ(0.0f, c.b());

    c = Color3f::fromByteColor(255, 255, 255);
    EXPECT_FLOAT_EQ(1.0f, c.r());
    EXPECT_FLOAT_EQ(1.0f, c.g());
    EXPECT_FLOAT_EQ(1.0f, c.b());

    c = Color3f::fromByteColor(127, 127, 127);
    EXPECT_LT(c.r(), 0.5);
    EXPECT_LT(c.g(), 0.5);
    EXPECT_LT(c.b(), 0.5);

    c = Color3f::fromByteColor(128, 128, 128);
    EXPECT_GT(c.r(), 0.5);
    EXPECT_GT(c.g(), 0.5);
    EXPECT_GT(c.b(), 0.5);

    c = Color3f::fromByteColor(1, 1, 254);
    EXPECT_NEAR(c.r(), 0.004, 0.001);
    EXPECT_NEAR(c.g(), 0.004, 0.001);
    EXPECT_NEAR(c.b(), 0.996, 0.001);

    c = Color3f::fromByteColor(254, 1, 1);
    EXPECT_NEAR(c.r(), 0.996, 0.001);
    EXPECT_NEAR(c.g(), 0.004, 0.001);
    EXPECT_NEAR(c.b(), 0.004, 0.001);

    c = Color3f::fromByteColor(64, 128, 192);
    EXPECT_NEAR(c.r(), 0.2509, 0.001);
    EXPECT_NEAR(c.g(), 0.5020, 0.001);
    EXPECT_NEAR(c.b(), 0.7539, 0.001);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3fTest, GetAsByteColorComponents)
{
    Color3f c(0, 0, 0);
    EXPECT_EQ(0, c.rByte());
    EXPECT_EQ(0, c.gByte());
    EXPECT_EQ(0, c.bByte());

    c.set(1.0f, 1.0f, 1.0f);
    EXPECT_EQ(255, c.rByte());
    EXPECT_EQ(255, c.gByte());
    EXPECT_EQ(255, c.bByte());

    c.set(-1.0f, -2.0f, -3.0f);
    EXPECT_EQ(0, c.rByte());
    EXPECT_EQ(0, c.gByte());
    EXPECT_EQ(0, c.bByte());

    c.set(1.0f, 2.0f, 2.0f);
    EXPECT_EQ(255, c.rByte());
    EXPECT_EQ(255, c.gByte());
    EXPECT_EQ(255, c.bByte());


    c = Color3f::fromByteColor(0, 1, 2);
    EXPECT_EQ(0, c.rByte());
    EXPECT_EQ(1, c.gByte());
    EXPECT_EQ(2, c.bByte());

    c = Color3f::fromByteColor(253, 254, 255);
    EXPECT_EQ(253, c.rByte());
    EXPECT_EQ(254, c.gByte());
    EXPECT_EQ(255, c.bByte());

    c = Color3f::fromByteColor(64, 128, 192);
    EXPECT_EQ(64, c.rByte());
    EXPECT_EQ(128, c.gByte());
    EXPECT_EQ(192, c.bByte());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3ubTest, Constructors)
{
    // Default constructor
    const Color3ub clr0;
    EXPECT_EQ(0, clr0.r());
    EXPECT_EQ(0, clr0.g());
    EXPECT_EQ(0, clr0.b());

    // Constructor with component init
    const Color3ub clr1(11, 12, 13);
    EXPECT_EQ(11, clr1.r());
    EXPECT_EQ(12, clr1.g());
    EXPECT_EQ(13, clr1.b());

    const Color3ub clr2(10, 20, 30);
    EXPECT_EQ(10, clr2.r());
    EXPECT_EQ(20, clr2.g());
    EXPECT_EQ(30, clr2.b());


    // Copy constructor
    {
        Color3ub c1(clr1);
        EXPECT_EQ(11, c1.r());
        EXPECT_EQ(12, c1.g());
        EXPECT_EQ(13, c1.b());

        Color3ub c2 = clr1;
        EXPECT_EQ(11, c2.r());
        EXPECT_EQ(12, c2.g());
        EXPECT_EQ(13, c2.b());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3ubTest, ConstructionFromColorIdent)
{
    {
        Color3ub c(Color3::RED);
        EXPECT_EQ(255, c.r());
        EXPECT_EQ(0,   c.g());
        EXPECT_EQ(0,   c.b());
    }

    {
        Color3ub c(Color3::GREEN);
        EXPECT_EQ(0,   c.r());
        EXPECT_EQ(255, c.g());
        EXPECT_EQ(0,   c.b());
    }

    {
        Color3ub c(Color3::BLUE);
        EXPECT_EQ(0,   c.r());
        EXPECT_EQ(0,   c.g());
        EXPECT_EQ(255, c.b());
    }

    {
        Color3ub c(Color3::GRAY);
        EXPECT_EQ(128, c.r());
        EXPECT_EQ(128, c.g());
        EXPECT_EQ(128, c.b());
    }

    {
        Color3ub c(Color3::ORANGE);
        EXPECT_EQ(255, c.r());
        EXPECT_EQ(165, c.g());
        EXPECT_EQ(0,   c.b());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3ubTest, AssignmentOperators)
{
    Color3ub c0(10, 20, 30);

    {
        Color3ub c;
        c = c0;
        EXPECT_EQ(10, c.r());
        EXPECT_EQ(20, c.g());
        EXPECT_EQ(30, c.b());
    }

    {
        Color3ub c;
        c = Color3::RED;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3ubTest, Comparison)
{
    const Color3ub c0(1, 2, 3);
    const Color3ub c1(5, 6, 7);
    const Color3ub c2(5, 6, 7);

    EXPECT_TRUE(c0 == c0);
    EXPECT_TRUE(c1 == c1);
    EXPECT_TRUE(c1 == c2);
    EXPECT_FALSE(c0 == c1);

    EXPECT_FALSE(c0 != c0);
    EXPECT_FALSE(c1 != c1);
    EXPECT_FALSE(c1 != c2);
    EXPECT_TRUE(c0 != c1);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3ubTest, SettersAndGetters)
{
    // Component getters
    {
        const Color3ub c(1, 2, 3);
        EXPECT_EQ(1, c.r());
        EXPECT_EQ(2, c.g());
        EXPECT_EQ(3, c.b());
    }

    // Component setters
    {
        Color3ub c;
        c.r() = 5;
        c.g() = 6;
        c.b() = 7;
        EXPECT_EQ(5, c.r());
        EXPECT_EQ(6, c.g());
        EXPECT_EQ(7, c.b());
    }

    {
        Color3ub c(0, 0, 0);
        c.set(1, 2, 3);
        EXPECT_EQ(1, c.r());
        EXPECT_EQ(2, c.g());
        EXPECT_EQ(3, c.b());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3ubTest, ConstructionFromFloatColor)
{
    const Color3f cf0(0, 0, 0);
    const Color3f cf1(1, 1, 1);
    const Color3f cf2(0.2f, 0.4f, 0.6f);

    {
        Color3ub c(cf0);
        EXPECT_EQ(0, c.r());
        EXPECT_EQ(0, c.g());
        EXPECT_EQ(0, c.b());
    }

    {
        Color3ub c(cf1);
        EXPECT_EQ(255, c.r());
        EXPECT_EQ(255, c.g());
        EXPECT_EQ(255, c.b());
    }

    {
        Color3ub c(cf2);
        EXPECT_EQ(51, c.r());
        EXPECT_EQ(102, c.g());
        EXPECT_EQ(153, c.b());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color3ubTest, GetNativePointer)
{
    Color3ub c(1, 2, 3);
    EXPECT_EQ(1, c.r());
    EXPECT_EQ(2, c.g());
    EXPECT_EQ(3, c.b());

    const ubyte* p = c.ptr();
    EXPECT_EQ(1, p[0]);
    EXPECT_EQ(2, p[1]);
    EXPECT_EQ(3, p[2]);
}









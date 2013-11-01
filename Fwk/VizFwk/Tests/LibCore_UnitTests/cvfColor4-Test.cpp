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
#include "cvfColor4.h"
#include "cvfColor3.h"

#include "gtest/gtest.h"

using namespace cvf;



//==================================================================================================
//
// Color4fTest
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, Constructors)
{
    // Default constructor
    Color4f c0;
    EXPECT_FLOAT_EQ(0.0f, c0.r());
    EXPECT_FLOAT_EQ(0.0f, c0.g());
    EXPECT_FLOAT_EQ(0.0f, c0.b());
    EXPECT_FLOAT_EQ(1.0f, c0.a());

    // Constructor with component init
    Color4f c1(0.1f, 0.2f, 0.3f, 0.4f);
    EXPECT_FLOAT_EQ(0.1f, c1.r());
    EXPECT_FLOAT_EQ(0.2f, c1.g());
    EXPECT_FLOAT_EQ(0.3f, c1.b());
    EXPECT_FLOAT_EQ(0.4f, c1.a());

    // Copy constructor
    Color4f c2(c1);
    EXPECT_FLOAT_EQ(0.1f, c2.r());
    EXPECT_FLOAT_EQ(0.2f, c2.g());
    EXPECT_FLOAT_EQ(0.3f, c2.b());
    EXPECT_FLOAT_EQ(0.4f, c2.a());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, ConstructionFromColor3)
{
    const Color3f rgb(0.1f, 0.2f, 0.3f);

    {
        Color4f c(rgb);
        EXPECT_FLOAT_EQ(0.1f, c.r());
        EXPECT_FLOAT_EQ(0.2f, c.g());
        EXPECT_FLOAT_EQ(0.3f, c.b());
        EXPECT_FLOAT_EQ(1.0f, c.a());
    }

    {
        Color4f c(rgb, 0.5f);
        EXPECT_FLOAT_EQ(0.1f, c.r());
        EXPECT_FLOAT_EQ(0.2f, c.g());
        EXPECT_FLOAT_EQ(0.3f, c.b());
        EXPECT_FLOAT_EQ(0.5f, c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, ConstructionFromColorIdent)
{
    {
        Color4f c(Color3::RED);
        EXPECT_FLOAT_EQ(1.0f, c.r());
        EXPECT_FLOAT_EQ(0.0f, c.g());
        EXPECT_FLOAT_EQ(0.0f, c.b());
        EXPECT_FLOAT_EQ(1.0f, c.a());
    }

    {
        Color4f c(Color3f::ORANGE);
        EXPECT_NEAR(1.00000f, c.r(), 1e-5);
        EXPECT_NEAR(0.64706f, c.g(), 1e-5);
        EXPECT_NEAR(0.00000f, c.b(), 1e-5);
        EXPECT_FLOAT_EQ(1.0f, c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, ConstructionFromByteColor)
{
    const Color4ub cub0(0, 0, 0, 0);
    const Color4ub cub1(255, 255, 255, 255);
    const Color4ub cub2(51, 102, 153, 204);

    {
        Color4f c(cub0);
        EXPECT_FLOAT_EQ(0.0f, c.r());
        EXPECT_FLOAT_EQ(0.0f, c.g());
        EXPECT_FLOAT_EQ(0.0f, c.b());
        EXPECT_FLOAT_EQ(0.0f, c.a());
    }

    {
        Color4f c(cub1);
        EXPECT_FLOAT_EQ(1.0f, c.r());
        EXPECT_FLOAT_EQ(1.0f, c.g());
        EXPECT_FLOAT_EQ(1.0f, c.b());
        EXPECT_FLOAT_EQ(1.0f, c.a());
    }

    {
        Color4f c(cub2);
        EXPECT_FLOAT_EQ(0.2f, c.r());
        EXPECT_FLOAT_EQ(0.4f, c.g());
        EXPECT_FLOAT_EQ(0.6f, c.b());
        EXPECT_FLOAT_EQ(0.8f, c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, Assignment)
{
    Color4f c0(0.1f, 0.2f, 0.3f, 0.4f);

    Color4f c1;
    c1 = c0;
    EXPECT_FLOAT_EQ(0.1f, c1.r());
    EXPECT_FLOAT_EQ(0.2f, c1.g());
    EXPECT_FLOAT_EQ(0.3f, c1.b());
    EXPECT_FLOAT_EQ(0.4f, c1.a());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, Comparison)
{
    const Color4f c0(0.1f, 0.2f, 0.3f, 0.4f);
    const Color4f c1(0.5f, 0.6f, 0.7f, 0.8f);
    const Color4f c2(0.5f, 0.6f, 0.7f, 0.8f);

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
TEST(Color4fTest, SettersAndGetters)
{
    // Component getters
    {
        const Color4f c(0.1f, 0.2f, 0.3f, 0.4f);
        EXPECT_FLOAT_EQ(0.1f, c.r());
        EXPECT_FLOAT_EQ(0.2f, c.g());
        EXPECT_FLOAT_EQ(0.3f, c.b());
        EXPECT_FLOAT_EQ(0.4f, c.a());
    }

    // Component setters
    {
        Color4f c;
        c.r() = 0.5f;
        c.g() = 0.6f;
        c.b() = 0.7f;
        c.a() = 0.8f;
        EXPECT_FLOAT_EQ(0.5f, c.r());
        EXPECT_FLOAT_EQ(0.6f, c.g());
        EXPECT_FLOAT_EQ(0.7f, c.b());
        EXPECT_FLOAT_EQ(0.8f, c.a());
    }


    {
        Color4f c;
        c.set(0.1f, 0.2f, 0.3f, 0.4f);
        EXPECT_FLOAT_EQ(0.1f, c.r());
        EXPECT_FLOAT_EQ(0.2f, c.g());
        EXPECT_FLOAT_EQ(0.3f, c.b());
        EXPECT_FLOAT_EQ(0.4f, c.a());
    }

    {
        const Color3f rgb1(1.0f, 2.0f, 3.0f);
        Color4f c;
        c.set(rgb1, 4.0f);
        EXPECT_FLOAT_EQ(1.0f, c.r());
        EXPECT_FLOAT_EQ(2.0f, c.g());
        EXPECT_FLOAT_EQ(3.0f, c.b());
        EXPECT_FLOAT_EQ(4.0f, c.a());
    }

    {
        const Color3f rgb2(4.0f, 5.0f, 6.0f);
        Color4f c;
        c.set(rgb2);
        EXPECT_FLOAT_EQ(4.0f, c.r());
        EXPECT_FLOAT_EQ(5.0f, c.g());
        EXPECT_FLOAT_EQ(6.0f, c.b());
        EXPECT_FLOAT_EQ(1.0f, c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, GetNativePointer)
{
    Color4f c(0.1f, 0.2f, 0.3f, 0.4f);
    EXPECT_FLOAT_EQ(0.1f, c.r());
    EXPECT_FLOAT_EQ(0.2f, c.g());
    EXPECT_FLOAT_EQ(0.3f, c.b());
    EXPECT_FLOAT_EQ(0.4f, c.a());

    const float* p = c.ptr();
    EXPECT_FLOAT_EQ(0.1f, p[0]);
    EXPECT_FLOAT_EQ(0.2f, p[1]);
    EXPECT_FLOAT_EQ(0.3f, p[2]);
    EXPECT_FLOAT_EQ(0.4f, p[3]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, Validation)
{
    EXPECT_TRUE(Color4f(0.0f, 0.0f, 0.0f, 0.0f).isValid());
    EXPECT_TRUE(Color4f(1.0f, 1.0f, 1.0f, 1.0f).isValid());
    EXPECT_TRUE(Color4f(0.5f, 0.5f, 0.5f, 0.5f).isValid());
    EXPECT_TRUE(Color4f(0.0f, 1.0f, 0.0f, 1.0f).isValid());
    EXPECT_TRUE(Color4f(0.1f, 0.2f, 0.3f, 0.4f).isValid());

    EXPECT_FALSE(Color4f( 2.0f,  0.5f,  0.5f,  0.5f).isValid());
    EXPECT_FALSE(Color4f( 0.5f,  2.0f,  0.5f,  0.5f).isValid());
    EXPECT_FALSE(Color4f( 0.5f,  0.5f,  2.0f,  0.5f).isValid());
    EXPECT_FALSE(Color4f( 0.5f,  0.5f,  0.5f,  2.0f).isValid());
    EXPECT_FALSE(Color4f(-0.5f,  0.5f,  0.5f,  0.5f).isValid());
    EXPECT_FALSE(Color4f( 0.5f, -0.5f,  0.5f,  0.5f).isValid());
    EXPECT_FALSE(Color4f(00.5f,  0.5f, -0.5f,  0.5f).isValid());
    EXPECT_FALSE(Color4f(00.5f,  0.5f,  0.5f, -0.5f).isValid());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4fTest, ConvertToColor3f)
{
    const Color4f c(0.1f, 0.2f, 0.3f, 0.4f);

    Color3f rgb = c.toColor3f();
    EXPECT_FLOAT_EQ(0.1f, rgb.r());
    EXPECT_FLOAT_EQ(0.2f, rgb.g());
    EXPECT_FLOAT_EQ(0.3f, rgb.b());
}



//==================================================================================================
//
// Color4ubTest
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4ubTest, Constructors)
{
    // Default constructor
    const Color4ub clr0;
    EXPECT_EQ(0, clr0.r());
    EXPECT_EQ(0, clr0.g());
    EXPECT_EQ(0, clr0.b());
    EXPECT_EQ(255, clr0.a());

    // Constructor with component init
    const Color4ub clr1(11, 12, 13, 14);
    EXPECT_EQ(11, clr1.r());
    EXPECT_EQ(12, clr1.g());
    EXPECT_EQ(13, clr1.b());
    EXPECT_EQ(14, clr1.a());

    const Color4ub clr2(10, 20, 30, 40);
    EXPECT_EQ(10, clr2.r());
    EXPECT_EQ(20, clr2.g());
    EXPECT_EQ(30, clr2.b());
    EXPECT_EQ(40, clr2.a());

    // Copy constructor
    {
        Color4ub c1(clr1);
        EXPECT_EQ(11, c1.r());
        EXPECT_EQ(12, c1.g());
        EXPECT_EQ(13, c1.b());
        EXPECT_EQ(14, c1.a());

        Color4ub c2 = clr1;
        EXPECT_EQ(11, c2.r());
        EXPECT_EQ(12, c2.g());
        EXPECT_EQ(13, c2.b());
        EXPECT_EQ(14, c2.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4ubTest, ConstructionFromColor3)
{
    Color3ub rgb(1, 2, 3);

    {
        Color4ub c(rgb);
        EXPECT_EQ(1, c.r());
        EXPECT_EQ(2,   c.g());
        EXPECT_EQ(3,   c.b());
        EXPECT_EQ(255, c.a());
    }

    {
        Color4ub c(rgb, 100);
        EXPECT_EQ(1, c.r());
        EXPECT_EQ(2,   c.g());
        EXPECT_EQ(3,   c.b());
        EXPECT_EQ(100, c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4ubTest, ConstructionFromColorIdent)
{
    {
        Color4ub c(Color3::RED);
        EXPECT_EQ(255, c.r());
        EXPECT_EQ(0,   c.g());
        EXPECT_EQ(0,   c.b());
        EXPECT_EQ(255, c.a());
    }

    {
        Color4ub c(Color3::GREEN);
        EXPECT_EQ(0,   c.r());
        EXPECT_EQ(255, c.g());
        EXPECT_EQ(0,   c.b());
        EXPECT_EQ(255, c.a());
    }

    {
        Color4ub c(Color3::BLUE);
        EXPECT_EQ(0,   c.r());
        EXPECT_EQ(0,   c.g());
        EXPECT_EQ(255, c.b());
        EXPECT_EQ(255, c.a());
    }

    {
        Color4ub c(Color3::GRAY);
        EXPECT_EQ(128, c.r());
        EXPECT_EQ(128, c.g());
        EXPECT_EQ(128, c.b());
        EXPECT_EQ(255, c.a());
    }

    {
        Color4ub c(Color3::ORANGE);
        EXPECT_EQ(255, c.r());
        EXPECT_EQ(165, c.g());
        EXPECT_EQ(0,   c.b());
        EXPECT_EQ(255, c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4ubTest, AssignmentOperators)
{
    Color4ub c0(10, 20, 30, 40);

    {
        Color4ub c;
        c = c0;
        EXPECT_EQ(10, c.r());
        EXPECT_EQ(20, c.g());
        EXPECT_EQ(30, c.b());
        EXPECT_EQ(40, c.a());
    }

    {
        Color4ub c;
        c = Color4ub(Color3::RED);

        EXPECT_EQ(255,  c.r());
        EXPECT_EQ(0,    c.g());
        EXPECT_EQ(0,    c.b());
        EXPECT_EQ(255,  c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4ubTest, Comparison)
{
    const Color4ub c0(1, 2, 3, 4);
    const Color4ub c1(5, 6, 7, 8);
    const Color4ub c2(5, 6, 7, 8);

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
TEST(Color4ubTest, SettersAndGetters)
{
    // Component getters
    {
        const Color4ub c(1, 2, 3, 4);
        EXPECT_EQ(1, c.r());
        EXPECT_EQ(2, c.g());
        EXPECT_EQ(3, c.b());
        EXPECT_EQ(4, c.a());
    }

    // Component setters
    {
        Color4ub c;
        c.r() = 5;
        c.g() = 6;
        c.b() = 7;
        c.a() = 8;
        EXPECT_EQ(5, c.r());
        EXPECT_EQ(6, c.g());
        EXPECT_EQ(7, c.b());
        EXPECT_EQ(8, c.a());
    }

    {
        Color4ub c(0, 0, 0, 0);
        c.set(1, 2, 3, 4);
        EXPECT_EQ(1, c.r());
        EXPECT_EQ(2, c.g());
        EXPECT_EQ(3, c.b());
        EXPECT_EQ(4, c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4ubTest, ConstructionFromFloatColor)
{
    const Color4f cf0(0, 0, 0, 0);
    const Color4f cf1(1, 1, 1, 1);
    const Color4f cf2(0.2f, 0.4f, 0.6f, 0.8f);

    {
        Color4ub c(cf0);
        EXPECT_EQ(0, c.r());
        EXPECT_EQ(0, c.g());
        EXPECT_EQ(0, c.b());
        EXPECT_EQ(0, c.a());
    }

    {
        Color4ub c(cf1);
        EXPECT_EQ(255, c.r());
        EXPECT_EQ(255, c.g());
        EXPECT_EQ(255, c.b());
        EXPECT_EQ(255, c.a());
    }

    {
        Color4ub c(cf2);
        EXPECT_EQ(51, c.r());
        EXPECT_EQ(102, c.g());
        EXPECT_EQ(153, c.b());
        EXPECT_EQ(204, c.a());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Color4ubTest, GetNativePointer)
{
    Color4ub c(1, 2, 3, 4);
    EXPECT_EQ(1, c.r());
    EXPECT_EQ(2, c.g());
    EXPECT_EQ(3, c.b());
    EXPECT_EQ(4, c.a());

    const ubyte* p = c.ptr();
    EXPECT_EQ(1, p[0]);
    EXPECT_EQ(2, p[1]);
    EXPECT_EQ(3, p[2]);
    EXPECT_EQ(4, p[3]);
}


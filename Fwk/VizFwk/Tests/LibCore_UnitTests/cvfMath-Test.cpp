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


// Brings in the math constants from math.h (via cmath include)
// May not work on other platforms!
#define _USE_MATH_DEFINES

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfSystem.h"

#include "gtest/gtest.h"

#include <cmath>

using namespace cvf;

#ifdef WIN32
// We intentionally divide by 0 so nuke the warning
#pragma warning (disable: 4723)
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, MathAssumptions)
{
    double dZero = 0.0;
    double dNaN = sqrt(-1.0);
    double dPosInf = 1.0/dZero;
    double dNegInf = -1.0/dZero;

    EXPECT_NE(dNaN, dNaN);
    EXPECT_FALSE(dNaN == dNaN);
    EXPECT_TRUE(dNaN != dNaN);
    EXPECT_FALSE(1 > dNaN);
    EXPECT_FALSE(1 < dNaN);
    EXPECT_TRUE(!(1 > dNaN));
    EXPECT_TRUE(!(1 < dNaN));

    EXPECT_EQ(dPosInf, dPosInf);
    EXPECT_TRUE(dPosInf == dPosInf);
    EXPECT_GT(dPosInf, 100);

    EXPECT_EQ(dNegInf, dNegInf);
    EXPECT_TRUE(dNegInf == dNegInf);
    EXPECT_GT(-100, dNegInf);

    EXPECT_GT(dPosInf, dNegInf);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, DegreesRadiansConversion)
{
    ASSERT_DOUBLE_EQ(0,         Math::toRadians(0.0));
    ASSERT_DOUBLE_EQ(M_PI,      Math::toRadians(180.0));
    ASSERT_DOUBLE_EQ(M_PI_2,    Math::toRadians(90.0));
    ASSERT_DOUBLE_EQ(M_PI_4,    Math::toRadians(45.0));
    ASSERT_DOUBLE_EQ(-M_PI_4,   Math::toRadians(-45.0));

    ASSERT_DOUBLE_EQ(0.0,       Math::toDegrees(0.0));
    ASSERT_DOUBLE_EQ(180.0,     Math::toDegrees(M_PI));
    ASSERT_DOUBLE_EQ(90,        Math::toDegrees(M_PI_2));
    ASSERT_DOUBLE_EQ(45.0,      Math::toDegrees(M_PI_4));
    ASSERT_DOUBLE_EQ(-45.0,     Math::toDegrees(-M_PI_4));

    ASSERT_FLOAT_EQ(0.0f,                       Math::toRadians(0.0f));
    ASSERT_FLOAT_EQ(static_cast<float>(M_PI_4), Math::toRadians(45.0f));
    ASSERT_FLOAT_EQ(static_cast<float>(-M_PI_4),Math::toRadians(-45.0f));

    ASSERT_DOUBLE_EQ(0.0f,      Math::toDegrees(0.0f));
    ASSERT_FLOAT_EQ(45.0f,      Math::toDegrees(static_cast<float>(M_PI_4)));
    ASSERT_FLOAT_EQ(-45.0f,     Math::toDegrees(static_cast<float>(-M_PI_4)));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, CosAcos)
{
    EXPECT_EQ(cos(2.5),     Math::cos(2.5));
    EXPECT_EQ(cosf(1.5f),   Math::cos(1.5f));

    EXPECT_EQ(acos(-0.5),   Math::acos(-0.5));
    EXPECT_EQ(acosf(0.5f),  Math::acos(0.5f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, SinAsin)
{
    EXPECT_EQ(sin(2.5),     Math::sin(2.5));
    EXPECT_EQ(sinf(1.5f),   Math::sin(1.5f));

    EXPECT_EQ(asin(-0.5),   Math::asin(-0.5));
    EXPECT_EQ(asinf(0.5f),  Math::asin(0.5f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, TanAtan)
{
    EXPECT_EQ(tan(2.5),     Math::tan(2.5));
    EXPECT_EQ(tanf(1.5f),   Math::tan(1.5f));

    EXPECT_EQ(atan(-0.5),   Math::atan(-0.5));
    EXPECT_EQ(atanf(0.5f),  Math::atan(0.5f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, Sqrt)
{
    EXPECT_EQ(sqrt(9.0),     Math::sqrt(9.0));
    EXPECT_EQ(sqrtf(16.0f),  Math::sqrt(16.0f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, Floor)
{
    EXPECT_DOUBLE_EQ(1.0,  Math::floor(1.5));
    EXPECT_EQ(floor(1.5),  Math::floor(1.5));

    EXPECT_FLOAT_EQ(2.0f,  Math::floor(2.5f));
    EXPECT_EQ(floorf(2.5), Math::floor(2.5f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, Ceil)
{
    EXPECT_DOUBLE_EQ(2.0, Math::ceil(1.5));
    EXPECT_EQ(ceil(1.5),  Math::ceil(1.5));

    EXPECT_FLOAT_EQ(3.0f, Math::ceil(2.5f));
    EXPECT_EQ(ceilf(2.5), Math::ceil(2.5f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, Fmod)
{
    EXPECT_DOUBLE_EQ(1.0,     Math::fmod(3.0, 2.0));
    EXPECT_EQ(fmod(7.0, 5.0), Math::fmod(7.0, 5.0));

    EXPECT_FLOAT_EQ(1.0f,        Math::fmod(5.0f, 2.0f));
    EXPECT_EQ(fmodf(9.0f, 5.0f), Math::fmod(9.0f, 5.0f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, IsPow2)
{
    EXPECT_TRUE(Math::isPow2(1));
    EXPECT_TRUE(Math::isPow2(2));
    EXPECT_TRUE(Math::isPow2(1024));
    EXPECT_TRUE(Math::isPow2(2147483648));

    EXPECT_FALSE(Math::isPow2(0));
    EXPECT_FALSE(Math::isPow2(3));
    EXPECT_FALSE(Math::isPow2(1023));
    EXPECT_FALSE(Math::isPow2(2147483648 - 1));
    EXPECT_FALSE(Math::isPow2(2147483648 + 1));
    EXPECT_FALSE(Math::isPow2(4294967295));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, RoundUpPow2)
{
    EXPECT_EQ(1, Math::roundUpPow2(0));
    EXPECT_EQ(1, Math::roundUpPow2(1));
    EXPECT_EQ(2, Math::roundUpPow2(2));
    EXPECT_EQ(4, Math::roundUpPow2(3));

    EXPECT_EQ( 64, Math::roundUpPow2(63));
    EXPECT_EQ( 64, Math::roundUpPow2(64));
    EXPECT_EQ(128, Math::roundUpPow2(65));

    EXPECT_EQ(2147483648, Math::roundUpPow2(2147483648 - 1));
    EXPECT_EQ(2147483648, Math::roundUpPow2(2147483648));

    EXPECT_EQ(0, Math::roundUpPow2(2147483648 + 1));
    EXPECT_EQ(0, Math::roundUpPow2(4294967295));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, ValueInRange)
{
    EXPECT_TRUE(Math::valueInRange((int) 0,  0, 0));
    EXPECT_TRUE(Math::valueInRange((int) 5,  5, 5));
    EXPECT_TRUE(Math::valueInRange((int) 5, -9, 9));
    EXPECT_TRUE(Math::valueInRange((int)-5, -9, 9));
    EXPECT_FALSE(Math::valueInRange((int)0, 1, 2));
    EXPECT_FALSE(Math::valueInRange((int)5, 3, 4));

    EXPECT_TRUE(Math::valueInRange( 0.0f,  0.0f, 0.0f));
    EXPECT_TRUE(Math::valueInRange( 5.0f,  5.0f, 5.0f));
    EXPECT_TRUE(Math::valueInRange( 5.0f, -9.0f, 9.0f));
    EXPECT_TRUE(Math::valueInRange(-5.0f, -9.0f, 9.0f));
    EXPECT_FALSE(Math::valueInRange(0.0f,  1.0f, 2.0f));
    EXPECT_FALSE(Math::valueInRange(5.0f,  3.0f, 4.0f));

    EXPECT_TRUE(Math::valueInRange( 0.0,  0.0, 0.0));
    EXPECT_TRUE(Math::valueInRange( 5.0,  5.0, 5.0));
    EXPECT_TRUE(Math::valueInRange( 5.0, -9.0, 9.0));
    EXPECT_TRUE(Math::valueInRange(-5.0, -9.0, 9.0));
    EXPECT_FALSE(Math::valueInRange(0.0,  1.0, 2.0));
    EXPECT_FALSE(Math::valueInRange(5.0,  3.0, 4.0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, ValueInRange_InfNaN)
{
    double dZero = 0.0;
    double dNaN = sqrt(-1.0);
    double dPosInf = 1.0/dZero;
    double dNegInf = -1.0/dZero;

    EXPECT_FALSE(Math::valueInRange(dNaN, -99.0,  99.0));

    EXPECT_FALSE(Math::valueInRange(dPosInf, -99.0, 99.0));
    EXPECT_FALSE(Math::valueInRange(dNegInf, -99.0, 99.0));

    EXPECT_TRUE(Math::valueInRange(  0.0, dNegInf, dPosInf));
    EXPECT_TRUE(Math::valueInRange( 99.0, dNegInf, dPosInf));
    EXPECT_TRUE(Math::valueInRange(-99.0, dNegInf, dPosInf));

    EXPECT_TRUE(Math::valueInRange(std::numeric_limits<double>::min(), dNegInf, dPosInf));
    EXPECT_TRUE(Math::valueInRange(std::numeric_limits<double>::max(), dNegInf, dPosInf));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, Clamp)
{
    EXPECT_DOUBLE_EQ(0.0, Math::clamp( 1.0, 0.0, 0.0));
    EXPECT_DOUBLE_EQ(0.0, Math::clamp(-1.0, 0.0, 0.0));

    EXPECT_DOUBLE_EQ( 5.0, Math::clamp( 5.0,  5.0, 6.0));
    EXPECT_DOUBLE_EQ( 6.0, Math::clamp( 6.0,  5.0, 6.0));
    EXPECT_DOUBLE_EQ(-6.0, Math::clamp(-6.0, -6.0, -5.0));
    EXPECT_DOUBLE_EQ(-5.0, Math::clamp(-5.0, -6.0, -5.0));

    EXPECT_DOUBLE_EQ(2.0, Math::clamp( 3.0, 1.0, 2.0));
    EXPECT_DOUBLE_EQ(1.0, Math::clamp( 1.0, 1.0, 2.0));
    EXPECT_DOUBLE_EQ(1.0, Math::clamp(-1.0, 1.0, 2.0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, Clamp_InfNan)
{
    double dZero = 0.0;
    double dNaN = sqrt(-1.0);
    double dPosInf = 1.0/dZero;
    double dNegInf = -1.0/dZero;

    EXPECT_DOUBLE_EQ( 0.0, Math::clamp( 0.0, dNegInf, dPosInf));
    EXPECT_DOUBLE_EQ( 1.0, Math::clamp( 1.0, dNegInf, dPosInf));
    EXPECT_DOUBLE_EQ(-1.0, Math::clamp(-1.0, dNegInf, dPosInf));
    
    EXPECT_DOUBLE_EQ(-5.0, Math::clamp(dNegInf, -5.0, 5.0));
    EXPECT_DOUBLE_EQ( 5.0, Math::clamp(dPosInf, -5.0, 5.0));
    
    EXPECT_DOUBLE_EQ(-5.0, Math::clamp(dNaN, -5.0, 5.0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, IsUndefined)
{
    {
        double val = UNDEFINED_DOUBLE;
        EXPECT_TRUE(Math::isUndefined(val));

        val = 123.0;
        EXPECT_FALSE(Math::isUndefined(val));

        val = -123.0;
        EXPECT_FALSE(Math::isUndefined(val));
    }

    {
        float val = UNDEFINED_FLOAT;
        EXPECT_TRUE(Math::isUndefined(val));

        val = 123.0f;
        EXPECT_FALSE(Math::isUndefined(val));

        val = -123.0f;
        EXPECT_FALSE(Math::isUndefined(val));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, UndefinedUint)
{
    cvf::uint ui = UNDEFINED_UINT;
    ASSERT_EQ(UNDEFINED_UINT, ui);
    ASSERT_EQ(-1, ui);
    ASSERT_EQ(4294967295u, ui);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, UndefinedSize_t)
{
    size_t st = UNDEFINED_SIZE_T;
    ASSERT_EQ(UNDEFINED_SIZE_T, st);
    ASSERT_EQ(-1, st);

    if (System::is64Bit())
    {
        ASSERT_EQ(18446744073709551615u, st);
    }
    else
    {
        ASSERT_EQ(4294967295u, st);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, Abs)
{
    {
        int i = 134;
        int j = -134;

        EXPECT_EQ(134, Math::abs(i));
        EXPECT_EQ(134, Math::abs(j));
        EXPECT_EQ(134, Math::abs(j*2 + i));
    }

    {
        float i = 134.0f;
        float j = -134.0f;

        EXPECT_EQ(134.0f, Math::abs(i));
        EXPECT_EQ(134.0f, Math::abs(j));
        EXPECT_EQ(134.0f, Math::abs(j*2 + i));
    }


    {
        double i = 134.0;
        double j = -134.0;

        EXPECT_EQ(134.0, Math::abs(i));
        EXPECT_EQ(134.0, Math::abs(j));
        EXPECT_EQ(134.0, Math::abs(j*2 + i));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(MathTest, Sign)
{
    {
        double val = UNDEFINED_DOUBLE;

        val = 10.0;
        EXPECT_EQ(1, Math::sign(val));

        val = 0.0001;
        EXPECT_EQ(1, Math::sign(val));

        val = 0.0;
        EXPECT_EQ(1, Math::sign(val));

        val = -0.0;
        EXPECT_EQ(1, Math::sign(val));

        val = -0.0001;
        EXPECT_EQ(-1, Math::sign(val));

        val = -10.0;
        EXPECT_EQ(-1, Math::sign(val));
    }

    {
        float val = UNDEFINED_FLOAT;

        val = 10.0f;
        EXPECT_EQ(1, Math::sign(val));

        val = 0.0001f;
        EXPECT_EQ(1, Math::sign(val));

        val = 0.0f;
        EXPECT_EQ(1, Math::sign(val));

        val = -0.0f;
        EXPECT_EQ(1, Math::sign(val));

        val = -0.0001f;
        EXPECT_EQ(-1, Math::sign(val));

        val = -10.0f;
        EXPECT_EQ(-1, Math::sign(val));
    }

    {
        int val = UNDEFINED_INT;

        val = 10;
        EXPECT_EQ(1, Math::sign(val));

        val = 1;
        EXPECT_EQ(1, Math::sign(val));

        val = 0;
        EXPECT_EQ(1, Math::sign(val));

        val = -0;
        EXPECT_EQ(1, Math::sign(val));

        val = -1;
        EXPECT_EQ(-1, Math::sign(val));

        val = -10;
        EXPECT_EQ(-1, Math::sign(val));
    }

}

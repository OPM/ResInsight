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
#include "cvfVector3.h"
#include "cvfMatrix4.h"
#include "cvfString.h"              // Used for String vector 
#include "cvfMath.h"

#include "gtest/gtest.h"
#include <iostream>

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Constants)
{
    EXPECT_FLOAT_EQ(1.0f, Vec3f::X_AXIS.x());
    EXPECT_FLOAT_EQ(0.0f, Vec3f::X_AXIS.y());
    EXPECT_FLOAT_EQ(0.0f, Vec3f::X_AXIS.z());

    EXPECT_FLOAT_EQ(0.0f, Vec3f::Y_AXIS.x());
    EXPECT_FLOAT_EQ(1.0f, Vec3f::Y_AXIS.y());
    EXPECT_FLOAT_EQ(0.0f, Vec3f::Y_AXIS.z());

    EXPECT_FLOAT_EQ(0.0f, Vec3f::Z_AXIS.x());
    EXPECT_FLOAT_EQ(0.0f, Vec3f::Z_AXIS.y());
    EXPECT_FLOAT_EQ(1.0f, Vec3f::Z_AXIS.z());

    EXPECT_FLOAT_EQ(0.0f, Vec3f::ZERO.x());
    EXPECT_FLOAT_EQ(0.0f, Vec3f::ZERO.y());
    EXPECT_FLOAT_EQ(0.0f, Vec3f::ZERO.z());


    EXPECT_DOUBLE_EQ(1.0, Vec3d::X_AXIS.x());
    EXPECT_DOUBLE_EQ(0.0, Vec3d::X_AXIS.y());
    EXPECT_DOUBLE_EQ(0.0, Vec3d::X_AXIS.z());

    EXPECT_DOUBLE_EQ(0.0, Vec3d::Y_AXIS.x());
    EXPECT_DOUBLE_EQ(1.0, Vec3d::Y_AXIS.y());
    EXPECT_DOUBLE_EQ(0.0, Vec3d::Y_AXIS.z());

    EXPECT_DOUBLE_EQ(0.0, Vec3d::Z_AXIS.x());
    EXPECT_DOUBLE_EQ(0.0, Vec3d::Z_AXIS.y());
    EXPECT_DOUBLE_EQ(1.0, Vec3d::Z_AXIS.z());

    EXPECT_DOUBLE_EQ(0.0, Vec3d::ZERO.x());
    EXPECT_DOUBLE_EQ(0.0, Vec3d::ZERO.y());
    EXPECT_DOUBLE_EQ(0.0, Vec3d::ZERO.z());

    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec3d::UNDEFINED.x());
    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec3d::UNDEFINED.y());
    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec3d::UNDEFINED.z());

    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec3f::UNDEFINED.x());
    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec3f::UNDEFINED.y());
    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec3f::UNDEFINED.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, BasicConstruction)
{
    // float
    Vec3f v;
    v.set(1.0f, 2.5f, 3.0f);

    ASSERT_FLOAT_EQ(1.0f, v.x());
    ASSERT_FLOAT_EQ(2.5f, v.y());
    ASSERT_FLOAT_EQ(3.0f, v.z());

    Vec3f v2;
    v2.setZero();

    ASSERT_FLOAT_EQ(0.0f, v2.x());
    ASSERT_FLOAT_EQ(0.0f, v2.y());
    ASSERT_FLOAT_EQ(0.0f, v2.z());

    ASSERT_TRUE(v2.isZero());
    ASSERT_FALSE(v.isZero());

    // double
    Vec3d d;
    d.set(1.0, 2.5, 3.0);

    ASSERT_DOUBLE_EQ(1.0, d.x());
    ASSERT_DOUBLE_EQ(2.5, d.y());
    ASSERT_DOUBLE_EQ(3.0, d.z());

    Vec3f d2;
    d2.setZero();

    ASSERT_DOUBLE_EQ(0.0, d2.x());
    ASSERT_DOUBLE_EQ(0.0, d2.y());
    ASSERT_DOUBLE_EQ(0.0, d2.z());

    ASSERT_TRUE(d2.isZero());
    ASSERT_FALSE(d.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, CopyConstructor)
{
    // float
    Vec3f v1(1.0f, 2.5f, 3.0f);
    Vec3f v2(v1);
    Vec3f v3 = v1;
    Vec3f v4;
    v4 = v1;

    ASSERT_FLOAT_EQ(1.0f, v1.x());
    ASSERT_FLOAT_EQ(2.5f, v1.y());
    ASSERT_FLOAT_EQ(3.0f, v1.z());
    ASSERT_FLOAT_EQ(1.0f, v2.x());
    ASSERT_FLOAT_EQ(2.5f, v2.y());
    ASSERT_FLOAT_EQ(3.0f, v2.z());
    ASSERT_FLOAT_EQ(1.0f, v3.x());
    ASSERT_FLOAT_EQ(2.5f, v3.y());
    ASSERT_FLOAT_EQ(3.0f, v3.z());
    ASSERT_FLOAT_EQ(1.0f, v4.x());
    ASSERT_FLOAT_EQ(2.5f, v4.y());
    ASSERT_FLOAT_EQ(3.0f, v4.z());

    // double
    Vec3d d1(1.0, 2.5, 3.0);
    Vec3d d2(d1);
    Vec3d d3 = d1;
    Vec3d d4;
    d4 = d1;

    ASSERT_DOUBLE_EQ(1.0, d1.x());
    ASSERT_DOUBLE_EQ(2.5, d1.y());
    ASSERT_DOUBLE_EQ(3.0, d1.z());
    ASSERT_DOUBLE_EQ(1.0, d2.x());
    ASSERT_DOUBLE_EQ(2.5, d2.y());
    ASSERT_DOUBLE_EQ(3.0, d2.z());
    ASSERT_DOUBLE_EQ(1.0, d3.x());
    ASSERT_DOUBLE_EQ(2.5, d3.y());
    ASSERT_DOUBLE_EQ(3.0, d3.z());
    ASSERT_DOUBLE_EQ(1.0, d4.x());
    ASSERT_DOUBLE_EQ(2.5, d4.y());
    ASSERT_DOUBLE_EQ(3.0, d4.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, ConstructionFromScalars)
{
    float data[] = {1.0f,2.2f,3.0f};

    Vec3f vf = Vec3f(data[0], data[1], data[2]);

    EXPECT_EQ(1.0f, vf[0]);
    EXPECT_EQ(2.2f, vf[1]);
    EXPECT_EQ(3.0f, vf[2]);

    double datad[] = {1.0,2.2,3.0};

    Vec3d vd = Vec3d(datad[0], datad[1], datad[2]);

    EXPECT_EQ(1.0, vd[0]);
    EXPECT_EQ(2.2, vd[1]);
    EXPECT_EQ(3.0, vd[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, ConstructionFrom2DVector)
{
    {
        const Vec2d v0(1.0, 2.0);

        Vec3d v1(v0);
        EXPECT_EQ(1.0, v1.x());
        EXPECT_EQ(2.0, v1.y());
        EXPECT_EQ(0.0, v1.z());

        Vec3d v2(v0, 9.0);
        EXPECT_EQ(1.0, v2.x());
        EXPECT_EQ(2.0, v2.y());
        EXPECT_EQ(9.0, v2.z());
    }

    {
        const Vec2f v0(1.0f, 2.0f);

        Vec3f v1(v0);
        EXPECT_EQ(1.0f, v1.x());
        EXPECT_EQ(2.0f, v1.y());
        EXPECT_EQ(0.0f, v1.z());

        Vec3f v2(v0, 9.0f);
        EXPECT_EQ(1.0f, v2.x());
        EXPECT_EQ(2.0f, v2.y());
        EXPECT_EQ(9.0f, v2.z());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, AssignMemebers)
{
    // float
    Vec3f vf;

    vf.x() = 1.0f + 2.5f*3.0f;
    vf.y() = 2.0f - 2.5f*3.0f;
    vf.z() = 3.0f + 2.5f*2.0f;

    ASSERT_FLOAT_EQ(8.5f, vf.x());
    ASSERT_FLOAT_EQ(-5.5f, vf.y());
    ASSERT_FLOAT_EQ(8.0f, vf.z());

    vf.x() = 0.0f;
    vf.y() = 0.0f;
    vf.z() = 0.0f;
    ASSERT_TRUE(vf.isZero());

    vf.y() = 0.1f;
    ASSERT_FALSE(vf.isZero());

    // double
    Vec3d vd;

    vd.x() = 1.0 + 2.5*3.0;
    vd.y() = 2.0 - 2.5*3.0;
    vd.z() = 3.0 + 2.5*2.0;

    ASSERT_DOUBLE_EQ(8.5, vd.x());
    ASSERT_DOUBLE_EQ(-5.5, vd.y());
    ASSERT_DOUBLE_EQ(8.0, vd.z());

    vd.x() = 0.0;
    vd.y() = 0.0;
    vd.z() = 0.0;
    ASSERT_TRUE(vd.isZero());

    vd.y() = 0.1;
    ASSERT_FALSE(vd.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, IndexOperator)
{
    Vec3d v;
    
    v[0] = 1.0;
    v[1] = 2.0;
    v[2] = 3.0;

    EXPECT_EQ(1.0, v[0]);
    EXPECT_EQ(2.0, v[1]);
    EXPECT_EQ(3.0, v[2]);

    Vec3f vf;

    vf[0] = 1.0f;
    vf[1] = 2.0f;
    vf[2] = 3.0f;

    EXPECT_EQ(1.0f, vf[0]);
    EXPECT_EQ(2.0f, vf[1]);
    EXPECT_EQ(3.0f, vf[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(Vector3DeathTest, IndexOperator)
{
    Vec3d v;

    EXPECT_DEATH(v[-1], "Assertion");
    EXPECT_DEATH(v[3], "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Conversion)
{
    Vec3f vf1(1.0f, 2.0f, 3.0f);
    Vec3d vd1(vf1);
    Vec3f vf2(4.0f, 5.0f, 6.0f);
    Vec3d vd2;
    vd2.set(vf2);

    ASSERT_FLOAT_EQ(1.0, vf1.x());
    ASSERT_FLOAT_EQ(2.0, vf1.y());
    ASSERT_FLOAT_EQ(3.0, vf1.z());
    ASSERT_DOUBLE_EQ(1.0, vd1.x());
    ASSERT_DOUBLE_EQ(2.0, vd1.y());
    ASSERT_DOUBLE_EQ(3.0, vd1.z());

    ASSERT_FLOAT_EQ(4.0, vf2.x());
    ASSERT_FLOAT_EQ(5.0, vf2.y());
    ASSERT_FLOAT_EQ(6.0, vf2.z());
    ASSERT_DOUBLE_EQ(4.0, vd2.x());
    ASSERT_DOUBLE_EQ(5.0, vd2.y());
    ASSERT_DOUBLE_EQ(6.0, vd2.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, DataAccess)
{
    Vec3f vf(1.0f, 2.5f, 3.0f);
    Vec3d vd(4,5.5,6);
    
    float* pfData = vf.ptr();
    double* pdData = vd.ptr();

    ASSERT_EQ(1.0f, pfData[0]);
    ASSERT_EQ(2.5f, pfData[1]);
    ASSERT_EQ(3.0f, pfData[2]);
    ASSERT_EQ(vf.x(), pfData[0]);
    ASSERT_EQ(vf.y(), pfData[1]);
    ASSERT_EQ(vf.z(), pfData[2]);

    ASSERT_EQ(4.0, pdData[0]);
    ASSERT_EQ(5.5, pdData[1]);
    ASSERT_EQ(6.0, pdData[2]);
    ASSERT_EQ(vf.x(), pfData[0]);
    ASSERT_EQ(vf.y(), pfData[1]);
    ASSERT_EQ(vf.z(), pfData[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Length)
{
    Vec3f vf(1.0f, 2.0f, 3.0f);
    Vec3d vd(4,5,6);
    Vec3d vzero(0,0,0);

    ASSERT_FLOAT_EQ(3.7416575f, vf.length());
    ASSERT_DOUBLE_EQ(8.77496438739212206, vd.length());
    ASSERT_DOUBLE_EQ(0.0, vzero.length());

    ASSERT_FLOAT_EQ(14.0f, vf.lengthSquared());
    ASSERT_DOUBLE_EQ(77.0, vd.lengthSquared());
    ASSERT_DOUBLE_EQ(0.0, vzero.lengthSquared());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, LengthOfConstants)
{
    const Vec3d vx = Vec3d::X_AXIS;
    const Vec3d vy = Vec3d::Y_AXIS;
    const Vec3d vz = Vec3d::Z_AXIS;
    ASSERT_DOUBLE_EQ(1, vx.length());
    ASSERT_DOUBLE_EQ(1, vy.length());
    ASSERT_DOUBLE_EQ(1, vz.length());

    const Vec3d vxneg = -vx;
    const Vec3d vyneg = -vy;
    const Vec3d vzneg = -vz;
    ASSERT_DOUBLE_EQ(1, vxneg.length());
    ASSERT_DOUBLE_EQ(1, vyneg.length());
    ASSERT_DOUBLE_EQ(1, vzneg.length());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, SetLength)
{
    Vec3d vx = Vec3d::X_AXIS;
    vx.setLength(10);
    ASSERT_DOUBLE_EQ(10, vx.length());

    Vec3d vy = Vec3d::Y_AXIS;
    vy.setLength(20);
    ASSERT_DOUBLE_EQ(20, vy.length());

    Vec3d vz = Vec3d::Z_AXIS;
    vz.setLength(30);
    ASSERT_DOUBLE_EQ(30, vz.length());


    Vec3d v0(0, 0, 0);
    ASSERT_FALSE(v0.setLength(2));
    ASSERT_DOUBLE_EQ(0, v0.length());

    Vec3d v1(1, 1, 1);
    ASSERT_TRUE(v1.setLength(0));
    ASSERT_DOUBLE_EQ(0, v1.length());

    Vec3d v2(1, 1, 1);
    ASSERT_TRUE(v2.setLength(12));
    ASSERT_DOUBLE_EQ(12, v2.length());

    Vec3d v3(4, -5, 6);
    ASSERT_TRUE(v3.setLength(33));
    ASSERT_DOUBLE_EQ(33, v3.length());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Addition)
{
    Vec3f f1(1.0f, 2.6f, 3.0f);
    Vec3f f2(4.0f,5.2f,6.0f);
    Vec3f fa = f1 + f2;

    ASSERT_FLOAT_EQ(5.0f, fa.x());
    ASSERT_FLOAT_EQ(7.8f, fa.y());
    ASSERT_FLOAT_EQ(9.0f, fa.z());

    fa += f1;

    ASSERT_FLOAT_EQ(6.0f, fa.x());
    ASSERT_FLOAT_EQ(10.4f, fa.y());
    ASSERT_FLOAT_EQ(12.0f, fa.z());

    fa.add(f2);

    ASSERT_FLOAT_EQ(10.0f, fa.x());
    ASSERT_FLOAT_EQ(15.6f, fa.y());
    ASSERT_FLOAT_EQ(18.0f, fa.z());

    // double
    Vec3d d1(1.0, 2.6, 3.0);
    Vec3d d2(4,5.2,6);
    Vec3d da = d1 + d2;

    ASSERT_DOUBLE_EQ(5.0, da.x());
    ASSERT_DOUBLE_EQ(7.8, da.y());
    ASSERT_DOUBLE_EQ(9.0, da.z());

    da += d1;

    ASSERT_DOUBLE_EQ(6.0, da.x());
    ASSERT_DOUBLE_EQ(10.4, da.y());
    ASSERT_DOUBLE_EQ(12.0, da.z());

    da.add(d2);

    ASSERT_DOUBLE_EQ(10.0, da.x());
    ASSERT_DOUBLE_EQ(15.6, da.y());
    ASSERT_DOUBLE_EQ(18.0, da.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Subtract)
{
    Vec3f f1(1.0f, 2.6f, 3.0f);
    Vec3f f2(4.0f,5.2f,6.0f);
    Vec3f fa = f1 - f2;

    ASSERT_FLOAT_EQ(-3.0f, fa.x());
    ASSERT_FLOAT_EQ(-2.6f, fa.y());
    ASSERT_FLOAT_EQ(-3.0f, fa.z());

    fa -= f1;

    ASSERT_FLOAT_EQ(-4.0f, fa.x());
    ASSERT_FLOAT_EQ(-5.2f, fa.y());
    ASSERT_FLOAT_EQ(-6.0f, fa.z());

    fa.subtract(f2);

    ASSERT_FLOAT_EQ(-8.0f, fa.x());
    ASSERT_FLOAT_EQ(-10.4f, fa.y());
    ASSERT_FLOAT_EQ(-12.0f, fa.z());

    // double
    Vec3d d1(1.0, 2.6, 3.0);
    Vec3d d2(4,5.2,6);
    Vec3d da = d1 - d2;

    ASSERT_DOUBLE_EQ(-3.0, da.x());
    ASSERT_DOUBLE_EQ(-2.6, da.y());
    ASSERT_DOUBLE_EQ(-3.0, da.z());

    da -= d1;

    ASSERT_DOUBLE_EQ(-4.0, da.x());
    ASSERT_DOUBLE_EQ(-5.2, da.y());
    ASSERT_DOUBLE_EQ(-6.0, da.z());

    da.subtract(d2);

    ASSERT_DOUBLE_EQ(-8.0, da.x());
    ASSERT_DOUBLE_EQ(-10.4, da.y());
    ASSERT_DOUBLE_EQ(-12.0, da.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Scale)
{
    Vec3d d2(1.0,2.0,3.7);

    d2.scale(2.3);
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());
    ASSERT_DOUBLE_EQ(8.51, d2.z());

    d2.scale(0.0);
    ASSERT_TRUE(d2.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Multiply)
{
    // float
    Vec3f f1(1.0f,2.0f,3.7f);
    
    Vec3f f2 = f1*2.3f;
    ASSERT_FLOAT_EQ(2.3f, f2.x());
    ASSERT_FLOAT_EQ(4.6f, f2.y());
    ASSERT_FLOAT_EQ(8.51f, f2.z());

    f2 = 2.3f*f1;
    ASSERT_FLOAT_EQ(2.3f, f2.x());
    ASSERT_FLOAT_EQ(4.6f, f2.y());
    ASSERT_FLOAT_EQ(8.51f, f2.z());

    f2 *= 3.5f;
    ASSERT_FLOAT_EQ(8.05f, f2.x());
    ASSERT_FLOAT_EQ(16.1f, f2.y());
    ASSERT_FLOAT_EQ(29.785f, f2.z());
    
    f2 *= 0.0f;
    ASSERT_TRUE(f2.isZero());

    // double
    Vec3d d1(1.0,2.0,3.7);
    
    Vec3d d2 = d1*2.3;
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());
    ASSERT_DOUBLE_EQ(8.51, d2.z());

    d2 = 2.3*d1;
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());
    ASSERT_DOUBLE_EQ(8.51, d2.z());

    d2 *= 3.5;
    ASSERT_DOUBLE_EQ(8.05, d2.x());
    ASSERT_DOUBLE_EQ(16.1, d2.y());
    ASSERT_DOUBLE_EQ(29.785, d2.z());

    d2 *= 0.0;
    ASSERT_TRUE(d2.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Divide)
{
    // float
    Vec3f f1(1.0f,2.0f,3.7f);
    Vec3f f2 = f1*2.3f;
    ASSERT_FLOAT_EQ(2.3f, f2.x());
    ASSERT_FLOAT_EQ(4.6f, f2.y());
    ASSERT_FLOAT_EQ(8.51f, f2.z());

    f2 *= 3.5f;
    ASSERT_FLOAT_EQ(8.05f, f2.x());
    ASSERT_FLOAT_EQ(16.1f, f2.y());
    ASSERT_FLOAT_EQ(29.785f, f2.z());

    // double
    Vec3d d1(1.0,2.0,3.7);
    Vec3d d2 = d1*2.3;
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());
    ASSERT_DOUBLE_EQ(8.51, d2.z());

    d2 *= 3.5;
    ASSERT_DOUBLE_EQ(8.05, d2.x());
    ASSERT_DOUBLE_EQ(16.1, d2.y());
    ASSERT_DOUBLE_EQ(29.785, d2.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Negate)
{
    Vec3f f(1.0f, -2.0f, 3.7f);
    Vec3f fn = -f;
    ASSERT_FLOAT_EQ(-1.0f, fn.x());
    ASSERT_FLOAT_EQ( 2.0f, fn.y());
    ASSERT_FLOAT_EQ(-3.7f, fn.z());

    Vec3d d(1.0, -2.0, 3.7);
    Vec3d dn = -d;
    ASSERT_DOUBLE_EQ(-1.0, dn.x());
    ASSERT_DOUBLE_EQ( 2.0, dn.y());
    ASSERT_DOUBLE_EQ(-3.7, dn.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, CrossProduct)
{
    // float
    const Vec3f f1(1.0f,2.0f,3.7f);
    const Vec3f f2(5.0f,3.2f, 2.3f);

    Vec3f f3 = f1^f2;
    ASSERT_FLOAT_EQ(-7.24f, f3.x());
    ASSERT_FLOAT_EQ(16.2f, f3.y());
    ASSERT_FLOAT_EQ(-6.8f, f3.z());

    Vec3f f4 = f2^f1;
    ASSERT_FLOAT_EQ(7.24f, f4.x());
    ASSERT_FLOAT_EQ(-16.2f, f4.y());
    ASSERT_FLOAT_EQ(6.8f, f4.z());

    Vec3f f5;
    f5.cross(f2, f1);
    ASSERT_FLOAT_EQ(7.24f, f5.x());
    ASSERT_FLOAT_EQ(-16.2f, f5.y());
    ASSERT_FLOAT_EQ(6.8f, f5.z());


    // double
    const Vec3d d1(1.0,2.0,3.7);
    const Vec3d d2(5.0,3.2, 2.3);

    Vec3d d3 = d1^d2;
    ASSERT_DOUBLE_EQ(-7.24, d3.x());
    ASSERT_DOUBLE_EQ(16.2, d3.y());
    ASSERT_DOUBLE_EQ(-6.8, d3.z());

    Vec3d d4 = d2^d1;
    ASSERT_DOUBLE_EQ(7.24, d4.x());
    ASSERT_DOUBLE_EQ(-16.2, d4.y());
    ASSERT_DOUBLE_EQ(6.8, d4.z());

    Vec3d d5;
    d5.cross(d2, d1);
    ASSERT_DOUBLE_EQ(7.24, d5.x());
    ASSERT_DOUBLE_EQ(-16.2, d5.y());
    ASSERT_DOUBLE_EQ(6.8, d5.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, DotProduct)
{
    // float
    Vec3f f1(1.0f,2.0f,3.7f);
    Vec3f f2(5,3.2f, 2.3f);

    float fdot1 = f1*f2;
    ASSERT_FLOAT_EQ(19.91f, fdot1);

    float fdot2 = f2.dot(f1);
    ASSERT_FLOAT_EQ(19.91f, fdot2);

    // Double
    Vec3d d1(1.0,2.0,3.7);
    Vec3d d2(5,3.2, 2.3);

    double ddot1 = d1*d2;
    ASSERT_DOUBLE_EQ(19.91, ddot1);

    double ddot2 = d2.dot(d1);
    ASSERT_DOUBLE_EQ(19.91, ddot2);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Normalize)
{
    // float
    Vec3f vf(1.5f, 2.5f, 3.2f);
    Vec3f vfz(0.0f, 0.0f, 0.0f);

    ASSERT_TRUE(vf.normalize());
    ASSERT_FALSE(vfz.normalize());

    ASSERT_FLOAT_EQ(1.0f, vf.length());
    ASSERT_FLOAT_EQ(0.0f, vfz.length());

    // double
    Vec3d vd(1.5, 2.5, 3.2);
    Vec3d vdz(0.0, 0.0, 0.0);

    ASSERT_TRUE(vd.normalize());
    ASSERT_FALSE(vdz.normalize());

    ASSERT_DOUBLE_EQ(1.0, vd.length());
    ASSERT_DOUBLE_EQ(0.0, vdz.length());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, GetNormalized)
{
    {
        const Vec3f v(1.5f, 2.5f, 3.2f);
        const Vec3f vz(0.0f, 0.0f, 0.0f);

        Vec3f n1 = v.getNormalized();
        ASSERT_FLOAT_EQ(1.0f, n1.length());

        bool normOK = false;
        Vec3f n2 = v.getNormalized(&normOK);
        ASSERT_TRUE(normOK);
        ASSERT_FLOAT_EQ(1.0f, n2.length());

        Vec3f nz = vz.getNormalized(&normOK);
        ASSERT_FALSE(normOK);
        ASSERT_FLOAT_EQ(0.0f, nz.length());
    }

    {
        const Vec3d v(1.5, 2.5, 3.2);
        const Vec3d vz(0.0, 0.0, 0.0);

        Vec3d n1 = v.getNormalized();
        ASSERT_DOUBLE_EQ(1.0, n1.length());

        bool normOK = false;
        Vec3d n2 = v.getNormalized(&normOK);
        ASSERT_TRUE(normOK);
        ASSERT_DOUBLE_EQ(1.0, n2.length());

        Vec3d nz = vz.getNormalized(&normOK);
        ASSERT_FALSE(normOK);
        ASSERT_DOUBLE_EQ(0.0, nz.length());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Comparison)
{
    // float
    Vec3f f1(1,2,3);
    Vec3f f2(1,2,3);
    Vec3f f3(1,2,3.1f);

    ASSERT_TRUE(f1.equals(f2));
    ASSERT_TRUE(f1 == f2);
    ASSERT_FALSE(f1 != f2);
    ASSERT_FALSE(f1 == f3);
    ASSERT_TRUE(f1 != f3);

    // double
    Vec3d d1(1,2,3);
    Vec3d d2(1,2,3);
    Vec3d d3(1,2,3.1f);

    ASSERT_TRUE(d1.equals(d2));
    ASSERT_TRUE(d1 == d2);
    ASSERT_FALSE(d1 != d2);
    ASSERT_FALSE(d1 == d3);
    ASSERT_TRUE(d1 != d3);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, PointDistance)
{
    {
        const Vec3d pt1(1, 2, 3);
        const Vec3d pt2(6, 5, 4);

        EXPECT_DOUBLE_EQ(0.0, pt1.pointDistanceSquared(pt1));
        EXPECT_DOUBLE_EQ(0.0, pt2.pointDistanceSquared(pt2));
        EXPECT_DOUBLE_EQ(35.0, pt1.pointDistanceSquared(pt2));
        EXPECT_DOUBLE_EQ(35.0, pt2.pointDistanceSquared(pt1));

        EXPECT_DOUBLE_EQ(0.0, pt1.pointDistance(pt1));
        EXPECT_DOUBLE_EQ(0.0, pt2.pointDistance(pt2));
        EXPECT_DOUBLE_EQ(5.9160797830996161, pt1.pointDistance(pt2));
        EXPECT_DOUBLE_EQ(5.9160797830996161, pt2.pointDistance(pt1));
    }

    {
        const Vec3i pt1(1, 2, 3);
        const Vec3i pt2(6, 5, 4);

        EXPECT_DOUBLE_EQ(0.0, pt1.pointDistanceSquared(pt1));
        EXPECT_DOUBLE_EQ(0.0, pt2.pointDistanceSquared(pt2));
        EXPECT_DOUBLE_EQ(35.0, pt1.pointDistanceSquared(pt2));
        EXPECT_DOUBLE_EQ(35.0, pt2.pointDistanceSquared(pt1));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, Transform)
{
    // float
    Vec3f f1(1,2.5, -3);
    Matrix4<float> M;
    
    f1.transformPoint(M);
    
    ASSERT_FLOAT_EQ(1.0f, f1.x());
    ASSERT_FLOAT_EQ(2.5f, f1.y());
    ASSERT_FLOAT_EQ(-3.0f, f1.z());

    f1.transformVector(M);

    ASSERT_FLOAT_EQ(1.0f, f1.x());
    ASSERT_FLOAT_EQ(2.5f, f1.y());
    ASSERT_FLOAT_EQ(-3.0f, f1.z());

    Vec3f v(2.5f, 2.2f, 6.2f);
    Matrix4<float> m;
    m.setRowCol(2, 0, 1.5f);
    m.setRowCol(3, 2, 2.3f);
    m.setRowCol(1, 3, 1.5f);
    m.setRowCol(0, 1, 2.0f);

    Vec3f v1 = v;
    v1.transformPoint(m);

    Vec3f v2 = v;
    v2.transformVector(m);

    ASSERT_FLOAT_EQ(6.9f, v1.x());
    ASSERT_FLOAT_EQ(3.7f, v1.y());
    ASSERT_FLOAT_EQ(9.95f, v1.z());

    ASSERT_FLOAT_EQ(6.9f, v2.x());
    ASSERT_FLOAT_EQ(2.2f, v2.y());
    ASSERT_FLOAT_EQ(9.95f, v2.z());

    // double
    Vec3d d1(1,2.5, -3);
    Matrix4<double> dM;

    d1.transformPoint(dM);

    ASSERT_DOUBLE_EQ(1.0f, d1.x());
    ASSERT_DOUBLE_EQ(2.5f, d1.y());
    ASSERT_DOUBLE_EQ(-3.0f, d1.z());

    d1.transformVector(dM);

    ASSERT_DOUBLE_EQ(1.0f, d1.x());
    ASSERT_DOUBLE_EQ(2.5f, d1.y());
    ASSERT_DOUBLE_EQ(-3.0f, d1.z());

    Vec3d d(2.5, 2.2, 6.2);
    Matrix4<double> dm;
    dm.setRowCol(2, 0, 1.5);
    dm.setRowCol(3, 2, 2.3);
    dm.setRowCol(1, 3, 1.5);
    dm.setRowCol(0, 1, 2.0);

    Vec3d dd1 = d;
    dd1.transformPoint(dm);

    Vec3d d2 = d;
    d2.transformVector(dm);

    ASSERT_DOUBLE_EQ(6.9, dd1.x());
    ASSERT_DOUBLE_EQ(3.7, dd1.y());
    ASSERT_DOUBLE_EQ(9.95, dd1.z());

    ASSERT_DOUBLE_EQ(6.9, d2.x());
    ASSERT_DOUBLE_EQ(2.2, d2.y());
    ASSERT_DOUBLE_EQ(9.95, d2.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, TransformVsTransformed)
{
    {
        const Vec3d inp(1, 2, 3);
        const Mat4d m1( 1,  2,  3,  4, 
                        5,  6,  7,  8, 
                        9, 10, 11, 12,
                       13, 14, 15, 16);
        
        Vec3d v(inp);
        Vec3d p(inp);
        v.transformVector(m1);
        p.transformPoint(m1);
        Vec3d tv = inp.getTransformedVector(m1);
        Vec3d tp = inp.getTransformedPoint(m1);
        ASSERT_TRUE(v == tv);
        ASSERT_TRUE(p == tp);
    }

    {
        const Vec3f inp(1, 2, 3);
        const Mat4f m1( 1,  2,  3,  4, 
                        5,  6,  7,  8, 
                        9, 10, 11, 12,
                       13, 14, 15, 16);
        
        Vec3f v(inp);
        Vec3f p(inp);
        v.transformVector(m1);
        p.transformPoint(m1);
        Vec3f tv = inp.getTransformedVector(m1);
        Vec3f tp = inp.getTransformedPoint(m1);
        ASSERT_TRUE(v == tv);
        ASSERT_TRUE(p == tp);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, OtherDataTypes)
{
    Vec3i iVec(1,2,3);
    Vec3i iVec2(5,6,7);
    Vec3i iVec3 = iVec + iVec2;

    ASSERT_EQ(6, iVec3.x());
    ASSERT_EQ(8, iVec3.y());
    ASSERT_EQ(10, iVec3.z());
    ASSERT_EQ(200, iVec3.lengthSquared());
    

    typedef Vector3<String> Vec3s;

    Vec3s s1("Yes", "No", "Maybe");
    Vec3s s2("Ja", "Nei", "Kanskje");
    Vec3s s3 = s1 + s2;

    ASSERT_TRUE(s3.x() == "YesJa");
    ASSERT_TRUE(s3.y() == "NoNei");
    ASSERT_TRUE(s3.z() == "MaybeKanskje");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, IsUndefined)
{
    {
        Vec3f v = Vec3f::UNDEFINED;
        EXPECT_TRUE(v.isUndefined());

        v.x() = 0;
        EXPECT_TRUE(v.isUndefined());

        v.y() = 0;
        EXPECT_TRUE(v.isUndefined());

        v.z() = 0;
        EXPECT_FALSE(v.isUndefined());
    }

    {
        Vec3d v = Vec3d::UNDEFINED;
        EXPECT_TRUE(v.isUndefined());

        v.x() = 0;
        EXPECT_TRUE(v.isUndefined());

        v.y() = 0;
        EXPECT_TRUE(v.isUndefined());

        v.z() = 0;
        EXPECT_FALSE(v.isUndefined());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, CreateOrthonormalBasisGlobalAxes)
{
    EXPECT_FALSE(Vec3d::ZERO.createOrthonormalBasis(0, NULL, NULL, NULL));

    Vec3d u(0, 0, 0);  
    Vec3d v(0, 0, 0);  
    Vec3d w(0, 0, 0);

    {
        const Vec3d globAxis = Vec3d::X_AXIS;

        EXPECT_TRUE(globAxis.createOrthonormalBasis(0, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, u*globAxis);
        EXPECT_DOUBLE_EQ(1.0, u*(v^w));

        EXPECT_TRUE(globAxis.createOrthonormalBasis(1, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, v*globAxis);
        EXPECT_DOUBLE_EQ(1.0, v*(w^u));

        EXPECT_TRUE(globAxis.createOrthonormalBasis(2, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, w*globAxis);
        EXPECT_DOUBLE_EQ(1.0, w*(u^v));
    }

    {
        const Vec3d globAxis = -Vec3d::X_AXIS;

        EXPECT_TRUE(globAxis.createOrthonormalBasis(0, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, u*globAxis);
        EXPECT_DOUBLE_EQ(1.0, u*(v^w));

        EXPECT_TRUE(globAxis.createOrthonormalBasis(1, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, v*globAxis);
        EXPECT_DOUBLE_EQ(1.0, v*(w^u));

        EXPECT_TRUE(globAxis.createOrthonormalBasis(2, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, w*globAxis);
        EXPECT_DOUBLE_EQ(1.0, w*(u^v));
    }

    {
        const Vec3d globAxis = Vec3d::Y_AXIS;

        EXPECT_TRUE(globAxis.createOrthonormalBasis(0, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, u*globAxis);
        EXPECT_DOUBLE_EQ(1.0, u*(v^w));

        EXPECT_TRUE(globAxis.createOrthonormalBasis(1, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, v*globAxis);
        EXPECT_DOUBLE_EQ(1.0, v*(w^u));

        EXPECT_TRUE(globAxis.createOrthonormalBasis(2, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, w*globAxis);
        EXPECT_DOUBLE_EQ(1.0, w*(u^v));
    }

    {
        const Vec3d globAxis = -Vec3d::Z_AXIS;

        EXPECT_TRUE(globAxis.createOrthonormalBasis(0, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, u*globAxis);
        EXPECT_DOUBLE_EQ(1.0, u*(v^w));

        EXPECT_TRUE(globAxis.createOrthonormalBasis(1, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, v*globAxis);
        EXPECT_DOUBLE_EQ(1.0, v*(w^u));

        EXPECT_TRUE(globAxis.createOrthonormalBasis(2, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, w*globAxis);
        EXPECT_DOUBLE_EQ(1.0, w*(u^v));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, CreateOrthonormalBasis)
{
    Vec3d u(0, 0, 0);  
    Vec3d v(0, 0, 0);  
    Vec3d w(0, 0, 0);

    {
        const Vec3d vector(1, 2, 3);

        EXPECT_TRUE(vector.createOrthonormalBasis(0, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, u*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, u*(v^w));

        EXPECT_TRUE(vector.createOrthonormalBasis(1, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, v*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, v*(w^u));

        EXPECT_TRUE(vector.createOrthonormalBasis(2, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, w*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, w*(u^v));
    }

    {
        const Vec3d vector(-50, 100, 25);

        EXPECT_TRUE(vector.createOrthonormalBasis(0, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, u*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, u*(v^w));

        EXPECT_TRUE(vector.createOrthonormalBasis(1, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, v*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, v*(w^u));

        EXPECT_TRUE(vector.createOrthonormalBasis(2, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, w*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, w*(u^v));
    }

    {
        const Vec3d vector(-3, 333, 33333);

        EXPECT_TRUE(vector.createOrthonormalBasis(0, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, u*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, u*(v^w));

        EXPECT_TRUE(vector.createOrthonormalBasis(1, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, v*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, v*(w^u));

        EXPECT_TRUE(vector.createOrthonormalBasis(2, &u, &v, &w));
        EXPECT_DOUBLE_EQ(1.0, w*vector.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, w*(u^v));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector3Test, perpendicular)
{
    {
        Vec3f a(Vec3f::X_AXIS);
        bool ok = false;
        Vec3f n = a.perpendicularVector(&ok);
        EXPECT_FLOAT_EQ(0.0f, a*n);
        EXPECT_TRUE(ok);
    }

    {
        Vec3f a(Vec3f::Y_AXIS);
        Vec3f n = a.perpendicularVector();
        EXPECT_FLOAT_EQ(0.0f, a*n);
    }

    {
        Vec3f a(Vec3f::Z_AXIS);
        Vec3f n = a.perpendicularVector();
        EXPECT_FLOAT_EQ(0.0f, a*n);
    }

    {
        Vec3d a(Vec3d::X_AXIS);
        Vec3d n = a.perpendicularVector();
        EXPECT_DOUBLE_EQ(0.0, a*n);
    }

    {
        Vec3d a(Vec3d::Y_AXIS);
        Vec3d n = a.perpendicularVector();
        EXPECT_DOUBLE_EQ(0.0, a*n);
    }

    {
        Vec3d a(Vec3d::Z_AXIS);
        Vec3d n = a.perpendicularVector();
        EXPECT_DOUBLE_EQ(0.0, a*n);
    }

    {
        Vec3f a(Vec3f::ZERO);
        bool ok = false;
        Vec3f n = a.perpendicularVector(&ok);
        EXPECT_TRUE(n.isZero());
        EXPECT_FALSE(ok);
    }
}

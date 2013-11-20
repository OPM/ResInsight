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
#include "cvfVector4.h"
#include "cvfMatrix4.h"
#include "cvfString.h"              // Used for String vector test
#include "cvfMath.h"

#include "gtest/gtest.h"
#include <iostream>

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Constants)
{
    ASSERT_FLOAT_EQ(0.0f, Vec4f::ZERO.x());
    ASSERT_FLOAT_EQ(0.0f, Vec4f::ZERO.y());
    ASSERT_FLOAT_EQ(0.0f, Vec4f::ZERO.z());

    ASSERT_DOUBLE_EQ(0.0, Vec4d::ZERO.x());
    ASSERT_DOUBLE_EQ(0.0, Vec4d::ZERO.y());
    ASSERT_DOUBLE_EQ(0.0, Vec4d::ZERO.z());

    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec4d::UNDEFINED.x());
    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec4d::UNDEFINED.y());
    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec4d::UNDEFINED.z());
    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec4d::UNDEFINED.w());

    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec4f::UNDEFINED.x());
    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec4f::UNDEFINED.y());
    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec4f::UNDEFINED.z());
    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec4f::UNDEFINED.w());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, BasicConstruction)
{
    // float
    Vec4f v;
    v.set(1.0f, 2.5f, 3.0f, 4.0f);

    ASSERT_FLOAT_EQ(1.0f, v.x());
    ASSERT_FLOAT_EQ(2.5f, v.y());
    ASSERT_FLOAT_EQ(3.0f, v.z());
    ASSERT_FLOAT_EQ(4.0f, v.w());

    Vec4f v2;
    v2.setZero();

    ASSERT_FLOAT_EQ(0.0f, v2.x());
    ASSERT_FLOAT_EQ(0.0f, v2.y());
    ASSERT_FLOAT_EQ(0.0f, v2.z());
    ASSERT_FLOAT_EQ(0.0f, v2.w());

    ASSERT_TRUE(v2.isZero());
    ASSERT_FALSE(v.isZero());

    // double
    Vec4d d;
    d.set(1.0, 2.5, 3.0, 4.0);

    ASSERT_DOUBLE_EQ(1.0, d.x());
    ASSERT_DOUBLE_EQ(2.5, d.y());
    ASSERT_DOUBLE_EQ(3.0, d.z());
    ASSERT_DOUBLE_EQ(4.0, d.w());

    Vec4f d2;
    d2.setZero();

    ASSERT_DOUBLE_EQ(0.0, d2.x());
    ASSERT_DOUBLE_EQ(0.0, d2.y());
    ASSERT_DOUBLE_EQ(0.0, d2.z());
    ASSERT_DOUBLE_EQ(0.0, d2.w());

    ASSERT_TRUE(d2.isZero());
    ASSERT_FALSE(d.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, CopyConstructor)
{
    // float
    Vec4f v1(1.0f, 2.5f, 3.0f, 4.0f);
    Vec4f v2(v1);
    Vec4f v3 = v1;
    Vec4f v4;
    v4 = v1;

    ASSERT_FLOAT_EQ(1.0f, v1.x());
    ASSERT_FLOAT_EQ(2.5f, v1.y());
    ASSERT_FLOAT_EQ(3.0f, v1.z());
    ASSERT_FLOAT_EQ(4.0f, v1.w());
    ASSERT_FLOAT_EQ(1.0f, v2.x());
    ASSERT_FLOAT_EQ(2.5f, v2.y());
    ASSERT_FLOAT_EQ(3.0f, v2.z());
    ASSERT_FLOAT_EQ(4.0f, v2.w());
    ASSERT_FLOAT_EQ(1.0f, v3.x());
    ASSERT_FLOAT_EQ(2.5f, v3.y());
    ASSERT_FLOAT_EQ(3.0f, v3.z());
    ASSERT_FLOAT_EQ(4.0f, v3.w());
    ASSERT_FLOAT_EQ(1.0f, v4.x());
    ASSERT_FLOAT_EQ(2.5f, v4.y());
    ASSERT_FLOAT_EQ(3.0f, v4.z());
    ASSERT_FLOAT_EQ(4.0f, v4.w());

    // double
    Vec4d d1(1.0, 2.5, 3.0, 4.0);
    Vec4d d2(d1);
    Vec4d d3 = d1;
    Vec4d d4;
    d4 = d1;

    ASSERT_DOUBLE_EQ(1.0, d1.x());
    ASSERT_DOUBLE_EQ(2.5, d1.y());
    ASSERT_DOUBLE_EQ(3.0, d1.z());
    ASSERT_DOUBLE_EQ(4.0, d1.w());
    ASSERT_DOUBLE_EQ(1.0, d2.x());
    ASSERT_DOUBLE_EQ(2.5, d2.y());
    ASSERT_DOUBLE_EQ(3.0, d2.z());
    ASSERT_DOUBLE_EQ(4.0, d2.w());
    ASSERT_DOUBLE_EQ(1.0, d3.x());
    ASSERT_DOUBLE_EQ(2.5, d3.y());
    ASSERT_DOUBLE_EQ(3.0, d3.z());
    ASSERT_DOUBLE_EQ(4.0, d3.w());
    ASSERT_DOUBLE_EQ(1.0, d4.x());
    ASSERT_DOUBLE_EQ(2.5, d4.y());
    ASSERT_DOUBLE_EQ(3.0, d4.z());
    ASSERT_DOUBLE_EQ(4.0, d4.w());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, ConstructionFromScalars)
{
    float data[] = {1.0f,2.2f,3.0f,4.0f};

    Vec4f vf = Vec4f(data[0], data[1], data[2], data[3]);

    EXPECT_EQ(1.0f, vf[0]);
    EXPECT_EQ(2.2f, vf[1]);
    EXPECT_EQ(3.0f, vf[2]);
    EXPECT_EQ(4.0f, vf[3]);

    double datad[] = {1.0,2.2,3.0,4.0};

    Vec4d vd = Vec4d(datad[0], datad[1], datad[2], datad[3]);

    EXPECT_EQ(1.0, vd[0]);
    EXPECT_EQ(2.2, vd[1]);
    EXPECT_EQ(3.0, vd[2]);
    EXPECT_EQ(4.0, vd[3]);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, AssignMemebers)
{
    // float
    Vec4f vf;

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
    Vec4d vd;

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
TEST(Vector4Test, IndexOperator)
{
    Vec4d v;

    v[0] = 1.0;
    v[1] = 2.0;
    v[2] = 3.0;
    v[3] = 4.0;

    EXPECT_EQ(1.0, v[0]);
    EXPECT_EQ(2.0, v[1]);
    EXPECT_EQ(3.0, v[2]);
    EXPECT_EQ(4.0, v[3]);

    Vec4f vf;

    vf[0] = 1.0f;
    vf[1] = 2.0f;
    vf[2] = 3.0f;
    vf[3] = 4.0f;

    EXPECT_EQ(1.0f, vf[0]);
    EXPECT_EQ(2.0f, vf[1]);
    EXPECT_EQ(3.0f, vf[2]);
    EXPECT_EQ(4.0f, vf[3]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(Vector4DeathTest, IndexOperator)
{
    Vec3d v;

    EXPECT_DEATH(v[-1], "Assertion");
    EXPECT_DEATH(v[3], "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Conversion)
{
    Vec4f vf1(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4d vd1(vf1);
    Vec4f vf2(4.0f, 5.0f, 6.0f, 7.0f);
    Vec4d vd2;
    vd2.set(vf2);

    ASSERT_FLOAT_EQ(1.0, vf1.x());
    ASSERT_FLOAT_EQ(2.0, vf1.y());
    ASSERT_FLOAT_EQ(3.0, vf1.z());
    ASSERT_FLOAT_EQ(4.0, vf1.w());
    ASSERT_DOUBLE_EQ(1.0, vd1.x());
    ASSERT_DOUBLE_EQ(2.0, vd1.y());
    ASSERT_DOUBLE_EQ(3.0, vd1.z());
    ASSERT_DOUBLE_EQ(4.0, vd1.w());

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
TEST(Vector4Test, ConversionFromVector3)
{
    Vec3d vec3(1, 2, 3);

    Vec4d v(vec3, 9.0);
    EXPECT_DOUBLE_EQ(1.0, v.x());
    EXPECT_DOUBLE_EQ(2.0, v.y());
    EXPECT_DOUBLE_EQ(3.0, v.z());
    EXPECT_DOUBLE_EQ(9.0, v.w());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, DataAccess)
{
    Vec4f vf(1.0f, 2.5f, 3.0f, 4.0f);
    Vec4d vd(4,5.5,6,7);
    
    float* pfData = vf.ptr();
    double* pdData = vd.ptr();

    ASSERT_EQ(1.0f, pfData[0]);
    ASSERT_EQ(2.5f, pfData[1]);
    ASSERT_EQ(3.0f, pfData[2]);
    ASSERT_EQ(4.0f, pfData[3]);
    ASSERT_EQ(vf.x(), pfData[0]);
    ASSERT_EQ(vf.y(), pfData[1]);
    ASSERT_EQ(vf.z(), pfData[2]);
    ASSERT_EQ(vf.w(), pfData[3]);

    ASSERT_EQ(4.0, pdData[0]);
    ASSERT_EQ(5.5, pdData[1]);
    ASSERT_EQ(6.0, pdData[2]);
    ASSERT_EQ(7.0, pdData[3]);
    ASSERT_EQ(vf.x(), pfData[0]);
    ASSERT_EQ(vf.y(), pfData[1]);
    ASSERT_EQ(vf.z(), pfData[2]);
    ASSERT_EQ(vf.w(), pfData[3]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Length)
{
    Vec4f vf(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4d vd(4,5,6,7);
    Vec4d vzero(0,0,0,0);

    ASSERT_FLOAT_EQ(5.4772255f, vf.length());
    ASSERT_DOUBLE_EQ(11.22497216032182415675, vd.length());
    ASSERT_DOUBLE_EQ(0.0, vzero.length());

    ASSERT_FLOAT_EQ(30.0f, vf.lengthSquared());
    ASSERT_DOUBLE_EQ(126.0, vd.lengthSquared());
    ASSERT_DOUBLE_EQ(0.0, vzero.lengthSquared());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, SetLength)
{
    Vec4d vx = Vec4d(1,0,0,0);
    vx.setLength(10);
    ASSERT_DOUBLE_EQ(10, vx.length());

    Vec4d vy = Vec4d(0,0,0,1);
    vy.setLength(10);
    ASSERT_DOUBLE_EQ(10, vy.length());


    Vec4d v0(0, 0, 0, 0);
    ASSERT_FALSE(v0.setLength(2));
    ASSERT_DOUBLE_EQ(0, v0.length());

    Vec4d v1(1, 1, 1, 1);
    ASSERT_TRUE(v1.setLength(0));
    ASSERT_DOUBLE_EQ(0, v1.length());

    Vec4d v2(1, 1, 1, 1);
    ASSERT_TRUE(v2.setLength(12));
    ASSERT_DOUBLE_EQ(12, v2.length());

    Vec4d v3(4, -5, 6, 5);
    ASSERT_TRUE(v3.setLength(33));
    ASSERT_DOUBLE_EQ(33, v3.length());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Addition)
{
    Vec4f f1(1.0f, 2.6f, 3.0f, 4.0f);
    Vec4f f2(4.0f,5.2f,6.0f, 7.0f);
    Vec4f fa = f1 + f2;

    ASSERT_FLOAT_EQ(5.0f, fa.x());
    ASSERT_FLOAT_EQ(7.8f, fa.y());
    ASSERT_FLOAT_EQ(9.0f, fa.z());
    ASSERT_FLOAT_EQ(11.0f, fa.w());

    fa += f1;

    ASSERT_FLOAT_EQ(6.0f, fa.x());
    ASSERT_FLOAT_EQ(10.4f, fa.y());
    ASSERT_FLOAT_EQ(12.0f, fa.z());
    ASSERT_FLOAT_EQ(15.0f, fa.w());

    fa.add(f2);

    ASSERT_FLOAT_EQ(10.0f, fa.x());
    ASSERT_FLOAT_EQ(15.6f, fa.y());
    ASSERT_FLOAT_EQ(18.0f, fa.z());
    ASSERT_FLOAT_EQ(22.0f, fa.w());

    // double
    Vec4d d1(1.0, 2.6, 3.0, 4.0);
    Vec4d d2(4,5.2,6,7);
    Vec4d da = d1 + d2;

    ASSERT_DOUBLE_EQ(5.0, da.x());
    ASSERT_DOUBLE_EQ(7.8, da.y());
    ASSERT_DOUBLE_EQ(9.0, da.z());
    ASSERT_DOUBLE_EQ(11.0, da.w());

    da += d1;

    ASSERT_DOUBLE_EQ(6.0, da.x());
    ASSERT_DOUBLE_EQ(10.4, da.y());
    ASSERT_DOUBLE_EQ(12.0, da.z());
    ASSERT_DOUBLE_EQ(15.0, da.w());

    da.add(d2);

    ASSERT_DOUBLE_EQ(10.0, da.x());
    ASSERT_DOUBLE_EQ(15.6, da.y());
    ASSERT_DOUBLE_EQ(18.0, da.z());
    ASSERT_DOUBLE_EQ(22.0, da.w());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Subtraction)
{
    Vec4f f1(1.0f, 2.6f, 3.0f, 4.0f);
    Vec4f f2(4.0f,5.2f,6.0f, 7.0f);
    Vec4f fa = f1 - f2;

    ASSERT_FLOAT_EQ(-3.0f, fa.x());
    ASSERT_FLOAT_EQ(-2.6f, fa.y());
    ASSERT_FLOAT_EQ(-3.0f, fa.z());
    ASSERT_FLOAT_EQ(-3.0f, fa.w());

    fa -= f1;

    ASSERT_FLOAT_EQ(-4.0f, fa.x());
    ASSERT_FLOAT_EQ(-5.2f, fa.y());
    ASSERT_FLOAT_EQ(-6.0f, fa.z());
    ASSERT_FLOAT_EQ(-7.0f, fa.w());

    fa.subtract(f2);

    ASSERT_FLOAT_EQ(-8.0f, fa.x());
    ASSERT_FLOAT_EQ(-10.4f, fa.y());
    ASSERT_FLOAT_EQ(-12.0f, fa.z());
    ASSERT_FLOAT_EQ(-14.0f, fa.w());

    // double
    Vec4d d1(1.0, 2.6, 3.0, 4.0);
    Vec4d d2(4,5.2,6,7);
    Vec4d da = d1 - d2;

    ASSERT_DOUBLE_EQ(-3.0, da.x());
    ASSERT_DOUBLE_EQ(-2.6, da.y());
    ASSERT_DOUBLE_EQ(-3.0, da.z());
    ASSERT_DOUBLE_EQ(-3.0, da.w());

    da -= d1;

    ASSERT_DOUBLE_EQ(-4.0, da.x());
    ASSERT_DOUBLE_EQ(-5.2, da.y());
    ASSERT_DOUBLE_EQ(-6.0, da.z());
    ASSERT_DOUBLE_EQ(-7.0, da.w());

    da.subtract(d2);

    ASSERT_DOUBLE_EQ(-8.0, da.x());
    ASSERT_DOUBLE_EQ(-10.4, da.y());
    ASSERT_DOUBLE_EQ(-12.0, da.z());
    ASSERT_DOUBLE_EQ(-14.0, da.w());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Scale)
{
    Vec4d d2(1.0,2.0,3.7, 2.0);
    
    d2.scale(2.3);
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());
    ASSERT_DOUBLE_EQ(8.51, d2.z());
    ASSERT_DOUBLE_EQ(4.6, d2.w());

    d2.scale(0.0);
    ASSERT_TRUE(d2.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Multiply)
{
    // float
    Vec4f f1(1.0f,2.0f,3.7f, 2.0f);
    Vec4f f2 = f1*2.3f;
    ASSERT_FLOAT_EQ(2.3f, f2.x());
    ASSERT_FLOAT_EQ(4.6f, f2.y());
    ASSERT_FLOAT_EQ(8.51f, f2.z());
    ASSERT_FLOAT_EQ(4.6f, f2.w());

    f2 *= 3.5f;
    ASSERT_FLOAT_EQ(8.05f, f2.x());
    ASSERT_FLOAT_EQ(16.1f, f2.y());
    ASSERT_FLOAT_EQ(29.785f, f2.z());
    ASSERT_FLOAT_EQ(16.1f, f2.w());
    
    f2 *= 0.0f;
    ASSERT_TRUE(f2.isZero());

    // double
    Vec4d d1(1.0,2.0,3.7, 2.0);
    Vec4d d2 = d1*2.3;
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());
    ASSERT_DOUBLE_EQ(8.51, d2.z());
    ASSERT_DOUBLE_EQ(4.6, d2.w());

    d2 *= 3.5;
    ASSERT_DOUBLE_EQ(8.05, d2.x());
    ASSERT_DOUBLE_EQ(16.1, d2.y());
    ASSERT_DOUBLE_EQ(29.785, d2.z());
    ASSERT_DOUBLE_EQ(16.1, d2.w());

    d2 *= 0.0;
    ASSERT_TRUE(d2.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Divide)
{
    // float
    Vec4f f1(1.0f,2.0f,3.7f, 2.0f);
    Vec4f f2 = f1*2.3f;
    ASSERT_FLOAT_EQ(2.3f, f2.x());
    ASSERT_FLOAT_EQ(4.6f, f2.y());
    ASSERT_FLOAT_EQ(8.51f, f2.z());
    ASSERT_FLOAT_EQ(4.6f, f2.w());

    f2 *= 3.5f;
    ASSERT_FLOAT_EQ(8.05f, f2.x());
    ASSERT_FLOAT_EQ(16.1f, f2.y());
    ASSERT_FLOAT_EQ(29.785f, f2.z());
    ASSERT_FLOAT_EQ(16.1f, f2.w());

    // double
    Vec4d d1(1.0,2.0,3.7, 2.0);
    Vec4d d2 = d1*2.3;
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());
    ASSERT_DOUBLE_EQ(8.51, d2.z());
    ASSERT_DOUBLE_EQ(4.6, d2.w());

    d2 *= 3.5;
    ASSERT_DOUBLE_EQ(8.05, d2.x());
    ASSERT_DOUBLE_EQ(16.1, d2.y());
    ASSERT_DOUBLE_EQ(29.785, d2.z());
    ASSERT_DOUBLE_EQ(16.1, d2.w());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Negate)
{
    Vec4f f(1.0f, -2.0f, 3.7f, 3.0f);
    Vec4f fn = -f;
    ASSERT_FLOAT_EQ(-1.0f, fn.x());
    ASSERT_FLOAT_EQ( 2.0f, fn.y());
    ASSERT_FLOAT_EQ(-3.7f, fn.z());
    ASSERT_FLOAT_EQ(-3.0f, fn.w());

    Vec4d d(1.0, -2.0, 3.7, 3.0);
    Vec4d dn = -d;
    ASSERT_DOUBLE_EQ(-1.0, dn.x());
    ASSERT_DOUBLE_EQ( 2.0, dn.y());
    ASSERT_DOUBLE_EQ(-3.7, dn.z());
    ASSERT_DOUBLE_EQ(-3.0, dn.w());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, DotProduct)
{
    // float
    Vec4f f1(1.0f,2.0f,3.7f, 2.0f);
    Vec4f f2(5,3.2f, 2.3f, 3.2f);

    float fdot1 = f1*f2;
    ASSERT_FLOAT_EQ(26.31f, fdot1);

    float fdot2 = f2.dot(f1);
    ASSERT_FLOAT_EQ(26.31f, fdot2);

    // Double
    Vec4d d1(1.0, 2.0, 3.7, 2.0);
    Vec4d d2(5, 3.2, 2.3, 3.2);

    double ddot1 = d1*d2;
    ASSERT_DOUBLE_EQ(26.31, ddot1);

    double ddot2 = d2.dot(d1);
    ASSERT_DOUBLE_EQ(26.31, ddot2);

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Normalize)
{
    // float
    Vec4f vf(1.5f, 2.5f, 3.2f, 6.3f);
    Vec4f vfz(0.0f, 0.0f, 0.0f, 0.0f);

    ASSERT_TRUE(vf.normalize());
    ASSERT_FALSE(vfz.normalize());

    ASSERT_FLOAT_EQ(1.0f, vf.length());
    ASSERT_FLOAT_EQ(0.0f, vfz.length());

    // double
    Vec4d vd(1.5, 2.5, 3.2, 6.3);
    Vec4d vdz(0.0, 0.0, 0.0, 0.0);

    ASSERT_TRUE(vd.normalize());
    ASSERT_FALSE(vdz.normalize());

    ASSERT_DOUBLE_EQ(1.0, vd.length());
    ASSERT_DOUBLE_EQ(0.0, vdz.length());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, GetNormalized)
{
    {
        const Vec4f v(1.5f, 2.5f, 3.2f, 6.3f);
        const Vec4f vz(0.0f, 0.0f, 0.0f, 0.0f);

        Vec4f n1 = v.getNormalized();
        ASSERT_FLOAT_EQ(1.0f, n1.length());

        bool normOK = false;
        Vec4f n2 = v.getNormalized(&normOK);
        ASSERT_TRUE(normOK);
        ASSERT_FLOAT_EQ(1.0f, n2.length());

        Vec4f nz = vz.getNormalized(&normOK);
        ASSERT_FALSE(normOK);
        ASSERT_FLOAT_EQ(0.0f, nz.length());
    }

    {
        const Vec4d v(1.5, 2.5, 3.2, 6.3);
        const Vec4d vz(0.0, 0.0, 0.0, 0.0);

        Vec4d n1 = v.getNormalized();
        ASSERT_DOUBLE_EQ(1.0, n1.length());

        bool normOK = false;
        Vec4d n2 = v.getNormalized(&normOK);
        ASSERT_TRUE(normOK);
        ASSERT_DOUBLE_EQ(1.0, n2.length());

        Vec4d nz = vz.getNormalized(&normOK);
        ASSERT_FALSE(normOK);
        ASSERT_DOUBLE_EQ(0.0, nz.length());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, Comparison)
{
    // float
    Vec4f f1(1,2,3,4);
    Vec4f f2(1,2,3,4);
    Vec4f f3(1,2,3.1f,3.4f);

    ASSERT_TRUE(f1.equals(f2));
    ASSERT_TRUE(f1 == f2);
    ASSERT_FALSE(f1 != f2);
    ASSERT_FALSE(f1 == f3);
    ASSERT_TRUE(f1 != f3);

    // double
    Vec4d d1(1,2,3,4);
    Vec4d d2(1,2,3,4);
    Vec4d d3(1,2,3.1f,3.4);

    ASSERT_TRUE(d1.equals(d2));
    ASSERT_TRUE(d1 == d2);
    ASSERT_FALSE(d1 != d2);
    ASSERT_FALSE(d1 == d3);
    ASSERT_TRUE(d1 != d3);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector4Test, OtherDataTypes)
{
    typedef Vector4<int> Vec3i;

    Vec3i iVec(1,2,3,4);
    Vec3i iVec2(5,6,7,8);
    Vec3i iVec3 = iVec + iVec2;

    ASSERT_EQ(6, iVec3.x());
    ASSERT_EQ(8, iVec3.y());
    ASSERT_EQ(10, iVec3.z());
    ASSERT_EQ(12, iVec3.w());
    ASSERT_EQ(344, iVec3.lengthSquared());
    

    typedef Vector4<String> Vec3s;

    Vec3s s1("Yes", "No", "Maybe", "w");
    Vec3s s2("Ja", "Nei", "Kanskje", "W");
    Vec3s s3 = s1 + s2;

    ASSERT_TRUE(s3.x() == "YesJa");
    ASSERT_TRUE(s3.y() == "NoNei");
    ASSERT_TRUE(s3.z() == "MaybeKanskje");
    ASSERT_TRUE(s3.w() == "wW");
}

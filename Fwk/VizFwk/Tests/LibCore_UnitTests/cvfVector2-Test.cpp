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
#include "cvfVector2.h"
#include "cvfMatrix4.h"
#include "cvfString.h"              // Used for String vector 
#include "cvfMath.h"

#include "gtest/gtest.h"
#include <iostream>

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Constants)
{
    EXPECT_FLOAT_EQ(1.0f, Vec2f::X_AXIS.x());
    EXPECT_FLOAT_EQ(0.0f, Vec2f::X_AXIS.y());

    EXPECT_FLOAT_EQ(0.0f, Vec2f::Y_AXIS.x());
    EXPECT_FLOAT_EQ(1.0f, Vec2f::Y_AXIS.y());

    EXPECT_FLOAT_EQ(0.0f, Vec2f::ZERO.x());
    EXPECT_FLOAT_EQ(0.0f, Vec2f::ZERO.y());


    EXPECT_DOUBLE_EQ(1.0, Vec2d::X_AXIS.x());
    EXPECT_DOUBLE_EQ(0.0, Vec2d::X_AXIS.y());

    EXPECT_DOUBLE_EQ(0.0, Vec2d::Y_AXIS.x());
    EXPECT_DOUBLE_EQ(1.0, Vec2d::Y_AXIS.y());

    EXPECT_DOUBLE_EQ(0.0, Vec2d::ZERO.x());
    EXPECT_DOUBLE_EQ(0.0, Vec2d::ZERO.y());

    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec2d::UNDEFINED.x());
    EXPECT_DOUBLE_EQ(UNDEFINED_DOUBLE, Vec2d::UNDEFINED.y());

    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec2f::UNDEFINED.x());
    EXPECT_FLOAT_EQ(UNDEFINED_FLOAT, Vec2f::UNDEFINED.y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, BasicConstruction)
{
    // float
    Vec2f v;
    v.set(1.0f, 2.5f);

    ASSERT_FLOAT_EQ(1.0f, v.x());
    ASSERT_FLOAT_EQ(2.5f, v.y());

    Vec2f v2;
    v2.setZero();

    ASSERT_FLOAT_EQ(0.0f, v2.x());
    ASSERT_FLOAT_EQ(0.0f, v2.y());

    ASSERT_TRUE(v2.isZero());
    ASSERT_FALSE(v.isZero());

    // double
    Vec2d d;
    d.set(1.0, 2.5);

    ASSERT_DOUBLE_EQ(1.0, d.x());
    ASSERT_DOUBLE_EQ(2.5, d.y());

    Vec2f d2;
    d2.setZero();

    ASSERT_DOUBLE_EQ(0.0, d2.x());
    ASSERT_DOUBLE_EQ(0.0, d2.y());

    ASSERT_TRUE(d2.isZero());
    ASSERT_FALSE(d.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, ConstructionFromScalars)
{
    float data[] = {1.0f, 2.2f};

    Vec2f vf = Vec2f(data[0], data[1]);

    EXPECT_EQ(1.0f, vf[0]);
    EXPECT_EQ(2.2f, vf[1]);

    double datad[] = {1.0, 2.2};

    Vec2d vd = Vec2d(datad[0], datad[1]);

    EXPECT_EQ(1.0, vd[0]);
    EXPECT_EQ(2.2, vd[1]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, CopyConstructor)
{
    // float
    Vec2f v1(1.0f, 2.5f);
    Vec2f v2(v1);
    Vec2f v3 = v1;
    Vec2f v4;
    v4 = v1;

    ASSERT_FLOAT_EQ(1.0f, v1.x());
    ASSERT_FLOAT_EQ(2.5f, v1.y());
    ASSERT_FLOAT_EQ(1.0f, v2.x());
    ASSERT_FLOAT_EQ(2.5f, v2.y());
    ASSERT_FLOAT_EQ(1.0f, v3.x());
    ASSERT_FLOAT_EQ(2.5f, v3.y());
    ASSERT_FLOAT_EQ(1.0f, v4.x());
    ASSERT_FLOAT_EQ(2.5f, v4.y());

    // double
    Vec2d d1(1.0, 2.5);
    Vec2d d2(d1);
    Vec2d d3 = d1;
    Vec2d d4;
    d4 = d1;

    ASSERT_DOUBLE_EQ(1.0, d1.x());
    ASSERT_DOUBLE_EQ(2.5, d1.y());
    ASSERT_DOUBLE_EQ(1.0, d2.x());
    ASSERT_DOUBLE_EQ(2.5, d2.y());
    ASSERT_DOUBLE_EQ(1.0, d3.x());
    ASSERT_DOUBLE_EQ(2.5, d3.y());
    ASSERT_DOUBLE_EQ(1.0, d4.x());
    ASSERT_DOUBLE_EQ(2.5, d4.y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, AssignMemebers)
{
    // float
    Vec2f vf;

    vf.x() = 1.0f + 2.5f*3.0f;
    vf.y() = 2.0f - 2.5f*3.0f;

    ASSERT_FLOAT_EQ(8.5f, vf.x());
    ASSERT_FLOAT_EQ(-5.5f, vf.y());

    vf.x() = 0.0f;
    vf.y() = 0.0f;
    ASSERT_TRUE(vf.isZero());

    vf.y() = 0.1f;
    ASSERT_FALSE(vf.isZero());

    // double
    Vec2d vd;

    vd.x() = 1.0 + 2.5*3.0;
    vd.y() = 2.0 - 2.5*3.0;

    ASSERT_DOUBLE_EQ(8.5, vd.x());
    ASSERT_DOUBLE_EQ(-5.5, vd.y());

    vd.x() = 0.0;
    vd.y() = 0.0;
    ASSERT_TRUE(vd.isZero());

    vd.y() = 0.1;
    ASSERT_FALSE(vd.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, IndexOperator)
{
    Vec2d v;
    v[0] = 1.0;
    v[1] = 2.0;
    EXPECT_EQ(1.0, v[0]);
    EXPECT_EQ(2.0, v[1]);

    Vec2f vf;
    vf[0] = 1.0f;
    vf[1] = 2.0f;
    EXPECT_EQ(1.0f, vf[0]);
    EXPECT_EQ(2.0f, vf[1]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(Vector2DeathTest, IndexOperator)
{
    Vec2d v;

    EXPECT_DEATH(v[-1], "Assertion");
    EXPECT_DEATH(v[2], "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Conversion)
{
    Vec2f vf1(1.0f, 2.0f);
    Vec2d vd1(vf1);
    Vec2f vf2(4.0f, 5.0f);
    Vec2d vd2;
    vd2.set(vf2);

    ASSERT_FLOAT_EQ(1.0, vf1.x());
    ASSERT_FLOAT_EQ(2.0, vf1.y());
    ASSERT_DOUBLE_EQ(1.0, vd1.x());
    ASSERT_DOUBLE_EQ(2.0, vd1.y());

    ASSERT_FLOAT_EQ(4.0, vf2.x());
    ASSERT_FLOAT_EQ(5.0, vf2.y());
    ASSERT_DOUBLE_EQ(4.0, vd2.x());
    ASSERT_DOUBLE_EQ(5.0, vd2.y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, DataAccess)
{
    Vec2f vf(1.0f, 2.5f);
    Vec2d vd(4, 5.5);
    
    float* pfData = vf.ptr();
    double* pdData = vd.ptr();

    ASSERT_EQ(1.0f, pfData[0]);
    ASSERT_EQ(2.5f, pfData[1]);
    ASSERT_EQ(vf.x(), pfData[0]);
    ASSERT_EQ(vf.y(), pfData[1]);

    ASSERT_EQ(4.0, pdData[0]);
    ASSERT_EQ(5.5, pdData[1]);
    ASSERT_EQ(vf.x(), pfData[0]);
    ASSERT_EQ(vf.y(), pfData[1]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Length)
{
    Vec2f vf(1.0f, 2.0f);
    Vec2d vd(4, 5);
    Vec2d vzero(0, 0);

    ASSERT_FLOAT_EQ(2.23606798f, vf.length());
    ASSERT_DOUBLE_EQ(6.4031242374328485, vd.length());
    ASSERT_DOUBLE_EQ(0.0, vzero.length());

    ASSERT_FLOAT_EQ(5.0f, vf.lengthSquared());
    ASSERT_DOUBLE_EQ(41.0, vd.lengthSquared());
    ASSERT_DOUBLE_EQ(0.0, vzero.lengthSquared());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, LengthOfConstants)
{
    const Vec2d vx = Vec2d::X_AXIS;
    const Vec2d vy = Vec2d::Y_AXIS;
    ASSERT_DOUBLE_EQ(1, vx.length());
    ASSERT_DOUBLE_EQ(1, vy.length());

    const Vec2d vxneg = -vx;
    const Vec2d vyneg = -vy;
    ASSERT_DOUBLE_EQ(1, vxneg.length());
    ASSERT_DOUBLE_EQ(1, vyneg.length());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, SetLength)
{
    Vec2d vx = Vec2d::X_AXIS;
    vx.setLength(10);
    ASSERT_DOUBLE_EQ(10, vx.length());

    Vec2d vy = Vec2d::Y_AXIS;
    vy.setLength(20);
    ASSERT_DOUBLE_EQ(20, vy.length());


    Vec2d v0(0, 0);
    ASSERT_FALSE(v0.setLength(2));
    ASSERT_DOUBLE_EQ(0, v0.length());

    Vec2d v1(1, 1);
    ASSERT_TRUE(v1.setLength(0));
    ASSERT_DOUBLE_EQ(0, v1.length());

    Vec2d v2(1, 1);
    ASSERT_TRUE(v2.setLength(12));
    ASSERT_DOUBLE_EQ(12, v2.length());

    Vec2d v3(4, -5);
    ASSERT_TRUE(v3.setLength(33));
    ASSERT_DOUBLE_EQ(33, v3.length());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, perpendicularVector)
{
    {
        const Vec2d v(0, 1);
        const Vec2d perp = v.perpendicularVector();
        EXPECT_DOUBLE_EQ(1, perp.x());
        EXPECT_DOUBLE_EQ(0, perp.y());
    }
    {
        const Vec2d v(1, 0);
        const Vec2d perp = v.perpendicularVector();
        EXPECT_DOUBLE_EQ(0, perp.x());
        EXPECT_DOUBLE_EQ(-1, perp.y());
    }
    {
        const Vec2d v(0, -2);
        const Vec2d perp = v.perpendicularVector();
        EXPECT_DOUBLE_EQ(-1, perp.x());
        EXPECT_DOUBLE_EQ(0, perp.y());
    }
    {
        const Vec2d v(-3, 0);
        const Vec2d perp = v.perpendicularVector();
        EXPECT_DOUBLE_EQ(0, perp.x());
        EXPECT_DOUBLE_EQ(1, perp.y());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Addition)
{
    Vec2f f1(1.0f, 2.6f);
    Vec2f f2(4.0f, 5.2f);
    Vec2f fa = f1 + f2;

    ASSERT_FLOAT_EQ(5.0f, fa.x());
    ASSERT_FLOAT_EQ(7.8f, fa.y());

    fa += f1;

    ASSERT_FLOAT_EQ(6.0f, fa.x());
    ASSERT_FLOAT_EQ(10.4f, fa.y());

    fa.add(f2);

    ASSERT_FLOAT_EQ(10.0f, fa.x());
    ASSERT_FLOAT_EQ(15.6f, fa.y());

    // double
    Vec2d d1(1.0, 2.6);
    Vec2d d2(4, 5.2);
    Vec2d da = d1 + d2;

    ASSERT_DOUBLE_EQ(5.0, da.x());
    ASSERT_DOUBLE_EQ(7.8, da.y());

    da += d1;

    ASSERT_DOUBLE_EQ(6.0, da.x());
    ASSERT_DOUBLE_EQ(10.4, da.y());

    da.add(d2);

    ASSERT_DOUBLE_EQ(10.0, da.x());
    ASSERT_DOUBLE_EQ(15.6, da.y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Subtraction)
{
    Vec2f f1(1.0f, 2.6f);
    Vec2f f2(4.0f,5.2f);
    Vec2f fa = f1 - f2;

    ASSERT_FLOAT_EQ(-3.0f, fa.x());
    ASSERT_FLOAT_EQ(-2.6f, fa.y());

    fa -= f1;

    ASSERT_FLOAT_EQ(-4.0f, fa.x());
    ASSERT_FLOAT_EQ(-5.2f, fa.y());

    fa.subtract(f2);

    ASSERT_FLOAT_EQ(-8.0f, fa.x());
    ASSERT_FLOAT_EQ(-10.4f, fa.y());

    // double
    Vec2d d1(1.0, 2.6);
    Vec2d d2(4, 5.2);
    Vec2d da = d1 - d2;

    ASSERT_DOUBLE_EQ(-3.0, da.x());
    ASSERT_DOUBLE_EQ(-2.6, da.y());

    da -= d1;

    ASSERT_DOUBLE_EQ(-4.0, da.x());
    ASSERT_DOUBLE_EQ(-5.2, da.y());

    da.subtract(d2);

    ASSERT_DOUBLE_EQ(-8.0, da.x());
    ASSERT_DOUBLE_EQ(-10.4, da.y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Scale)
{
    Vec2d d2(1.0,2.0);

    d2.scale(2.3);
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());

    d2.scale(0.0);
    ASSERT_TRUE(d2.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Multiply)
{
    // float
    Vec2f f1(1.0f, 2.0f);
    
    Vec2f f2 = f1*2.3f;
    ASSERT_FLOAT_EQ(2.3f, f2.x());
    ASSERT_FLOAT_EQ(4.6f, f2.y());

    f2 = 2.3f*f1;
    ASSERT_FLOAT_EQ(2.3f, f2.x());
    ASSERT_FLOAT_EQ(4.6f, f2.y());

    f2 *= 3.5f;
    ASSERT_FLOAT_EQ(8.05f, f2.x());
    ASSERT_FLOAT_EQ(16.1f, f2.y());
    
    f2 *= 0.0f;
    ASSERT_TRUE(f2.isZero());

    // double
    Vec2d d1(1.0, 2.0);
    
    Vec2d d2 = d1*2.3;
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());

    d2 = 2.3*d1;
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());

    d2 *= 3.5;
    ASSERT_DOUBLE_EQ(8.05, d2.x());
    ASSERT_DOUBLE_EQ(16.1, d2.y());

    d2 *= 0.0;
    ASSERT_TRUE(d2.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Divide)
{
    // float
    Vec2f f1(1.0f, 2.0f);
    Vec2f f2 = f1*2.3f;
    ASSERT_FLOAT_EQ(2.3f, f2.x());
    ASSERT_FLOAT_EQ(4.6f, f2.y());

    f2 *= 3.5f;
    ASSERT_FLOAT_EQ(8.05f, f2.x());
    ASSERT_FLOAT_EQ(16.1f, f2.y());

    // double
    Vec2d d1(1.0, 2.0);
    Vec2d d2 = d1*2.3;
    ASSERT_DOUBLE_EQ(2.3, d2.x());
    ASSERT_DOUBLE_EQ(4.6, d2.y());

    d2 *= 3.5;
    ASSERT_DOUBLE_EQ(8.05, d2.x());
    ASSERT_DOUBLE_EQ(16.1, d2.y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Negate)
{
    Vec2f f(1.0f, -2.0f);
    Vec2f fn = -f;
    ASSERT_FLOAT_EQ(-1.0f, fn.x());
    ASSERT_FLOAT_EQ( 2.0f, fn.y());

    Vec2d d(1.0, -2.0);
    Vec2d dn = -d;
    ASSERT_DOUBLE_EQ(-1.0, dn.x());
    ASSERT_DOUBLE_EQ( 2.0, dn.y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, DotProduct)
{
    // float
    Vec2f f1(1.0f, 2.0f);
    Vec2f f2(5, 3.2f);

    float fdot1 = f1*f2;
    ASSERT_FLOAT_EQ(11.4f, fdot1);

    float fdot2 = f2.dot(f1);
    ASSERT_FLOAT_EQ(11.4f, fdot2);

    // Double
    Vec2d d1(1.0, 2.0);
    Vec2d d2(5, 3.2);

    double ddot1 = d1*d2;
    ASSERT_DOUBLE_EQ(11.4, ddot1);

    double ddot2 = d2.dot(d1);
    ASSERT_DOUBLE_EQ(11.4, ddot2);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Normalize)
{
    // float
    Vec2f vf(1.5f, 2.5f);
    Vec2f vfz(0.0f, 0.0f);

    ASSERT_TRUE(vf.normalize());
    ASSERT_FALSE(vfz.normalize());

    ASSERT_FLOAT_EQ(1.0f, vf.length());
    ASSERT_FLOAT_EQ(0.0f, vfz.length());

    // double
    Vec2d vd(1.5, 2.5);
    Vec2d vdz(0.0, 0.0);

    ASSERT_TRUE(vd.normalize());
    ASSERT_FALSE(vdz.normalize());

    ASSERT_DOUBLE_EQ(1.0, vd.length());
    ASSERT_DOUBLE_EQ(0.0, vdz.length());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, GetNormalized)
{
    {
        const Vec2f v(1.5f, 2.5f);
        const Vec2f vz(0.0f, 0.0f);

        Vec2f n1 = v.getNormalized();
        ASSERT_FLOAT_EQ(1.0f, n1.length());

        bool normOK = false;
        Vec2f n2 = v.getNormalized(&normOK);
        ASSERT_TRUE(normOK);
        ASSERT_FLOAT_EQ(1.0f, n2.length());

        Vec2f nz = vz.getNormalized(&normOK);
        ASSERT_FALSE(normOK);
        ASSERT_FLOAT_EQ(0.0f, nz.length());
    }

    {
        const Vec2d v(1.5, 2.5);
        const Vec2d vz(0.0, 0.0);

        Vec2d n1 = v.getNormalized();
        ASSERT_DOUBLE_EQ(1.0, n1.length());

        bool normOK = false;
        Vec2d n2 = v.getNormalized(&normOK);
        ASSERT_TRUE(normOK);
        ASSERT_DOUBLE_EQ(1.0, n2.length());

        Vec2d nz = vz.getNormalized(&normOK);
        ASSERT_FALSE(normOK);
        ASSERT_DOUBLE_EQ(0.0, nz.length());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, Comparison)
{
    // float
    Vec2f f1(1,3);
    Vec2f f2(1,3);
    Vec2f f3(1,3.1f);

    ASSERT_TRUE(f1.equals(f2));
    ASSERT_TRUE(f1 == f2);
    ASSERT_FALSE(f1 != f2);
    ASSERT_FALSE(f1 == f3);
    ASSERT_TRUE(f1 != f3);

    // double
    Vec2d d1(1,3);
    Vec2d d2(1,3);
    Vec2d d3(1,3.1f);

    ASSERT_TRUE(d1.equals(d2));
    ASSERT_TRUE(d1 == d2);
    ASSERT_FALSE(d1 != d2);
    ASSERT_FALSE(d1 == d3);
    ASSERT_TRUE(d1 != d3);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, OtherDataTypes)
{
    Vec2i iVec(1,2);
    Vec2i iVec2(5,6);
    Vec2i iVec3 = iVec + iVec2;

    ASSERT_EQ(6, iVec3.x());
    ASSERT_EQ(8, iVec3.y());
    ASSERT_EQ(100, iVec3.lengthSquared());
    

    typedef Vector2<String> Vec2s;

    Vec2s s1("Yes", "No");
    Vec2s s2("Ja", "Nei");
    Vec2s s3 = s1 + s2;

    ASSERT_TRUE(s3.x() == "YesJa");
    ASSERT_TRUE(s3.y() == "NoNei");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Vector2Test, IsUndefined)
{
    {
        Vec2f v = Vec2f::UNDEFINED;
        EXPECT_TRUE(v.isUndefined());

        v.x() = 0;
        EXPECT_TRUE(v.isUndefined());

        v.y() = 0;
        EXPECT_FALSE(v.isUndefined());

        v.x() = UNDEFINED_FLOAT;
        EXPECT_TRUE(v.isUndefined());
    }

    {
        Vec2d v = Vec2d::UNDEFINED;
        EXPECT_TRUE(v.isUndefined());

        v.x() = 0;
        EXPECT_TRUE(v.isUndefined());

        v.y() = 0;
        EXPECT_FALSE(v.isUndefined());

        v.x() = UNDEFINED_DOUBLE;
        EXPECT_TRUE(v.isUndefined());
    }
}


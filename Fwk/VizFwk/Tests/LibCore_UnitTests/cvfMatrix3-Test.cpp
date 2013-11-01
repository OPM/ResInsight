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
#include "cvfMatrix3.h"
#include "cvfMath.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, Constants)
{
    Mat3d mdi;
    Mat3d mdz;
    mdz.setZero();

    ASSERT_TRUE(mdi == Mat3d::IDENTITY);
    ASSERT_TRUE(mdz == Mat3d::ZERO);


    Mat3f mfi;
    Mat3f mfz;
    mfz.setZero();

    ASSERT_TRUE(mfi == Mat3f::IDENTITY);
    ASSERT_TRUE(mfz == Mat3f::ZERO);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, DefaultConstructor)
{
    // Constructs using default constructor and manually checks for identity
    Matrix3<float> mf;
    ASSERT_FLOAT_EQ(1, mf.ptr()[0]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[1]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[2]);

    ASSERT_FLOAT_EQ(0, mf.ptr()[3]);
    ASSERT_FLOAT_EQ(1, mf.ptr()[4]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[5]);

    ASSERT_FLOAT_EQ(0, mf.ptr()[6]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[7]);
    ASSERT_FLOAT_EQ(1, mf.ptr()[8]);


    Matrix3<double> md;
    ASSERT_DOUBLE_EQ(1, md.ptr()[0]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[1]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[2]);

    ASSERT_DOUBLE_EQ(0, md.ptr()[3]);
    ASSERT_DOUBLE_EQ(1, md.ptr()[4]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[5]);

    ASSERT_DOUBLE_EQ(0, md.ptr()[6]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[7]);
    ASSERT_DOUBLE_EQ(1, md.ptr()[8]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, CopyConstructor)
{
    Matrix3<double> m1( 1, 4, 7,
                        2, 5, 8,
                        3, 6, 9);

    Matrix3<double> m2(m1);
    m1.setZero();

    const double* pM1 = m1.ptr();
    const double* pM2 = m2.ptr();

    int i;
    for (i = 0; i < 9; i++)
    {
        ASSERT_DOUBLE_EQ(0, pM1[i]);
        ASSERT_DOUBLE_EQ(i + 1, pM2[i]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, AllElementsConstructor)
{
    Matrix3<double> m(  11, 12, 13,
                        21, 22, 23,
                        31, 32, 33);


    ASSERT_DOUBLE_EQ(11, m.ptr()[0]);
    ASSERT_DOUBLE_EQ(21, m.ptr()[1]);
    ASSERT_DOUBLE_EQ(31, m.ptr()[2]);

    ASSERT_DOUBLE_EQ(12, m.ptr()[3]);
    ASSERT_DOUBLE_EQ(22, m.ptr()[4]);
    ASSERT_DOUBLE_EQ(32, m.ptr()[5]);

    ASSERT_DOUBLE_EQ(13, m.ptr()[6]);
    ASSERT_DOUBLE_EQ(23, m.ptr()[7]);
    ASSERT_DOUBLE_EQ(33, m.ptr()[8]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, ConversionConstructor)
{
    Mat3d md0( 1, 4, 7,
               2, 5, 8,
               3, 6, 9);

    Mat3d mf0( 1.1f, 1.2f, 1.3f,
               2.1f, 2.2f, 2.3f,
               3.1f, 3.2f, 3.3f);


    Mat3f mf(md0);
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 0), mf.rowCol(0, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 1), mf.rowCol(0, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 2), mf.rowCol(0, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 0), mf.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 1), mf.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 2), mf.rowCol(1, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 0), mf.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 1), mf.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 2), mf.rowCol(2, 2));

    Mat3f md(mf0);
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 0), md.rowCol(0, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 1), md.rowCol(0, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 2), md.rowCol(0, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 0), md.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 1), md.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 2), md.rowCol(1, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 0), md.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 1), md.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 2), md.rowCol(2, 2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, AssignmentOperator)
{
    Matrix3<double> m1( 1, 4, 7,
                        2, 5, 8,
                        3, 6, 9);

    Matrix3<double> m2;
    Matrix3<double> m3;
    m3 = m2 = m1;
    m1.setZero();

    const double* pM1 = m1.ptr();
    const double* pM2 = m2.ptr();
    const double* pM3 = m3.ptr();

    int i;
    for (i = 0; i < 9; i++)
    {
        ASSERT_DOUBLE_EQ(0, pM1[i]);
        ASSERT_DOUBLE_EQ(i + 1, pM2[i]);
        ASSERT_DOUBLE_EQ(i + 1, pM3[i]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, Equals)
{
    {
        Matrix3<double> mI;
        Matrix3<double> mZ;
        mI.setIdentity();
        mZ.setZero();
        EXPECT_TRUE(mI.equals(mI));
        EXPECT_TRUE(mZ.equals(mZ));
        EXPECT_FALSE(mZ.equals(mI));
    }

    {
        const Matrix3<double> mA( 11, 12, 13, 
                                  21, 22, 23, 
                                  31, 32, 33);

        const Matrix3<double> mB(mA);

        ASSERT_TRUE(mA.equals(mB));

        for (int r = 0; r < 3; r++)
        {
            for (int c = 0; c < 3; c++)
            {
                Matrix3<double> m(mA);
                m.setRowCol(r, c, 99);
                ASSERT_FALSE(m.equals(mA));
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, ComparisonOperatorEqual)
{
    Matrix3<double> mI;
    Matrix3<double> mZ;
    mI.setIdentity();
    mZ.setZero();

    ASSERT_TRUE(mI == mI);
    ASSERT_TRUE(mZ == mZ);
    ASSERT_FALSE(mZ == mI);

    Matrix3<double> mA( 11, 12, 13,
                        21, 22, 23,
                        31, 32, 33);

    Matrix3<double> mB(mA);

    ASSERT_TRUE(mA == mB);
    ASSERT_FALSE(mA == mI);

    int r;
    for (r = 0; r < 3; r++)
    {
        int c;
        for (c = 0; c < 3; c++)
        {
            Matrix3<double> m(mA);

            m.setRowCol(r, c, 99);
            ASSERT_FALSE(m == mA);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, ComparisonOperatorNotEqual)
{
    Matrix3<double> mI;
    Matrix3<double> mZ;
    mI.setIdentity();
    mZ.setZero();

    ASSERT_TRUE(mZ != mI);
    ASSERT_FALSE(mI != mI);
    ASSERT_FALSE(mZ != mZ);

    Matrix3<double> mA( 11, 12, 13,
                        21, 22, 23,
                        31, 32, 33);


    ASSERT_TRUE(mA != mI);
    ASSERT_TRUE(mA != mZ);

    Matrix3<double> mB(mA);
    ASSERT_FALSE(mA != mB);

    int r;
    for (r = 0; r < 3; r++)
    {
        int c;
        for (c = 0; c < 3; c++)
        {
            Matrix3<double> m(mA);

            m.setRowCol(r, c, 99);
            ASSERT_TRUE(m != mA);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, MatrixMultiplyWithIdentityAndZero)
{
    Matrix3<double> mA( 11, 12, 13,
                        21, 22, 23,
                        31, 32, 33);

    Matrix3<double> mI;
    Matrix3<double> mZ;
    mI.setIdentity();
    mZ.setZero();

    {
        Matrix3<double> m1 = mZ*mZ;
        Matrix3<double> m2(mZ); m2.multiply(mZ);
        ASSERT_TRUE(m1.isZero());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mI*mZ;
        Matrix3<double> m2(mI); m2.multiply(mZ);
        ASSERT_TRUE(m1.isZero());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mZ*mI;
        Matrix3<double> m2(mZ); m2.multiply(mI);
        ASSERT_TRUE(m1.isZero());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mI*mI;
        Matrix3<double> m2(mI); m2.multiply(mI);
        ASSERT_TRUE(m1.isIdentity());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mA*mZ;
        Matrix3<double> m2(mA); m2.multiply(mZ);
        ASSERT_TRUE(m1.isZero());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mA*mI;
        Matrix3<double> m2(mA); m2.multiply(mI);
        ASSERT_TRUE((m1 == mA));
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mI*mA;
        Matrix3<double> m2(mI); m2.multiply(mA);
        ASSERT_TRUE((m1 == mA));
        ASSERT_TRUE((m1 == m2));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, MatrixMultiplication)
{
    // Test data computed using online matrix calculator at:
    // http://www.bluebit.gr/matrix-calculator/matrix_multiplication.aspx

    const Matrix3<double> mA( 1,  2,   3,
                              4,  5,   6,
                              7,  8,   9);

    const Matrix3<double> mB(-1,  4,  -7,
                             -2,  5,  -8,
                             -3,  6,  -9);


    const Matrix3<double> mAA(  30,   36,   42,
                                66,   81,   96,
                               102,  126,  150);

    const Matrix3<double> mBB(  14,  -26,  38,
                                16,  -31,  46,
                                18,  -36,  54);

    const Matrix3<double> mAB( -14,   32,  -50,
                               -32,   77, -122,
                               -50,  122, -194);

    const Matrix3<double> mBA( -34,  -38,  -42,
                               -38,  -43,  -48,
                               -42,  -48,  -54);


    {
        Matrix3<double> m1 = mA*mA;
        Matrix3<double> m2(mA); m2.multiply(mA);
        ASSERT_TRUE((mAA == m1));
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mB*mB;
        Matrix3<double> m2(mB); m2.multiply(mB);
        ASSERT_TRUE((mBB == m1));
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mA*mB;
        Matrix3<double> m2(mA); m2.multiply(mB);
        ASSERT_TRUE((mAB == m1));
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix3<double> m1 = mB*mA;
        Matrix3<double> m2(mB); m2.multiply(mA);
        ASSERT_TRUE((mBA == m1));
        ASSERT_TRUE((m1 == m2));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, SetAndCheckForIdentity)
{
    Matrix3<float>  mf1;
    Matrix3<double> md1;
    ASSERT_TRUE(mf1.isIdentity());
    ASSERT_TRUE(md1.isIdentity());

    Matrix3<float>  mf2(1, 0, 0,  0, 1, 0,  0, 0, 1);
    Matrix3<double> md2(1, 0, 0,  0, 1, 0,  0, 0, 1);
    ASSERT_TRUE(mf2.isIdentity());
    ASSERT_TRUE(md2.isIdentity());

    Matrix3<float>  mf3(9, 9, 9,  9, 9, 9,  9, 9, 9);
    Matrix3<double> md3(9, 9, 9,  9, 9, 9,  9, 9, 9);
    ASSERT_FALSE(mf3.isIdentity());
    ASSERT_FALSE(md3.isIdentity());
    mf3.setIdentity();
    md3.setIdentity();
    ASSERT_TRUE(mf3.isIdentity());
    ASSERT_TRUE(md3.isIdentity());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, VerifyThatIdentityCheckFails)
{
    int r;
    for (r = 0; r < 3; r++)
    {
        int c;
        for (c = 0; c < 3; c++)
        {
            Matrix3<float>  mf;
            Matrix3<double> md;
            ASSERT_TRUE(mf.isIdentity());
            ASSERT_TRUE(md.isIdentity());

            mf.setRowCol(r, c, 99);
            md.setRowCol(r, c, 99);
            ASSERT_FALSE(mf.isIdentity());
            ASSERT_FALSE(md.isIdentity());
        }
    }
}
    

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, SetAndCheckForZero)
{
    Matrix3<float>  mf1(0, 0, 0,  0, 0, 0,  0, 0, 0);
    Matrix3<double> md1(0, 0, 0,  0, 0, 0,  0, 0, 0);
    ASSERT_TRUE(mf1.isZero());
    ASSERT_TRUE(md1.isZero());

    Matrix3<float>  mf2(9, 9, 9,  9, 9, 9,  9, 9, 9);
    Matrix3<double> md2(9, 9, 9,  9, 9, 9,  9, 9, 9);
    mf2.setZero();
    md2.setZero();

    int i;
    for (i = 0; i < 9; i++)
    {
        ASSERT_FLOAT_EQ(0.0f, mf2.ptr()[i]);
        ASSERT_DOUBLE_EQ(0.0, md2.ptr()[i]);
    }

    ASSERT_TRUE(mf2.isZero());
    ASSERT_TRUE(md2.isZero());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, VerifyThatZeroCheckFails)
{
    int r;
    for (r = 0; r < 3; r++)
    {
        int c;
        for (c = 0; c < 3; c++)
        {
            Matrix3<float>  mf;
            Matrix3<double> md;
            mf.setZero();
            md.setZero();
            ASSERT_TRUE(mf.isZero());
            ASSERT_TRUE(md.isZero());

            mf.setRowCol(r, c, 99);
            md.setRowCol(r, c, 99);
            ASSERT_FALSE(mf.isZero());
            ASSERT_FALSE(md.isZero());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, ElementGetter)
{
    Matrix3<double> m(   0,  1,  2,
                        10, 11, 12,
                        20, 21, 22);

    ASSERT_DOUBLE_EQ( 0, m.rowCol(0, 0));
    ASSERT_DOUBLE_EQ( 1, m.rowCol(0, 1));
    ASSERT_DOUBLE_EQ( 2, m.rowCol(0, 2));

    ASSERT_DOUBLE_EQ(10, m.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(11, m.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(12, m.rowCol(1, 2));

    ASSERT_DOUBLE_EQ(20, m.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(21, m.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(22, m.rowCol(2, 2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, ElementSetter)
{
    Matrix3<double> m;
    m.setZero();

    int r;
    for (r = 0; r < 3; r++)
    {
        int c;
        for (c = 0; c < 3; c++)
        {
            ASSERT_DOUBLE_EQ(0, m.rowCol(r, c));

            double v = 10*(r + 1) + (c + 1);
            m.setRowCol(r, c, v);
            ASSERT_DOUBLE_EQ( v, m.rowCol(r, c));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, GetOperator)
{
    const Matrix3<double> m(  0,  1,  2,
                             10, 11, 12,
                             20, 21, 22);
    
    ASSERT_DOUBLE_EQ( 0, m(0, 0));
    ASSERT_DOUBLE_EQ( 1, m(0, 1));
    ASSERT_DOUBLE_EQ( 2, m(0, 2));

    ASSERT_DOUBLE_EQ(10, m(1, 0));
    ASSERT_DOUBLE_EQ(11, m(1, 1));
    ASSERT_DOUBLE_EQ(12, m(1, 2));

    ASSERT_DOUBLE_EQ(20, m(2, 0));
    ASSERT_DOUBLE_EQ(21, m(2, 1));
    ASSERT_DOUBLE_EQ(22, m(2, 2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, SetGetOperator)
{
    Matrix3<double> m;
    
    m(0,0) = 0;
    m(0,1) = 1;
    m(0,2) = 2;
    
    m(1,0) = 10;
    m(1,1) = 11;
    m(1,2) = 12;

    m(2,0) = 20;
    m(2,1) = 21;
    m(2,2) = 22;

    ASSERT_DOUBLE_EQ( 0, m(0, 0));
    ASSERT_DOUBLE_EQ( 1, m(0, 1));
    ASSERT_DOUBLE_EQ( 2, m(0, 2));

    ASSERT_DOUBLE_EQ(10, m(1, 0));
    ASSERT_DOUBLE_EQ(11, m(1, 1));
    ASSERT_DOUBLE_EQ(12, m(1, 2));

    ASSERT_DOUBLE_EQ(20, m(2, 0));
    ASSERT_DOUBLE_EQ(21, m(2, 1));
    ASSERT_DOUBLE_EQ(22, m(2, 2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, CreateMatrixWithRotation)
{
    Mat3d mx1 = Mat3d::fromRotation(Vec3d(1, 0, 0), Math::toRadians( 90.0));
    Mat3d mx2 = Mat3d::fromRotation(Vec3d(1, 0, 0), Math::toRadians(-90.0));
    Mat3d mx3 = Mat3d::fromRotation(Vec3d(1, 0, 0), Math::toRadians( 45.0));
    Mat3d my1 = Mat3d::fromRotation(Vec3d(0, 1, 0), Math::toRadians( 90.0));
    Mat3d my2 = Mat3d::fromRotation(Vec3d(0, 1, 0), Math::toRadians(-90.0));
    Mat3d mz1 = Mat3d::fromRotation(Vec3d(0, 0, 1), Math::toRadians( 90.0));
    Mat3d mz2 = Mat3d::fromRotation(Vec3d(0, 0, 1), Math::toRadians(-90.0));

    const double absErr = 1e-15;

    // Rotation axis and vector parallell
    {
        const Mat3d m(mx1);
        const Vec3d iv(1, 0, 0);
        const Vec3d ev(1, 0, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat3d m(my1);
        const Vec3d iv(0, 1, 0);
        const Vec3d ev(0, 1, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat3d m(mz1);
        const Vec3d iv(0, 0, 1);
        const Vec3d ev(0, 0, 1);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }


    // Rotations round X-axis
    {
        const Mat3d m(mx1);
        const Vec3d iv(0, 0, 1);
        const Vec3d ev(0, -1, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat3d m(mx2);
        const Vec3d iv(0, 1, 0);
        const Vec3d ev(0, 0, -1);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat3d m(mx3);
        const Vec3d iv(0, 1, 0);
        const Vec3d ev(0, 1, 1);

        Vec3d v(iv);
        v.transformVector(m);
        v.setLength(ev.length());
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }


    // Rotations round Y-axis
    {
        const Mat3d m(my1);
        const Vec3d iv(1, 0, 0);
        const Vec3d ev(0, 0, -1);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat3d m(my2);
        const Vec3d iv(0, 0, 1);
        const Vec3d ev(-1, 0, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }


    // Rotations round Z-axis
    {
        const Mat3d m(mz1);
        const Vec3d iv(1, 0, 0);
        const Vec3d ev(0, 1, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat3d m(mz2);
        const Vec3d iv(0, 1, 0);
        const Vec3d ev(1, 0, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, Determinant)
{
    Matrix3<double> md0;
    Matrix3<float>  mf0;
    md0.setZero();
    mf0.setZero();
    EXPECT_DOUBLE_EQ(0, md0.determinant());
    EXPECT_FLOAT_EQ(0, mf0.determinant());

    Matrix3<double> md1;
    Matrix3<float> mf1;
    EXPECT_DOUBLE_EQ(1.0, md1.determinant());
    EXPECT_FLOAT_EQ(1.0f, mf1.determinant());

    Matrix3<double> md2(  1,  2,  3,
                          4,  5,  6,
                          7,  8,  9);
    Matrix3<float> mf2(md2);

    EXPECT_DOUBLE_EQ(0.0, md2.determinant());
    EXPECT_FLOAT_EQ(0.0f, mf2.determinant());

    Matrix3<double> md3(-13.0,  14.0, -26.0,
                          0.0, -32.0,  15.0,
                        -40.0, -43.0,  35.0);
    Matrix3<float> mf3(md3);

    EXPECT_DOUBLE_EQ(31055.0, md3.determinant());
    EXPECT_FLOAT_EQ(31055.0, mf3.determinant());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, InvertIdentityAndZero)
{
    Mat3d md0;
    md0.setZero();
    ASSERT_FALSE(md0.invert());
    ASSERT_TRUE(md0.isZero());

    Mat3f mf0;
    mf0.setZero();
    ASSERT_FALSE(mf0.invert());
    ASSERT_TRUE(mf0.isZero());


    Mat3d md1;
    ASSERT_TRUE(md1.invert());
    ASSERT_TRUE(md1.isIdentity());

    Mat3f mf1;
    ASSERT_TRUE(mf1.invert());
    ASSERT_TRUE(mf1.isIdentity());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, Inverse1)
{
    Matrix3<double> M(-13.0,  14.0, -26.0,
                        0.0, -32.0,  15.0,
                      -40.0, -43.0,  35.0);

    Matrix3<double> MI(-0.015295444,  0.020222186, -0.020028981,
                       -0.019320560, -0.048140396,  0.006279182,
                       -0.041217195, -0.036032845,  0.013395588);


    {
        const double absErr = 1e-9;

        bool wasInverted = false;
        Matrix3<double> m = M.getInverted(&wasInverted);
        EXPECT_TRUE(wasInverted);
        EXPECT_NEAR(MI.rowCol(0, 0), m.rowCol(0, 0), absErr);
        EXPECT_NEAR(MI.rowCol(0, 1), m.rowCol(0, 1), absErr);
        EXPECT_NEAR(MI.rowCol(0, 2), m.rowCol(0, 2), absErr);
        EXPECT_NEAR(MI.rowCol(1, 0), m.rowCol(1, 0), absErr);
        EXPECT_NEAR(MI.rowCol(1, 1), m.rowCol(1, 1), absErr);
        EXPECT_NEAR(MI.rowCol(1, 2), m.rowCol(1, 2), absErr);
        EXPECT_NEAR(MI.rowCol(2, 0), m.rowCol(2, 0), absErr);
        EXPECT_NEAR(MI.rowCol(2, 1), m.rowCol(2, 1), absErr);
        EXPECT_NEAR(MI.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }

    {
        const double absErr = 1e-6;

        bool wasInverted = false;
        Matrix3<double> m = MI.getInverted(&wasInverted);
        EXPECT_TRUE(wasInverted);
        EXPECT_NEAR(M.rowCol(0, 0), m.rowCol(0, 0), absErr);
        EXPECT_NEAR(M.rowCol(0, 1), m.rowCol(0, 1), absErr);
        EXPECT_NEAR(M.rowCol(0, 2), m.rowCol(0, 2), absErr);
        EXPECT_NEAR(M.rowCol(1, 0), m.rowCol(1, 0), absErr);
        EXPECT_NEAR(M.rowCol(1, 1), m.rowCol(1, 1), absErr);
        EXPECT_NEAR(M.rowCol(1, 2), m.rowCol(1, 2), absErr);
        EXPECT_NEAR(M.rowCol(2, 0), m.rowCol(2, 0), absErr);
        EXPECT_NEAR(M.rowCol(2, 1), m.rowCol(2, 1), absErr);
        EXPECT_NEAR(M.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }


    {
        const double absErr = 1e-8;

        Matrix3<float> m(M);
        EXPECT_TRUE(m.invert());
        EXPECT_NEAR(MI.rowCol(0, 0), m.rowCol(0, 0), absErr);
        EXPECT_NEAR(MI.rowCol(0, 1), m.rowCol(0, 1), absErr);
        EXPECT_NEAR(MI.rowCol(0, 2), m.rowCol(0, 2), absErr);
        EXPECT_NEAR(MI.rowCol(1, 0), m.rowCol(1, 0), absErr);
        EXPECT_NEAR(MI.rowCol(1, 1), m.rowCol(1, 1), absErr);
        EXPECT_NEAR(MI.rowCol(1, 2), m.rowCol(1, 2), absErr);
        EXPECT_NEAR(MI.rowCol(2, 0), m.rowCol(2, 0), absErr);
        EXPECT_NEAR(MI.rowCol(2, 1), m.rowCol(2, 1), absErr);
        EXPECT_NEAR(MI.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }

    {
        const double absErr = 1e-6;

        Matrix3<float> m(MI);
        EXPECT_TRUE(m.invert());
        EXPECT_NEAR(M.rowCol(0, 0), m.rowCol(0, 0), absErr);
        EXPECT_NEAR(M.rowCol(0, 1), m.rowCol(0, 1), absErr);
        EXPECT_NEAR(M.rowCol(0, 2), m.rowCol(0, 2), absErr);
        EXPECT_NEAR(M.rowCol(1, 0), m.rowCol(1, 0), absErr);
        EXPECT_NEAR(M.rowCol(1, 1), m.rowCol(1, 1), absErr);
        EXPECT_NEAR(M.rowCol(1, 2), m.rowCol(1, 2), absErr);
        EXPECT_NEAR(M.rowCol(2, 0), m.rowCol(2, 0), absErr);
        EXPECT_NEAR(M.rowCol(2, 1), m.rowCol(2, 1), absErr);
        EXPECT_NEAR(M.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, Inverse2)
{
    Matrix3<double> M( 0.9, -3.4,  3.4,
                      -0.8, -4.7, -4.9,
                      -3.9,  4.5, -3.9);

    Matrix3<double> MI(-0.436135053, -0.022033569, -0.352537101,
                       -0.172704297, -0.105307498, -0.018253300,
                        0.236860864, -0.099475083,  0.075065345);

    const double absErr = 0.0000001;

    {
        bool wasInverted = false;
        Matrix3<double> m = M.getInverted(&wasInverted);
        ASSERT_TRUE(wasInverted);
        ASSERT_NEAR(MI.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(MI.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(MI.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(MI.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(MI.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(MI.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(MI.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(MI.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(MI.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }

    {
        bool wasInverted = false;
        Matrix3<double> m = MI.getInverted(&wasInverted);
        ASSERT_TRUE(wasInverted);
        ASSERT_NEAR(M.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(M.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(M.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(M.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(M.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(M.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(M.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(M.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(M.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, Transpose)
{
    const Matrix3<double> m0( 11, 12, 13,
                              21, 22, 23,
                              31, 32, 33);

    const Matrix3<double> mt( 11, 21, 31,
                              12, 22, 32,
                              13, 23, 33);

    Matrix3<double> m(m0);
    m.transpose();
    
    EXPECT_TRUE(m == mt);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix3Test, FromRotation)
{
    const double absErr = 0.00001;

    // Expected data from Calc 3D Prof app

    {
        const Vector3<double> axis(1, 1, 1);
        const double angleDeg = 45;
        const Matrix3<double> me( 0.80474, -0.31062,  0.50588,
                                  0.50588,  0.80474, -0.31062,
                                 -0.31062,  0.50588,  0.80474);

        Matrix3<double> m = Matrix3<double>::fromRotation(axis, Math::toRadians(angleDeg));

        ASSERT_NEAR(me.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(me.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(me.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(me.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(me.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(me.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(me.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(me.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(me.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }

    {
        const Vector3<double> axis(1, 1, 1);
        const double angleDeg = 200;
        const Matrix3<double> me(-0.29313,  0.84403,  0.44910,
                                  0.44910, -0.29313,  0.84403,
                                  0.84403,  0.44910, -0.29313);

        Matrix3<double> m = Matrix3<double>::fromRotation(axis, Math::toRadians(angleDeg));

        ASSERT_NEAR(me.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(me.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(me.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(me.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(me.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(me.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(me.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(me.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(me.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }

    {
        const Vector3<double> axis(1, 2, 3);
        const double angleDeg = 150;
        const Matrix3<double> me(-0.73274, -0.13432,  0.66712,
                                  0.66747, -0.33288,  0.66609,
                                  0.13260,  0.93336,  0.33356);

        Matrix3<double> m = Matrix3<double>::fromRotation(axis, Math::toRadians(angleDeg));

        ASSERT_NEAR(me.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(me.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(me.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(me.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(me.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(me.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(me.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(me.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(me.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }

    {
        const Vector3<double> axis(1, 0.1, 0.2);
        const double angleDeg = 160;
        const Matrix3<double> me( 0.90763,  0.11798,  0.40284,
                                  0.25149, -0.92122, -0.29683,
                                  0.33609,  0.37072, -0.86580);

        Matrix3<double> m = Matrix3<double>::fromRotation(axis, Math::toRadians(angleDeg));

        ASSERT_NEAR(me.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(me.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(me.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(me.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(me.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(me.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(me.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(me.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(me.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }

    {
        const Vector3<double> axis(0.2, 1, 0.1);
        const double angleDeg = 170;
        const Matrix3<double> me(-0.90920,  0.36111,  0.20727,
                                  0.39500,  0.90549,  0.15514,
                                 -0.13166,  0.22292, -0.96590);

        Matrix3<double> m = Matrix3<double>::fromRotation(axis, Math::toRadians(angleDeg));

        ASSERT_NEAR(me.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(me.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(me.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(me.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(me.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(me.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(me.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(me.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(me.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }

    {
        const Vector3<double> axis(0.1, 0.2, 1);
        const double angleDeg = 150;
        const Matrix3<double> me(-0.848250, -0.45241,  0.27531,
                                  0.523490, -0.79494,  0.30664,
                                  0.080127,  0.40423,  0.91114);

        Matrix3<double> m = Matrix3<double>::fromRotation(axis, Math::toRadians(angleDeg));

        ASSERT_NEAR(me.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(me.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(me.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(me.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(me.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(me.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(me.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(me.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(me.rowCol(2, 2), m.rowCol(2, 2), absErr);
    }
}


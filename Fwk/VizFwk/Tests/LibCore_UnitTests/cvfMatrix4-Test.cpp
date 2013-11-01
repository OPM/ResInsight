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
#include "cvfMatrix4.h"
#include "cvfMath.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, Constants)
{
    Mat4d mdi;
    Mat4d mdz;
    mdz.setZero();

    ASSERT_TRUE(mdi == Mat4d::IDENTITY);
    ASSERT_TRUE(mdz == Mat4d::ZERO);


    Mat4f mfi;
    Mat4f mfz;
    mfz.setZero();

    ASSERT_TRUE(mfi == Mat4f::IDENTITY);
    ASSERT_TRUE(mfz == Mat4f::ZERO);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, DefaultConstructor)
{
    // Constructs using default constructor and manually checks for identity
    Matrix4<float> mf;

    ASSERT_FLOAT_EQ(1, mf.ptr()[0]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[1]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[2]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[3]);

    ASSERT_FLOAT_EQ(0, mf.ptr()[4]);
    ASSERT_FLOAT_EQ(1, mf.ptr()[5]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[6]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[7]);

    ASSERT_FLOAT_EQ(0, mf.ptr()[8]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[9]);
    ASSERT_FLOAT_EQ(1, mf.ptr()[10]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[11]);

    ASSERT_FLOAT_EQ(0, mf.ptr()[12]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[13]);
    ASSERT_FLOAT_EQ(0, mf.ptr()[14]);
    ASSERT_FLOAT_EQ(1, mf.ptr()[15]);


    Matrix4<double> md;

    ASSERT_DOUBLE_EQ(1, md.ptr()[0]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[1]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[2]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[3]);

    ASSERT_DOUBLE_EQ(0, md.ptr()[4]);
    ASSERT_DOUBLE_EQ(1, md.ptr()[5]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[6]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[7]);

    ASSERT_DOUBLE_EQ(0, md.ptr()[8]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[9]);
    ASSERT_DOUBLE_EQ(1, md.ptr()[10]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[11]);

    ASSERT_DOUBLE_EQ(0, md.ptr()[12]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[13]);
    ASSERT_DOUBLE_EQ(0, md.ptr()[14]);
    ASSERT_DOUBLE_EQ(1, md.ptr()[15]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, CopyConstructor)
{
    Matrix4<double> m1( 1, 5,  9, 13, 
                        2, 6, 10, 14, 
                        3, 7, 11, 15, 
                        4, 8, 12, 16);

    Matrix4<double> m2(m1);
    m1.setZero();

    const double* pM1 = m1.ptr();
    const double* pM2 = m2.ptr();

    int i;
    for (i = 0; i < 16; i++)
    {
        ASSERT_DOUBLE_EQ(0, pM1[i]);
        ASSERT_DOUBLE_EQ(i + 1, pM2[i]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, AllElementsConstructor)
{
    Matrix4<double> m(  11, 12, 13, 14, 
                        21, 22, 23, 24, 
                        31, 32, 33, 34, 
                        41, 42, 43, 44);


    ASSERT_DOUBLE_EQ(11, m.ptr()[0]);
    ASSERT_DOUBLE_EQ(21, m.ptr()[1]);
    ASSERT_DOUBLE_EQ(31, m.ptr()[2]);
    ASSERT_DOUBLE_EQ(41, m.ptr()[3]);

    ASSERT_DOUBLE_EQ(12, m.ptr()[4]);
    ASSERT_DOUBLE_EQ(22, m.ptr()[5]);
    ASSERT_DOUBLE_EQ(32, m.ptr()[6]);
    ASSERT_DOUBLE_EQ(42, m.ptr()[7]);

    ASSERT_DOUBLE_EQ(13, m.ptr()[8]);
    ASSERT_DOUBLE_EQ(23, m.ptr()[9]);
    ASSERT_DOUBLE_EQ(33, m.ptr()[10]);
    ASSERT_DOUBLE_EQ(43, m.ptr()[11]);

    ASSERT_DOUBLE_EQ(14, m.ptr()[12]);
    ASSERT_DOUBLE_EQ(24, m.ptr()[13]);
    ASSERT_DOUBLE_EQ(34, m.ptr()[14]);
    ASSERT_DOUBLE_EQ(44, m.ptr()[15]);

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, ConversionConstructor)
{
    Mat4d md0( 1, 5,  9, 13, 
               2, 6, 10, 14, 
               3, 7, 11, 15, 
               4, 8, 12, 16);

    Mat4f mf0( 1.1f, 1.2f, 1.3f, 1.4f, 
               2.1f, 2.2f, 2.3f, 2.4f, 
               3.1f, 3.2f, 3.3f, 3.4f, 
               4.1f, 4.2f, 4.3f, 4.4f);


    Mat4f mf(md0);
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 0), mf.rowCol(0, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 1), mf.rowCol(0, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 2), mf.rowCol(0, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 3), mf.rowCol(0, 3));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 0), mf.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 1), mf.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 2), mf.rowCol(1, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 3), mf.rowCol(1, 3));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 0), mf.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 1), mf.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 2), mf.rowCol(2, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 3), mf.rowCol(2, 3));
    ASSERT_DOUBLE_EQ(md0.rowCol(3, 0), mf.rowCol(3, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(3, 1), mf.rowCol(3, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(3, 2), mf.rowCol(3, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(3, 3), mf.rowCol(3, 3));

    Mat4d md(mf0);
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 0), md.rowCol(0, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 1), md.rowCol(0, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 2), md.rowCol(0, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 3), md.rowCol(0, 3));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 0), md.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 1), md.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 2), md.rowCol(1, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 3), md.rowCol(1, 3));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 0), md.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 1), md.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 2), md.rowCol(2, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 3), md.rowCol(2, 3));
    ASSERT_DOUBLE_EQ(mf0.rowCol(3, 0), md.rowCol(3, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(3, 1), md.rowCol(3, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(3, 2), md.rowCol(3, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(3, 3), md.rowCol(3, 3));
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, ConstructFrom3x3)
{
    const Matrix3<double> m3( 11, 12, 13,
                              21, 22, 23,
                              31, 32, 33);

    const Matrix4<double> m4(m3);

    ASSERT_DOUBLE_EQ(11, m4.rowCol(0, 0));
    ASSERT_DOUBLE_EQ(12, m4.rowCol(0, 1));
    ASSERT_DOUBLE_EQ(13, m4.rowCol(0, 2));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(0, 3));

    ASSERT_DOUBLE_EQ(21, m4.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(22, m4.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(23, m4.rowCol(1, 2));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(1, 3));

    ASSERT_DOUBLE_EQ(31, m4.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(32, m4.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(33, m4.rowCol(2, 2));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(2, 3));

    ASSERT_DOUBLE_EQ( 0, m4.rowCol(3, 0));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(3, 1));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(3, 2));
    ASSERT_DOUBLE_EQ( 1, m4.rowCol(3, 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, AssignmentOperator)
{
    Matrix4<double> m1( 1, 5,  9, 13, 
        2, 6, 10, 14, 
        3, 7, 11, 15, 
        4, 8, 12, 16);

    Matrix4<double> m2;
    Matrix4<double> m3;
    m3 = m2 = m1;
    m1.setZero();

    const double* pM1 = m1.ptr();
    const double* pM2 = m2.ptr();
    const double* pM3 = m3.ptr();

    int i;
    for (i = 0; i < 16; i++)
    {
        ASSERT_DOUBLE_EQ(0, pM1[i]);
        ASSERT_DOUBLE_EQ(i + 1, pM2[i]);
        ASSERT_DOUBLE_EQ(i + 1, pM3[i]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, Equals)
{
    {
        Mat4d mI;
        Mat4d mZ;
        mI.setIdentity();
        mZ.setZero();
        EXPECT_TRUE(mI.equals(mI));
        EXPECT_TRUE(mZ.equals(mZ));
        EXPECT_FALSE(mZ.equals(mI));
    }

    {
        const Matrix4<double> mA( 11, 12, 13, 14, 
                                  21, 22, 23, 24, 
                                  31, 32, 33, 34, 
                                  41, 42, 43, 44);

        const Matrix4<double> mB(mA);

        ASSERT_TRUE(mA.equals(mB));

        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                Matrix4<double> m(mA);
                m.setRowCol(r, c, 99);
                ASSERT_FALSE(m.equals(mA));
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, ComparisonOperatorEqual)
{
    Matrix4<double> mI;
    Matrix4<double> mZ;
    mI.setIdentity();
    mZ.setZero();

    ASSERT_TRUE(mI == mI);
    ASSERT_TRUE(mZ == mZ);
    ASSERT_FALSE(mZ == mI);

    Matrix4<double> mA( 11, 12, 13, 14, 
                        21, 22, 23, 24, 
                        31, 32, 33, 34, 
                        41, 42, 43, 44);

    Matrix4<double> mB(mA);

    ASSERT_TRUE(mA == mB);
    ASSERT_FALSE(mA == mI);

    int r;
    for (r = 0; r < 4; r++)
    {
        int c;
        for (c = 0; c < 4; c++)
        {
            Matrix4<double> m(mA);

            m.setRowCol(r, c, 99);
            ASSERT_FALSE(m == mA);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, ComparisonOperatorNotEqual)
{
    Matrix4<double> mI;
    Matrix4<double> mZ;
    mI.setIdentity();
    mZ.setZero();

    ASSERT_TRUE(mZ != mI);
    ASSERT_FALSE(mI != mI);
    ASSERT_FALSE(mZ != mZ);

    Matrix4<double> mA( 11, 12, 13, 14, 
                        21, 22, 23, 24, 
                        31, 32, 33, 34, 
                        41, 42, 43, 44);


    ASSERT_TRUE(mA != mI);
    ASSERT_TRUE(mA != mZ);

    Matrix4<double> mB(mA);
    ASSERT_FALSE(mA != mB);

    int r;
    for (r = 0; r < 4; r++)
    {
        int c;
        for (c = 0; c < 4; c++)
        {
            Matrix4<double> m(mA);

            m.setRowCol(r, c, 99);
            ASSERT_TRUE(m != mA);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, MatrixMultiplyWithIdentityAndZero)
{
    Matrix4<double> mA( 11, 12, 13, 14, 
                        21, 22, 23, 24, 
                        31, 32, 33, 34, 
                        41, 42, 43, 44);

    Matrix4<double> mI;
    Matrix4<double> mZ;
    mI.setIdentity();
    mZ.setZero();

    {
        Matrix4<double> m1 = mZ*mZ;
        Matrix4<double> m2(mZ); m2.multiply(mZ);
        ASSERT_TRUE(m1.isZero());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mI*mZ;
        Matrix4<double> m2(mI); m2.multiply(mZ);
        ASSERT_TRUE(m1.isZero());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mZ*mI;
        Matrix4<double> m2(mZ); m2.multiply(mI);
        ASSERT_TRUE(m1.isZero());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mI*mI;
        Matrix4<double> m2(mI); m2.multiply(mI);
        ASSERT_TRUE(m1.isIdentity());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mA*mZ;
        Matrix4<double> m2(mA); m2.multiply(mZ);
        ASSERT_TRUE(m1.isZero());
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mA*mI;
        Matrix4<double> m2(mA); m2.multiply(mI);
        ASSERT_TRUE((m1 == mA));
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mI*mA;
        Matrix4<double> m2(mI); m2.multiply(mA);
        ASSERT_TRUE((m1 == mA));
        ASSERT_TRUE((m1 == m2));
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, MatrixMultiplication)
{
    // Test data computed using online matrix calculator at:
    // http://www.bluebit.gr/matrix-calculator/matrix_multiplication.aspx

    const Matrix4<double> mA(  1,   2,   3,   4, 
                               5,   6,   7,   8, 
                               9,  10,  11,  12, 
                              13,  14,  15,  16);

    const Matrix4<double> mB( -1,   5,  -9,  13, 
                              -2,   6, -10,  14, 
                              -3,   7, -11,  15, 
                              -4,   8, -12,  16);


    const Matrix4<double> mAA(  90,  100,  110,  120,
                               202,  228,  254,  280,
                               314,  356,  398,  440,
                               426,  484,  542,  600);

    const Matrix4<double> mBB( -34,   66,  -98,  130,
                               -36,   68, -100,  132,
                               -38,   70, -102,  134,
                               -40,   72, -104,  136);

    const Matrix4<double> mAB( -30,   70, -110,  150,
                               -70,  174, -278,  382,
                              -110,  278, -446,  614,
                              -150,  382, -614,  846);

    const Matrix4<double> mBA( 112,  120,  128,  136,
                               120,  128,  136,  144,
                               128,  136,  144,  152,
                               136,  144,  152,  160);

    {
        Matrix4<double> m1 = mA*mA;
        Matrix4<double> m2(mA); m2.multiply(mA);
        ASSERT_TRUE((mAA == m1));
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mB*mB;
        Matrix4<double> m2(mB); m2.multiply(mB);
        ASSERT_TRUE((mBB == m1));
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mA*mB;
        Matrix4<double> m2(mA); m2.multiply(mB);
        ASSERT_TRUE((mAB == m1));
        ASSERT_TRUE((m1 == m2));
    }

    {
        Matrix4<double> m1 = mB*mA;
        Matrix4<double> m2(mB); m2.multiply(mA);
        ASSERT_TRUE((mBA == m1));
        ASSERT_TRUE((m1 == m2));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, MatrixVec4Multiplication)
{
    // Test data computed using online matrix calculator at:
    // http://www.bluebit.gr/matrix-calculator/matrix_multiplication.aspx

    const Matrix4<double> mA(  1,   2,   3,   4, 
                               5,   6,   7,   8, 
                               9,  10,  11,  12, 
                              13,  14,  15,  16);

    const Matrix4<double> mB( -1,   5,  -9,  13, 
                              -2,   6, -10,  14, 
                              -3,   7, -11,  15, 
                              -4,   8, -12,  16);

    const Vector4<double> v1(  1,   2,  3,  4);
    const Vector4<double> v2( -5,   6, -7,  8);

    const Vector4<double> vA1( 30,  70, 110, 150);
    const Vector4<double> vA2( 18,  26,  34,  42);
    const Vector4<double> vB1( 34,  36,  38,  40);
    const Vector4<double> vB2(202, 228, 254, 280);
    
    Vector4<double> v;

    v = mA*v1;
    EXPECT_TRUE((vA1 == v));

    v = mA*v2;
    EXPECT_TRUE((vA2 == v));

    v = mB*v1;
    EXPECT_TRUE((vB1 == v));

    v = mB*v2;
    EXPECT_TRUE((vB2 == v));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, SetAndCheckForIdentity)
{
    Matrix4<float>  mf1;
    Matrix4<double> md1;
    ASSERT_TRUE(mf1.isIdentity());
    ASSERT_TRUE(md1.isIdentity());

    Matrix4<float>  mf2(1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1);
    Matrix4<double> md2(1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1);
    ASSERT_TRUE(mf2.isIdentity());
    ASSERT_TRUE(md2.isIdentity());

    Matrix4<float>  mf3(9, 9, 9, 9,  9, 9, 9, 9,  9, 9, 9, 9,  9, 9, 9, 9);
    Matrix4<double> md3(9, 9, 9, 9,  9, 9, 9, 9,  9, 9, 9, 9,  9, 9, 9, 9);
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
TEST(Matrix4Test, VerifyThatIdentityCheckFails)
{
    int r;
    for (r = 0; r < 4; r++)
    {
        int c;
        for (c = 0; c < 4; c++)
        {
            Matrix4<float>  mf;
            Matrix4<double> md;
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
TEST(Matrix4Test, SetAndCheckForZero)
{
    Matrix4<float>  mf1(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0);
    Matrix4<double> md1(0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0);
    ASSERT_TRUE(mf1.isZero());
    ASSERT_TRUE(md1.isZero());

    Matrix4<float>  mf2(9, 9, 9, 9,  9, 9, 9, 9,  9, 9, 9, 9,  9, 9, 9, 9);
    Matrix4<double> md2(9, 9, 9, 9,  9, 9, 9, 9,  9, 9, 9, 9,  9, 9, 9, 9);
    mf2.setZero();
    md2.setZero();

    int i;
    for (i = 0; i < 16; i++)
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
TEST(Matrix4Test, VerifyThatZeroCheckFails)
{
    int r;
    for (r = 0; r < 4; r++)
    {
        int c;
        for (c = 0; c < 4; c++)
        {
            Matrix4<float>  mf;
            Matrix4<double> md;
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
TEST(Matrix4Test, ElementGetter)
{
    Matrix4<double> m(   0,  1,  2,  3, 
                        10, 11, 12, 13, 
                        20, 21, 22, 23, 
                        30, 31, 32, 33);

    ASSERT_DOUBLE_EQ( 0, m.rowCol(0, 0));
    ASSERT_DOUBLE_EQ( 1, m.rowCol(0, 1));
    ASSERT_DOUBLE_EQ( 2, m.rowCol(0, 2));
    ASSERT_DOUBLE_EQ( 3, m.rowCol(0, 3));

    ASSERT_DOUBLE_EQ(10, m.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(11, m.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(12, m.rowCol(1, 2));
    ASSERT_DOUBLE_EQ(13, m.rowCol(1, 3));

    ASSERT_DOUBLE_EQ(20, m.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(21, m.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(22, m.rowCol(2, 2));
    ASSERT_DOUBLE_EQ(23, m.rowCol(2, 3));

    ASSERT_DOUBLE_EQ(30, m.rowCol(3, 0));
    ASSERT_DOUBLE_EQ(31, m.rowCol(3, 1));
    ASSERT_DOUBLE_EQ(32, m.rowCol(3, 2));
    ASSERT_DOUBLE_EQ(33, m.rowCol(3, 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, ElementSetter)
{
    Matrix4<double> m;
    m.setZero();

    int r;
    for (r = 0; r < 4; r++)
    {
        int c;
        for (c = 0; c < 4; c++)
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
TEST(Matrix4Test, GetOperator)
{
    const Matrix4<double> m(  0,  1,  2,  3, 
                             10, 11, 12, 13, 
                             20, 21, 22, 23, 
                             30, 31, 32, 33);
    
    ASSERT_DOUBLE_EQ( 0, m(0, 0));
    ASSERT_DOUBLE_EQ( 1, m(0, 1));
    ASSERT_DOUBLE_EQ( 2, m(0, 2));
    ASSERT_DOUBLE_EQ( 3, m(0, 3));

    ASSERT_DOUBLE_EQ(10, m(1, 0));
    ASSERT_DOUBLE_EQ(11, m(1, 1));
    ASSERT_DOUBLE_EQ(12, m(1, 2));
    ASSERT_DOUBLE_EQ(13, m(1, 3));

    ASSERT_DOUBLE_EQ(20, m(2, 0));
    ASSERT_DOUBLE_EQ(21, m(2, 1));
    ASSERT_DOUBLE_EQ(22, m(2, 2));
    ASSERT_DOUBLE_EQ(23, m(2, 3));

    ASSERT_DOUBLE_EQ(30, m(3, 0));
    ASSERT_DOUBLE_EQ(31, m(3, 1));
    ASSERT_DOUBLE_EQ(32, m(3, 2));
    ASSERT_DOUBLE_EQ(33, m(3, 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, SetGetOperator)
{
    Matrix4<double> m;
    
    m(0,0) = 0;
    m(0,1) = 1;
    m(0,2) = 2;
    m(0,3) = 3;
    
    m(1,0) = 10;
    m(1,1) = 11;
    m(1,2) = 12;
    m(1,3) = 13;

    m(2,0) = 20;
    m(2,1) = 21;
    m(2,2) = 22;
    m(2,3) = 23;

    m(3,0) = 30;
    m(3,1) = 31;
    m(3,2) = 32;
    m(3,3) = 33;

    ASSERT_DOUBLE_EQ( 0, m(0, 0));
    ASSERT_DOUBLE_EQ( 1, m(0, 1));
    ASSERT_DOUBLE_EQ( 2, m(0, 2));
    ASSERT_DOUBLE_EQ( 3, m(0, 3));

    ASSERT_DOUBLE_EQ(10, m(1, 0));
    ASSERT_DOUBLE_EQ(11, m(1, 1));
    ASSERT_DOUBLE_EQ(12, m(1, 2));
    ASSERT_DOUBLE_EQ(13, m(1, 3));

    ASSERT_DOUBLE_EQ(20, m(2, 0));
    ASSERT_DOUBLE_EQ(21, m(2, 1));
    ASSERT_DOUBLE_EQ(22, m(2, 2));
    ASSERT_DOUBLE_EQ(23, m(2, 3));

    ASSERT_DOUBLE_EQ(30, m(3, 0));
    ASSERT_DOUBLE_EQ(31, m(3, 1));
    ASSERT_DOUBLE_EQ(32, m(3, 2));
    ASSERT_DOUBLE_EQ(33, m(3, 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, RowSetGet)
{
    const Matrix4<double> m0(11, 12, 13, 14, 
                             21, 22, 23, 24, 
                             31, 32, 33, 34, 
                             41, 42, 43, 44);

    {
        Matrix4<double> m;
        m.setZero();

        m.setRow(0, Vector4<double>(11, 12, 13, 14));
        m.setRow(1, Vector4<double>(21, 22, 23, 24));
        m.setRow(2, Vector4<double>(31, 32, 33, 34));
        m.setRow(3, Vector4<double>(41, 42, 43, 44));

        EXPECT_TRUE(m == m0);
    }

    {
        const Vector4<double> r0 = m0.row(0);
        const Vector4<double> r1 = m0.row(1);
        const Vector4<double> r2 = m0.row(2);
        const Vector4<double> r3 = m0.row(3);
        EXPECT_TRUE(Vector4<double>(11, 12, 13, 14) == r0);
        EXPECT_TRUE(Vector4<double>(21, 22, 23, 24) == r1);
        EXPECT_TRUE(Vector4<double>(31, 32, 33, 34) == r2);
        EXPECT_TRUE(Vector4<double>(41, 42, 43, 44) == r3);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, ColSetGet)
{
    const Matrix4<double> m0(11, 12, 13, 14, 
                             21, 22, 23, 24, 
                             31, 32, 33, 34, 
                             41, 42, 43, 44);

    {
        Matrix4<double> m;
        m.setZero();

        m.setCol(0, Vector4<double>(11, 21, 31, 41));
        m.setCol(1, Vector4<double>(12, 22, 32, 42));
        m.setCol(2, Vector4<double>(13, 23, 33, 43));
        m.setCol(3, Vector4<double>(14, 24, 34, 44));

        EXPECT_TRUE(m == m0);
    }

    {
        const Vector4<double> c0 = m0.col(0);
        const Vector4<double> c1 = m0.col(1);
        const Vector4<double> c2 = m0.col(2);
        const Vector4<double> c3 = m0.col(3);
        EXPECT_TRUE(Vector4<double>(11, 21, 31, 41) == c0);
        EXPECT_TRUE(Vector4<double>(12, 22, 32, 42) == c1);
        EXPECT_TRUE(Vector4<double>(13, 23, 33, 43) == c2);
        EXPECT_TRUE(Vector4<double>(14, 24, 34, 44) == c3);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, SetFromMatrixWithDifferentBasicType)
{
    const Mat4d md0( 1, 5,  9, 13, 
                     2, 6, 10, 14, 
                     3, 7, 11, 15, 
                     4, 8, 12, 16);

    const Mat4f mf0( 1.1f, 1.2f, 1.3f, 1.4f, 
                     2.1f, 2.2f, 2.3f, 2.4f, 
                     3.1f, 3.2f, 3.3f, 3.4f, 
                     4.1f, 4.2f, 4.3f, 4.4f);


    Mat4f mf;
    mf.set(md0);
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 0), mf.rowCol(0, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 1), mf.rowCol(0, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 2), mf.rowCol(0, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(0, 3), mf.rowCol(0, 3));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 0), mf.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 1), mf.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 2), mf.rowCol(1, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(1, 3), mf.rowCol(1, 3));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 0), mf.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 1), mf.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 2), mf.rowCol(2, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(2, 3), mf.rowCol(2, 3));
    ASSERT_DOUBLE_EQ(md0.rowCol(3, 0), mf.rowCol(3, 0));
    ASSERT_DOUBLE_EQ(md0.rowCol(3, 1), mf.rowCol(3, 1));
    ASSERT_DOUBLE_EQ(md0.rowCol(3, 2), mf.rowCol(3, 2));
    ASSERT_DOUBLE_EQ(md0.rowCol(3, 3), mf.rowCol(3, 3));

    Mat4d md;
    md.set(mf0);
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 0), md.rowCol(0, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 1), md.rowCol(0, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 2), md.rowCol(0, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(0, 3), md.rowCol(0, 3));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 0), md.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 1), md.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 2), md.rowCol(1, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(1, 3), md.rowCol(1, 3));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 0), md.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 1), md.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 2), md.rowCol(2, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(2, 3), md.rowCol(2, 3));
    ASSERT_DOUBLE_EQ(mf0.rowCol(3, 0), md.rowCol(3, 0));
    ASSERT_DOUBLE_EQ(mf0.rowCol(3, 1), md.rowCol(3, 1));
    ASSERT_DOUBLE_EQ(mf0.rowCol(3, 2), md.rowCol(3, 2));
    ASSERT_DOUBLE_EQ(mf0.rowCol(3, 3), md.rowCol(3, 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, GetAndSetTranslation)
{
    {
        Mat4d m;
        Vec3d v = m.translation();
        ASSERT_TRUE(v.isZero());

        m = Mat4d::fromTranslation(Vec3d(1, 2, 3));
        v = m.translation();
        ASSERT_DOUBLE_EQ(1, v.x());
        ASSERT_DOUBLE_EQ(2, v.y());
        ASSERT_DOUBLE_EQ(3, v.z());

        m.setTranslation(Vec3d(4, 5, 6));
        v = m.translation();
        ASSERT_DOUBLE_EQ(4, v.x());
        ASSERT_DOUBLE_EQ(5, v.y());
        ASSERT_DOUBLE_EQ(6, v.z());
    }

    {
        Mat4f m;
        Vec3f v = m.translation();
        ASSERT_TRUE(v.isZero());

        m = Mat4f::fromTranslation(Vec3f(1, 2, 3));
        v = m.translation();
        ASSERT_FLOAT_EQ(1, v.x());
        ASSERT_FLOAT_EQ(2, v.y());
        ASSERT_FLOAT_EQ(3, v.z());

        m.setTranslation(Vec3f(4, 5, 6));
        v = m.translation();
        ASSERT_FLOAT_EQ(4, v.x());
        ASSERT_FLOAT_EQ(5, v.y());
        ASSERT_FLOAT_EQ(6, v.z());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, AddTranslation)
{
    Mat4d m;
    Vec3d v(0, 0, 0);

    // Identity matrix
    v.set(1, 2, 3);
    v.transformPoint(m);
    ASSERT_DOUBLE_EQ(1, v.x());
    ASSERT_DOUBLE_EQ(2, v.y());
    ASSERT_DOUBLE_EQ(3, v.z());

    m.translatePostMultiply(Vec3d(10, 20, 30));
    v.set(1, 2, 3);
    v.transformPoint(m);
    ASSERT_DOUBLE_EQ(11, v.x());
    ASSERT_DOUBLE_EQ(22, v.y());
    ASSERT_DOUBLE_EQ(33, v.z());

    // Transform vector instead of point
    v.set(1, 2, 3);
    v.transformVector(m);
    ASSERT_DOUBLE_EQ(1, v.x());
    ASSERT_DOUBLE_EQ(2, v.y());
    ASSERT_DOUBLE_EQ(3, v.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, PreAndPostMultiplyTranslate)
{
    const Vec3d trans(1, 2, 0);
    const Mat4d T = Mat4d::fromTranslation(trans);
    const Mat4d R = Mat4d::fromRotation(Vec3d(0, 0, 1), PI_D/2);

    const Vec3d inputPoint(10, 20, 30);
    const Vec3d expectedPre (-19, 12, 30);
    const Vec3d expectedPost(-22, 11, 30);

    // Pre multiply with translation matrix
    {
        Mat4d m = T*R;

        Vec3d p = inputPoint.getTransformedPoint(m);
        ASSERT_DOUBLE_EQ(expectedPre.x(), p.x());
        ASSERT_DOUBLE_EQ(expectedPre.y(), p.y());
        ASSERT_DOUBLE_EQ(expectedPre.z(), p.z());
    }

    // Pre translate
    {
        Mat4d m = R;
        m.translatePreMultiply(trans);

        Vec3d p = inputPoint.getTransformedPoint(m);
        ASSERT_DOUBLE_EQ(expectedPre.x(), p.x());
        ASSERT_DOUBLE_EQ(expectedPre.y(), p.y());
        ASSERT_DOUBLE_EQ(expectedPre.z(), p.z());
    }

    // Post multiply with translation matrix
    {
        Mat4d m = R*T;

        Vec3d p = inputPoint.getTransformedPoint(m);
        ASSERT_DOUBLE_EQ(expectedPost.x(), p.x());
        ASSERT_DOUBLE_EQ(expectedPost.y(), p.y());
        ASSERT_DOUBLE_EQ(expectedPost.z(), p.z());
    }

    // Post translate
    {
        Mat4d m = R;
        m.translatePostMultiply(trans);

        Vec3d p = inputPoint.getTransformedPoint(m);
        ASSERT_DOUBLE_EQ(expectedPost.x(), p.x());
        ASSERT_DOUBLE_EQ(expectedPost.y(), p.y());
        ASSERT_DOUBLE_EQ(expectedPost.z(), p.z());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, CreateMatrixWithTranslation)
{
    Vec3d trans(10, 20, 30);

    Mat4f mf = Mat4f::fromTranslation(Vec3f(trans));
    Mat4d md = Mat4d::fromTranslation(trans);

    Vec3f vf(1, 2, 3);
    vf.transformPoint(mf);
    ASSERT_FLOAT_EQ(11, vf.x());
    ASSERT_FLOAT_EQ(22, vf.y());
    ASSERT_FLOAT_EQ(33, vf.z());

    Vec3d vd(1, 2, 3);
    vd.transformPoint(md);
    ASSERT_DOUBLE_EQ(11, vd.x());
    ASSERT_DOUBLE_EQ(22, vd.y());
    ASSERT_DOUBLE_EQ(33, vd.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, CreateMatrixWithScaling)
{
    const Vec3d scale(1, 2, 3);

    Mat4f mf = Mat4f::fromScaling(Vec3f(scale));
    Mat4d md = Mat4d::fromScaling(scale);

    Vec3f vf(10, 100, 1000);
    vf.transformPoint(mf);
    ASSERT_FLOAT_EQ(10,   vf.x());
    ASSERT_FLOAT_EQ(200,  vf.y());
    ASSERT_FLOAT_EQ(3000, vf.z());

    Vec3d vd(10, 100, 1000);
    vd.transformPoint(md);
    ASSERT_DOUBLE_EQ(10,   vd.x());
    ASSERT_DOUBLE_EQ(200,  vd.y());
    ASSERT_DOUBLE_EQ(3000, vd.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, CreateMatrixWithRotation)
{
    Mat4d mx1 = Mat4d::fromRotation(Vec3d(1, 0, 0), Math::toRadians( 90.0));
    Mat4d mx2 = Mat4d::fromRotation(Vec3d(1, 0, 0), Math::toRadians(-90.0));
    Mat4d mx3 = Mat4d::fromRotation(Vec3d(1, 0, 0), Math::toRadians( 45.0));
    Mat4d my1 = Mat4d::fromRotation(Vec3d(0, 1, 0), Math::toRadians( 90.0));
    Mat4d my2 = Mat4d::fromRotation(Vec3d(0, 1, 0), Math::toRadians(-90.0));
    Mat4d mz1 = Mat4d::fromRotation(Vec3d(0, 0, 1), Math::toRadians( 90.0));
    Mat4d mz2 = Mat4d::fromRotation(Vec3d(0, 0, 1), Math::toRadians(-90.0));

    const double absErr = 1e-15;

    // Rotation axis and vector parallell
    {
        const Mat4d m(mx1);
        const Vec3d iv(1, 0, 0);
        const Vec3d ev(1, 0, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat4d m(my1);
        const Vec3d iv(0, 1, 0);
        const Vec3d ev(0, 1, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat4d m(mz1);
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
        const Mat4d m(mx1);
        const Vec3d iv(0, 0, 1);
        const Vec3d ev(0, -1, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat4d m(mx2);
        const Vec3d iv(0, 1, 0);
        const Vec3d ev(0, 0, -1);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat4d m(mx3);
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
        const Mat4d m(my1);
        const Vec3d iv(1, 0, 0);
        const Vec3d ev(0, 0, -1);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat4d m(my2);
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
        const Mat4d m(mz1);
        const Vec3d iv(1, 0, 0);
        const Vec3d ev(0, 1, 0);

        Vec3d v(iv);
        v.transformVector(m);
        ASSERT_NEAR(ev.x(), v.x(), absErr);
        ASSERT_NEAR(ev.y(), v.y(), absErr);
        ASSERT_NEAR(ev.z(), v.z(), absErr);
    }

    {
        const Mat4d m(mz2);
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
TEST(Matrix4Test, CreateMatrixFromAxesIdentityRotation)
{
    const Vec3d ax(1, 0, 0);
    const Vec3d ay(0, 1, 0);
    const Vec3d az(0, 0, 1);

    const Vec3d ex(1, 0, 0);
    const Vec3d ey(0, 1, 0);
    const Vec3d ez(0, 0, 1);

    // All three specified
    {
        Mat4d m = Mat4d::fromCoordSystemAxes(&ax, &ay, &az);
        EXPECT_TRUE(m.isIdentity());
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_TRUE(ex == tx);
        EXPECT_TRUE(ey == ty);
        EXPECT_TRUE(ez == tz);
    }

    // Two axes specified
    {
        Mat4d m = Mat4d::fromCoordSystemAxes(&ax, &ay, NULL);
        EXPECT_TRUE(m.isIdentity());
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_TRUE(ex == tx);
        EXPECT_TRUE(ey == ty);
        EXPECT_TRUE(ez == tz);
    }

    {
        Mat4d m = Mat4d::fromCoordSystemAxes(&ax, NULL, &az);
        EXPECT_TRUE(m.isIdentity());
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_TRUE(ex == tx);
        EXPECT_TRUE(ey == ty);
        EXPECT_TRUE(ez == tz);
    }

    {
        Mat4d m = Mat4d::fromCoordSystemAxes(NULL, &ay, &az);
        EXPECT_TRUE(m.isIdentity());
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_TRUE(ex == tx);
        EXPECT_TRUE(ey == ty);
        EXPECT_TRUE(ez == tz);
    }

    // Just one axis specified
    {
        Mat4d m = Mat4d::fromCoordSystemAxes(&ax, NULL, NULL);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_TRUE(ex == tx);
        EXPECT_EQ(0, tx*ty);
        EXPECT_EQ(0, tx*tz);
        EXPECT_EQ(0, ty*tz);
    }

    {
        Mat4d m = Mat4d::fromCoordSystemAxes(NULL, &ay, NULL);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_TRUE(ey == ty);
        EXPECT_EQ(0, ty*tz);
        EXPECT_EQ(0, ty*tx);
        EXPECT_EQ(0, tx*tz);
    }

    {
        Mat4d m = Mat4d::fromCoordSystemAxes(NULL, NULL, &az);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_TRUE(ez == tz);
        EXPECT_EQ(0, tz*tx);
        EXPECT_EQ(0, tz*ty);
        EXPECT_EQ(0, tx*ty);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, CreateMatrixFromAxes)
{
    const Vec3d ax = Vec3d( 1, 0, 1).getNormalized();
    const Vec3d ay = Vec3d( 0, 1, 0).getNormalized();
    const Vec3d az = Vec3d(-1, 0, 1).getNormalized();
    ASSERT_TRUE((ax ^ ay) == az);

    const Vec3d ex( 1, 0, 1);
    const Vec3d ey( 0, 1, 0);
    const Vec3d ez(-1, 0, 1);

    // All three specified
    {
        Mat4d m = Mat4d::fromCoordSystemAxes(&ax, &ay, &az);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_DOUBLE_EQ(1.0, tx*ex.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, ty*ey.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, tz*ez.getNormalized());
    }

    // Two axes specified
    {
        Mat4d m = Mat4d::fromCoordSystemAxes(&ax, &ay, NULL);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_DOUBLE_EQ(1.0, tx*ex.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, ty*ey.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, tz*ez.getNormalized());
    }

    {
        Mat4d m = Mat4d::fromCoordSystemAxes(&ax, NULL, &az);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_DOUBLE_EQ(1.0, tx*ex.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, ty*ey.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, tz*ez.getNormalized());
    }

    {
        Mat4d m = Mat4d::fromCoordSystemAxes(NULL, &ay, &az);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_DOUBLE_EQ(1.0, tx*ex.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, ty*ey.getNormalized());
        EXPECT_DOUBLE_EQ(1.0, tz*ez.getNormalized());
    }

    // Just one
    {
        Mat4d m = Mat4d::fromCoordSystemAxes(&ax, NULL, NULL);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_DOUBLE_EQ(1.0, tx*ex.getNormalized());
        EXPECT_EQ(0, tx*ty);
        EXPECT_EQ(0, tx*tz);
        EXPECT_EQ(0, ty*tz);
    }

    {
        Mat4d m = Mat4d::fromCoordSystemAxes(NULL, &ay, NULL);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_DOUBLE_EQ(1.0, ty*ey.getNormalized());
        EXPECT_EQ(0, ty*tz);
        EXPECT_EQ(0, ty*tx);
        EXPECT_EQ(0, tx*tz);
    }

    {
        Mat4d m = Mat4d::fromCoordSystemAxes(NULL, NULL, &az);
        const Vec3d tx = Vec3d::X_AXIS.getTransformedVector(m);
        const Vec3d ty = Vec3d::Y_AXIS.getTransformedVector(m);
        const Vec3d tz = Vec3d::Z_AXIS.getTransformedVector(m);
        EXPECT_DOUBLE_EQ(1.0, tz*ez.getNormalized());
        EXPECT_EQ(0, tz*tx);
        EXPECT_EQ(0, tz*ty);
        EXPECT_EQ(0, tx*ty);
    }

    /*
    const Vec3d ax = Vec3d(0, 1, 0);
    const Vec3d az = Vec3d(0, 0, 1);
    Mat4d m = Mat4d::fromCoordSystemAxes(&ax, NULL, &az);

    const Vec3d tx = Vec3d(1, 0, 0).getTransformedVector(m);
    const Vec3d ty = Vec3d(0, 1, 0).getTransformedVector(m);
    const Vec3d tz = Vec3d(0, 0, 1).getTransformedVector(m);
    EXPECT_TRUE(Vec3d(0, 1, 0) == tx);
    EXPECT_TRUE(Vec3d(0, 0, 1) == ty);
    EXPECT_TRUE(Vec3d(1, 0, 0) == tz);
    */
}

 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, Determinant)
{
    Matrix4<double> md0;
    Matrix4<float>  mf0;
    md0.setZero();
    mf0.setZero();
    EXPECT_DOUBLE_EQ(0, md0.determinant());
    EXPECT_FLOAT_EQ(0, mf0.determinant());

    Matrix4<double> md1;
    Matrix4<float> mf1;
    EXPECT_DOUBLE_EQ(1.0, md1.determinant());
    EXPECT_FLOAT_EQ(1.0f, mf1.determinant());

    Matrix4<double> md2(  1,  2,  3,  4, 
                          5,  6,  7,  8, 
                          9, 10, 11, 12,
                         13, 14, 15, 16);
    Matrix4<float> mf2(md2);

    EXPECT_DOUBLE_EQ(0.0, md2.determinant());
    EXPECT_FLOAT_EQ(0.0f, mf2.determinant());

    Matrix4<double> md3(-13.0,  14.0, -26.0,  37.0,
                          0.0, -32.0,  15.0,  28.0,
                        -40.0, -43.0,  35.0, -21.0,
                        -36.0,  18.0, -16.0, -13.0);
    Matrix4<float> mf3(md3);

    EXPECT_DOUBLE_EQ(-105217.0, md3.determinant());
    EXPECT_FLOAT_EQ(-105217.0, mf3.determinant());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, InvertIdentityAndZero)
{
    Mat4d md0;
    md0.setZero();
    ASSERT_FALSE(md0.invert());
    ASSERT_TRUE(md0.isZero());

    Mat4f mf0;
    mf0.setZero();
    ASSERT_FALSE(mf0.invert());
    ASSERT_TRUE(mf0.isZero());


    Mat4d md1;
    ASSERT_TRUE(md1.invert());
    ASSERT_TRUE(md1.isIdentity());

    Mat4f mf1;
    ASSERT_TRUE(mf1.invert());
    ASSERT_TRUE(mf1.isIdentity());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, Inverse1)
{
    Matrix4<double> M(-13.0,  14.0, -26.0,  37.0,
                        0.0, -32.0,  15.0,  28.0,
                      -40.0, -43.0,  35.0, -21.0,
                      -36.0,  18.0, -16.0, -13.0);

    Matrix4<double> MI(-0.122423183,  0.146687322, -0.097018543,  0.124228974,
                        0.539266468, -0.707556764,  0.407719285, -0.647756541,
                        0.675328131, -0.881920222,  0.528355684, -0.830930363,
                        0.254521608, -0.300464754,  0.182917209, -0.295151924);


    {
        const double absErr = 1e-9;

        bool wasInverted = false;
        Matrix4<double> m = M.getInverted(&wasInverted);
        EXPECT_TRUE(wasInverted);
        EXPECT_NEAR(MI.rowCol(0, 0), m.rowCol(0, 0), absErr);
        EXPECT_NEAR(MI.rowCol(0, 1), m.rowCol(0, 1), absErr);
        EXPECT_NEAR(MI.rowCol(0, 2), m.rowCol(0, 2), absErr);
        EXPECT_NEAR(MI.rowCol(0, 3), m.rowCol(0, 3), absErr);
        EXPECT_NEAR(MI.rowCol(1, 0), m.rowCol(1, 0), absErr);
        EXPECT_NEAR(MI.rowCol(1, 1), m.rowCol(1, 1), absErr);
        EXPECT_NEAR(MI.rowCol(1, 2), m.rowCol(1, 2), absErr);
        EXPECT_NEAR(MI.rowCol(1, 3), m.rowCol(1, 3), absErr);
        EXPECT_NEAR(MI.rowCol(2, 0), m.rowCol(2, 0), absErr);
        EXPECT_NEAR(MI.rowCol(2, 1), m.rowCol(2, 1), absErr);
        EXPECT_NEAR(MI.rowCol(2, 2), m.rowCol(2, 2), absErr);
        EXPECT_NEAR(MI.rowCol(2, 3), m.rowCol(2, 3), absErr);
        EXPECT_NEAR(MI.rowCol(3, 0), m.rowCol(3, 0), absErr);
        EXPECT_NEAR(MI.rowCol(3, 1), m.rowCol(3, 1), absErr);
        EXPECT_NEAR(MI.rowCol(3, 2), m.rowCol(3, 2), absErr);
        EXPECT_NEAR(MI.rowCol(3, 3), m.rowCol(3, 3), absErr);
    }

    {
        const double absErr = 1e-5;

        bool wasInverted = false;
        Matrix4<double> m = MI.getInverted(&wasInverted);
        EXPECT_TRUE(wasInverted);
        EXPECT_NEAR(M.rowCol(0, 0), m.rowCol(0, 0), absErr);
        EXPECT_NEAR(M.rowCol(0, 1), m.rowCol(0, 1), absErr);
        EXPECT_NEAR(M.rowCol(0, 2), m.rowCol(0, 2), absErr);
        EXPECT_NEAR(M.rowCol(0, 3), m.rowCol(0, 3), absErr);
        EXPECT_NEAR(M.rowCol(1, 0), m.rowCol(1, 0), absErr);
        EXPECT_NEAR(M.rowCol(1, 1), m.rowCol(1, 1), absErr);
        EXPECT_NEAR(M.rowCol(1, 2), m.rowCol(1, 2), absErr);
        EXPECT_NEAR(M.rowCol(1, 3), m.rowCol(1, 3), absErr);
        EXPECT_NEAR(M.rowCol(2, 0), m.rowCol(2, 0), absErr);
        EXPECT_NEAR(M.rowCol(2, 1), m.rowCol(2, 1), absErr);
        EXPECT_NEAR(M.rowCol(2, 2), m.rowCol(2, 2), absErr);
        EXPECT_NEAR(M.rowCol(2, 3), m.rowCol(2, 3), absErr);
        EXPECT_NEAR(M.rowCol(3, 0), m.rowCol(3, 0), absErr);
        EXPECT_NEAR(M.rowCol(3, 1), m.rowCol(3, 1), absErr);
        EXPECT_NEAR(M.rowCol(3, 2), m.rowCol(3, 2), absErr);
        EXPECT_NEAR(M.rowCol(3, 3), m.rowCol(3, 3), absErr);
    }


    {
        const double absErr = 1e-7;

        Matrix4<float> m(M);
        EXPECT_TRUE(m.invert());
        EXPECT_NEAR(MI.rowCol(0, 0), m.rowCol(0, 0), absErr);
        EXPECT_NEAR(MI.rowCol(0, 1), m.rowCol(0, 1), absErr);
        EXPECT_NEAR(MI.rowCol(0, 2), m.rowCol(0, 2), absErr);
        EXPECT_NEAR(MI.rowCol(0, 3), m.rowCol(0, 3), absErr);
        EXPECT_NEAR(MI.rowCol(1, 0), m.rowCol(1, 0), absErr);
        EXPECT_NEAR(MI.rowCol(1, 1), m.rowCol(1, 1), absErr);
        EXPECT_NEAR(MI.rowCol(1, 2), m.rowCol(1, 2), absErr);
        EXPECT_NEAR(MI.rowCol(1, 3), m.rowCol(1, 3), absErr);
        EXPECT_NEAR(MI.rowCol(2, 0), m.rowCol(2, 0), absErr);
        EXPECT_NEAR(MI.rowCol(2, 1), m.rowCol(2, 1), absErr);
        EXPECT_NEAR(MI.rowCol(2, 2), m.rowCol(2, 2), absErr);
        EXPECT_NEAR(MI.rowCol(2, 3), m.rowCol(2, 3), absErr);
        EXPECT_NEAR(MI.rowCol(3, 0), m.rowCol(3, 0), absErr);
        EXPECT_NEAR(MI.rowCol(3, 1), m.rowCol(3, 1), absErr);
        EXPECT_NEAR(MI.rowCol(3, 2), m.rowCol(3, 2), absErr);
        EXPECT_NEAR(MI.rowCol(3, 3), m.rowCol(3, 3), absErr);
    }

    {
        // TODO: This err is too high, need internal double calculations in invert()
        const double absErr = 1e-4;

        Matrix4<float> m(MI);
        EXPECT_TRUE(m.invert());
        EXPECT_NEAR(M.rowCol(0, 0), m.rowCol(0, 0), absErr);
        EXPECT_NEAR(M.rowCol(0, 1), m.rowCol(0, 1), absErr);
        EXPECT_NEAR(M.rowCol(0, 2), m.rowCol(0, 2), absErr);
        EXPECT_NEAR(M.rowCol(0, 3), m.rowCol(0, 3), absErr);
        EXPECT_NEAR(M.rowCol(1, 0), m.rowCol(1, 0), absErr);
        EXPECT_NEAR(M.rowCol(1, 1), m.rowCol(1, 1), absErr);
        EXPECT_NEAR(M.rowCol(1, 2), m.rowCol(1, 2), absErr);
        EXPECT_NEAR(M.rowCol(1, 3), m.rowCol(1, 3), absErr);
        EXPECT_NEAR(M.rowCol(2, 0), m.rowCol(2, 0), absErr);
        EXPECT_NEAR(M.rowCol(2, 1), m.rowCol(2, 1), absErr);
        EXPECT_NEAR(M.rowCol(2, 2), m.rowCol(2, 2), absErr);
        EXPECT_NEAR(M.rowCol(2, 3), m.rowCol(2, 3), absErr);
        EXPECT_NEAR(M.rowCol(3, 0), m.rowCol(3, 0), absErr);
        EXPECT_NEAR(M.rowCol(3, 1), m.rowCol(3, 1), absErr);
        EXPECT_NEAR(M.rowCol(3, 2), m.rowCol(3, 2), absErr);
        EXPECT_NEAR(M.rowCol(3, 3), m.rowCol(3, 3), absErr);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, Inverse2)
{
    Matrix4<double> M( 0.9, -3.4,  3.4,  1.3,
                      -0.8, -4.7, -4.9,  3.9,
                      -3.9,  4.5, -3.9,  2.8,
                       0.0, -0.2, -2.8, -3.5);

    Matrix4<double> MI(-0.751158835,  0.128090640, -0.456029606, -0.501095968,
                       -0.304537855, -0.042482372, -0.061563634, -0.209702468,
                        0.261860776, -0.111388761,  0.083278386,  0.039766378,
                       -0.192086458,  0.091538573, -0.063104787, -0.305544390);

    const double absErr = 0.00001;

    {
        bool wasInverted = false;
        Matrix4<double> m = M.getInverted(&wasInverted);
        ASSERT_TRUE(wasInverted);
        ASSERT_NEAR(MI.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(MI.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(MI.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(MI.rowCol(0, 3), m.rowCol(0, 3), absErr);
        ASSERT_NEAR(MI.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(MI.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(MI.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(MI.rowCol(1, 3), m.rowCol(1, 3), absErr);
        ASSERT_NEAR(MI.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(MI.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(MI.rowCol(2, 2), m.rowCol(2, 2), absErr);
        ASSERT_NEAR(MI.rowCol(2, 3), m.rowCol(2, 3), absErr);
        ASSERT_NEAR(MI.rowCol(3, 0), m.rowCol(3, 0), absErr);
        ASSERT_NEAR(MI.rowCol(3, 1), m.rowCol(3, 1), absErr);
        ASSERT_NEAR(MI.rowCol(3, 2), m.rowCol(3, 2), absErr);
        ASSERT_NEAR(MI.rowCol(3, 3), m.rowCol(3, 3), absErr);
    }

    {
        bool wasInverted = false;
        Matrix4<double> m = MI.getInverted(&wasInverted);
        ASSERT_TRUE(wasInverted);
        ASSERT_NEAR(M.rowCol(0, 0), m.rowCol(0, 0), absErr);
        ASSERT_NEAR(M.rowCol(0, 1), m.rowCol(0, 1), absErr);
        ASSERT_NEAR(M.rowCol(0, 2), m.rowCol(0, 2), absErr);
        ASSERT_NEAR(M.rowCol(0, 3), m.rowCol(0, 3), absErr);
        ASSERT_NEAR(M.rowCol(1, 0), m.rowCol(1, 0), absErr);
        ASSERT_NEAR(M.rowCol(1, 1), m.rowCol(1, 1), absErr);
        ASSERT_NEAR(M.rowCol(1, 2), m.rowCol(1, 2), absErr);
        ASSERT_NEAR(M.rowCol(1, 3), m.rowCol(1, 3), absErr);
        ASSERT_NEAR(M.rowCol(2, 0), m.rowCol(2, 0), absErr);
        ASSERT_NEAR(M.rowCol(2, 1), m.rowCol(2, 1), absErr);
        ASSERT_NEAR(M.rowCol(2, 2), m.rowCol(2, 2), absErr);
        ASSERT_NEAR(M.rowCol(2, 3), m.rowCol(2, 3), absErr);
        ASSERT_NEAR(M.rowCol(3, 0), m.rowCol(3, 0), absErr);
        ASSERT_NEAR(M.rowCol(3, 1), m.rowCol(3, 1), absErr);
        ASSERT_NEAR(M.rowCol(3, 2), m.rowCol(3, 2), absErr);
        ASSERT_NEAR(M.rowCol(3, 3), m.rowCol(3, 3), absErr);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, Transpose)
{
    const Matrix4<double> m0( 11, 12, 13, 14, 
                              21, 22, 23, 24, 
                              31, 32, 33, 34, 
                              41, 42, 43, 44);

    const Matrix4<double> mt( 11, 21, 31, 41, 
                              12, 22, 32, 42, 
                              13, 23, 33, 43, 
                              14, 24, 34, 44);

    Matrix4<double> m(m0);
    m.transpose();
    
    EXPECT_TRUE(m == mt);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, GetTransposed)
{
    const Matrix4<double> m0( 11, 12, 13, 14, 
                              21, 22, 23, 24, 
                              31, 32, 33, 34, 
                              41, 42, 43, 44);

    const Matrix4<double> mt( 11, 21, 31, 41, 
                              12, 22, 32, 42, 
                              13, 23, 33, 43, 
                              14, 24, 34, 44);

    Matrix4<double> m = m0.getTransposed();
    
    EXPECT_TRUE(m == mt);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, SetFromMatrix3)
{
    const Matrix3<double> m3( 11, 12, 13,
                              21, 22, 23,
                              31, 32, 33);

    Matrix4<double> m4(99, 99, 99, 99,
                       99, 99, 99, 99,
                       99, 99, 99, 99,
                       99, 99, 99, 99);
    m4.setFromMatrix3(m3);

    ASSERT_DOUBLE_EQ(11, m4.rowCol(0, 0));
    ASSERT_DOUBLE_EQ(12, m4.rowCol(0, 1));
    ASSERT_DOUBLE_EQ(13, m4.rowCol(0, 2));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(0, 3));

    ASSERT_DOUBLE_EQ(21, m4.rowCol(1, 0));
    ASSERT_DOUBLE_EQ(22, m4.rowCol(1, 1));
    ASSERT_DOUBLE_EQ(23, m4.rowCol(1, 2));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(1, 3));

    ASSERT_DOUBLE_EQ(31, m4.rowCol(2, 0));
    ASSERT_DOUBLE_EQ(32, m4.rowCol(2, 1));
    ASSERT_DOUBLE_EQ(33, m4.rowCol(2, 2));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(2, 3));

    ASSERT_DOUBLE_EQ( 0, m4.rowCol(3, 0));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(3, 1));
    ASSERT_DOUBLE_EQ( 0, m4.rowCol(3, 2));
    ASSERT_DOUBLE_EQ( 1, m4.rowCol(3, 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, ToMatrix3)
{
    const Matrix4<double> m4( 11, 12, 13, 14, 
                              21, 22, 23, 24, 
                              31, 32, 33, 34, 
                              41, 42, 43, 44);

    {
        Matrix3<double> m3 = m4.toMatrix3();

        ASSERT_DOUBLE_EQ(11, m3.rowCol(0, 0));
        ASSERT_DOUBLE_EQ(12, m3.rowCol(0, 1));
        ASSERT_DOUBLE_EQ(13, m3.rowCol(0, 2));

        ASSERT_DOUBLE_EQ(21, m3.rowCol(1, 0));
        ASSERT_DOUBLE_EQ(22, m3.rowCol(1, 1));
        ASSERT_DOUBLE_EQ(23, m3.rowCol(1, 2));

        ASSERT_DOUBLE_EQ(31, m3.rowCol(2, 0));
        ASSERT_DOUBLE_EQ(32, m3.rowCol(2, 1));
        ASSERT_DOUBLE_EQ(33, m3.rowCol(2, 2));
    }

    {
        Matrix3<double> m3;
        m4.toMatrix3(&m3);

        ASSERT_DOUBLE_EQ(11, m3.rowCol(0, 0));
        ASSERT_DOUBLE_EQ(12, m3.rowCol(0, 1));
        ASSERT_DOUBLE_EQ(13, m3.rowCol(0, 2));

        ASSERT_DOUBLE_EQ(21, m3.rowCol(1, 0));
        ASSERT_DOUBLE_EQ(22, m3.rowCol(1, 1));
        ASSERT_DOUBLE_EQ(23, m3.rowCol(1, 2));

        ASSERT_DOUBLE_EQ(31, m3.rowCol(2, 0));
        ASSERT_DOUBLE_EQ(32, m3.rowCol(2, 1));
        ASSERT_DOUBLE_EQ(33, m3.rowCol(2, 2));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, FromRotation)
{
    const double absErr = 0.00001;

    // Expected data from Calc 3D Prof app

    {
        const Vector3<double> axis(1, 1, 1);
        const double angleDeg = 45;
        const Matrix4<double> me( 0.80474, -0.31062,  0.50588, 0,
                                  0.50588,  0.80474, -0.31062, 0,
                                 -0.31062,  0.50588,  0.80474, 0,
                                  0,        0,        0,       1);

        Matrix4<double> m = Matrix4<double>::fromRotation(axis, Math::toRadians(angleDeg));

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
        const Matrix4<double> me(-0.29313,  0.84403,  0.44910, 0,
                                  0.44910, -0.29313,  0.84403, 0,
                                  0.84403,  0.44910, -0.29313, 0,
                                  0,        0,        0,       1);

        Matrix4<double> m = Matrix4<double>::fromRotation(axis, Math::toRadians(angleDeg));

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
        const Matrix4<double> me(-0.73274, -0.13432,  0.66712, 0,
                                  0.66747, -0.33288,  0.66609, 0,
                                  0.13260,  0.93336,  0.33356, 0,
                                  0,        0,        0,       1);

        Matrix4<double> m = Matrix4<double>::fromRotation(axis, Math::toRadians(angleDeg));

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
        const Matrix4<double> me( 0.90763,  0.11798,  0.40284, 0,
                                  0.25149, -0.92122, -0.29683, 0,
                                  0.33609,  0.37072, -0.86580, 0,
                                  0,        0,        0,       1);

        Matrix4<double> m = Matrix4<double>::fromRotation(axis, Math::toRadians(angleDeg));

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
        const Matrix4<double> me(-0.90920,  0.36111,  0.20727, 0,
                                  0.39500,  0.90549,  0.15514, 0,
                                 -0.13166,  0.22292, -0.96590, 0,
                                  0,        0,        0,       1);

        Matrix4<double> m = Matrix4<double>::fromRotation(axis, Math::toRadians(angleDeg));

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
        const Matrix4<double> me(-0.848250, -0.45241,  0.27531, 0,
                                  0.523490, -0.79494,  0.30664, 0,
                                  0.080127,  0.40423,  0.91114, 0,
                                  0,         0,        0,       1);

        Matrix4<double> m = Matrix4<double>::fromRotation(axis, Math::toRadians(angleDeg));

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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Matrix4Test, PostMultVec4)
{
    Mat4d m;
    Vec4d v(1,2,3,4);

    Vec4d r = m * v;

    EXPECT_DOUBLE_EQ(1, r.x());
    EXPECT_DOUBLE_EQ(2, r.y());
    EXPECT_DOUBLE_EQ(3, r.z());
    EXPECT_DOUBLE_EQ(4, r.w());

    Mat4d M( 1,  2,  3,  4,
             5,  6,  7,  8,
             9,-10, 11, 12,
            13, 14, 15, 16);

    Vec4d V(1, -2, 3, 4);

    Vec4d V1 = M*V;

    EXPECT_DOUBLE_EQ(22, V1.x());
    EXPECT_DOUBLE_EQ(46, V1.y());
    EXPECT_DOUBLE_EQ(110, V1.z());
    EXPECT_DOUBLE_EQ(94, V1.w());
}

/*
#ifdef _DEBUG
TEST(Matrix4DeathTest, CheckElementAccessAsserts)
{
    Matrix4<double> m;

    ASSERT_DEATH(m.rowCol(-1, 0), "");
}
#endif
*/

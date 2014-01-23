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
#include "cvfArray.h"

#include "gtest/gtest.h"
#include <iostream>
#include <algorithm>

using namespace cvf;


class MyIntValueArr : public IntValueArray
{
public:
    virtual int     val(size_t index) const  { return static_cast<int>(100 + index); }
    virtual size_t  size() const             { return 5; }
};




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, DefaultConstructor)
{
    IntArray ia;
    ASSERT_EQ(0u, ia.size());
    ASSERT_EQ(0u, ia.capacity());
    ASSERT_TRUE(ia.ptr() == NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, CopyConstructor)
{
    IntArray a0;

    {
        // Copy constructor with empty array should be OK
        IntArray a(a0);
        ASSERT_EQ(0u, a.size());
    }

    a0.resize(3);
    ASSERT_EQ(3u, a0.size());
    a0[0] = 99;
    a0[1] = 10;
    a0[2] = 3;
    EXPECT_EQ(99, a0[0]);
    EXPECT_EQ(10, a0[1]);
    EXPECT_EQ(3,  a0[2]);

    {
        IntArray a(a0);
        ASSERT_EQ(3u, a.size());
        EXPECT_EQ(99, a[0]);
        EXPECT_EQ(10, a[1]);
        EXPECT_EQ(3,  a[2]);

        a0[1] = -1;

        EXPECT_EQ(99, a[0]);
        EXPECT_EQ(10, a[1]);
        EXPECT_EQ(3,  a[2]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ConstructorFromPointer)
{
    int* data = new int[3];
    data[0] = 1;
    data[1] = 2;
    data[2] = 99;

    IntArray* ai = new IntArray(data, 3);
    ASSERT_EQ(3u, ai->size());
    ASSERT_EQ(1, (*ai)[0]);
    ASSERT_EQ(2, (*ai)[1]);
    ASSERT_EQ(99, (*ai)[2]);

    data[2] = 3;

    ASSERT_EQ(99, (*ai)[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ArrayDeathTest, IllegalConstructorFromPointer)
{
    EXPECT_DEATH(new IntArray(NULL, 0), "Assertion");
    EXPECT_DEATH(new IntArray(NULL, 1), "Assertion");

    int* data = new int[3];
    EXPECT_DEATH(new IntArray(data, 0), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ConstructorWithSize)
{
    IntArray a(2);
    ASSERT_EQ(2u, a.size());
    a[0] = 10;
    a[1] = 20;
    ASSERT_EQ(10, a[0]);
    ASSERT_EQ(20, a[1]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ArrayDeathTest, ConstructorWithZeroSize)
{
    EXPECT_DEATH(new IntArray(0), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ConstructorFromValueArray)
{
    MyIntValueArr valArr;
    ASSERT_EQ(5, valArr.size());
    EXPECT_EQ(104, valArr.val(4));

    IntArray* ai = new IntArray(valArr);
    ASSERT_EQ(5u, ai->size());
    ASSERT_EQ(100, (*ai)[0]);
    ASSERT_EQ(101, (*ai)[1]);
    ASSERT_EQ(102, (*ai)[2]);
    ASSERT_EQ(103, (*ai)[3]);
    ASSERT_EQ(104, (*ai)[4]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, BasicIntArray)
{
    // IntArray
    IntArray iA;

    iA.resize(3);
    ASSERT_EQ(3u, iA.size());

    iA[0] = 99;
    iA[1] = 10;
    iA[2] = 3;

    ASSERT_EQ(99, iA[0]);
    ASSERT_EQ(10, iA[1]);
    ASSERT_EQ(3, iA[2]);

    iA.clear();
    ASSERT_EQ(0u, iA.size());

    int* data = new int[3];
    data[0] = 1;
    data[1] = 2;
    data[2] = 99;

    IntArray* ai = new IntArray(data, 3);
    ASSERT_EQ(3u, ai->size());
    ASSERT_EQ(1, (*ai)[0]);
    ASSERT_EQ(2, (*ai)[1]);
    ASSERT_EQ(99, (*ai)[2]);

    data[2] = 3;

    ASSERT_EQ(99, (*ai)[2]);

    IntArray i2(*ai);
    delete ai;
    ai = NULL;

    ASSERT_EQ(3u, i2.size());
    ASSERT_EQ(1, i2[0]);
    ASSERT_EQ(2, i2[1]);
    ASSERT_EQ(99, i2[2]);

    IntArray i3(2);
    i3[0] = 10;
    i3[1] = 20;
    ASSERT_EQ(2u, i3.size());
    ASSERT_EQ(10, i3[0]);
    ASSERT_EQ(20, i3[1]);

    int* p = i3.ptr();
    ASSERT_EQ(10, p[0]);
    ASSERT_EQ(20, p[1]);

    const IntArray* pi = &i3;
    const int* cp = pi->ptr();
    ASSERT_EQ(10, cp[0]);
    ASSERT_EQ(20, cp[1]);

    const IntArray& ci = i3;
    ASSERT_EQ(10, ci[0]);
    ASSERT_EQ(20, ci[1]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, AssignmentOperator)
{
    FloatArray a0;

    FloatArray a1;
    a1.resize(2);
    a1[0] = 10;
    a1[1] = 11;

    {
        FloatArray a;
        a = a0;

        ASSERT_EQ(0u, a.size());
    }

    {
        FloatArray a;
        a = a1;

        ASSERT_EQ(2u, a.size());
        ASSERT_EQ(10, a[0]);
        ASSERT_EQ(11, a[1]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ArrayDeathTest, AssignWithIllegalData)
{
    FloatArray a;
    EXPECT_DEATH(a.assign(NULL, 0), "Assertion");
    EXPECT_DEATH(a.assign(NULL, 1), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, GetterAndSetter)
{
    FloatArray a1;
    a1.resize(2);
    a1.set(0, 10);
    a1.set(1, 11);

    ASSERT_EQ(10, a1.get(0));
    ASSERT_EQ(11, a1.get(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(ArrayDeathTest, SetterAndGetter)
{
    FloatArray a;
    EXPECT_DEATH(a.set(0, 99), "Assertion");
    EXPECT_DEATH(a.get(0), "Assertion");
    EXPECT_DEATH(a[0], "Assertion");

    a.resize(2);
    EXPECT_DEATH(a.set(2, 99), "Assertion");

    EXPECT_DEATH(a.get(2), "Assertion");
    EXPECT_DEATH(a[2], "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, SetAllElements)
{
    FloatArray a1;
    a1.resize(2);
    a1.setAll(1.23f);

    ASSERT_EQ(1.23f, a1.get(0));
    ASSERT_EQ(1.23f, a1.get(1));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, BasicVec3fArray)
{
    // Vec3f array
    Vec3fArray vA;
    vA.resize(4);
    ASSERT_EQ(4u, vA.size());

    vA[0] = Vec3f(1,2,3);
    vA[1] = Vec3f(1.1f, 2.2f, 3.3f);
    vA[2] = Vec3f(0,0,0);
    vA[3] = Vec3f(4,5,6);

    ASSERT_EQ(true, vA[0] == Vec3f(1, 2, 3));
    ASSERT_EQ(true, vA[1] == Vec3f(1.1f, 2.2f, 3.3f));
    ASSERT_EQ(true, vA[2] == Vec3f(0, 0, 0));
    ASSERT_EQ(true, vA[3] == Vec3f(4, 5, 6));

    const float* pf = vA.ptr()->ptr();

    ASSERT_FLOAT_EQ(1.0f, pf[0]);
    ASSERT_FLOAT_EQ(2.0f, pf[1]);
    ASSERT_FLOAT_EQ(3.0f, pf[2]);
    ASSERT_FLOAT_EQ(1.1f, pf[3]);
    ASSERT_FLOAT_EQ(2.2f, pf[4]);
    ASSERT_FLOAT_EQ(3.3f, pf[5]);
    ASSERT_FLOAT_EQ(0.0f, pf[6]);
    ASSERT_FLOAT_EQ(0.0f, pf[7]);
    ASSERT_FLOAT_EQ(0.0f, pf[8]);
    ASSERT_FLOAT_EQ(4.0f, pf[9]);
    ASSERT_FLOAT_EQ(5.0f, pf[10]);
    ASSERT_FLOAT_EQ(6.0f, pf[11]);

    vA.clear();

    ASSERT_EQ(0u, vA.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, BasicVec3dArray)
{
    // Vec3f array
    Vec3dArray vA;
    vA.resize(4);
    ASSERT_EQ(4u, vA.size());

    vA[0] = Vec3d(1,2,3);
    vA[1] = Vec3d(1.1, 2.2, 3.3);
    vA[2] = Vec3d(0,0,0);
    vA[3] = Vec3d(4,5,6);

    ASSERT_EQ(true, vA[0] == Vec3d(1, 2, 3));
    ASSERT_EQ(true, vA[1] == Vec3d(1.1, 2.2, 3.3));
    ASSERT_EQ(true, vA[2] == Vec3d(0, 0, 0));
    ASSERT_EQ(true, vA[3] == Vec3d(4, 5, 6));

    const double* pf = vA.ptr()->ptr();

    ASSERT_DOUBLE_EQ(1.0, pf[0]);
    ASSERT_DOUBLE_EQ(2.0, pf[1]);
    ASSERT_DOUBLE_EQ(3.0, pf[2]);
    ASSERT_DOUBLE_EQ(1.1, pf[3]);
    ASSERT_DOUBLE_EQ(2.2, pf[4]);
    ASSERT_DOUBLE_EQ(3.3, pf[5]);
    ASSERT_DOUBLE_EQ(0.0, pf[6]);
    ASSERT_DOUBLE_EQ(0.0, pf[7]);
    ASSERT_DOUBLE_EQ(0.0, pf[8]);
    ASSERT_DOUBLE_EQ(4.0, pf[9]);
    ASSERT_DOUBLE_EQ(5.0, pf[10]);
    ASSERT_DOUBLE_EQ(6.0, pf[11]);

    vA.clear();

    ASSERT_EQ(0u, vA.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ptrToIdx)
{
    // Vec3f array
    Vec3fArray vA;
    vA.resize(4);
    ASSERT_EQ(4u, vA.size());

    vA[0] = Vec3f(1,2,3);
    vA[1] = Vec3f(1.1f, 2.2f, 3.3f);
    vA[2] = Vec3f(0,0,0);
    vA[3] = Vec3f(4,5,6);

    Vec3f* p1 = vA.ptr(1);
    ASSERT_FLOAT_EQ(1.1f, p1->x());
    ASSERT_FLOAT_EQ(2.2f, p1->y());
    ASSERT_FLOAT_EQ(3.3f, p1->z());

    Vec3f* p3 = vA.ptr(3);
    ASSERT_FLOAT_EQ(4, p3->x());
    ASSERT_FLOAT_EQ(5, p3->y());
    ASSERT_FLOAT_EQ(6, p3->z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, SharedData)
{
    float* f = new float[6];
    f[0] = 1;
    f[1] = 2;
    f[2] = 3;
    f[3] = 10;
    f[4] = 11;
    f[5] = 12;

    {
        Vec3fArray af;
        af.setSharedPtr(reinterpret_cast<Vec3f*> (f), 2);       // Naughty! How to do this differently

        ASSERT_FLOAT_EQ(1.0f, af[0].x());
        ASSERT_FLOAT_EQ(2.0f, af[0].y());
        ASSERT_FLOAT_EQ(3.0f, af[0].z());
        ASSERT_EQ(true, af[1] == Vec3f(10,11,12));
    }

    ASSERT_FLOAT_EQ(1.0f, f[0]);
    ASSERT_FLOAT_EQ(2.0f, f[1]);
    ASSERT_FLOAT_EQ(3.0f, f[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ArrayDeathTest, IllegalUsageWithSharedData)
{
    float* f = new float[2];
    f[0] = 1;
    f[1] = 2;

    {
        FloatArray a;
        a.setSharedPtr(f, 2);

        std::vector<float> sv;
        sv.push_back(1);

        EXPECT_DEATH(a.assign(&sv[0], 1), "Assertion");
        EXPECT_DEATH(a.assign(sv), "Assertion");
        EXPECT_DEATH(a.resize(10), "Assertion");

        EXPECT_DEATH(a.setPtr(&sv[0], 1), "Assertion");

        EXPECT_DEATH(a.reserve(10), "Assertion");
        EXPECT_DEATH(a.squeeze(), "Assertion");

#if CVF_ENABLE_TIGHT_ASSERTS == 1
        EXPECT_DEATH(a.setSizeZero(), "Assertion");
#endif
    }
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, SetPtr)
{
    float* f = new float[6];
    f[0] = 1;
    f[1] = 2;
    f[2] = 3;
    f[3] = 10;
    f[4] = 11;
    f[5] = 12;

    Vec3fArray af;
    af.setPtr(reinterpret_cast<Vec3f*> (f), 2);     // Naughty! How to do this differently
    f = 0; // af has owership

    ASSERT_FLOAT_EQ(1.0f, af[0].x());
    ASSERT_FLOAT_EQ(2.0f, af[0].y());
    ASSERT_FLOAT_EQ(3.0f, af[0].z());
    ASSERT_EQ(true, af[1] == Vec3f(10,11,12));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ArrayDeathTest, SetPtrWithIllegalData)
{
    double* pD = new double[5];

    DoubleArray a;
    EXPECT_DEATH(a.setPtr(NULL, 1), "Assertion");
    EXPECT_DEATH(a.setPtr(pD, 0), "Assertion");

    EXPECT_DEATH(a.setSharedPtr(NULL, 1), "Assertion");
    EXPECT_DEATH(a.setSharedPtr(pD, 0), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, AssignFromRawArray)
{
    float* f = new float[6];
    f[0] = 1;
    f[1] = 2;
    f[2] = 3;
    f[3] = 10;
    f[4] = 11;
    f[5] = 12;

    Vec3fArray af;
    af.assign(reinterpret_cast<Vec3f*> (f), 2);     // Naughty! How to do this differently
    delete[] f;
    f = 0;

    ASSERT_FLOAT_EQ(1.0f, af[0].x());
    ASSERT_FLOAT_EQ(2.0f, af[0].y());
    ASSERT_FLOAT_EQ(3.0f, af[0].z());
    ASSERT_EQ(true, af[1] == Vec3f(10,11,12));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, resize)
{
    // Vec3f array
    Vec3dArray vA;
    vA.resize(4);
    ASSERT_EQ(4u, vA.size());

    vA[0] = Vec3d(1,2,3);
    vA[1] = Vec3d(1.1, 2.2, 3.3);
    vA[2] = Vec3d(0,0,0);
    vA[3] = Vec3d(4,5,6);

    vA.resize(5);
    vA[4] = Vec3d(9.9, 0, 3.5);

    ASSERT_EQ(5u, vA.size());
    ASSERT_EQ(true, vA[0] == Vec3d(1,2,3));
    ASSERT_EQ(true, vA[4] == Vec3d(9.9,0,3.5));

    vA.resize(3);
    ASSERT_EQ(3u, vA.size());
    ASSERT_EQ(true, vA[0] == Vec3d(1, 2, 3));
    ASSERT_EQ(true, vA[1] == Vec3d(1.1, 2.2, 3.3));
    ASSERT_EQ(true, vA[2] == Vec3d(0, 0, 0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, CopyData)
{
    DoubleArray a;

    a.resize(6);

    a[0] = 1.23;
    a[1] = 4.56;
    a[2] = 7.89;

    double data[] = { 3.2, 1.0, 0.0 };
    
    a.copyData(data, 3, 3);

    ASSERT_DOUBLE_EQ(3.2, a[3]);
    ASSERT_DOUBLE_EQ(1.0, a[4]);
    ASSERT_DOUBLE_EQ(0.0, a[5]);

    DoubleArray b;
    b.resize(3);
    b[0] = 10.0;
    b[1] = 20.0;
    b[2] = 30.0;

    a.copyData(b, b.size(), 2, 0);

    ASSERT_DOUBLE_EQ(1.23, a[0]);
    ASSERT_DOUBLE_EQ(4.56, a[1]);
    ASSERT_DOUBLE_EQ(10.0, a[2]);
    ASSERT_DOUBLE_EQ(20.0, a[3]);
    ASSERT_DOUBLE_EQ(30.0, a[4]);
    ASSERT_DOUBLE_EQ(0.0, a[5]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ArrayDeathTest, CopyDataRawPtrWithIllegalData)
{
    float* pf = new float[6];
    FloatArray a;
    
    EXPECT_DEATH(a.copyData(NULL, 0, 0), "Assertion");
    EXPECT_DEATH(a.copyData(NULL, 1, 0), "Assertion");

    EXPECT_DEATH(a.copyData(pf, 1, 0), "Assertion");
    EXPECT_DEATH(a.copyData(pf, 1, 1), "Assertion");

    a.resize(5);
    EXPECT_DEATH(a.copyData(pf, 6, 0), "Assertion");
    EXPECT_DEATH(a.copyData(pf, 1, 5), "Assertion");
    EXPECT_DEATH(a.copyData(pf, 2, 4), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ArrayDeathTest, CopyDataArrayWithIllegalData)
{
    FloatArray src;
    FloatArray a;

    EXPECT_DEATH(a.copyData(src, 0, 0, 0), "Assertion");
    EXPECT_DEATH(a.copyData(src, 1, 0, 0), "Assertion");

    src.resize(6);
    EXPECT_DEATH(a.copyData(src, 5, 0, 0), "Assertion");

    a.resize(5);
    EXPECT_DEATH(a.copyData(src, 6, 0, 0), "Assertion");
    EXPECT_DEATH(a.copyData(src, 6, 0, 0), "Assertion");
    EXPECT_DEATH(a.copyData(src, 5, 1, 0), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, CopyConvertedData)
{
    // Double array to float
    {
        DoubleArray ad;
        ad.resize(4);
        ad[0] = 0.0;
        ad[1] = 1.0;
        ad[2] = 2.0;
        ad[3] = 3.0;

        // Copy full array
        FloatArray af;
        af.resize(4);
        af.copyConvertedData(ad, 4, 0, 0);
        EXPECT_FLOAT_EQ(0.0f, af[0]);
        EXPECT_FLOAT_EQ(1.0f, af[1]);
        EXPECT_FLOAT_EQ(2.0f, af[2]);
        EXPECT_FLOAT_EQ(3.0f, af[3]);

        // Copy partial array to float array
        af.resize(2);
        af.setAll(0);
        af.copyConvertedData(ad, 2, 0, 1);

        EXPECT_FLOAT_EQ(1.0f, af[0]);
        EXPECT_FLOAT_EQ(2.0f, af[1]);
    }

    // Vec3d to Vec3f and Vec3i
    {
        Vec3dArray ad;
        ad.resize(2);
        ad[0].set(1.1, 2.5, 3.9);
        ad[1].set(11.1, 12.5, 13.9);

        Vec3fArray af;
        af.resize(2);
        af.copyConvertedData(ad, 2, 0, 0);
        EXPECT_FLOAT_EQ(1.1f,  af[0].x());
        EXPECT_FLOAT_EQ(2.5f,  af[0].y());
        EXPECT_FLOAT_EQ(3.9f,  af[0].z());
        EXPECT_FLOAT_EQ(11.1f, af[1].x());
        EXPECT_FLOAT_EQ(12.5f, af[1].y());
        EXPECT_FLOAT_EQ(13.9f, af[1].z());

        Array<Vec3i> ai;
        ai.resize(2);
        ai.copyConvertedData(ad, 2, 0, 0);
        EXPECT_EQ(1,  ai[0].x());
        EXPECT_EQ(2,  ai[0].y());
        EXPECT_EQ(3,  ai[0].z());
        EXPECT_EQ(11, ai[1].x());
        EXPECT_EQ(12, ai[1].y());
        EXPECT_EQ(13, ai[1].z());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ReserveAndAdd)
{
    FloatArray a;
    a.reserve(5);
    a.add(1.0f);
    a.add(3.3f);
    a.add(5.5f);

    ASSERT_EQ(3u, a.size());
    ASSERT_TRUE(a.capacity() >= 5);
    ASSERT_EQ(1.0f, a[0]);
    ASSERT_EQ(3.3f, a[1]);
    ASSERT_EQ(5.5f, a[2]);

    // To test reuse of buffer
    float* before = a.ptr();

    a.reserve(3);
    ASSERT_TRUE(a.capacity() >= 5);

    float* after = a.ptr();

    // Check that no realloc has been done
    ASSERT_EQ(before, after);
    ASSERT_TRUE(a.capacity() >= 5);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ReserveGrow)
{
    FloatArray a;
    a.resize(3);
    a.set(0, 1.0f);
    a.set(1, 3.3f);
    a.set(2, 5.5f);

    float* before = a.ptr();
    a.reserve(8);
    float* after = a.ptr();
    a.add(2.2f);
    a.add(2.5f);

    ASSERT_NE(before, after);

    ASSERT_EQ(5u, a.size());
    ASSERT_TRUE(a.capacity() >= 8);
    ASSERT_EQ(1.0f, a[0]);
    ASSERT_EQ(3.3f, a[1]);
    ASSERT_EQ(5.5f, a[2]);
    ASSERT_EQ(2.2f, a[3]);
    ASSERT_EQ(2.5f, a[4]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(ArrayDeathTest, AddBeyondCapacity)
{
    DoubleArray a;
    EXPECT_DEATH(a.add(100), "Assertion");

    a.reserve(2);
    a.add(100);
    a.add(101);
    EXPECT_DEATH(a.add(102), "Assertion");

    a.reserve(4);
    a.add(102);
    a.add(103);
    EXPECT_DEATH(a.add(104), "Assertion");

    ASSERT_EQ(4u, a.size());
    EXPECT_DOUBLE_EQ(100, a[0]);
    EXPECT_DOUBLE_EQ(101, a[1]);
    EXPECT_DOUBLE_EQ(102, a[2]);
    EXPECT_DOUBLE_EQ(103, a[3]);
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, Squeeze)
{
    FloatArray a;
    a.reserve(5);
    a.add(1.0f);
    a.add(3.3f);
    a.add(5.5f);

    ASSERT_EQ(3u, a.size());
    ASSERT_TRUE(5 <= a.capacity());

    a.squeeze();
    ASSERT_EQ(3u, a.size());
    ASSERT_EQ(3u, a.capacity());
    ASSERT_EQ(1.0f, a[0]);
    ASSERT_EQ(3.3f, a[1]);
    ASSERT_EQ(5.5f, a[2]);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, SqueezeEmptyArray)
{
    FloatArray a;
    a.reserve(5);

    ASSERT_EQ(0, a.size());
    ASSERT_TRUE(5 <= a.capacity());

    a.squeeze();
    ASSERT_EQ(0, a.size());
    ASSERT_EQ(0, a.capacity());
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, SetSizeZero)
{
    FloatArray a;
    a.reserve(3);
    a.add(1.0f);
    a.add(3.3f);
    a.add(5.5f);

    float* before = a.ptr();
    a.setSizeZero();
    float* after = a.ptr();

    ASSERT_EQ(before, after);
    ASSERT_EQ(0u, a.size());
    ASSERT_TRUE(3 <= a.capacity());

    a.add(1.1f);
    a.add(3.4f);
    a.add(5.6f);

    ASSERT_EQ(3u, a.size());
    ASSERT_EQ(1.1f, a[0]);
    ASSERT_EQ(3.4f, a[1]);
    ASSERT_EQ(5.6f, a[2]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, StdInterop)
{
    // Assignment and construction with empty std::Vector
    std::vector<Vec3f> sa0;
    Vec3fArray a0(sa0);
    ASSERT_EQ(0u, a0.size());

    a0.assign(sa0);
    ASSERT_EQ(0u, a0.size());


    std::vector<Vec3f> sa;
    sa.push_back(Vec3f(1.0f,2.0f,3.3f));
    sa.push_back(Vec3f(1.1f,2.1f,3.4f));
    sa.push_back(Vec3f(1.2f,2.2f,3.5f));

    Vec3fArray a1(sa);

    ASSERT_EQ(3u, a1.size());
    ASSERT_EQ(true, a1[0] == Vec3f(1.0f,2.0f,3.3f));
    ASSERT_EQ(true, a1[1] == Vec3f(1.1f,2.1f,3.4f));
    ASSERT_EQ(true, a1[2] == Vec3f(1.2f,2.2f,3.5f));

    Vec3fArray a2;
    a2.assign(sa);

    ASSERT_EQ(3u, a2.size());
    ASSERT_EQ(true, a2[0] == Vec3f(1.0f,2.0f,3.3f));
    ASSERT_EQ(true, a2[1] == Vec3f(1.1f,2.1f,3.4f));
    ASSERT_EQ(true, a2[2] == Vec3f(1.2f,2.2f,3.5f));

    Vec3fArray a3;
    a3.assign(sa);

    ASSERT_EQ(3u, a3.size());
    ASSERT_EQ(true, a3[0] == Vec3f(1.0f,2.0f,3.3f));
    ASSERT_EQ(true, a3[1] == Vec3f(1.1f,2.1f,3.4f));
    ASSERT_EQ(true, a3[2] == Vec3f(1.2f,2.2f,3.5f));

    std::vector<Vec3f> sv;
    a3.toStdVector(&sv);

    ASSERT_EQ(3u, sv.size());
    ASSERT_EQ(true, sv[0] == Vec3f(1.0f,2.0f,3.3f));
    ASSERT_EQ(true, sv[1] == Vec3f(1.1f,2.1f,3.4f));
    ASSERT_EQ(true, sv[2] == Vec3f(1.2f,2.2f,3.5f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, MinMaxFloat)
{
    FloatArray a;

    // Test empty arrays
    EXPECT_FLOAT_EQ(std::numeric_limits<float>::max(), a.min());
    EXPECT_FLOAT_EQ(std::numeric_limits<float>::min(), a.max());

    a.reserve(5);
    a.add(1.0f);
    a.add(-3.3f);
    a.add(123.5f);
    a.add(999.9f);
    a.add(-2.3f);

    float min = a.min();
    float max = a.max();

    EXPECT_FLOAT_EQ(-3.3f, min);
    EXPECT_FLOAT_EQ(999.9f, max);

    size_t minIdx = 0;
    size_t maxIdx = 0;

    a.min(&minIdx);
    a.max(&maxIdx);

    EXPECT_EQ(1, minIdx);
    EXPECT_EQ(3, maxIdx);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, MinMaxInt)
{
    IntArray a;

    // Test empty arrays
    EXPECT_EQ(std::numeric_limits<int>::max(), a.min());
    EXPECT_EQ(std::numeric_limits<int>::min(), a.max());

    a.reserve(5);
    a.add(999);
    a.add(1);
    a.add(-2);
    a.add(123);
    a.add(-3);

    int min = a.min();
    int max = a.max();

    EXPECT_EQ(-3, min);
    EXPECT_EQ(999, max);

    size_t minIdx = 0;
    size_t maxIdx = 0;

    a.min(&minIdx);
    a.max(&maxIdx);

    EXPECT_EQ(4, minIdx);
    EXPECT_EQ(0, maxIdx);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, Iterators)
{
    int data[] = { 5, 2, 99, 1, 4, 55, 48 };
    IntArray a(data, sizeof(data)/sizeof(int));
    EXPECT_EQ(7, a.size());

    int sum = 0;
    IntArray::iterator it;
    for(it = a.begin(); it != a.end(); it++)
    {
        sum += *it;
        *it = 0;
    }

    EXPECT_EQ(214, sum);

    EXPECT_EQ(0, a[0]);
    EXPECT_EQ(0, a[1]);
    EXPECT_EQ(0, a[2]);
    EXPECT_EQ(0, a[3]);
    EXPECT_EQ(0, a[4]);
    EXPECT_EQ(0, a[5]);
    EXPECT_EQ(0, a[6]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ConstIterators)
{
    int data[] = { 5, 2, 99, 1, -4, 55, 48 };
    IntArray a(data, sizeof(data)/sizeof(int));
    EXPECT_EQ(7, a.size());

    int sum = 0;
    IntArray::const_iterator it;
    for(it = a.begin(); it != a.end(); it++)
    {
        sum += *it;
        //*it = 0;  compiler error!
    }

    EXPECT_EQ(206, sum);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, SortAndBinSearchTest)
{
    int data[] =
    {
        5,2,1,4
    };

    Array<int> arr(data, sizeof(data)/sizeof(int));
    EXPECT_EQ(4, arr.size());

    // Using standard stl sort algorithm
    std::sort(arr.begin(), arr.end());

    EXPECT_EQ(1, arr[0]);
    EXPECT_EQ(2, arr[1]);
    EXPECT_EQ(4, arr[2]);
    EXPECT_EQ(5, arr[3]);

    EXPECT_TRUE(std::binary_search(arr.begin(), arr.end(), 4));
    EXPECT_TRUE(std::binary_search(arr.begin(), arr.end(), 5));
    EXPECT_FALSE(std::binary_search(arr.begin(), arr.end(), 88));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void testFunc(int& val) { val = val*2; }
TEST(ArrayTest, ForEach)
{
    int data[] =
    {
        5,2,1,4
    };

    Array<int> arr(data, sizeof(data)/sizeof(int));
    EXPECT_EQ(4, arr.size());

    std::for_each(arr.begin(), arr.end(), testFunc);

    EXPECT_EQ(10, arr[0]);
    EXPECT_EQ(4, arr[1]);
    EXPECT_EQ(2, arr[2]);
    EXPECT_EQ(8, arr[3]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, Reverse)
{
    float data[] =
    {
        5.5f, 2.2f, 1.1f ,4.4f
    };

    FloatArray arr(data, sizeof(data)/sizeof(float));
    EXPECT_EQ(4, arr.size());

    std::reverse(arr.begin(), arr.end());

    EXPECT_FLOAT_EQ(4.4f, arr[0]);
    EXPECT_FLOAT_EQ(1.1f, arr[1]);
    EXPECT_FLOAT_EQ(2.2f, arr[2]);
    EXPECT_FLOAT_EQ(5.5f, arr[3]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool lessThan3(float val) { return val < 3.0f; }
TEST(ArrayTest, AlgPartition)
{
    float data[] =
    {
        5.5f, 2.2f, 1.1f ,4.4f, 3.3f
    };

    FloatArray arr(data, sizeof(data)/sizeof(float));
    EXPECT_EQ(5, arr.size());

    FloatArray::iterator partitionIt = std::partition(arr.begin(), arr.end(), lessThan3);

    int countLT3 = 0;
    FloatArray::iterator it;
    for (it = arr.begin(); it != partitionIt; it++)
    {
        EXPECT_TRUE(*it < 3.0f);
        countLT3++;
    }

    int countGT3 = 0;
    for (it = partitionIt; it != arr.end(); it++)
    {
        EXPECT_TRUE(*it > 3.0f);
        countGT3++;
    }

    EXPECT_EQ(2, countLT3);
    EXPECT_EQ(3, countGT3);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ExtractElementsFromIndexedArrayEmpty)
{
    FloatArray source;
    UIntArray indices;

    ref<FloatArray> arr = source.extractElements(indices);
    ASSERT_TRUE(arr.notNull());
    EXPECT_EQ(0, arr->size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, ExtractElementsFromIndexedArray)
{
    ///   source               = {2.0, 5.5, 100.0}
    ///   perItemSourceIndices = {  0,     2,   1,   0,     2}
    ///   -> output            = {2.0, 100.0, 5.5, 2.0, 100.0}
    FloatArray source;
    source.reserve(3);
    source.add(2.0f);
    source.add(5.5f);
    source.add(100.0f);

    UIntArray indices;
    indices.reserve(5);
    indices.add(0);
    indices.add(2);
    indices.add(1);
    indices.add(0);
    indices.add(2);

    ref<FloatArray> arr = source.extractElements(indices);
    ASSERT_EQ(5, arr->size());
    EXPECT_FLOAT_EQ(  2.0, arr->get(0));
    EXPECT_FLOAT_EQ(100.0, arr->get(1));
    EXPECT_FLOAT_EQ(  5.5, arr->get(2));
    EXPECT_FLOAT_EQ(  2.0, arr->get(3));
    EXPECT_FLOAT_EQ(100.0, arr->get(4));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, setConsecutiveEmpty)
{
    UIntArray arr;
    arr.setConsecutive(0);

    EXPECT_EQ(0, arr.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ArrayTest, setConsecutive)
{
    {
        UIntArray arr(3);
        arr.setConsecutive(0);

        ASSERT_EQ(3, arr.size());
        EXPECT_EQ(0, arr.get(0));
        EXPECT_EQ(1, arr.get(1));
        EXPECT_EQ(2, arr.get(2));
    }

    {
        UIntArray arr(4);
        arr.setConsecutive(9);

        ASSERT_EQ(4, arr.size());
        EXPECT_EQ(9, arr.get(0));
        EXPECT_EQ(10, arr.get(1));
        EXPECT_EQ(11, arr.get(2));
        EXPECT_EQ(12, arr.get(3));
    }
}

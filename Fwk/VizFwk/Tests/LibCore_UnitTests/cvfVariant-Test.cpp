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
#include "cvfVariant.h"

#include "gtest/gtest.h"
#include <iostream>

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, DefaultConstructor)
{
    Variant var;
    ASSERT_EQ(Variant::INVALID, var.type());
    ASSERT_FALSE(var.isValid());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, CopyConstructor)
{
    {
        const Variant var(-1234);
        ASSERT_TRUE(var.isValid());
        ASSERT_EQ(Variant::INT, var.type());

        Variant var2(var);
        ASSERT_TRUE(var2.isValid());
        ASSERT_EQ(Variant::INT, var2.type());
        ASSERT_EQ(-1234, var2.getInt());

        Variant var3 = var;
        ASSERT_TRUE(var3.isValid());
        ASSERT_EQ(Variant::INT, var3.type());
        ASSERT_EQ(-1234, var3.getInt());
    }

    {
        const String theString("MyStr");
        const Variant var(theString);
        ASSERT_TRUE(var.isValid());
        ASSERT_EQ(Variant::STRING, var.type());

        Variant var2(var);
        ASSERT_TRUE(var2.isValid());
        ASSERT_EQ(Variant::STRING, var2.type());
        ASSERT_TRUE(var2.getString() == theString);

        Variant var3 = var;
        ASSERT_TRUE(var3.isValid());
        ASSERT_EQ(Variant::STRING, var3.type());
        ASSERT_TRUE(var3.getString() == theString);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, AssignmentOperator)
{
    {
        const Variant var(-1234);
        ASSERT_TRUE(var.isValid());
        ASSERT_EQ(Variant::INT, var.type());

        Variant var2;
        var2 = var;
        ASSERT_TRUE(var2.isValid());
        ASSERT_EQ(Variant::INT, var2.type());
        ASSERT_EQ(-1234, var2.getInt());
    }

    {
        const String theString("MyStr");
        const Variant var(theString);
        ASSERT_TRUE(var.isValid());
        ASSERT_EQ(Variant::STRING, var.type());

        Variant var2;
        var2 = var;
        ASSERT_TRUE(var2.isValid());
        ASSERT_EQ(Variant::STRING, var2.type());
        ASSERT_TRUE(var2.getString() == theString);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, ComparisonOperatorAtomicVariants)
{
    const Variant v;
    const Variant vi(-1234);
    const Variant vui(5678u);
    const Variant vd(1.234);
    const Variant vf(2.234f);
    const Variant vb(true);
    const Variant vv3d(Vec3d(1, 2, 3));
    const Variant vc3f(Color3f(0.1f, 0.2f, 0.3f));
    const Variant vs("myString");

    ASSERT_TRUE(v == Variant());
    ASSERT_FALSE(v == Variant(1));

    ASSERT_TRUE(vi == Variant(-1234));
    ASSERT_FALSE(vi == Variant(-12345));
    
    ASSERT_TRUE(vui == Variant(5678u));
    ASSERT_FALSE(vui == Variant(56789u));
    ASSERT_FALSE(vui == Variant(5678));

    ASSERT_TRUE(vd == Variant(1.234));
    ASSERT_FALSE(vd == Variant(1.2345));

    ASSERT_TRUE(vf == Variant(2.234f));
    ASSERT_FALSE(vf == Variant(2.2345f));

    ASSERT_TRUE(vb == Variant(true));
    ASSERT_FALSE(vb == Variant(false));

    ASSERT_TRUE(vv3d == Variant(Vec3d(1, 2, 3)));
    ASSERT_FALSE(vv3d == Variant(Vec3d(1, 2, 4)));

    ASSERT_TRUE(vc3f == Variant(Color3f(0.1f, 0.2f, 0.3f)));
    ASSERT_FALSE(vc3f == Variant(Color3f(0.1f, 0.2f, 0.4f)));

    ASSERT_TRUE(vs == Variant("myString"));
    ASSERT_FALSE(vs == Variant("notMyString"));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, ComparisonOperatorArrayVariant)
{
    std::vector<Variant> arr1;
    arr1.push_back(Variant(123));
    arr1.push_back(Variant("myString"));

    std::vector<Variant> arr2;
    arr2.push_back(Variant(123));
    arr2.push_back(Variant("myString"));

    Variant v1(arr1);
    Variant v2(arr2);

    ASSERT_TRUE(arr1 == arr2);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, TypeInt)
{
    const int val = -123;
    Variant var(val);
    ASSERT_EQ(Variant::INT, var.type());
    ASSERT_TRUE(var.isValid());

    int v = var.getInt();
    EXPECT_EQ(val, v);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, TypeUInt)
{
    const cvf::uint val = 123;
    Variant var(val);
    ASSERT_EQ(Variant::UINT, var.type());
    ASSERT_TRUE(var.isValid());

    cvf::uint v = var.getUInt();
    EXPECT_EQ(val, v);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, TypeDouble)
{
    const double val = 123.1234567;
    Variant var(val);
    ASSERT_EQ(Variant::DOUBLE, var.type());
    ASSERT_TRUE(var.isValid());

    double v = var.getDouble();
    EXPECT_EQ(val, v);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, TypeFloat)
{
    const float val = 123.1234567f;
    Variant var(val);
    ASSERT_EQ(Variant::FLOAT, var.type());
    ASSERT_TRUE(var.isValid());

    float v = var.getFloat();
    EXPECT_EQ(val, v);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, TypeBool)
{
    {
        const bool val = true;
        Variant var(val);
        ASSERT_EQ(Variant::BOOL, var.type());
        ASSERT_TRUE(var.isValid());

        bool v = var.getBool();
        EXPECT_EQ(val, v);
    }

    {
        const bool val = false;
        Variant var(val);
        ASSERT_EQ(Variant::BOOL, var.type());
        ASSERT_TRUE(var.isValid());

        bool v = var.getBool();
        EXPECT_EQ(val, v);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, TypeVec3d)
{
    const Vec3d val(1.0, -2000.1234, 3000.6789);
    Variant var(val);
    ASSERT_EQ(Variant::VEC3D, var.type());
    ASSERT_TRUE(var.isValid());

    Vec3d v = var.getVec3d();
    ASSERT_EQ(val.x(), v.x());
    ASSERT_EQ(val.y(), v.y());
    ASSERT_EQ(val.z(), v.z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, TypeColor3f)
{
    const Color3f val(0.2f, 0.2f, 0.3f);
    Variant var(val);
    ASSERT_EQ(Variant::COLOR3F, var.type());
    ASSERT_TRUE(var.isValid());

    Color3f v = var.getColor3f();
    ASSERT_EQ(val.r(), v.r());
    ASSERT_EQ(val.g(), v.g());
    ASSERT_EQ(val.b(), v.b());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, TypeString)
{
    {
        const String val("ABC");
        Variant var(val);
        ASSERT_EQ(Variant::STRING, var.type());
        ASSERT_TRUE(var.isValid());

        String v = var.getString();
        ASSERT_EQ(3, v.size());
        ASSERT_EQ(L'A', v[0]);
        ASSERT_EQ(L'B', v[1]);
        ASSERT_EQ(L'C', v[2]);
    }
    {
        const String val("");
        Variant var(val);
        ASSERT_EQ(Variant::STRING, var.type());
        ASSERT_TRUE(var.isValid());

        String v = var.getString();
        ASSERT_EQ(0, v.size());
        ASSERT_TRUE(v == "");
    }
    {
        // Wil use the const char* constructor
        Variant var("abc");
        ASSERT_EQ(Variant::STRING, var.type());
        ASSERT_TRUE(var.isValid());

        String v = var.getString();
        ASSERT_TRUE(v == "abc");
    }
    {
        const String val(static_cast<wchar_t*>(NULL));
        Variant var(val);
        ASSERT_EQ(Variant::STRING, var.type());
        ASSERT_TRUE(var.isValid());

        String v = var.getString();
        ASSERT_EQ(0, v.size());
        ASSERT_TRUE(v == "");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, ArrayPopulatedWithVariants)
{
    std::vector<Variant> arr;
    arr.push_back(Variant(1));
    arr.push_back(Variant(1.2));
    arr.push_back(Variant(Vec3d(1,2,3)));
    ASSERT_EQ(3, arr.size());

    {
        Variant var = arr[0];
        ASSERT_EQ(Variant::INT, var.type());
        ASSERT_EQ(1, var.getInt());
    }

    {
        Variant var = arr[1];
        ASSERT_EQ(Variant::DOUBLE, var.type());
        ASSERT_EQ(1.2, var.getDouble());
    }

    {
        Variant var = arr[2];
        ASSERT_EQ(Variant::VEC3D, var.type());
        ASSERT_TRUE(var.getVec3d() == Vec3d(1,2,3));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, VariantContainingAnArray)
{
    std::vector<Variant> orgArr;
    orgArr.push_back(Variant(1));
    orgArr.push_back(Variant(1.2));
    orgArr.push_back(Variant(Vec3d(1,2,3)));
    ASSERT_EQ(3, orgArr.size());

    Variant varContainingArray(orgArr);
    ASSERT_EQ(Variant::ARRAY, varContainingArray.type());

    std::vector<Variant> arr = varContainingArray.getArray();
    ASSERT_EQ(3, arr.size());
    EXPECT_EQ(1, arr[0].getInt());
    EXPECT_EQ(1.2, arr[1].getDouble());
    EXPECT_TRUE(arr[2].getVec3d() == Vec3d(1,2,3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, VariantContainingEmptyArray)
{
    std::vector<Variant> orgArr;

    Variant varContainingArray(orgArr);
    ASSERT_EQ(Variant::ARRAY, varContainingArray.type());

    std::vector<Variant> arr = varContainingArray.getArray();
    ASSERT_EQ(0, arr.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(VariantTest, NestedVariantArrays)
{
    std::vector<Variant> orgArr;

    {
        std::vector<Variant> orgSubArr;
        orgSubArr.push_back(Variant(1));
        orgSubArr.push_back(Variant(1.9));
        ASSERT_EQ(2, orgSubArr.size());

        Variant varWithArray(orgSubArr);
        orgArr.push_back(varWithArray);
    }

    orgArr.push_back(99);

    {
        std::vector<Variant> orgSubArr;
        orgSubArr.push_back(Variant(2));
        orgSubArr.push_back(Variant(2.9));
        ASSERT_EQ(2, orgSubArr.size());

        orgArr.push_back(orgSubArr);
    }

    orgArr.push_back(99.99);

    Variant variantContainingArray(orgArr);
    ASSERT_EQ(Variant::ARRAY, variantContainingArray.type());

    std::vector<Variant> arrayOfVariants = variantContainingArray.getArray();
    ASSERT_EQ(4, arrayOfVariants.size());
    ASSERT_EQ(cvf::Variant::ARRAY,  arrayOfVariants[0].type());
    ASSERT_EQ(cvf::Variant::INT,    arrayOfVariants[1].type());
    ASSERT_EQ(cvf::Variant::ARRAY,  arrayOfVariants[2].type());
    ASSERT_EQ(cvf::Variant::DOUBLE, arrayOfVariants[3].type());

    {
        Variant var = arrayOfVariants[0];
        std::vector<Variant> arr = var.getArray();
        ASSERT_EQ(2, arr.size());
        EXPECT_EQ(1, arr[0].getInt());
        EXPECT_EQ(1.9, arr[1].getDouble());
    }

    {
        Variant var = arrayOfVariants[1];
        EXPECT_EQ(99, var.getInt());
    }

    {
        Variant var = arrayOfVariants[2];
        std::vector<Variant> arr = var.getArray();
        ASSERT_EQ(2, arr.size());
        EXPECT_EQ(2, arr[0].getInt());
        EXPECT_EQ(2.9, arr[1].getDouble());
    }

    {
        Variant var = arrayOfVariants[3];
        EXPECT_EQ(99.99, var.getDouble());
    }
}



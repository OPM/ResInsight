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
#include "cvfUniform.h"

#include "gtest/gtest.h"

using namespace cvf;





//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformIntTest, BasicConstruction)
{
    UniformInt u("dummy");

    EXPECT_STREQ("dummy", u.name());
    EXPECT_EQ(Uniform::UNDEFINED, u.type());
    EXPECT_EQ(0, u.valueCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformIntTest, ConstructorsWithInitialization)
{
    {
        UniformInt u("singleValue", 10);
        EXPECT_EQ(Uniform::INT, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(10, u.intPtr()[0]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformIntDeathTest, ConstructionWithEmptyName)
{
    EXPECT_DEATH(new UniformInt(""), "Assertion");
    EXPECT_DEATH(new UniformInt(NULL), "Assertion");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformIntTest, SetSingleValue)
{
    {
        UniformInt u("singleValue");
        u.set(11);
        EXPECT_EQ(Uniform::INT, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(11, u.intPtr()[0]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformIntTest, SetArrays)
{
    {
        IntArray a(3);
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;

        UniformInt u("array");
        u.setArray(a);
        EXPECT_EQ(Uniform::INT, u.type());
        EXPECT_EQ(3, u.valueCount());
        EXPECT_EQ(1, u.intPtr()[0]);
        EXPECT_EQ(2, u.intPtr()[1]);
        EXPECT_EQ(3, u.intPtr()[2]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformFloatTest, BasicConstruction)
{
    UniformFloat u("dummy");

    EXPECT_STREQ("dummy", u.name());
    EXPECT_EQ(Uniform::UNDEFINED, u.type());
    EXPECT_EQ(0, u.valueCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformFloatTest, ConstructorsWithInitialization)
{
    {
        UniformFloat u("singleValue", 1.0f);
        EXPECT_EQ(Uniform::FLOAT, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
    }

    {
        UniformFloat u("singleValue", Vec3f(1.0f, 2.0f, 3.0f));
        EXPECT_EQ(Uniform::FLOAT_VEC3, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
        EXPECT_EQ(2.0f, u.floatPtr()[1]);
        EXPECT_EQ(3.0f, u.floatPtr()[2]);
    }

    {
        UniformFloat u("singleValue", Color3f(1.0f, 2.0f, 3.0f));
        EXPECT_EQ(Uniform::FLOAT_VEC3, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
        EXPECT_EQ(2.0f, u.floatPtr()[1]);
        EXPECT_EQ(3.0f, u.floatPtr()[2]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformFloatDeathTest, ConstructionWithEmptyName)
{
    EXPECT_DEATH(new UniformFloat(""), "Assertion");
    EXPECT_DEATH(new UniformFloat(NULL), "Assertion");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformFloatTest, SetSingleValue)
{
    {
        UniformFloat u("singleValue");
        u.set(1.0f);
        EXPECT_EQ(Uniform::FLOAT, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
    }

    {
        UniformFloat u("singleValue");
        u.set(Vec2f(1.0f, 2.0f));
        EXPECT_EQ(Uniform::FLOAT_VEC2, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
        EXPECT_EQ(2.0f, u.floatPtr()[1]);
    }

    {
        UniformFloat u("singleValue");
        u.set(Vec3f(1.0f, 2.0f, 3.0f));
        EXPECT_EQ(Uniform::FLOAT_VEC3, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
        EXPECT_EQ(2.0f, u.floatPtr()[1]);
        EXPECT_EQ(3.0f, u.floatPtr()[2]);
    }

    {
        UniformFloat u("singleValue");
        u.set(Color3f(1.0f, 2.0f, 3.0f));
        EXPECT_EQ(Uniform::FLOAT_VEC3, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
        EXPECT_EQ(2.0f, u.floatPtr()[1]);
        EXPECT_EQ(3.0f, u.floatPtr()[2]);
    }

    {
        UniformFloat u("singleValue");
        u.set(Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
        EXPECT_EQ(Uniform::FLOAT_VEC4, u.type());
        EXPECT_EQ(1, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
        EXPECT_EQ(2.0f, u.floatPtr()[1]);
        EXPECT_EQ(3.0f, u.floatPtr()[2]);
        EXPECT_EQ(4.0f, u.floatPtr()[3]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformFloatTest, SetArrays)
{
    {
        FloatArray a(3);
        a[0] = 1.0f;
        a[1] = 2.0f;
        a[2] = 3.0f;

        UniformFloat u("array");
        u.setArray(a);
        EXPECT_EQ(Uniform::FLOAT, u.type());
        EXPECT_EQ(3, u.valueCount());
        EXPECT_EQ(1.0f, u.floatPtr()[0]);
        EXPECT_EQ(2.0f, u.floatPtr()[1]);
        EXPECT_EQ(3.0f, u.floatPtr()[2]);
    }

    {
        Vec3fArray a(3);
        a[0] = Vec3f(1.1f, 1.2f, 1.3f);
        a[1] = Vec3f(2.1f, 2.2f, 2.3f);
        a[2] = Vec3f(3.1f, 3.2f, 3.3f);

        UniformFloat u("array");
        u.setArray(a);
        EXPECT_EQ(Uniform::FLOAT_VEC3, u.type());
        EXPECT_EQ(3, u.valueCount());
        EXPECT_EQ(1.1f, u.floatPtr()[0]);
        EXPECT_EQ(1.2f, u.floatPtr()[1]);
        EXPECT_EQ(1.3f, u.floatPtr()[2]);
        EXPECT_EQ(2.1f, u.floatPtr()[3]);
        EXPECT_EQ(2.2f, u.floatPtr()[4]);
        EXPECT_EQ(2.3f, u.floatPtr()[5]);
        EXPECT_EQ(3.1f, u.floatPtr()[6]);
        EXPECT_EQ(3.2f, u.floatPtr()[7]);
        EXPECT_EQ(3.3f, u.floatPtr()[8]);
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformMatrixfTest, BasicConstruction)
{
    UniformMatrixf u("dummy");

    EXPECT_STREQ("dummy", u.name());
    EXPECT_EQ(Uniform::UNDEFINED, u.type());
    EXPECT_EQ(0, u.valueCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformMatrixfTest, ConstructorsWithInitialization)
{
    Mat4f m(11.0f, 12.0f, 13.0f, 14.0f, 
            21.0f, 22.0f, 23.0f, 24.0f, 
            31.0f, 32.0f, 33.0f, 34.0f, 
            41.0f, 42.0f, 43.0f, 44.0f);

    UniformMatrixf u("singleValue", m);
    EXPECT_EQ(Uniform::FLOAT_MAT4, u.type());
    EXPECT_EQ(1, u.valueCount());

    const float* pf = u.floatPtr();
    EXPECT_FLOAT_EQ(11.0f, pf[0]);
    EXPECT_FLOAT_EQ(21.0f, pf[1]);
    EXPECT_FLOAT_EQ(31.0f, pf[2]);
    EXPECT_FLOAT_EQ(41.0f, pf[3]);

    EXPECT_FLOAT_EQ(12.0f, pf[4]);
    EXPECT_FLOAT_EQ(22.0f, pf[5]);
    EXPECT_FLOAT_EQ(32.0f, pf[6]);
    EXPECT_FLOAT_EQ(42.0f, pf[7]);

    EXPECT_FLOAT_EQ(13.0f, pf[8]);
    EXPECT_FLOAT_EQ(23.0f, pf[9]);
    EXPECT_FLOAT_EQ(33.0f, pf[10]);
    EXPECT_FLOAT_EQ(43.0f, pf[11]);

    EXPECT_FLOAT_EQ(14.0f, pf[12]);
    EXPECT_FLOAT_EQ(24.0f, pf[13]);
    EXPECT_FLOAT_EQ(34.0f, pf[14]);
    EXPECT_FLOAT_EQ(44.0f, pf[15]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformMatrixfTest, SetSingleValue)
{
    Mat4f m(11.0f, 12.0f, 13.0f, 14.0f, 
            21.0f, 22.0f, 23.0f, 24.0f, 
            31.0f, 32.0f, 33.0f, 34.0f, 
            41.0f, 42.0f, 43.0f, 44.0f);

    UniformMatrixf u("singleValue");
    u.set(m);
    EXPECT_EQ(Uniform::FLOAT_MAT4, u.type());
    EXPECT_EQ(1, u.valueCount());
    
    const float* pf = u.floatPtr();
    EXPECT_FLOAT_EQ(11.0f, pf[0]);
    EXPECT_FLOAT_EQ(21.0f, pf[1]);
    EXPECT_FLOAT_EQ(31.0f, pf[2]);
    EXPECT_FLOAT_EQ(41.0f, pf[3]);

    EXPECT_FLOAT_EQ(12.0f, pf[4]);
    EXPECT_FLOAT_EQ(22.0f, pf[5]);
    EXPECT_FLOAT_EQ(32.0f, pf[6]);
    EXPECT_FLOAT_EQ(42.0f, pf[7]);

    EXPECT_FLOAT_EQ(13.0f, pf[8]);
    EXPECT_FLOAT_EQ(23.0f, pf[9]);
    EXPECT_FLOAT_EQ(33.0f, pf[10]);
    EXPECT_FLOAT_EQ(43.0f, pf[11]);

    EXPECT_FLOAT_EQ(14.0f, pf[12]);
    EXPECT_FLOAT_EQ(24.0f, pf[13]);
    EXPECT_FLOAT_EQ(34.0f, pf[14]);
    EXPECT_FLOAT_EQ(44.0f, pf[15]);
}




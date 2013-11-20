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
#include "cvfEdgeKey.h"
#include "cvfMath.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EdgeKeyTest, Constructor)
{
    {
        EdgeKey ek(1, 2);
        EXPECT_EQ(1, ek.index1());
        EXPECT_EQ(2, ek.index2());
    }

    {
        EdgeKey ek(2, 1);
        EXPECT_EQ(1, ek.index1());
        EXPECT_EQ(2, ek.index2());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EdgeKeyTest, EqualOperator)
{
    const EdgeKey ek01(0u, 1u);
    const EdgeKey ek10(1u, 0u);
    const EdgeKey ek12(1u, 2u);

    EXPECT_TRUE(ek01 == ek01);
    EXPECT_TRUE(ek01 == ek10);
    EXPECT_FALSE(ek01 == ek12);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EdgeKeyTest, LessThanOperator)
{
    const EdgeKey ek01(0u, 1u);
    const EdgeKey ek10(1u, 0u);
    const EdgeKey ek02(0u, 2u);
    const EdgeKey ek12(1u, 2u);

    EXPECT_FALSE(ek01 < ek01);
    EXPECT_FALSE(ek01 < ek10);
    EXPECT_FALSE(ek01.toKeyVal() < ek01.toKeyVal());
    EXPECT_FALSE(ek01.toKeyVal() < ek10.toKeyVal());


    EXPECT_TRUE(ek01 < ek02);
    EXPECT_TRUE(ek10 < ek02);
    EXPECT_TRUE(ek01 < ek12);
    EXPECT_TRUE(ek02 < ek12);
    EXPECT_TRUE(ek01.toKeyVal() < ek02.toKeyVal());
    EXPECT_TRUE(ek10.toKeyVal() < ek02.toKeyVal());
    EXPECT_TRUE(ek01.toKeyVal() < ek12.toKeyVal());
    EXPECT_TRUE(ek02.toKeyVal() < ek12.toKeyVal());


    EXPECT_FALSE(ek02 < ek01);
    EXPECT_FALSE(ek02 < ek10);
    EXPECT_FALSE(ek12 < ek01);
    EXPECT_FALSE(ek12 < ek02);
    EXPECT_FALSE(ek02.toKeyVal() < ek01.toKeyVal());
    EXPECT_FALSE(ek02.toKeyVal() < ek10.toKeyVal());
    EXPECT_FALSE(ek12.toKeyVal() < ek01.toKeyVal());
    EXPECT_FALSE(ek12.toKeyVal() < ek02.toKeyVal());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(EdgeKeyTest, ToFromKeyVal)
{
    {
        EdgeKey ek(987654321, 123456789);
        ASSERT_EQ(123456789, ek.index1());
        ASSERT_EQ(987654321, ek.index2());

        int64 val = ek.toKeyVal();

        EdgeKey ek2 = EdgeKey::fromkeyVal(val);
        ASSERT_EQ(123456789, ek2.index1());
        ASSERT_EQ(987654321, ek2.index2());
    }

    {
        const cvf::uint idx2 = UNDEFINED_UINT;
        const cvf::uint idx1 = UNDEFINED_UINT - 1;
        EdgeKey ek(idx2, idx1);
        ASSERT_EQ(idx1, ek.index1());
        ASSERT_EQ(idx2, ek.index2());

        int64 val = ek.toKeyVal();

        EdgeKey ek2 = EdgeKey::fromkeyVal(val);
        ASSERT_EQ(idx1, ek2.index1());
        ASSERT_EQ(idx2, ek2.index2());
    }

}



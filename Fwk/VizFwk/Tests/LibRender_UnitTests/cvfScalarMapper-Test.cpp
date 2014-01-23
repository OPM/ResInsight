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
#include "cvfScalarMapper.h"

#include "gtest/gtest.h"

using namespace cvf;


// Helper class used to test protected functions
class MyScalarMapper : ScalarMapper
{
public:
    static ref<Color3ubArray> colorTableArrayTest(ColorTable colorTable)
    {
        return colorTableArray(colorTable);
    }

    static ref<Color3ubArray> normalColorTableArrayTest(cvf::uint colorCount)
    {
        return normalColorTableArray(colorCount);
    }

    static ref<Color3ubArray> interpolateColorArrayTest(const Color3ubArray& colorArray, cvf::uint targetColorCount)
    {
        return interpolateColorArray(colorArray, targetColorCount);
    }
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperTest, ColorTable)
{
    ref<Color3ubArray> colors = NULL;

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::BLACK_WHITE);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::BLUE_RED);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::BLUE_GREEN);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::YELLOW_RED);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::GREEN_YELLOW_RED);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::RED_YELLOW);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::THERMAL_1);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::THERMAL_2);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::THERMAL_3);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);

    colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::METAL_CASTING);
    ASSERT_TRUE(colors.notNull());
    EXPECT_TRUE(colors->size() >= 2);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperTest, NormalColorTable)
{
    ref<Color3ubArray> colors = NULL;

    cvf::uint i;
    for (i = 1; i <= 13; i++)
    {
        colors = MyScalarMapper::normalColorTableArrayTest(i);
        ASSERT_TRUE(colors.notNull());
        EXPECT_TRUE(colors->size() == i);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperTest, Interpolation)
{
    {
        ref<Color3ubArray> colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::BLACK_WHITE);
        ref<Color3ubArray> interpolatedColors = MyScalarMapper::interpolateColorArrayTest(*colors, 3);
        EXPECT_TRUE(interpolatedColors->size() == 3);

        {
            Color3ub col = interpolatedColors->get(0);
            EXPECT_EQ(0, col.r());
            EXPECT_EQ(0, col.g());
            EXPECT_EQ(0, col.b());
        }

        {
            Color3ub col = interpolatedColors->get(1);
            EXPECT_EQ(128, col.r());
            EXPECT_EQ(128, col.g());
            EXPECT_EQ(128, col.b());
        }

        {
            Color3ub col = interpolatedColors->get(2);
            EXPECT_EQ(255, col.r());
            EXPECT_EQ(255, col.g());
            EXPECT_EQ(255, col.b());
        }
    }

    {
        ref<Color3ubArray> colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::BLUE_RED);
        ref<Color3ubArray> interpolatedColors = MyScalarMapper::interpolateColorArrayTest(*colors, 5);
        EXPECT_TRUE(interpolatedColors->size() == 5);

        {
            Color3ub col = interpolatedColors->get(0);
            EXPECT_EQ(  0, col.r());
            EXPECT_EQ(  0, col.g());
            EXPECT_EQ(255, col.b());
        }

        {
            Color3ub col = interpolatedColors->get(1);
            EXPECT_EQ( 64, col.r());
            EXPECT_EQ(  0, col.g());
            EXPECT_EQ(191, col.b());
        }

        {
            Color3ub col = interpolatedColors->get(2);
            EXPECT_EQ(128, col.r());
            EXPECT_EQ(  0, col.g());
            EXPECT_EQ(128, col.b());
        }

        {
            Color3ub col = interpolatedColors->get(3);
            EXPECT_EQ(191, col.r());
            EXPECT_EQ(  0, col.g());
            EXPECT_EQ( 64, col.b());
        }

        {
            Color3ub col = interpolatedColors->get(4);
            EXPECT_EQ(255, col.r());
            EXPECT_EQ(  0, col.g());
            EXPECT_EQ(  0, col.b());
        }
    }

    {
        ref<Color3ubArray> colors = MyScalarMapper::colorTableArrayTest(ScalarMapper::GREEN_YELLOW_RED);
        ref<Color3ubArray> interpolatedColors = MyScalarMapper::interpolateColorArrayTest(*colors, 5);
        EXPECT_TRUE(interpolatedColors->size() == 5);

        {
            Color3ub col = interpolatedColors->get(0);
            EXPECT_EQ(  0, col.r());
            EXPECT_EQ(255, col.g());
            EXPECT_EQ(  0, col.b());
        }
        {
            Color3ub col = interpolatedColors->get(1);
            EXPECT_EQ(128, col.r());
            EXPECT_EQ(255, col.g());
            EXPECT_EQ(  0, col.b());
        }
        {
            Color3ub col = interpolatedColors->get(2);
            EXPECT_EQ(255, col.r());
            EXPECT_EQ(255, col.g());
            EXPECT_EQ(  0, col.b());
        }
        {
            Color3ub col = interpolatedColors->get(3);
            EXPECT_EQ(255, col.r());
            EXPECT_EQ(128, col.g());
            EXPECT_EQ(  0, col.b());
        }
        {
            Color3ub col = interpolatedColors->get(4);
            EXPECT_EQ(255, col.r());
            EXPECT_EQ(  0, col.g());
            EXPECT_EQ(  0, col.b());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ScalarMapperDeathTest, InvalidParameters)
{
    {
        Color3ubArray colors(1);
        EXPECT_DEATH(MyScalarMapper::interpolateColorArrayTest(colors, 2), "Assertion");
    }

    {
        Color3ubArray colors(2);
        EXPECT_DEATH(MyScalarMapper::interpolateColorArrayTest(colors, 1), "Assertion");
    }

    EXPECT_DEATH(MyScalarMapper::normalColorTableArrayTest(14), "Assertion");
}
#endif


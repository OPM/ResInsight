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
#include "cvfScalarMapperUniformLevels.h"
#include "cvfTextureImage.h"

#include "gtest/gtest.h"

#include <cmath>

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, Constructor)
{
    ScalarMapperUniformLevels m;
    ASSERT_EQ(64, m.textureSize());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToColorWithOneLevel)
{
    Color3ubArray colors(1);
    colors[0] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setRange(-1, 1);

    EXPECT_TRUE(m.mapToColor(-2) == Color3::RED);
    EXPECT_TRUE(m.mapToColor(-1) == Color3::RED);
    EXPECT_TRUE(m.mapToColor( 0) == Color3::RED);
    EXPECT_TRUE(m.mapToColor( 1) == Color3::RED);
    EXPECT_TRUE(m.mapToColor( 2) == Color3::RED);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToColor)
{
    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setRange(0, 3);

    EXPECT_TRUE(m.mapToColor(-1)    == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor(0)     == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor(0.99)  == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor(1.01)  == Color3::GREEN);
    EXPECT_TRUE(m.mapToColor(1.99)  == Color3::GREEN);
    EXPECT_TRUE(m.mapToColor(2.01)  == Color3::RED);
    EXPECT_TRUE(m.mapToColor(2.99)  == Color3::RED);
    EXPECT_TRUE(m.mapToColor(3)     == Color3::RED);
    EXPECT_TRUE(m.mapToColor(4)     == Color3::RED);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToColorZeroRange)
{
    const double dNaN = sqrt(-1.0);
    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setRange(1, 1);

    EXPECT_TRUE(m.mapToColor(1.0)  == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor(0.0)  == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor(2.0)  == Color3::RED);
    EXPECT_TRUE(m.mapToColor(dNaN) == Color3::BLACK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToColorTinyRange)
{
    const double dNaN = sqrt(-1.0);
    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setRange(1.0e-200, 2.0e-200);

    EXPECT_TRUE(m.mapToColor(1.0e-200)  == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor(1.5e-200)  == Color3::GREEN);
    EXPECT_TRUE(m.mapToColor(2.0e-200)  == Color3::RED);

    EXPECT_TRUE(m.mapToColor(0.0)  == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor(1.0)  == Color3::RED);
    EXPECT_TRUE(m.mapToColor(dNaN) == Color3::BLACK);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToColorInvalidRange)
{
    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setRange(1, 0);

    EXPECT_TRUE(m.mapToColor(-1.0) == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor( 1.0) == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor( 0.0) == Color3::BLUE);
    EXPECT_TRUE(m.mapToColor( 2.0) == Color3::BLUE);

    const double dNaN = sqrt(-1.0);
    EXPECT_TRUE(m.mapToColor(dNaN) == Color3::BLUE);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToTextureCoord)
{
    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setRange(0, 3);

    m.setTextureSize(15);

    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord( -1.00 ).x(),  0.0f,    0.0f));
    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord(  0.00 ).x(),  0.0f,    0.0f));
    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord(  0.99 ).x(),  0.32f,   0.3333f));
    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord(  1.01 ).x(),  0.3333f, 0.34f));
    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord(  1.99 ).x(),  0.65f,   0.6666f));
    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord(  2.01 ).x(),  0.6666f, 0.67f));
    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord(  2.99 ).x(),  0.99f,   1.0f));
    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord(  3.00 ).x(),  1.0f,    1.0f));
    EXPECT_TRUE(Math::valueInRange( m.mapToTextureCoord(  4.00 ).x(),  1.0f,    1.0f));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToTextureCoordZeroRange)
{
    const double dNaN = sqrt(-1.0);

    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setTextureSize(15);
    m.setRange(1, 1);

    EXPECT_FLOAT_EQ(m.mapToTextureCoord(  1.00 ).x(),  0.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord(  0.00 ).x(),  0.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord(  2.00 ).x(),  1.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord(  dNaN ).x(),  0.0f);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToTextureCoordTinyRange)
{
    const double dNaN = sqrt(-1.0);
    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setTextureSize(15);
    m.setRange(1.0e-200, 2.0e-200);

    EXPECT_FLOAT_EQ(m.mapToTextureCoord( 1.0e-200 ).x(),  0.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord( 1.5e-200 ).x(),  0.5f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord( 2.0e-200 ).x(),  1.0f);

    EXPECT_FLOAT_EQ(m.mapToTextureCoord( 0.0  ).x(),  0.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord( 1.0  ).x(),  1.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord( dNaN ).x(),  0.0f);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, MapToTextureCoordInvalidRange)
{
    const double dNaN = sqrt(-1.0);
    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setTextureSize(15);
    m.setRange(1, 0);

    EXPECT_FLOAT_EQ(m.mapToTextureCoord( -1.00 ).x(),  0.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord(  1.00 ).x(),  0.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord(  0.00 ).x(),  0.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord(  2.00 ).x(),  0.0f);
    EXPECT_FLOAT_EQ(m.mapToTextureCoord(  dNaN ).x(),  0.0f);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(ScalarMapperUniformLevelsDeathTest, MapWithUninitializedObject)
{
    ScalarMapperUniformLevels m;

    EXPECT_DEATH(m.mapToColor(1.2), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, ImageDimensions)
{
    const Color3ubArray dummyColorArr(1);
    ScalarMapperUniformLevels m;
    m.setColors(dummyColorArr);
    
    TextureImage img;

    m.setTextureSize(16);
    EXPECT_TRUE(m.updateTexture(&img));
    EXPECT_EQ(16, img.width());
    EXPECT_EQ(1, img.height());

    m.setTextureSize(2);
    EXPECT_TRUE(m.updateTexture(&img));
    EXPECT_EQ(2, img.width());
    EXPECT_EQ(1, img.height());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperUniformLevelsTest, ImageWithThreeColors)
{
    Color3ubArray colors(3);
    colors[0] = Color3::BLUE;
    colors[1] = Color3::GREEN;
    colors[2] = Color3::RED;

    ScalarMapperUniformLevels m;
    m.setColors(colors);
    m.setTextureSize(8);
    m.setRange(0, 3);

    TextureImage img;
    EXPECT_TRUE(m.updateTexture(&img));
    EXPECT_EQ(8, img.width());
    EXPECT_EQ(1, img.height());

    EXPECT_TRUE(Color4ub(Color3::BLUE)  == img.pixel(0, 0));
    EXPECT_TRUE(Color4ub(Color3::BLUE)  == img.pixel(1, 0));
    EXPECT_TRUE(Color4ub(Color3::GREEN) == img.pixel(2, 0));
    EXPECT_TRUE(Color4ub(Color3::GREEN) == img.pixel(3, 0));
    EXPECT_TRUE(Color4ub(Color3::RED)   == img.pixel(4, 0));
    EXPECT_TRUE(Color4ub(Color3::RED)   == img.pixel(5, 0));

    // Repeat of top color
    EXPECT_TRUE(Color4ub(Color3::RED)   == img.pixel(6, 0));

    // Color for 'unused' area of texture
    EXPECT_TRUE(Color4ub(Color3::WHITE) == img.pixel(7, 0));
}



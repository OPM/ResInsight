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
#include "cvfTextureImage.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, Defaults)
{
    TextureImage t;

    EXPECT_EQ(0, t.width());
    EXPECT_EQ(0, t.height());
    EXPECT_EQ(NULL, t.ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, CopyConstructorAndAssignment)
{
    TextureImage t;
    ubyte rgba[] = {1,2,3,255,  5,6,1,2,  3,4,5,6,  1,2,3,4, 5,6,1,2, 3,4,5,6};
    t.setData(rgba, 2, 3);

    TextureImage t2(t);
    ASSERT_EQ(2, t2.width());
    ASSERT_EQ(3, t2.height());

    const ubyte* tData = t.ptr();
    const ubyte* t2Data = t2.ptr();

    EXPECT_FALSE(tData == t2Data);
    EXPECT_EQ(tData[0], t2Data[0]);
    EXPECT_EQ(tData[1], t2Data[1]);
    EXPECT_EQ(tData[2], t2Data[2]);
    EXPECT_EQ(tData[3], t2Data[3]);
    EXPECT_EQ(tData[4], t2Data[4]);
    EXPECT_EQ(tData[5], t2Data[5]);

    TextureImage t3;

    t3 = t;
    const ubyte* t3Data = t3.ptr();
    ASSERT_EQ(2, t3.width());
    ASSERT_EQ(3, t3.height());

    EXPECT_FALSE(tData == t3Data);
    EXPECT_EQ(tData[0], t3Data[0]);
    EXPECT_EQ(tData[1], t3Data[1]);
    EXPECT_EQ(tData[2], t3Data[2]);
    EXPECT_EQ(tData[3], t3Data[3]);
    EXPECT_EQ(tData[4], t3Data[4]);
    EXPECT_EQ(tData[5], t3Data[5]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, Allocate)
{
    TextureImage img;
    img.allocate(3, 2);
    EXPECT_EQ(3, img.width());
    EXPECT_EQ(2, img.height());

    ref<UByteArray> byteArr = img.toRgb();
    EXPECT_EQ(3*6, byteArr->size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, Clear)
{
    TextureImage img;
    img.allocate(3, 2);
    ASSERT_EQ(3, img.width());
    ASSERT_EQ(2, img.height());

    img.clear();
    EXPECT_EQ(0, img.width());
    EXPECT_EQ(0, img.height());
    EXPECT_EQ(NULL, img.ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, Swap)
{
    TextureImage t;
    ubyte rgba[] = {1,2,3,255,  5,6,1,2,  3,4,5,6,  1,2,3,4, 5,6,1,2, 3,4,5,6};
    t.setData(rgba, 2, 3);

    TextureImage t2;
    ubyte t2d[] = {5,6,1,2,  3,4,5,6,  1,2,3,4, 5,6,1,2};
    t2.setData(t2d, 1, 4);

    EXPECT_EQ(2, t.width());
    EXPECT_EQ(3, t.height());
    EXPECT_EQ(1, t2.width());
    EXPECT_EQ(4, t2.height());

    t.swap(t2);

    EXPECT_EQ(1, t.width());
    EXPECT_EQ(4, t.height());
    EXPECT_EQ(2, t2.width());
    EXPECT_EQ(3, t2.height());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, SetAndGetData)
{
    TextureImage t;
    ubyte rgba[] = {1,2,3,4,  5,6,1,2,  3,4,5,6,  1,2,3,4, 5,6,1,2, 3,4,5,6};
    t.setData(rgba, 2, 3);

    EXPECT_EQ(2, t.width());
    EXPECT_EQ(3, t.height());

    const ubyte* data = t.ptr();

    EXPECT_EQ(1, data[0]);
    EXPECT_EQ(2, data[1]);
    EXPECT_EQ(3, data[2]);
    EXPECT_EQ(4, data[3]);
    EXPECT_EQ(5, data[4]);
    EXPECT_EQ(6, data[5]);

    EXPECT_EQ(5, data[22]);
    EXPECT_EQ(6, data[23]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, SetFromRgb)
{
    TextureImage t;
    ubyte test[] = {1,2,3, 4,5,6,  1,2,3, 4,5,6,  1,2,3, 4,5,6};
    ref<UByteArray> rgb = new UByteArray(test, sizeof(test));

    t.setFromRgb(*rgb, 2, 3);

    EXPECT_EQ(2, t.width());
    EXPECT_EQ(3, t.height());

    const ubyte* data = t.ptr();

    EXPECT_EQ(1,   data[0]);
    EXPECT_EQ(2,   data[1]);
    EXPECT_EQ(3,   data[2]);
    EXPECT_EQ(255, data[3]);
    EXPECT_EQ(4,   data[4]);
    EXPECT_EQ(5,   data[5]);
    EXPECT_EQ(6,   data[6]);
    EXPECT_EQ(255, data[7]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, ToRgb)
{
    TextureImage t;
    ubyte rgba[] = {1,2,3,4,  5,6,1,2,  3,4,5,6,  4,2,1,4, 5,2,1,2, 3,4,5,6};

    t.setData(rgba, 2, 3);

    EXPECT_EQ(2, t.width());
    EXPECT_EQ(3, t.height());

    ref<UByteArray> rgb = t.toRgb();

    ASSERT_EQ(18, rgb->size());

    EXPECT_EQ(1, rgb->get(0));
    EXPECT_EQ(2, rgb->get(1));
    EXPECT_EQ(3, rgb->get(2));

    EXPECT_EQ(5, rgb->get(3));
    EXPECT_EQ(6, rgb->get(4));
    EXPECT_EQ(1, rgb->get(5));

    EXPECT_EQ(3, rgb->get(6));
    EXPECT_EQ(4, rgb->get(7));
    EXPECT_EQ(5, rgb->get(8));

    EXPECT_EQ(4, rgb->get(9));
    EXPECT_EQ(2, rgb->get(10));
    EXPECT_EQ(1, rgb->get(11));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, GetPixels)
{
    const ubyte rgba[] = { 0,0,9,9,  1,0,9,9,  
                           0,1,9,9,  1,1,9,9, 
                           0,2,9,9,  1,2,9,9};
    TextureImage img;
    img.setData(rgba, 2, 3);
    ASSERT_EQ(2, img.width());
    ASSERT_EQ(3, img.height());

    EXPECT_TRUE(Color4ub(0,0,9,9) == img.pixel(0, 0));
    EXPECT_TRUE(Color4ub(1,0,9,9) == img.pixel(1, 0));
    EXPECT_TRUE(Color4ub(0,1,9,9) == img.pixel(0, 1));
    EXPECT_TRUE(Color4ub(1,1,9,9) == img.pixel(1, 1));
    EXPECT_TRUE(Color4ub(0,2,9,9) == img.pixel(0, 2));
    EXPECT_TRUE(Color4ub(1,2,9,9) == img.pixel(1, 2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, SetGetPixels)
{
    const ubyte rgba[] = { 99,99,99,99,  99,99,99,99,  
                           99,99,99,99,  99,99,99,99,  
                           99,99,99,99,  99,99,99,99};
    TextureImage img;
    img.setData(rgba, 2, 3);
    ASSERT_EQ(2, img.width());
    ASSERT_EQ(3, img.height());

    img.setPixel(0, 0, Color4ub(0,0,0,9));
    img.setPixel(1, 0, Color4ub(1,0,0,9));
    img.setPixel(0, 1, Color4ub(0,1,0,9));
    img.setPixel(1, 1, Color4ub(1,1,0,9));
    img.setPixel(0, 2, Color4ub(0,2,0,9));
    img.setPixel(1, 2, Color4ub(1,2,0,9));

    EXPECT_TRUE(Color4ub(0,0,0,9) == img.pixel(0, 0));
    EXPECT_TRUE(Color4ub(1,0,0,9) == img.pixel(1, 0));
    EXPECT_TRUE(Color4ub(0,1,0,9) == img.pixel(0, 1));
    EXPECT_TRUE(Color4ub(1,1,0,9) == img.pixel(1, 1));
    EXPECT_TRUE(Color4ub(0,2,0,9) == img.pixel(0, 2));
    EXPECT_TRUE(Color4ub(1,2,0,9) == img.pixel(1, 2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, Fill)
{
    const ubyte rgba[] = { 99,99,99,99,  99,99,99,99,  
                           99,99,99,99,  99,99,99,99,  
                           99,99,99,99,  99,99,99,99};
    TextureImage img;
    img.setData(rgba, 2, 3);
    EXPECT_TRUE(Color4ub(99,99,99,99) == img.pixel(0, 0));
    EXPECT_TRUE(Color4ub(99,99,99,99) == img.pixel(1, 0));
    EXPECT_TRUE(Color4ub(99,99,99,99) == img.pixel(0, 1));
    EXPECT_TRUE(Color4ub(99,99,99,99) == img.pixel(1, 1));
    EXPECT_TRUE(Color4ub(99,99,99,99) == img.pixel(0, 2));
    EXPECT_TRUE(Color4ub(99,99,99,99) == img.pixel(1, 2));

    img.fill(Color4ub(1, 2, 3, 4));
    EXPECT_TRUE(Color4ub(1,2,3,4) == img.pixel(0, 0));
    EXPECT_TRUE(Color4ub(1,2,3,4) == img.pixel(1, 0));
    EXPECT_TRUE(Color4ub(1,2,3,4) == img.pixel(0, 1));
    EXPECT_TRUE(Color4ub(1,2,3,4) == img.pixel(1, 1));
    EXPECT_TRUE(Color4ub(1,2,3,4) == img.pixel(0, 2));
    EXPECT_TRUE(Color4ub(1,2,3,4) == img.pixel(1, 2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, FlipVerticalOddRows)
{
    TextureImage t;
    ubyte rgba[] = {1,2,3,4,      5,6,6,7,  
                    8,9,10,11,    12,13,14,15, 
                    16,17,18,19,  20,21,22,23};

    t.setData(rgba, 2, 3);
    EXPECT_EQ(2, t.width());
    EXPECT_EQ(3, t.height());

    t.flipVertical();
    EXPECT_EQ(2, t.width());
    EXPECT_EQ(3, t.height());

    const ubyte* data = t.ptr();

    EXPECT_EQ(16,  data[0]);
    EXPECT_EQ(17,  data[1]);
    EXPECT_EQ(18,  data[2]);
    EXPECT_EQ(19,  data[3]);

    EXPECT_EQ(20,  data[4]);
    EXPECT_EQ(21,  data[5]);
    EXPECT_EQ(22,  data[6]);
    EXPECT_EQ(23,  data[7]);

    EXPECT_EQ(8,  data[8]);
    EXPECT_EQ(9,  data[9]);
    EXPECT_EQ(10,  data[10]);
    EXPECT_EQ(11,  data[11]);

    EXPECT_EQ(12,  data[12]);
    EXPECT_EQ(13,  data[13]);
    EXPECT_EQ(14,  data[14]);
    EXPECT_EQ(15,  data[15]);

    EXPECT_EQ(1,  data[16]);
    EXPECT_EQ(2,  data[17]);
    EXPECT_EQ(3,  data[18]);
    EXPECT_EQ(4,  data[19]);

    EXPECT_EQ(5,  data[20]);
    EXPECT_EQ(6,  data[21]);
    EXPECT_EQ(6,  data[22]);
    EXPECT_EQ(7,  data[23]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureImageTest, FlipVerticalEvenRows)
{
    TextureImage t;
    ubyte rgba[] = {1,2,3,4,      5,6,6,7,  
                   16,17,18,19,  20,21,22,23};

    t.setData(rgba, 2, 2);
    EXPECT_EQ(2, t.width());
    EXPECT_EQ(2, t.height());

    t.flipVertical();
    EXPECT_EQ(2, t.width());
    EXPECT_EQ(2, t.height());

    const ubyte* data = t.ptr();

    EXPECT_EQ(16,  data[0]);
    EXPECT_EQ(17,  data[1]);
    EXPECT_EQ(18,  data[2]);
    EXPECT_EQ(19,  data[3]);

    EXPECT_EQ(20,  data[4]);
    EXPECT_EQ(21,  data[5]);
    EXPECT_EQ(22,  data[6]);
    EXPECT_EQ(23,  data[7]);

    EXPECT_EQ(1,  data[8]);
    EXPECT_EQ(2,  data[9]);
    EXPECT_EQ(3,  data[10]);
    EXPECT_EQ(4,  data[11]);

    EXPECT_EQ(5,  data[12]);
    EXPECT_EQ(6,  data[13]);
    EXPECT_EQ(6,  data[14]);
    EXPECT_EQ(7,  data[15]);
}

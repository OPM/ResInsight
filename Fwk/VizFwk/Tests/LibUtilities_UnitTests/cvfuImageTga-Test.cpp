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
#include "cvfuImageTga.h"

#include "gtest/gtest.h"
#include "gtest/cvftestUtils.h"
#include "cvfTrace.h"

using namespace cvf;
using namespace cvfu;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ImageTgaTest, LoadUncompressed24bit)
{
    String fullFileName = cvftest::TestDataDir::instance()->dataDir() + "TgaTestSuite/UTC24.TGA";  
    cvf::Trace::show("FN: %s\n", fullFileName.toAscii().ptr());
    ref<TextureImage> img = ImageTga::loadImage(fullFileName);
    ASSERT_TRUE(img.notNull());

    EXPECT_EQ(128, img->width());
    EXPECT_EQ(128, img->height());
    EXPECT_TRUE(img->pixel(0, 0) == Color4ub(255, 0, 0, 255));
    EXPECT_TRUE(img->pixel(127, 127) == Color4ub(255, 255, 255, 255));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ImageTgaTest, LoadCompressed24bit)
{
    String fullFileName = cvftest::TestDataDir::instance()->dataDir() + "TgaTestSuite/CTC24.TGA";  
    cvf::Trace::show("FN: %s\n", fullFileName.toAscii().ptr());
    ref<TextureImage> img = ImageTga::loadImage(fullFileName);
    ASSERT_TRUE(img.notNull());

    EXPECT_EQ(128, img->width());
    EXPECT_EQ(128, img->height());
    EXPECT_TRUE(img->pixel(0, 0) == Color4ub(255, 0, 0, 255));
    EXPECT_TRUE(img->pixel(127, 127) == Color4ub(255, 255, 255, 255));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ImageTgaTest, LoadUncompressed32bit)
{
    String fullFileName = cvftest::TestDataDir::instance()->dataDir() + "TgaTestSuite/UTC32.TGA";  
    ref<TextureImage> img = ImageTga::loadImage(fullFileName);
    ASSERT_TRUE(img.notNull());

    EXPECT_EQ(128, img->width());
    EXPECT_EQ(128, img->height());
    EXPECT_TRUE(img->pixel(0, 0) == Color4ub(255, 0, 0, 0));
    EXPECT_TRUE(img->pixel(127, 127) == Color4ub(255, 255, 255, 0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ImageTgaTest, LoadCompressed32bit)
{
    String fullFileName = cvftest::TestDataDir::instance()->dataDir() + "TgaTestSuite/CTC32.TGA";  
    ref<TextureImage> img = ImageTga::loadImage(fullFileName);
    ASSERT_TRUE(img.notNull());

    EXPECT_EQ(128, img->width());
    EXPECT_EQ(128, img->height());
    EXPECT_TRUE(img->pixel(0, 0) == Color4ub(255, 0, 0, 0));
    EXPECT_TRUE(img->pixel(127, 127) == Color4ub(255, 255, 255, 0));
}



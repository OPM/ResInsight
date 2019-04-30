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
#include "cvfqtUtils.h"

#include "gtest/gtest.h"
#include "cvfDebugTimer.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toQString)
{
    const cvf::String str("abc");
    QString qStr = cvfqt::Utils::toQString(str);

    EXPECT_STREQ(str.toAscii().ptr(), (const char*)qStr.toLatin1());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toQImage_withEmptyImage)
{
    cvf::TextureImage i;

    QImage qi = cvfqt::Utils::toQImage(i);
    EXPECT_EQ(0, qi.width());
    EXPECT_EQ(0, qi.height());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toQImage_withSinglePixelImage)
{
    cvf::TextureImage i;
    i.allocate(1, 1);
    i.fill(cvf::Color4ub(1, 2, 3,4));

    QImage qi = cvfqt::Utils::toQImage(i);
    ASSERT_EQ(1, qi.width());
    ASSERT_EQ(1, qi.height());
    EXPECT_EQ(qRgba(1, 2, 3, 4), qi.pixel(0,0));   
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toQImage)
{
    cvf::TextureImage i;
    i.allocate(4, 3);
    i.setPixel(0,2, cvf::Color4ub( 8,18,58,108));    i.setPixel(1,2, cvf::Color4ub( 9,19,59,109));    i.setPixel(2,2, cvf::Color4ub(10,20,60,110));    i.setPixel(3,2, cvf::Color4ub(11,21,61,111));
    i.setPixel(0,1, cvf::Color4ub( 4,14,54,104));    i.setPixel(1,1, cvf::Color4ub( 5,15,55,105));    i.setPixel(2,1, cvf::Color4ub( 6,16,56,106));    i.setPixel(3,1, cvf::Color4ub( 7,17,57,107));
    i.setPixel(0,0, cvf::Color4ub( 0,10,50,100));    i.setPixel(1,0, cvf::Color4ub( 1,11,51,101));    i.setPixel(2,0, cvf::Color4ub( 2,12,52,102));    i.setPixel(3,0, cvf::Color4ub( 3,13,53,103));

    QImage qi = cvfqt::Utils::toQImage(i);
    EXPECT_EQ(qRgba( 8,18,58,108), qi.pixel(0,0));   EXPECT_EQ(qRgba( 9,19,59,109), qi.pixel(1,0));   EXPECT_EQ(qRgba(10,20,60,110), qi.pixel(2,0));   EXPECT_EQ(qRgba(11,21,61,111), qi.pixel(3,0));
    EXPECT_EQ(qRgba( 4,14,54,104), qi.pixel(0,1));   EXPECT_EQ(qRgba( 5,15,55,105), qi.pixel(1,1));   EXPECT_EQ(qRgba( 6,16,56,106), qi.pixel(2,1));   EXPECT_EQ(qRgba( 7,17,57,107), qi.pixel(3,1));
    EXPECT_EQ(qRgba( 0,10,50,100), qi.pixel(0,2));   EXPECT_EQ(qRgba( 1,11,51,101), qi.pixel(1,2));   EXPECT_EQ(qRgba( 2,12,52,102), qi.pixel(2,2));   EXPECT_EQ(qRgba( 3,13,53,103), qi.pixel(3,2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toTextureImage_withEmptyImage)
{
    QImage qi;

    cvf::TextureImage i;
    cvfqt::Utils::toTextureImage(qi, &i);
    EXPECT_EQ(0, i.width());
    EXPECT_EQ(0, i.height());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toTextureImage_withSinglePixelImage)
{
    QImage qi(1, 1, QImage::Format_ARGB32);
    qi.setPixel(0,0, qRgba(1, 2, 3, 4)); 

    cvf::TextureImage i;
    cvfqt::Utils::toTextureImage(qi, &i);
    ASSERT_EQ(1, i.width());
    ASSERT_EQ(1, i.height());
    EXPECT_EQ(cvf::Color4ub(1, 2, 3, 4), i.pixel(0,0));  
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toTextureImage)
{
    QImage qi(4, 3, QImage::Format_ARGB32);
    qi.setPixel(0,0, qRgba( 0,10,50,100));   qi.setPixel(1,0, qRgba( 1,11,51,101));   qi.setPixel(2,0, qRgba( 2,12,52,102));   qi.setPixel(3,0, qRgba( 3,13,53,103));
    qi.setPixel(0,1, qRgba( 4,14,54,104));   qi.setPixel(1,1, qRgba( 5,15,55,105));   qi.setPixel(2,1, qRgba( 6,16,56,106));   qi.setPixel(3,1, qRgba( 7,17,57,107));
    qi.setPixel(0,2, qRgba( 8,18,58,108));   qi.setPixel(1,2, qRgba( 9,19,59,109));   qi.setPixel(2,2, qRgba(10,20,60,110));   qi.setPixel(3,2, qRgba(11,21,61,111));

    cvf::TextureImage i;
    cvfqt::Utils::toTextureImage(qi, &i);
    ASSERT_EQ(4, i.width());
    ASSERT_EQ(3, i.height());

    EXPECT_EQ(cvf::Color4ub( 0,10,50,100), i.pixel(0,2));  EXPECT_EQ(cvf::Color4ub( 1,11,51,101), i.pixel(1,2));  EXPECT_EQ(cvf::Color4ub( 2,12,52,102), i.pixel(2,2));  EXPECT_EQ(cvf::Color4ub( 3,13,53,103), i.pixel(3,2));
    EXPECT_EQ(cvf::Color4ub( 4,14,54,104), i.pixel(0,1));  EXPECT_EQ(cvf::Color4ub( 5,15,55,105), i.pixel(1,1));  EXPECT_EQ(cvf::Color4ub( 6,16,56,106), i.pixel(2,1));  EXPECT_EQ(cvf::Color4ub( 7,17,57,107), i.pixel(3,1));
    EXPECT_EQ(cvf::Color4ub( 8,18,58,108), i.pixel(0,0));  EXPECT_EQ(cvf::Color4ub( 9,19,59,109), i.pixel(1,0));  EXPECT_EQ(cvf::Color4ub(10,20,60,110), i.pixel(2,0));  EXPECT_EQ(cvf::Color4ub(11,21,61,111), i.pixel(3,0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toTextureImage_premultipliedImageFormat)
{
    QImage qi(4, 3, QImage::Format_ARGB32_Premultiplied);
    qi.setPixel(0,0, qRgba( 0,10,50,100));   qi.setPixel(1,0, qRgba( 1,11,51,101));   qi.setPixel(2,0, qRgba( 2,12,52,102));   qi.setPixel(3,0, qRgba( 3,13,53,103));
    qi.setPixel(0,1, qRgba( 4,14,54,104));   qi.setPixel(1,1, qRgba( 5,15,55,105));   qi.setPixel(2,1, qRgba( 6,16,56,106));   qi.setPixel(3,1, qRgba( 7,17,57,107));
    qi.setPixel(0,2, qRgba( 8,18,58,108));   qi.setPixel(1,2, qRgba( 9,19,59,109));   qi.setPixel(2,2, qRgba(10,20,60,110));   qi.setPixel(3,2, qRgba(11,21,61,111));

    cvf::TextureImage i;
    cvfqt::Utils::toTextureImage(qi, &i);
    ASSERT_EQ(4, i.width());
    ASSERT_EQ(3, i.height());

    EXPECT_EQ(cvf::Color4ub( 0,10,50,100), i.pixel(0,2));  EXPECT_EQ(cvf::Color4ub( 1,11,51,101), i.pixel(1,2));  EXPECT_EQ(cvf::Color4ub( 2,12,52,102), i.pixel(2,2));  EXPECT_EQ(cvf::Color4ub( 3,13,53,103), i.pixel(3,2));
    EXPECT_EQ(cvf::Color4ub( 4,14,54,104), i.pixel(0,1));  EXPECT_EQ(cvf::Color4ub( 5,15,55,105), i.pixel(1,1));  EXPECT_EQ(cvf::Color4ub( 6,16,56,106), i.pixel(2,1));  EXPECT_EQ(cvf::Color4ub( 7,17,57,107), i.pixel(3,1));
    EXPECT_EQ(cvf::Color4ub( 8,18,58,108), i.pixel(0,0));  EXPECT_EQ(cvf::Color4ub( 9,19,59,109), i.pixel(1,0));  EXPECT_EQ(cvf::Color4ub(10,20,60,110), i.pixel(2,0));  EXPECT_EQ(cvf::Color4ub(11,21,61,111), i.pixel(3,0));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toTextureImageRegion_emptyRegion)
{
    QImage qi(3, 4, QImage::Format_ARGB32);

    {
        cvf::TextureImage i;
        cvfqt::Utils::toTextureImageRegion(qi, cvf::Vec2ui(1, 1), cvf::Vec2ui(0, 0), &i);
        EXPECT_EQ(0, i.width());
        EXPECT_EQ(0, i.height());
    }
    {
        cvf::TextureImage i;
        cvfqt::Utils::toTextureImageRegion(qi, cvf::Vec2ui(1, 1), cvf::Vec2ui(1, 0), &i);
        EXPECT_EQ(0, i.width());
        EXPECT_EQ(0, i.height());
    }
    {
        cvf::TextureImage i;
        cvfqt::Utils::toTextureImageRegion(qi, cvf::Vec2ui(1, 1), cvf::Vec2ui(0, 1), &i);
        EXPECT_EQ(0, i.width());
        EXPECT_EQ(0, i.height());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UtilsTest, toTextureImageRegion)
{
    QImage qi(4, 3, QImage::Format_ARGB32);
    qi.setPixel(0,0, qRgba( 0,10,50,100));   qi.setPixel(1,0, qRgba( 1,11,51,101));   qi.setPixel(2,0, qRgba( 2,12,52,102));   qi.setPixel(3,0, qRgba( 3,13,53,103));
    qi.setPixel(0,1, qRgba( 4,14,54,104));   qi.setPixel(1,1, qRgba( 5,15,55,105));   qi.setPixel(2,1, qRgba( 6,16,56,106));   qi.setPixel(3,1, qRgba( 7,17,57,107));
    qi.setPixel(0,2, qRgba( 8,18,58,108));   qi.setPixel(1,2, qRgba( 9,19,59,109));   qi.setPixel(2,2, qRgba(10,20,60,110));   qi.setPixel(3,2, qRgba(11,21,61,111));

    {
        cvf::TextureImage i;
        cvfqt::Utils::toTextureImageRegion(qi, cvf::Vec2ui(0, 0), cvf::Vec2ui(2, 1), &i);
        ASSERT_EQ(2, i.width());
        ASSERT_EQ(1, i.height());
        EXPECT_EQ(cvf::Color4ub( 0,10,50,100), i.pixel(0,0));  EXPECT_EQ(cvf::Color4ub( 1,11,51,101), i.pixel(1,0));
    }
    {
        cvf::TextureImage i;
        cvfqt::Utils::toTextureImageRegion(qi, cvf::Vec2ui(2, 1), cvf::Vec2ui(2, 2), &i);
        ASSERT_EQ(2, i.width());
        ASSERT_EQ(2, i.height());
        EXPECT_EQ(cvf::Color4ub( 6,16,56,106), i.pixel(0,1));  EXPECT_EQ(cvf::Color4ub( 7,17,57,107), i.pixel(1,1));
        EXPECT_EQ(cvf::Color4ub(10,20,60,110), i.pixel(0,0));  EXPECT_EQ(cvf::Color4ub(11,21,61,111), i.pixel(1,0));
    }
    {
        cvf::TextureImage i;
        cvfqt::Utils::toTextureImageRegion(qi, cvf::Vec2ui(0, 2), cvf::Vec2ui(4, 1), &i);
        ASSERT_EQ(4, i.width());
        ASSERT_EQ(1, i.height());
        EXPECT_EQ(cvf::Color4ub( 8,18,58,108), i.pixel(0,0));  EXPECT_EQ(cvf::Color4ub( 9,19,59,109), i.pixel(1,0));  EXPECT_EQ(cvf::Color4ub(10,20,60,110), i.pixel(2,0));  EXPECT_EQ(cvf::Color4ub(11,21,61,111), i.pixel(3,0));
    }
    {
        cvf::TextureImage i;
        cvfqt::Utils::toTextureImageRegion(qi, cvf::Vec2ui(3, 0), cvf::Vec2ui(1, 3), &i);
        ASSERT_EQ(1, i.width());
        ASSERT_EQ(3, i.height());
        EXPECT_EQ(cvf::Color4ub( 3,13,53,103), i.pixel(0,2));
        EXPECT_EQ(cvf::Color4ub( 7,17,57,107), i.pixel(0,1));
        EXPECT_EQ(cvf::Color4ub(11,21,61,111), i.pixel(0,0));
    }
    
}


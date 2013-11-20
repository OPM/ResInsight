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
#include "cvfTexture.h"
#include "cvfTextureImage.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureTest, Construction)
{
    Texture t1(Texture::TEXTURE_RECTANGLE, Texture::RGBA32F);
    ref<TextureImage> img = new TextureImage;
    Texture t2(img.p());
    
    EXPECT_EQ(Texture::TEXTURE_RECTANGLE, t1.textureType());
    EXPECT_EQ(Texture::RGBA32F, t1.internalFormat());
    EXPECT_TRUE(t1.image() == NULL);
    EXPECT_EQ(0, t1.textureOglId());
    
    EXPECT_EQ(Texture::TEXTURE_2D, t2.textureType());
    EXPECT_EQ(Texture::RGBA, t2.internalFormat());
    EXPECT_TRUE(t2.image() == img.p());
    EXPECT_EQ(0, t2.textureOglId());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureTest, SetSize)
{
    Texture t1(Texture::TEXTURE_2D, Texture::RGBA);
    t1.setSize(100, 200);

    EXPECT_EQ(100, t1.width());
    EXPECT_EQ(200, t1.height());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(TextureDeathTest, IllegalSetSize)
{
    ref<TextureImage> img = new TextureImage;
    Texture t1(img.p());

    EXPECT_DEATH(t1.setSize(100, 200), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureTest, SetImage)
{
    ref<TextureImage> img = new TextureImage;
    img->allocate(100,200);

    Texture t(Texture::TEXTURE_2D, Texture::RGBA);

    t.setFromImage(img.p());

    EXPECT_EQ(2, img->refCount());
    EXPECT_EQ(100, t.width());
    EXPECT_EQ(200, t.height());
    EXPECT_TRUE(t.image() == img.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(TextureDeathTest, IllegalSetFromImage)
{
    ref<TextureImage> img = new TextureImage;
    Texture t(Texture::TEXTURE_CUBE_MAP, Texture::RGBA);

    EXPECT_DEATH(t.setFromImage(img.p()), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureTest, SetCubeMapImage)
{
    Texture t(Texture::TEXTURE_CUBE_MAP, Texture::RGBA);

    ref<TextureImage> imgPx = new TextureImage;    imgPx->allocate(100,200);
    t.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_X, imgPx.p());
    
    ref<TextureImage> imgNx = new TextureImage;    imgNx->allocate(100,200);
    t.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_X, imgNx.p());
    
    ref<TextureImage> imgPy = new TextureImage;    imgPy->allocate(100,200);
    t.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_Y, imgPy.p());
    
    ref<TextureImage> imgNy = new TextureImage;    imgNy->allocate(100,200);
    t.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_Y, imgNy.p());
    
    ref<TextureImage> imgPz = new TextureImage;    imgPz->allocate(100,200);
    t.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_Z, imgPz.p());
    
    ref<TextureImage> imgNz = new TextureImage;    imgNz->allocate(100,200);
    t.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_Z, imgNz.p());

    EXPECT_EQ(100, t.width());
    EXPECT_EQ(200, t.height());

    EXPECT_EQ(imgPx.p(), t.cubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_X));
    EXPECT_EQ(imgNx.p(), t.cubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_X));
    EXPECT_EQ(imgPy.p(), t.cubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_Y));
    EXPECT_EQ(imgNy.p(), t.cubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_Y));
    EXPECT_EQ(imgPz.p(), t.cubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_Z));
    EXPECT_EQ(imgNz.p(), t.cubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_Z));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_ASSERTS == 1
TEST(TextureDeathTest, IllegalSetCubeMapImage)
{
    ref<TextureImage> img = new TextureImage;
    Texture t(Texture::TEXTURE_2D, Texture::RGBA);

    EXPECT_DEATH(t.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_X, img.p()), "Assertion");


    Texture t2(Texture::TEXTURE_CUBE_MAP, Texture::RGBA);

    ref<TextureImage> imgPx = new TextureImage;    imgPx->allocate(100,200);
    t2.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_X, imgPx.p());

    ref<TextureImage> imgNx = new TextureImage;    imgNx->allocate(200,200);
    EXPECT_DEATH(t2.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_NEGATIVE_X, imgNx.p()), "Assertion");

}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TextureTest, ClearImages)
{
    ref<TextureImage> img = new TextureImage;
    Texture t1(img.p());    

    t1.clearImages();
    EXPECT_EQ(NULL, t1.image());

    Texture t2(Texture::TEXTURE_CUBE_MAP, Texture::RGBA);

    ref<TextureImage> imgPx = new TextureImage;    imgPx->allocate(100,200);
    t2.setCubeMapImage(Texture::TEXTURE_CUBE_MAP_POSITIVE_X, imgPx.p());
    t2.clearImages();
}

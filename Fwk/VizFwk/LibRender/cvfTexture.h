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


#pragma once

#include "cvfObject.h"
#include "cvfOpenGLTypes.h"
#include "cvfCollection.h"

namespace cvf {

class TextureImage;
class Sampler;
class OpenGLContext;
class OglRcTexture;


//==================================================================================================
//
// OpenGL Texture Object. 
//
//==================================================================================================
class Texture : public Object
{
public:
    enum InternalFormat
    {
        RGBA,
        RGBA32F,            // Requires OpenGLCapabilities::TEXTURE_FLOAT
        R32F,               // Requires OpenGLCapabilities::TEXTURE_RG
        DEPTH_COMPONENT16,
        DEPTH_COMPONENT24,
        DEPTH_COMPONENT32,
        DEPTH24_STENCIL8
    };

    enum TextureType
    {
        TEXTURE_2D,
        TEXTURE_RECTANGLE,  // Requires OpenGLCapabilities::TEXTURE_RECTANGLE
        TEXTURE_CUBE_MAP
    };

    enum CubeMapFace
    {
        // Order is important as this follows the OpenGL definitions
        TEXTURE_CUBE_MAP_POSITIVE_X = 0, 
        TEXTURE_CUBE_MAP_NEGATIVE_X, 
        TEXTURE_CUBE_MAP_POSITIVE_Y, 
        TEXTURE_CUBE_MAP_NEGATIVE_Y, 
        TEXTURE_CUBE_MAP_POSITIVE_Z, 
        TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

public:
    Texture(TextureType textureType, InternalFormat internalFormat);
    Texture(TextureImage* image);
    virtual ~Texture();

    void            setSize(uint width, uint height);

    uint            width() const;
    uint            height() const;

    TextureType     textureType() const;
    InternalFormat  internalFormat() const;

    void            setFromImage(TextureImage* image);
    void            setCubeMapImage(CubeMapFace face, TextureImage* cubeMapImage);
    TextureImage*   image();
    TextureImage*   cubeMapImage(CubeMapFace face);
    void            clearImages();

    void            enableMipmapGeneration(bool enableMipmapGeneration);
    bool            isMipmapGenerationEnabled() const;
    bool            hasMipmap() const;
    void            generateMipmap(OpenGLContext* oglContext);
    void            removeMipmap();

    void            bind(OpenGLContext* oglContext) const;
    bool            isBound(OpenGLContext* oglContext) const;

    bool            setupTexture(OpenGLContext* oglContext);
    void            setupTextureParamsFromSampler(OpenGLContext* oglContext, const Sampler& sampler) const;
    void            deleteTexture(OpenGLContext* oglContext);

    OglId           textureOglId() const;
    uint            versionTick() const;

    static bool     supportedOpenGL(OpenGLContext* oglContext);

private:
    void            forgetCurrentOglTexture();
    cvfGLenum       textureTypeOpenGL() const;
    cvfGLint        internalFormatOpenGL() const;

private:
    TextureType                 m_textureType;
    InternalFormat              m_internalFormat;
    ref<TextureImage>           m_image;
    Collection<TextureImage>    m_cubeMapImages;
    
    bool                        m_enableMipmapGeneration;   // Enable auto generation of mipmaps
    bool                        m_hasMipmaps;               // Set to true if we have mipmaps

    uint                        m_width;
    uint                        m_height;
    uint                        m_versionTick;              // Versioning to be able to detect changes

    ref<OglRcTexture>           m_oglRcTexture;		        // 
};

}  // namespace cvf

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

namespace cvf {

class TextureImage;
class OpenGLContext;
class OglRcTexture;


//==================================================================================================
//
// 
//
//==================================================================================================
class Texture2D_FF : public Object
{
public:
    enum WrapMode
    {
        REPEAT,
        CLAMP
    };

    enum Filter
    {
        NEAREST, 
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_NEAREST,
        LINEAR_MIPMAP_LINEAR
    };

public:
    Texture2D_FF(TextureImage* image = NULL);
    virtual ~Texture2D_FF();

    void            setImage(TextureImage* image);
    TextureImage*   image();

    void            setWrapMode(WrapMode wrapMode);
    void            setMinFilter(Filter minFilter);
    void            setMagFilter(Filter magFilter);

    void            bind(OpenGLContext* oglContext) const;
    bool            isBound(OpenGLContext* oglContext) const;

    bool            setupTexture(OpenGLContext* oglContext);
    void            setupTextureParams(OpenGLContext* oglContext) const;
    void            deleteTexture(OpenGLContext* oglContext);

    OglId           textureOglId() const;

private:
    cvfGLint        filterTypeOpenGL(Filter filter) const;

private:
    ref<TextureImage>   m_image;
    WrapMode            m_wrapMode;         // Wrap mode for both S and T
    Filter              m_minFilter;
    Filter              m_magFilter;
    ref<OglRcTexture>   m_oglRcTexture;		// 
};

}  // namespace cvf

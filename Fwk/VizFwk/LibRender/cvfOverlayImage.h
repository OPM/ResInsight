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

#include "cvfOverlayItem.h"

namespace cvf {

class TextureImage;
class Sampler;
class RenderState;
class Texture;
class ShaderProgram;

//==================================================================================================
//
// Overlay text box
//
//==================================================================================================
class OverlayImage : public OverlayItem
{
public:
    enum Blending
    {
        NO_BLENDING,
        GLOBAL_ALPHA,
        TEXTURE_ALPHA
    };

public:
    OverlayImage(TextureImage* image);
    ~OverlayImage();

    virtual Vec2ui      sizeHint();

    virtual void        render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);
    virtual void        renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);

    void                setImage(TextureImage* image);
    void                setPixelSize(const Vec2ui& size);
    void                setGlobalAlpha(float alphaFactor);
    void                setBlending(Blending mode);

    const TextureImage* image() const;
    float               globalAlpha() const;
    Blending            blending() const;

private:
    void render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software);

private:
    Vec2ui                          m_size;
    ref<TextureImage>               m_image;
    ref<TextureImage>               m_pow2Image;
    ref<Sampler>                    m_sampler;
    ref<RenderState>                m_textureBindings;
    ref<Texture>                    m_texture;
    ref<ShaderProgram>              m_shaderProgram;

    Blending                        m_blendMode;
    float                           m_alpha;
};

}

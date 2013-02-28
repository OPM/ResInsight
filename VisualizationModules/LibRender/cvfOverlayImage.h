//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
    virtual Vec2ui      maximumSize();
    virtual Vec2ui      minimumSize();

    virtual void        render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);
    virtual void        renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size);

    void                setImage(TextureImage* image);
    void                setPixelSize(const Vec2ui& size);
    void                setGlobalAlpha(float alphaFactor);
    void                setBlending(Blending mode);

    const TextureImage* image() const;

private:
    void render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software);

private:
    Vec2ui                          m_size;
    ref<TextureImage>               m_image;
    ref<Sampler>                    m_sampler;
    ref<RenderState>                m_textureBindings;
    ref<Texture>                    m_texture;
    ref<ShaderProgram>              m_shaderProgram;

    Blending                        m_blendMode;
    float                           m_alpha;
};

}

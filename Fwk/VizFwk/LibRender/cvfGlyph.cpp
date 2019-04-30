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
#include "cvfOpenGL.h"
#include "cvfGlyph.h"
#include "cvfMath.h"
#include "cvfTextureImage.h"
#include "cvfTexture.h"
#include "cvfSampler.h"
#include "cvfRenderStateTextureBindings.h"

#ifndef CVF_OPENGL_ES
#include "cvfTexture2D_FF.h"
#include "cvfRenderState_FF.h"
#endif

namespace cvf {


//==================================================================================================
///
/// \class cvf::Glyph
/// \ingroup Render
///
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
/// \param character The character this glyph is generated from
//--------------------------------------------------------------------------------------------------
Glyph::Glyph(wchar_t character)
:   m_character(character),
    m_width(0),
    m_height(0),
    m_horizontalBearingX(0),
    m_horizontalBearingY(0),
    m_horizontalAdvance(0),
    m_textureImage(new TextureImage),
    m_textureCoordinates(new FloatArray(8)),
    m_minFilter(NEAREST),
    m_magFilter(NEAREST)
{
    // Lower left
    m_textureCoordinates->set(0, 0.0f);
    m_textureCoordinates->set(1, 0.0f);

    // Lower right
    m_textureCoordinates->set(2, 1.0f);
    m_textureCoordinates->set(3, 0.0f);

    // Upper right
    m_textureCoordinates->set(4, 1.0f);
    m_textureCoordinates->set(5, 1.0f);

    // Upper left
    m_textureCoordinates->set(6, 0.0f);
    m_textureCoordinates->set(7, 1.0f);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Glyph::~Glyph()
{
}


//--------------------------------------------------------------------------------------------------
/// Get character this glyph is generated from
//--------------------------------------------------------------------------------------------------
wchar_t Glyph::character() const
{
    return m_character;
}


//--------------------------------------------------------------------------------------------------
/// Set width of the pre-rendered character, in pixels. 
///
/// \note May be smaller than texture image width. Use the texture coordinates to identify the glyph pixels.
///
/// \sa width(), setHeight(), height(), textureImage(), textureCoordinates()
//--------------------------------------------------------------------------------------------------
void Glyph::setWidth(uint width)
{
    m_width = width;
}


//--------------------------------------------------------------------------------------------------
/// Get width of the pre-rendered character, in pixels. 
///
/// \note May be smaller than texture image width. Use the texture coordinates to identify the glyph pixels.
///
/// \sa setWidth(), setHeight(), height(), textureImage(), textureCoordinates()
//--------------------------------------------------------------------------------------------------
uint Glyph::width() const
{
    return m_width;
}


//--------------------------------------------------------------------------------------------------
/// Set height of the pre-rendered character, in pixels. 
///
/// \note May be smaller than texture image height. Use the texture coordinates to identify the glyph pixels.
///
/// \sa height(), setWidth(), width(), textureImage(), textureCoordinates()
//--------------------------------------------------------------------------------------------------
void Glyph::setHeight(uint height)
{
    m_height = height;
}


//--------------------------------------------------------------------------------------------------
/// Get height of the pre-rendered character, in pixels. 
///
/// \note May be smaller than texture image height. Use the texture coordinates to identify the glyph pixels.
///
/// \sa setHeight(), setWidth(), width(), textureImage(), textureCoordinates()
//--------------------------------------------------------------------------------------------------
uint Glyph::height() const
{
    return m_height;
}


//--------------------------------------------------------------------------------------------------
/// Set pre-rendered image of m_character
///
/// \sa textureImage(), width(), height()
//--------------------------------------------------------------------------------------------------
void Glyph::setTextureImage(TextureImage* textureImage)
{
    m_textureImage = textureImage;
}


//--------------------------------------------------------------------------------------------------
/// Get pre-rendered image of m_character
///
/// \sa textureCoordinates(), width(), height()
//--------------------------------------------------------------------------------------------------
const TextureImage* Glyph::textureImage() const
{
    return m_textureImage.p();
}


//--------------------------------------------------------------------------------------------------
/// Get texture coordinates
///
/// \sa textureImage(), width(), height()
//--------------------------------------------------------------------------------------------------
const FloatArray* Glyph::textureCoordinates() const
{
    return m_textureCoordinates.p();
}


//--------------------------------------------------------------------------------------------------
/// Set horizontal distance from the current cursor position to the leftmost
/// border of the glyph image's bounding box
//--------------------------------------------------------------------------------------------------
void Glyph::setHorizontalBearingX(short bearing)
{
    m_horizontalBearingX = bearing;
}


//--------------------------------------------------------------------------------------------------
/// Get horizontal distance from the current cursor position to the leftmost
/// border of the glyph image's bounding box
//--------------------------------------------------------------------------------------------------
short Glyph::horizontalBearingX() const
{
    return m_horizontalBearingX;
}


//--------------------------------------------------------------------------------------------------
/// Set vertical distance from the current cursor position (on the baseline)
/// to the topmost border of the glyph image's bounding box.
//--------------------------------------------------------------------------------------------------
void Glyph::setHorizontalBearingY(short bearing)
{
    m_horizontalBearingY = bearing;
}


//--------------------------------------------------------------------------------------------------
/// Get vertical distance from the current cursor position (on the baseline)
/// to the topmost border of the glyph image's bounding box.
//--------------------------------------------------------------------------------------------------
short Glyph::horizontalBearingY() const
{
    return m_horizontalBearingY;
}


//--------------------------------------------------------------------------------------------------
/// Set horizontal distance used to increment the pen position 
/// when the glyph is drawn as part of a string of text.
//--------------------------------------------------------------------------------------------------
void Glyph::setHorizontalAdvance(uint advance)
{
    m_horizontalAdvance = advance;
}


//--------------------------------------------------------------------------------------------------
/// Get horizontal distance used to increment the pen position 
/// when the glyph is drawn as part of a string of text.
//--------------------------------------------------------------------------------------------------
uint Glyph::horizontalAdvance() const
{
    return m_horizontalAdvance;
}


//--------------------------------------------------------------------------------------------------
/// Setup and bind texture
//--------------------------------------------------------------------------------------------------
void Glyph::setupAndBindTexture(OpenGLContext* oglContext, bool software)
{
    // Short path first if everything is in place
    if (m_textureBindings.notNull())
    {
        RenderState::Type renderStateType = m_textureBindings->type();
        if (software)
        {
#ifndef CVF_OPENGL_ES
            if (renderStateType == RenderState::TEXTURE_MAPPING_FF)
            {
                RenderStateTextureMapping_FF* texMapping = static_cast<RenderStateTextureMapping_FF*>(m_textureBindings.p());
                texMapping->setupTexture(oglContext);
                texMapping->applyOpenGL(oglContext);
                return;
            }
#else
            CVF_FAIL_MSG("Not supported on OpenGL ES");
#endif
        }
        else
        {
            if (renderStateType == RenderState::TEXTURE_BINDINGS)
            {
                RenderStateTextureBindings* texBindings = static_cast<RenderStateTextureBindings*>(m_textureBindings.p());
                texBindings->setupTextures(oglContext);
                texBindings->applyOpenGL(oglContext);
                return;
            }
        }
    }

    m_textureBindings = NULL;

    if (m_textureBindings.isNull())
    {
        // TODO
        // Revisit the code that sets up the texture image
        // The code below ends up modifying the stored texture image
        // Is this intentional - there is an external getter for the image!!
        CVF_TIGHT_ASSERT(0 < m_textureImage->width());
        CVF_TIGHT_ASSERT(0 < m_textureImage->height());

        uint pow2Width = Math::roundUpPow2(m_width);
        uint pow2Height = Math::roundUpPow2(m_height);

        TextureImage* pow2Image = new TextureImage;
        pow2Image->allocate(pow2Width, pow2Height);
        pow2Image->fill(Color4ub(255, 255, 255, 0));

        uint i, j;
        for (j = 0; j < m_height; j++)
        {
            for (i = 0; i < m_width; i++)
            {
                pow2Image->setPixel(i, j, m_textureImage->pixel(i, j));
            }
        }

        float textureCoordinateMaxX = static_cast<float>(m_width) / static_cast<float>(pow2Width);
        float textureCoordinateMaxY = static_cast<float>(m_height) / static_cast<float>(pow2Height);

        // Lower left
        m_textureCoordinates->set(0, 0.0f);
        m_textureCoordinates->set(1, 0.0f);

        // Lower right
        m_textureCoordinates->set(2, textureCoordinateMaxX);
        m_textureCoordinates->set(3, 0.0f);

        // Upper right
        m_textureCoordinates->set(4, textureCoordinateMaxX);
        m_textureCoordinates->set(5, textureCoordinateMaxY);

        // Upper left
        m_textureCoordinates->set(6, 0.0f);
        m_textureCoordinates->set(7, textureCoordinateMaxY);

        m_textureImage = pow2Image;

        if (software)
        {
#ifdef CVF_OPENGL_ES
            CVF_FAIL_MSG("Not supported on OpenGL ES");
#else
            // Use fixed function texture setup
            ref<Texture2D_FF> texture = new Texture2D_FF(m_textureImage.p());
            texture->setWrapMode(Texture2D_FF::CLAMP);
            if (m_minFilter == NEAREST)
            {
                texture->setMinFilter(Texture2D_FF::NEAREST);
            }
            else
            {
                texture->setMinFilter(Texture2D_FF::LINEAR);
            }

            if (m_magFilter == NEAREST)
            {
                texture->setMagFilter(Texture2D_FF::NEAREST);
            }
            else
            {
                texture->setMagFilter(Texture2D_FF::LINEAR);
            }

            ref<RenderStateTextureMapping_FF> textureMapping = new RenderStateTextureMapping_FF(texture.p());
            textureMapping->setTextureFunction(RenderStateTextureMapping_FF::MODULATE);
            textureMapping->setupTexture(oglContext);

            m_textureBindings = textureMapping;
#endif
        }
        else
        {
            ref<Sampler> sampler = new Sampler;
            sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
            if (m_minFilter == NEAREST)
            {
                sampler->setMinFilter(Sampler::NEAREST);
            }
            else
            {
                sampler->setMinFilter(Sampler::LINEAR);
            }
            
            if (m_magFilter == NEAREST)
            {
                sampler->setMagFilter(Sampler::NEAREST);
            }
            else
            {
                sampler->setMagFilter(Sampler::LINEAR);
            }

            ref<Texture> texture = new Texture(m_textureImage.p());

            RenderStateTextureBindings* textureBindings = new RenderStateTextureBindings(texture.p(), sampler.p(), "dummy");
            textureBindings->setupTextures(oglContext);

            m_textureBindings = textureBindings;
        }
    }

    if (m_textureBindings.notNull())
    {
        m_textureBindings->applyOpenGL(oglContext);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Glyph::setMinFilter(TextureFilter filter)
{
    m_minFilter = filter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Glyph::setMagFilter(TextureFilter filter)
{
    m_magFilter = filter;
}

}  // namespace cvf

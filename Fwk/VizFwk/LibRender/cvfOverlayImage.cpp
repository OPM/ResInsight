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
#include "cvfOverlayImage.h"
#include "cvfMatrixState.h"
#include "cvfCamera.h"
#include "cvfShaderProgram.h"
#include "cvfOpenGL.h"
#include "cvfViewport.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfUniform.h"
#include "cvfTextureImage.h"
#include "cvfSampler.h"
#include "cvfTexture.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceRepository.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderProgram.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfRenderStateBlending.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif
#include "cvfTexture2D_FF.h"

namespace cvf {

//==================================================================================================
///
/// \class cvf::OverlayImage
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor. The specified font is used to draw all the text
//--------------------------------------------------------------------------------------------------
OverlayImage::OverlayImage(TextureImage* image)
:   m_alpha(1.0f)
{
    CVF_ASSERT(image);

    m_blendMode = NO_BLENDING;
    
    m_sampler = new cvf::Sampler;
    m_sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    m_sampler->setMinFilter(cvf::Sampler::NEAREST);
    m_sampler->setMagFilter(cvf::Sampler::NEAREST);

    setImage(image);
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
OverlayImage::~OverlayImage()
{
}


//--------------------------------------------------------------------------------------------------
/// Returns the wanted size in pixels
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui OverlayImage::sizeHint()
{
    return m_size;
}


//--------------------------------------------------------------------------------------------------
/// Render using Shaders
//--------------------------------------------------------------------------------------------------
void OverlayImage::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, false);
}


//--------------------------------------------------------------------------------------------------
/// Render using Fixed Function
//--------------------------------------------------------------------------------------------------
void OverlayImage::renderSoftware(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size)
{
    render(oglContext, position, size, true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayImage::render(OpenGLContext* oglContext, const Vec2i& position, const Vec2ui& size, bool software)
{
    CVF_CALLSITE_OPENGL(oglContext);

    Camera projCam;
    projCam.setViewport(position.x(), position.y(), size.x(), size.y());
    projCam.setProjectionAsPixelExact2D();
    projCam.setViewMatrix(Mat4d::IDENTITY);

    // Turn off depth test
    RenderStateDepth depth(false, RenderStateDepth::LESS, false);
    depth.applyOpenGL(oglContext);

    float vertexArray[12];
    float textureCoords[] = {0.0f, 0.0f,
                            1.0f, 0.0f,
                            1.0f, 1.0f,
                            0.0f, 1.0f};
    
    projCam.viewport()->applyOpenGL(oglContext, Viewport::DO_NOT_CLEAR);

    if (software)
    {
        // Create a POW2 texture for software rendering if needed
        if (m_image.notNull() && m_pow2Image.isNull() && (!Math::isPow2(m_image->width()) || !Math::isPow2(m_image->height())))
        {
            m_pow2Image = new TextureImage;
            m_pow2Image->allocate(Math::roundUpPow2(m_image->width()), Math::roundUpPow2(m_image->height()));
            m_pow2Image->fill(Color4ub(Color3::BLACK));

            for (uint y = 0; y < m_image->height(); ++y)
            {
                for (uint x = 0; x < m_image->width(); ++x)
                {
                    m_pow2Image->setPixel(x, y, m_image->pixel(x, y));
                }
            }
        }


        if (ShaderProgram::supportedOpenGL(oglContext))
        {
            ShaderProgram::useNoProgram(oglContext);
        }

#ifndef CVF_OPENGL_ES
        RenderStateMaterial_FF mat;
        mat.enableColorMaterial(true);
        mat.applyOpenGL(oglContext);

        RenderStateLighting_FF light(false);
        light.applyOpenGL(oglContext);

        if (m_textureBindings.isNull())
        {
            // Use fixed function texture setup
            ref<Texture2D_FF> texture = new Texture2D_FF(m_pow2Image.notNull() ? m_pow2Image.p() : m_image.p());
            texture->setWrapMode(Texture2D_FF::CLAMP);
            texture->setMinFilter(Texture2D_FF::NEAREST);
            texture->setMagFilter(Texture2D_FF::NEAREST);
            texture->setupTexture(oglContext);
            texture->setupTextureParams(oglContext);

            ref<RenderStateTextureMapping_FF> textureMapping = new RenderStateTextureMapping_FF(texture.p());
            textureMapping->setTextureFunction(m_blendMode == TEXTURE_ALPHA ? RenderStateTextureMapping_FF::MODULATE : RenderStateTextureMapping_FF::DECAL);

            m_textureBindings = textureMapping;
        }
#endif
        // Adjust texture coordinates
        if (m_pow2Image.notNull())
        {
            float xMax = static_cast<float>(m_image->width())/static_cast<float>(m_pow2Image->width());
            float yMax = static_cast<float>(m_image->height())/static_cast<float>(m_pow2Image->height());
            textureCoords[2] = xMax;
            textureCoords[4] = xMax;
            textureCoords[5] = yMax;
            textureCoords[7] = yMax;
        }

        projCam.applyOpenGL();
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(ShaderProgram::VERTEX);
        glEnableVertexAttribArray(ShaderProgram::TEX_COORD_2F_0);
        glVertexAttribPointer(ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertexArray);
        glVertexAttribPointer(ShaderProgram::TEX_COORD_2F_0, 2, GL_FLOAT, GL_FALSE, 0, textureCoords);

        if (m_shaderProgram.isNull())
        {
            ShaderProgramGenerator gen("OverlayImage_Shader", ShaderSourceProvider::instance());
            gen.addVertexCode(ShaderSourceRepository::vs_MinimalTexture);
            
            if (m_blendMode == GLOBAL_ALPHA)
            {
                gen.addFragmentCode(ShaderSourceRepository::src_TextureGlobalAlpha);
            }
            else
            {
                gen.addFragmentCode(ShaderSourceRepository::src_Texture);
            }

            gen.addFragmentCode(ShaderSourceRepository::fs_Unlit);
            m_shaderProgram = gen.generate();
            m_shaderProgram->linkProgram(oglContext);
        }

        if (m_shaderProgram->useProgram(oglContext))
        {
            MatrixState projMatrixState(projCam);
            m_shaderProgram->clearUniformApplyTracking();
            m_shaderProgram->applyFixedUniforms(oglContext, projMatrixState);
        }

        if (m_texture->textureOglId() == 0)
        {
            m_texture->setupTexture(oglContext);
        }

        if (m_textureBindings.isNull())
        {
            cvf::RenderStateTextureBindings* textureBindings = new cvf::RenderStateTextureBindings;
            textureBindings->addBinding(m_texture.p(), m_sampler.p(), "u_texture2D");
            m_textureBindings = textureBindings;
        }
    }

    float offset = 0.0f;
    Vec3f min(offset, offset, 0.0f);
    Vec3f max(static_cast<float>(size.x()) + offset, static_cast<float>(size.y()) + offset, 0.0f);

    // Setup the vertex array
    float* v1 = &vertexArray[0]; 
    float* v2 = &vertexArray[3];
    float* v3 = &vertexArray[6];
    float* v4 = &vertexArray[9];
    v1[0] = min.x(); v1[1] = min.y(); v1[2] = 0.0f;
    v2[0] = max.x(); v2[1] = min.y(); v2[2] = 0.0f;
    v3[0] = max.x(); v3[1] = max.y(); v3[2] = 0.0f;
    v4[0] = min.x(); v4[1] = max.y(); v4[2] = 0.0f;

    if (m_blendMode != NO_BLENDING)
    {
        RenderStateBlending blend;
        blend.configureTransparencyBlending();
        blend.applyOpenGL(oglContext);
    }

    m_textureBindings->applyOpenGL(oglContext);

    if (software)
    {
#ifndef CVF_OPENGL_ES
        glColor4f(1.0f, 1.0f, 1.0f, m_blendMode == GLOBAL_ALPHA ? m_alpha : 1.0f);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(textureCoords[0], textureCoords[1]);
        glVertex3fv(v1);
        glTexCoord2f(textureCoords[2], textureCoords[3]);
        glVertex3fv(v2);
        glTexCoord2f(textureCoords[4], textureCoords[5]);
        glVertex3fv(v3);
        glTexCoord2f(textureCoords[6], textureCoords[7]);
        glVertex3fv(v4);
        glEnd();
#endif
    }
    else
    {
        if (m_blendMode == GLOBAL_ALPHA)
        {
            UniformFloat alphaUniform("u_alpha", m_alpha);
            m_shaderProgram->applyUniform(oglContext, alphaUniform);
        }

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    if (m_blendMode != NO_BLENDING)
    {
        RenderStateBlending blend;
        blend.applyOpenGL(oglContext);
    }

    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL(oglContext);

    if (software)
    {
#ifndef CVF_OPENGL_ES
        RenderStateTextureMapping_FF resetTextureMapping;
        resetTextureMapping.applyOpenGL(oglContext);
#endif
    }

    if (!software)
    {
        glDisableVertexAttribArray(ShaderProgram::VERTEX);
        glDisableVertexAttribArray(ShaderProgram::TEX_COORD_2F_0);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayImage::setImage(TextureImage* image)
{
    m_image = image;
    m_pow2Image = NULL;

    m_size.x() = image->width();
    m_size.y() = image->height();

    m_texture = new Texture(image);

    m_textureBindings = NULL;
}


//--------------------------------------------------------------------------------------------------
/// Set the size (in pixels) of the text box
//--------------------------------------------------------------------------------------------------
void OverlayImage::setPixelSize( const Vec2ui& size )
{
    m_size = size;
}


//--------------------------------------------------------------------------------------------------
/// Returns the text shown in the text box
//--------------------------------------------------------------------------------------------------
const TextureImage* OverlayImage::image() const
{
    return m_image.p();
}


//--------------------------------------------------------------------------------------------------
/// Set the transparency of the image. 1.0 = opaque, 0.0 = invisible
//--------------------------------------------------------------------------------------------------
void OverlayImage::setGlobalAlpha(float alphaFactor)
{
    m_alpha = alphaFactor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OverlayImage::setBlending(Blending mode)
{
    m_blendMode = mode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float OverlayImage::globalAlpha() const
{
    return m_alpha;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OverlayImage::Blending OverlayImage::blending() const
{
    return m_blendMode;
}

} // namespace cvf

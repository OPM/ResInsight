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
#include "cvfTexture2D_FF.h"
#include "cvfTextureImage.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOglRc.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::Texture2D_FF
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture2D_FF::Texture2D_FF(TextureImage* image)
:   m_image(image),
    m_wrapMode(REPEAT),
    m_minFilter(NEAREST_MIPMAP_LINEAR),
    m_magFilter(LINEAR)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture2D_FF::~Texture2D_FF()
{
    // Just release our reference
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcTexture.p()));
    m_oglRcTexture = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture2D_FF::setImage(TextureImage* image)
{
    CVF_ASSERT(image);

    // Forget any existing OpenGL resource
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcTexture.p()));
    m_oglRcTexture = NULL;

    m_image = image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextureImage* Texture2D_FF::image()
{
    return m_image.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture2D_FF::setWrapMode(WrapMode wrapMode)
{
    m_wrapMode = wrapMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture2D_FF::setMinFilter(Filter minFilter)
{
    m_minFilter = minFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture2D_FF::setMagFilter(Filter magFilter)
{
    CVF_ASSERT(magFilter == NEAREST || magFilter == LINEAR);
    m_magFilter = magFilter;
}


//--------------------------------------------------------------------------------------------------
/// Note: Default unpack alignment (GL_UNPACK_ALIGNMENT) is 4 and 4 byte values is preferred, thus
///       we should always use RGBA when having byte textures as GPUs are optimized to 32 bit values
//--------------------------------------------------------------------------------------------------
bool Texture2D_FF::setupTexture(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);
    CVF_ASSERT(OglRc::safeOglId(m_oglRcTexture.p()) == 0);

    CVF_CLEAR_OGL_ERROR(oglContext);
    
    m_oglRcTexture = oglContext->resourceManager()->createOglRcTexture(oglContext);
    bind(oglContext);
    CVF_CHECK_OGL(oglContext);

    CVF_ASSERT(m_image.notNull());
    GLsizei width = static_cast<GLsizei>(m_image->width());
    GLsizei height = static_cast<GLsizei>(m_image->height());
    CVF_ASSERT(height > 0 && width > 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image->ptr());
    // Note: gluBuild2DMipmaps will scale the image to the closest power of 2 dimension, which is required by the software renderer
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_image->ptr());

    if (CVF_TEST_AND_REPORT_OPENGL_ERROR(oglContext, "Setup texture"))
    {
        deleteTexture(oglContext);
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture2D_FF::bind(OpenGLContext* /*oglContext*/) const
{
    CVF_ASSERT(OglRc::safeOglId(m_oglRcTexture.p()) != 0);
    glBindTexture(GL_TEXTURE_2D, m_oglRcTexture->oglId());    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Texture2D_FF::isBound(OpenGLContext* /*oglContext*/) const
{
    GLint currentTextureBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTextureBinding);

    if (currentTextureBinding != 0)
    {
        if (static_cast<OglId>(currentTextureBinding) == OglRc::safeOglId(m_oglRcTexture.p()))
        {
            return true;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture2D_FF::setupTextureParams(OpenGLContext* oglContext) const
{
    CVF_ASSERT(oglContext);
    CVF_ASSERT(isBound(oglContext));

    cvfGLint oglWrap = GL_REPEAT;
    switch (m_wrapMode)
    {
        case REPEAT:    oglWrap = GL_REPEAT; break;
        case CLAMP:      oglWrap = GL_CLAMP; break;
    }

    cvfGLint oglMinFilter = filterTypeOpenGL(m_minFilter);
    cvfGLint oglMagFilter = filterTypeOpenGL(m_magFilter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, oglWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, oglWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, oglMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, oglMagFilter);

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture2D_FF::deleteTexture(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    if (m_oglRcTexture.notNull())
    {
        m_oglRcTexture->deleteResource(oglContext);
        CVF_CHECK_OGL(oglContext);

        CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcTexture.p()));
        m_oglRcTexture = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OglId Texture2D_FF::textureOglId() const
{
    return OglRc::safeOglId(m_oglRcTexture.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLint Texture2D_FF::filterTypeOpenGL(Filter filter) const
{
    switch (filter)
    {
        case NEAREST:                   return GL_NEAREST; 
        case LINEAR:                    return GL_LINEAR; 
        case NEAREST_MIPMAP_NEAREST:    return GL_NEAREST_MIPMAP_NEAREST; 
        case NEAREST_MIPMAP_LINEAR:     return GL_NEAREST_MIPMAP_LINEAR; 
        case LINEAR_MIPMAP_NEAREST:     return GL_LINEAR_MIPMAP_NEAREST; 
        case LINEAR_MIPMAP_LINEAR:      return GL_LINEAR_MIPMAP_LINEAR; 
    }

    CVF_FAIL_MSG("Unhandled filter enum");
    return GL_LINEAR;
}

}  // namespace cvf

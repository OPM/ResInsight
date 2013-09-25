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
#include "cvfOpenGL.h"
#include "cvfSampler.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOglRc.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::Texture
/// \ingroup Render
///
/// Encapsulates an OpenGL texture object. Currently only supports 2D textures.
///
/// Stores the OpenGL id (name) of the texture and sets it up with glTexImage2D(). 
/// Can be created either with an Image for traditional texture mapping, or based on an internal format
/// and texture type for using textures as render targets in \link FrameBufferObject Frame buffer objects \endlink
///
/// \warning Requires OpenGL2 support
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture::Texture(TextureType textureType, InternalFormat internalFormat)
:   m_textureType(textureType),
    m_internalFormat(internalFormat),
    m_enableMipmapGeneration(false),
    m_hasMipmaps(false),
    m_width(0),
    m_height(0),
    m_versionTick(1)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture::Texture(TextureImage* image)
:   m_textureType(TEXTURE_2D),
    m_internalFormat(RGBA),
    m_enableMipmapGeneration(false),
    m_hasMipmaps(false),
    m_width(0),
    m_height(0),
    m_versionTick(0)
{
    setFromImage(image);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture::~Texture()
{
    // Just release our reference
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcTexture.p()));
    m_oglRcTexture = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture::setFromImage(TextureImage* image)
{
    CVF_ASSERT(textureType() == TEXTURE_2D || textureType() == TEXTURE_RECTANGLE);
    CVF_ASSERT(m_cubeMapImages.size() == 0);
    CVF_ASSERT(image);

    forgetCurrentOglTexture();

    m_image = image;
    m_width = image->width();
    m_height = image->height();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture::setCubeMapImage(CubeMapFace face, TextureImage* cubeMapImage)
{
    CVF_ASSERT(textureType() == TEXTURE_CUBE_MAP);
    CVF_ASSERT(m_image.isNull());
    CVF_ASSERT(cubeMapImage);

    forgetCurrentOglTexture();
    m_image = NULL;

    if (m_cubeMapImages.size() == 0)
    {
        m_cubeMapImages.resize(6);

        m_width = cubeMapImage->width();
        m_height = cubeMapImage->height();
    }
    else
    {
        CVF_ASSERT(m_width == cubeMapImage->width() && m_height == cubeMapImage->height());
    }

    m_cubeMapImages[face] = cubeMapImage;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextureImage* Texture::image()
{
    return m_image.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TextureImage* Texture::cubeMapImage(CubeMapFace face)
{
    CVF_ASSERT(m_cubeMapImages.size() == 6);

    return m_cubeMapImages[face].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture::setSize(uint width, uint height)
{
    CVF_ASSERT(m_image.isNull());
    CVF_ASSERT(m_cubeMapImages.size() == 0);

    forgetCurrentOglTexture();
    clearImages();

    m_width = width;
    m_height = height;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint Texture::width() const
{
    return m_width;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint Texture::height() const
{
    return m_height;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture::TextureType Texture::textureType() const
{
    return m_textureType;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Texture::InternalFormat Texture::internalFormat() const
{
    return m_internalFormat;
}


//--------------------------------------------------------------------------------------------------
/// Do setup of the texture
/// 
/// \warning Requires at least OpenGL2 capability. Will assert if this condition is not met.
/// \warning Default unpack alignment (GL_UNPACK_ALIGNMENT) is 4 and 4 byte values is preferred, thus
///          we should always use RGBA when having byte textures as GPUs are optimized to 32 bit values
//--------------------------------------------------------------------------------------------------
bool Texture::setupTexture(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_ASSERT(OglRc::safeOglId(m_oglRcTexture.p()) == 0);

    const OpenGLCapabilities* oglCaps = oglContext->capabilities();
    CVF_ASSERT(oglCaps->supportsOpenGL2());

    CVF_CLEAR_OGL_ERROR(oglContext);

    // Is manual generation of mipmaps through glGenerateMipmap() supported?
    bool supportsGenerateMipmapFunc = oglCaps->hasCapability(OpenGLCapabilities::GENERATE_MIPMAP_FUNC);
    
    m_hasMipmaps = false;
    
    m_oglRcTexture = oglContext->resourceManager()->createOglRcTexture(oglContext);
    
    bind(oglContext);

    CVF_ASSERT(m_image.isNull() || (m_image->width() == m_width && m_image->height() == m_height));

    switch (m_textureType)
    {
        case TEXTURE_2D:
        {
            if (m_internalFormat == DEPTH_COMPONENT16 || m_internalFormat == DEPTH_COMPONENT24 || m_internalFormat == DEPTH_COMPONENT32)
            {
                CVF_ASSERT(m_image.isNull());
                CVF_ASSERT(!m_enableMipmapGeneration);
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormatOpenGL(), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            }
            else if (m_internalFormat == DEPTH24_STENCIL8)
            {
#ifndef CVF_OPENGL_ES
                
                CVF_ASSERT(m_image.isNull());
                CVF_ASSERT(!m_enableMipmapGeneration);
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormatOpenGL(), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
#else
                CVF_FAIL_MSG("Not supported on IOS");
#endif
            }
            else
            {
#ifndef CVF_OPENGL_ES
                if (!supportsGenerateMipmapFunc)
                {
                    // Explicit mipmap generation not supported so must configure before specifying texture image
                    if (m_enableMipmapGeneration && m_image.notNull())  
                    {
                        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 
                        m_hasMipmaps = true;
                    }
                    else
                    {
                        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); 
                    }
                }
#endif

                glTexImage2D(GL_TEXTURE_2D, 0, internalFormatOpenGL(), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.notNull() ? m_image->ptr() : 0);

                if (supportsGenerateMipmapFunc && m_enableMipmapGeneration && m_image.notNull())
                {
                    glGenerateMipmap(GL_TEXTURE_2D);
                    m_hasMipmaps = true;
                }
            }

            break;
        }

        case TEXTURE_RECTANGLE:
        {
#ifndef CVF_OPENGL_ES
            CVF_ASSERT(!m_enableMipmapGeneration);

            if (m_internalFormat == DEPTH_COMPONENT16 || m_internalFormat == DEPTH_COMPONENT24 || m_internalFormat == DEPTH_COMPONENT32)
            {
                CVF_ASSERT(m_image.isNull());
                glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormatOpenGL(), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormatOpenGL(), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.notNull() ? m_image->ptr() : 0);
            }
#else
            CVF_FAIL_MSG("Not supported on iOS");
#endif
            break;
        }

        case TEXTURE_CUBE_MAP:
        {
            if (m_cubeMapImages.size() == 0)
            {
                uint i;
                for (i = 0; i < 6; i++)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormatOpenGL(), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                }
            }
            else
            {
#ifndef CVF_OPENGL_ES
                if (!supportsGenerateMipmapFunc)
                {
                    if (m_enableMipmapGeneration)
                    {
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE); 
                        m_hasMipmaps = true;
                    }
                    else
                    {
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_FALSE); 
                    }
                }
#endif

                CVF_ASSERT(m_cubeMapImages.size() == 6);
                uint i;
                for (i = 0; i < 6; i++)
                {
                    ref<TextureImage> img = m_cubeMapImages[i];
                    CVF_ASSERT(img.notNull());

                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormatOpenGL(), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGBA, GL_UNSIGNED_BYTE, img->ptr());
                }

                if (supportsGenerateMipmapFunc && m_enableMipmapGeneration)
                {
                    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
                    m_hasMipmaps = true;
                }
            }

            break;
        }
    }

    if (CVF_TEST_AND_REPORT_OPENGL_ERROR(oglContext, "Setup texture"))
    {
        deleteTexture(oglContext);
        return false;
    }

    m_versionTick++;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture::bind(OpenGLContext* /*oglContext*/) const
{
    CVF_ASSERT(OglRc::safeOglId(m_oglRcTexture.p()) != 0);
    glBindTexture(textureTypeOpenGL(), m_oglRcTexture->oglId());    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Texture::isBound(OpenGLContext* /*oglContext*/) const
{
    GLint currentTextureBinding = 0;

    switch (m_textureType)
    {
        case TEXTURE_2D:        glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTextureBinding); break;
        case TEXTURE_RECTANGLE: 
#ifndef CVF_OPENGL_ES
                                glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &currentTextureBinding); break;
#else       
                                CVF_FAIL_MSG("Not supported on iOS"); break;
#endif
        case TEXTURE_CUBE_MAP:  glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &currentTextureBinding); break;
    }

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
void Texture::setupTextureParamsFromSampler(OpenGLContext* oglContext, const Sampler& sampler) const
{
    CVF_ASSERT(oglContext);
    CVF_ASSERT(isBound(oglContext));

    cvfGLint oglWrapS = GL_CLAMP_TO_EDGE;
    cvfGLint oglWrapT = GL_CLAMP_TO_EDGE;
    cvfGLint oglMinFilter = GL_NEAREST;
    cvfGLint oglMagFilter = GL_NEAREST;

    switch(sampler.wrapModeS())
    {
        case Sampler::CLAMP_TO_EDGE:             oglWrapS = GL_CLAMP_TO_EDGE; break;
        case Sampler::REPEAT:                    oglWrapS = GL_REPEAT; break;
        case Sampler::CLAMP_TO_BORDER:
#ifdef CVF_OPENGL_ES
                                                 CVF_FAIL_MSG("CLAMP_TO_BORDER not supported"); break;
#else            
                                                 oglWrapS = GL_CLAMP_TO_BORDER; break;
#endif
        case Sampler::MIRRORED_REPEAT:           oglWrapS = GL_MIRRORED_REPEAT; break;
    }

    switch(sampler.wrapModeT())
    {
        case Sampler::CLAMP_TO_EDGE:             oglWrapT = GL_CLAMP_TO_EDGE; break;
        case Sampler::REPEAT:                    oglWrapT = GL_REPEAT; break;
#ifdef CVF_OPENGL_ES
        case Sampler::CLAMP_TO_BORDER:           CVF_FAIL_MSG("CLAMP_TO_BORDER not supported"); break;
#else            
        case Sampler::CLAMP_TO_BORDER:           oglWrapT = GL_CLAMP_TO_BORDER; break;
#endif
        case Sampler::MIRRORED_REPEAT:           oglWrapT = GL_MIRRORED_REPEAT; break;
    }

    switch(sampler.minFilter())
    {
        case Sampler::NEAREST:                   oglMinFilter = GL_NEAREST; break;
        case Sampler::LINEAR:                    oglMinFilter = GL_LINEAR; break;
        case Sampler::NEAREST_MIPMAP_NEAREST:    oglMinFilter = GL_NEAREST_MIPMAP_NEAREST; break;
        case Sampler::NEAREST_MIPMAP_LINEAR:     oglMinFilter = GL_NEAREST_MIPMAP_LINEAR; break;
        case Sampler::LINEAR_MIPMAP_NEAREST:     oglMinFilter = GL_LINEAR_MIPMAP_NEAREST; break;
        case Sampler::LINEAR_MIPMAP_LINEAR:      oglMinFilter = GL_LINEAR_MIPMAP_LINEAR; break;
    }

    switch(sampler.magFilter())
    {
        case Sampler::NEAREST:                   oglMagFilter = GL_NEAREST; break;
        case Sampler::LINEAR:                    oglMagFilter = GL_LINEAR; break;
        case Sampler::NEAREST_MIPMAP_NEAREST:    
        case Sampler::NEAREST_MIPMAP_LINEAR:     
        case Sampler::LINEAR_MIPMAP_NEAREST:     
        case Sampler::LINEAR_MIPMAP_LINEAR:      CVF_FAIL_MSG("Illegal mag format"); break;
    }

    cvfGLenum textureType = textureTypeOpenGL();
    glTexParameteri(textureType, GL_TEXTURE_WRAP_S, oglWrapS);
    glTexParameteri(textureType, GL_TEXTURE_WRAP_T, oglWrapT);
    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, oglMinFilter);
    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, oglMagFilter);

    // HACK
#ifndef CVF_OPENGL_ES
    if (m_internalFormat == DEPTH_COMPONENT16 || m_internalFormat == DEPTH_COMPONENT24 || m_internalFormat == DEPTH_COMPONENT32)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
#endif

    // Set these to openGL 
    //  TODO  TEXTURE_WRAP_R, TEXTURE_BORDER_COLOR, TEXTURE_MIN_LOD, TEXTURE_MAX_LOD, TEXTURE_LOD_BIAS, 
    //   TEXTURE_COMPARE_MODE, TEXTURE_COMPARE_FUNC

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture::deleteTexture(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    if (m_oglRcTexture.notNull())
    {
        m_oglRcTexture->deleteResource(oglContext);

        CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcTexture.p()));
        m_oglRcTexture = NULL;
    }

    m_hasMipmaps = false;
    m_versionTick++;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture::forgetCurrentOglTexture()
{
    // Just release our reference
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcTexture.p()));
    m_oglRcTexture = NULL;

    m_hasMipmaps = false;
    m_versionTick++;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OglId Texture::textureOglId() const
{
    return OglRc::safeOglId(m_oglRcTexture.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint Texture::versionTick() const
{
    return m_versionTick;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Texture::supportedOpenGL(OpenGLContext* oglContext)
{
    // We'll require baseline OpenGL2 support even if the actual requirement may be lower
    return oglContext->capabilities()->supportsOpenGL2();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture::clearImages()
{
    m_image = NULL;
    m_cubeMapImages.clear();
    m_versionTick++;
}


//--------------------------------------------------------------------------------------------------
/// Enable or disable mipmap generation for this texture
/// 
/// If enabled, mipmaps will be generated during texture setup.
/// 
/// \warning Mipmap generation requires at least OpenGL version 1.4
//--------------------------------------------------------------------------------------------------
void Texture::enableMipmapGeneration(bool enableMipmapGeneration)
{
    CVF_ASSERT(m_textureType == TEXTURE_2D || m_textureType == TEXTURE_CUBE_MAP);

    if (m_enableMipmapGeneration != enableMipmapGeneration)
    {
        m_enableMipmapGeneration = enableMipmapGeneration;

        // To force re-setup of texture
        forgetCurrentOglTexture();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Texture::isMipmapGenerationEnabled() const
{
    return m_enableMipmapGeneration;
}


//--------------------------------------------------------------------------------------------------
/// Explicitly generate mipmaps
/// 
/// \warning Requires the GENERATE_MIPMAP_FUNC capability. Will assert if this requirement is not met.
//--------------------------------------------------------------------------------------------------
void Texture::generateMipmap(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_ASSERT(oglContext->capabilities()->hasCapability(OpenGLCapabilities::GENERATE_MIPMAP_FUNC));

    bind(oglContext);

    if (m_textureType == TEXTURE_2D)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        m_hasMipmaps = true;
    }
    else if (m_textureType == TEXTURE_CUBE_MAP)
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        m_hasMipmaps = true;
    }
    else
    {
        CVF_FAIL_MSG("Mipmap generation not supported for this texture type");
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Texture::hasMipmap() const
{
    return m_hasMipmaps;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Texture::removeMipmap()
{
    // No way to actually delete mipmaps, just flag them as not present
    m_hasMipmaps = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLint Texture::internalFormatOpenGL() const
{
    switch(m_internalFormat)
    {
        case RGBA:              return GL_RGBA;
        case DEPTH_COMPONENT16: return GL_DEPTH_COMPONENT16;
#ifndef CVF_OPENGL_ES
        case RGBA32F:           return GL_RGBA32F;
        case R32F:              return GL_R32F;
        case DEPTH_COMPONENT24: return GL_DEPTH_COMPONENT24;
        case DEPTH_COMPONENT32: return GL_DEPTH_COMPONENT32;
        case DEPTH24_STENCIL8:  return GL_DEPTH24_STENCIL8;
#endif
    }

    CVF_FAIL_MSG("Illegal texture internal format");
    return GL_RGBA;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum Texture::textureTypeOpenGL() const
{
    switch(m_textureType)
    {
        case TEXTURE_2D:            return GL_TEXTURE_2D;
#ifndef CVF_OPENGL_ES
        case TEXTURE_RECTANGLE:     return GL_TEXTURE_RECTANGLE;
#endif
        case TEXTURE_CUBE_MAP:      return GL_TEXTURE_CUBE_MAP;
    }

    CVF_FAIL_MSG("Illegal texture type");
    return GL_TEXTURE_2D;
}

}  // namespace cvf

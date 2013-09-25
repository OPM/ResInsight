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
#include "cvfFramebufferObject.h"
#include "cvfRenderbufferObject.h"
#include "cvfTexture.h"
#include "cvfOpenGL.h"
#include "cvfString.h"
#include "cvfOglRc.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::FramebufferObject
/// \ingroup Render
///
/// Encapsulates FBOs which are used to redirect OpenGL (and thus shader) output from the default
/// Window Framebuffer to a number of custom buffers (either textures or renderbuffers)
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
FramebufferObject::FramebufferObject()
:   m_depthAttachmentVersionTick(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FramebufferObject::~FramebufferObject()
{
    // Just release our reference
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcBuffer.p()));
    m_oglRcBuffer = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::attachColorRenderbuffer(uint index, RenderbufferObject* renderbuffer)
{
    // Not legal to have both a texture and a render buffer attachment
    CVF_ASSERT(m_colorTextures.size() <= index || m_colorTextures[index].isNull());

    if (index >= m_colorRenderBuffers.size())
    {
        m_colorRenderBuffers.resize(index + 1);
    }

    m_colorRenderBuffers[index] = renderbuffer;
    setColorBufferVersionTick(index, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::attachColorTexture2d(uint index, Texture* texture)
{
    // Not legal to have both a texture and a render buffer attachment
    CVF_ASSERT(m_colorRenderBuffers.size() <= index || m_colorRenderBuffers[index].isNull());

    if (index >= m_colorTextures.size())
    {
        m_colorTextures.resize(index + 1);
    }

    m_colorTextures[index] = texture;

    setColorBufferVersionTick(index, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::attachColorTexture2dCubeMap(uint index, Texture::CubeMapFace face, Texture* texture)
{
    // Not legal to have both a texture and a render buffer attachment
    CVF_ASSERT(m_colorRenderBuffers.size() <= index || m_colorRenderBuffers[index].isNull());

    if (index >= m_colorTextures.size())
    {
        m_colorTextures.resize(index + 1);
    }

    m_colorTextures[index] = texture;
    m_colorCubeMapFaces[index] = face;

    setColorBufferVersionTick(index, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::attachDepthRenderbuffer(RenderbufferObject* renderbuffer)
{
    m_depthRenderBuffer = renderbuffer;
    m_depthTexture2d = NULL;
    m_depthStencilTexture2d = NULL;
    m_depthStencilRenderBuffer = NULL;
    m_depthAttachmentVersionTick = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::attachDepthTexture2d(Texture* texture)
{
    m_depthTexture2d = texture;
    m_depthRenderBuffer = NULL;
    m_depthStencilRenderBuffer = NULL;
    m_depthStencilTexture2d = NULL;
    m_depthAttachmentVersionTick = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::attachDepthStencilRenderbuffer(RenderbufferObject* renderbuffer)
{
    m_depthStencilRenderBuffer = renderbuffer;
    m_depthStencilTexture2d = NULL;
    m_depthTexture2d = NULL;
    m_depthRenderBuffer = NULL;
    m_depthAttachmentVersionTick = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::attachDepthStencilTexture2d(Texture* texture)
{
    m_depthStencilTexture2d = texture;
    m_depthStencilRenderBuffer = NULL;
    m_depthTexture2d = NULL;
    m_depthRenderBuffer = NULL;
    m_depthAttachmentVersionTick = 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::resizeAttachedBuffers(uint width, uint height)
{
    size_t i;
    for (i = 0; i < m_colorRenderBuffers.size(); i++)
    {
        RenderbufferObject* buffer = m_colorRenderBuffers[i].p();
        
        if (buffer)
        {
            buffer->setSize(width, height);
        }
    }

    for (i = 0; i < m_colorTextures.size(); i++)
    {
        Texture* texture = m_colorTextures[i].p();

        if (texture)
        {
            texture->setSize(width, height);
        }
    }

    if (m_depthRenderBuffer.notNull())
    {
        m_depthRenderBuffer->setSize(width, height);
    }

    if (m_depthTexture2d.notNull())
    {
        m_depthTexture2d->setSize(width, height);
    }

    if (m_depthStencilTexture2d.notNull())
    {
        m_depthStencilTexture2d->setSize(width, height);
    }

    if (m_depthStencilRenderBuffer.notNull())
    {
        m_depthStencilRenderBuffer->setSize(width, height);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::applyOpenGL(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_ASSERT(FramebufferObject::supportedOpenGL(oglContext));

    bool createdNewFrameBuffer = false;
    if (OglRc::safeOglId(m_oglRcBuffer.p()) == 0)
    {
        m_oglRcBuffer = oglContext->resourceManager()->createOglRcFramebuffer(oglContext);
        createdNewFrameBuffer = true;
    }

    bind(oglContext);

    bool attachmentsModified = createdNewFrameBuffer;

    uint i;
    for (i = 0; i < m_colorRenderBuffers.size(); i++)
    {
        RenderbufferObject* renderBuffer = m_colorRenderBuffers[i].p();

        if (renderBuffer)
        {
            if (renderBuffer->versionTick() != m_colorAttachmentVersionTicks[i])
            {
                if (renderBuffer->renderbufferOglId() == 0)
                {
                    if (!renderBuffer->create(oglContext))
                    {
                        return;
                    }
                }

                glFramebufferRenderbuffer(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), GL_RENDERBUFFER, renderBuffer->renderbufferOglId());

                setColorBufferVersionTick(i, renderBuffer->versionTick());
                attachmentsModified = true;
            }
        }
    }

    for (i = 0; i < m_colorTextures.size(); i++)
    {
        Texture* texture = m_colorTextures[i].p();

        if (texture)
        {
            if (texture->versionTick() != m_colorAttachmentVersionTicks[i])
            {
                if (texture->textureOglId() == 0)
                {
                    if (!texture->setupTexture(oglContext))
                    {
                        return;
                    }
                }

                // Assume we'll change the texture so flag mipmaps as invalid
                texture->removeMipmap();

                std::map<uint, Texture::CubeMapFace>::iterator faceIt = m_colorCubeMapFaces.find(i);

                if (faceIt != m_colorCubeMapFaces.end())
                {
                    glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIt->second), texture->textureOglId(), 0);
                }
                else
                {
                    if (texture->textureType() == Texture::TEXTURE_RECTANGLE)
                    {
#ifndef CVF_OPENGL_ES
                        glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), GL_TEXTURE_RECTANGLE, texture->textureOglId(), 0);
#else
                        CVF_FAIL_MSG("Not supported on iOS");
#endif
                    }
                    else
                    {
                        glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), GL_TEXTURE_2D, texture->textureOglId(), 0);                       
                    }
                }

                setColorBufferVersionTick(i, texture->versionTick());
                attachmentsModified = true;
            }
        }
    }

    // Setup draw buffers. Drawbuffer settings are stored in the FBO, so we only need to do this whenever the FBO attachments are changed.
#ifndef CVF_OPENGL_ES
    if (attachmentsModified)
    {
        size_t maxAttachmentIndex = CVF_MAX(m_colorRenderBuffers.size(), m_colorTextures.size());
        if (maxAttachmentIndex > 0)
        {
            GLenum* drawBuffers = new GLenum[maxAttachmentIndex];

            size_t i;
            for (i = 0; i < maxAttachmentIndex; i++)
            {
                bool attachmentInUse = false;

                if (m_colorRenderBuffers.size() > i && m_colorRenderBuffers[i].notNull())
                {
                    attachmentInUse = true;
                }

                if (m_colorTextures.size() > i && m_colorTextures[i].notNull())
                {
                    attachmentInUse = true;
                }

                if (attachmentInUse)
                {
                    drawBuffers[i] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i);
                }
                else
                {
                    drawBuffers[i] = GL_NONE;
                }
            }

            glDrawBuffers(static_cast<GLsizei>(maxAttachmentIndex), drawBuffers);

            delete[] drawBuffers;
        }
        else
        {
            glDrawBuffer(GL_NONE);
        }
    }
#endif

    // Depth attachment, can only have one
    CVF_ASSERT(!(m_depthTexture2d.notNull() && m_depthRenderBuffer.notNull()));

    if (m_depthRenderBuffer.notNull())
    {
        if (m_depthRenderBuffer->versionTick() != m_depthAttachmentVersionTick)
        {
            if (m_depthRenderBuffer->renderbufferOglId() == 0)
            {
                if (!m_depthRenderBuffer->create(oglContext))
                {
                    return;
                }
            }

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer->renderbufferOglId());

            m_depthAttachmentVersionTick = m_depthRenderBuffer->versionTick();
        }
    }

    if (m_depthTexture2d.notNull())
    {
        if (m_depthTexture2d->versionTick() != m_depthAttachmentVersionTick)
        {
            if (m_depthTexture2d->textureOglId() == 0)
            {
                if (!m_depthTexture2d->setupTexture(oglContext))
                {
                    return;
                }
            }

            if (m_depthTexture2d->textureType() == Texture::TEXTURE_2D)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture2d->textureOglId(), 0);
            }
            else if (m_depthTexture2d->textureType() == Texture::TEXTURE_RECTANGLE)
            {
#ifndef CVF_OPENGL_ES
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, m_depthTexture2d->textureOglId(), 0);
#else
                CVF_FAIL_MSG("Not supported on iOS");
#endif
            }
            else
            {
                CVF_FAIL_MSG("Not implemented");
            }

            m_depthAttachmentVersionTick = m_depthTexture2d->versionTick();
        }
    }

    if (m_depthStencilRenderBuffer.notNull())
    {
        if (m_depthStencilRenderBuffer->versionTick() != m_depthAttachmentVersionTick)
        {
            if (m_depthStencilRenderBuffer->renderbufferOglId() == 0)
            {
                if (!m_depthStencilRenderBuffer->create(oglContext))
                {
                    return;
                }
            }

#ifndef CVF_OPENGL_ES
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilRenderBuffer->renderbufferOglId());
#else
            CVF_FAIL_MSG("Not supported on iOS");
#endif

            m_depthAttachmentVersionTick = m_depthStencilRenderBuffer->versionTick();
        }
    }

    if (m_depthStencilTexture2d.notNull())
    {
        if (m_depthStencilTexture2d->versionTick() != m_depthAttachmentVersionTick)
        {
            if (m_depthStencilTexture2d->textureOglId() == 0)
            {
                if (!m_depthStencilTexture2d->setupTexture(oglContext))
                {
                    return;
                }
            }

            if (m_depthStencilTexture2d->textureType() == Texture::TEXTURE_2D)
            {
#ifndef CVF_OPENGL_ES                
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthStencilTexture2d->textureOglId(), 0);
#else
                CVF_FAIL_MSG("Not supported on iOS");
#endif
            }
            else if (m_depthStencilTexture2d->textureType() == Texture::TEXTURE_RECTANGLE)
            {
#ifndef CVF_OPENGL_ES
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE, m_depthStencilTexture2d->textureOglId(), 0);
#else
                CVF_FAIL_MSG("Not supported on iOS");
#endif
            }
            else
            {
                CVF_FAIL_MSG("Not implemented");
            }

            m_depthAttachmentVersionTick = m_depthStencilTexture2d->versionTick();
        }
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::bind(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    CVF_ASSERT(OglRc::safeOglId(m_oglRcBuffer.p()) != 0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_oglRcBuffer->oglId());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::useDefaultWindowFramebuffer(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

#ifdef CVF_OPENGL_ES
    CVF_FAIL_MSG("FrameBufferObject::useDefaultWindowFramebuffer() not supported on OpenGL ES");
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
#endif

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::deleteFramebuffer(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    if (m_oglRcBuffer.notNull())
    {
        m_oglRcBuffer->deleteResource(oglContext);
        CVF_CHECK_OGL(oglContext);

        CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcBuffer.p()));
        m_oglRcBuffer = NULL;

        m_colorAttachmentVersionTicks.clear();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::deleteOrReleaseOpenGLResources(OpenGLContext* oglContext)
{
    deleteFramebuffer(oglContext);

    size_t i;
    for (i = 0; i < m_colorRenderBuffers.size(); i++)
    {
        RenderbufferObject* buffer = m_colorRenderBuffers[i].p();

        if (buffer)
        {
            buffer->deleteRenderbuffer(oglContext);
        }
    }

    for (i = 0; i < m_colorTextures.size(); i++)
    {
        Texture* texture = m_colorTextures[i].p();

        if (texture)
        {
            texture->deleteTexture(oglContext);
        }
    }

    if (m_depthRenderBuffer.notNull())
    {
        m_depthRenderBuffer->deleteRenderbuffer(oglContext);
    }

    if (m_depthTexture2d.notNull())
    {
        m_depthTexture2d->deleteTexture(oglContext);
    }

    if (m_depthStencilTexture2d.notNull())
    {
        m_depthStencilTexture2d->deleteTexture(oglContext);
    }

    if (m_depthStencilRenderBuffer.notNull())
    {
        m_depthStencilRenderBuffer->deleteRenderbuffer(oglContext);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool FramebufferObject::supportedOpenGL(OpenGLContext* oglContext)
{
    return oglContext->capabilities()->hasCapability(OpenGLCapabilities::FRAMEBUFFER_OBJECT);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool FramebufferObject::isFramebufferComplete(OpenGLContext* oglContext, String* failReason) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    bind(oglContext);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          *failReason = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT is returned if any of the framebuffer attachment points are framebuffer incomplete."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  *failReason = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT is returned if the framebuffer does not have at least one image attached to it."; break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                    *failReason = "GL_FRAMEBUFFER_UNSUPPORTED is returned if the combination of internal formats of the attached images violates an implementation-dependent set of restrictions."; break;

#ifdef CVF_OPENGL_ES
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:          *failReason = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS. Not all attached images has same width and height."; break;                
#else
            case GL_FRAMEBUFFER_UNDEFINED:                      *failReason = "GL_FRAMEBUFFER_UNDEFINED is returned if target is the default framebuffer, but the default framebuffer does not exist."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         *failReason = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAWBUFFERi."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         *failReason = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER is returned if GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         *failReason = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is returned if the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES."; break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:       *failReason = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS is returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target."; break;
#endif
        }
    }

    return status == GL_FRAMEBUFFER_COMPLETE;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void FramebufferObject::setColorBufferVersionTick(uint index, uint versionTick)
{
    if (index >= m_colorAttachmentVersionTicks.size())
    {
        m_colorAttachmentVersionTicks.resize(index + 1, 0);
    }
    
    m_colorAttachmentVersionTicks[index] = versionTick;
}

} // namespace cvf

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
#include "cvfTexture.h"

#include <map>

namespace cvf {

class OpenGLContext;
class RenderbufferObject;
class String;
class OglRcFramebuffer;


//==================================================================================================
//
// Wrapper for OpenGL Frame Buffer Object
//
//==================================================================================================
class FramebufferObject : public Object
{
public:
    FramebufferObject();
    ~FramebufferObject();

    void        resizeAttachedBuffers(uint width, uint height);

    void        attachColorRenderbuffer(uint index, RenderbufferObject* renderbuffer);
    void        attachColorTexture2d(uint index, Texture* texture);
    void        attachColorTexture2dCubeMap(uint index, Texture::CubeMapFace face, Texture* texture);

    void        attachDepthRenderbuffer(RenderbufferObject* renderbuffer);
    void        attachDepthTexture2d(Texture* texture);

    void        attachDepthStencilRenderbuffer(RenderbufferObject* renderbuffer);
    void        attachDepthStencilTexture2d(Texture* texture);

    void        applyOpenGL(OpenGLContext* oglContext);
    void        bind(OpenGLContext* oglContext) const;
    static void useDefaultWindowFramebuffer(OpenGLContext* oglContext);
    void        deleteFramebuffer(OpenGLContext* oglContext);

    bool        isFramebufferComplete(OpenGLContext* oglContext, String* failReason) const;

    void        deleteOrReleaseOpenGLResources(OpenGLContext* oglContext);

    static bool supportedOpenGL(OpenGLContext* oglContext);

private:
    void        setColorBufferVersionTick(uint index, uint versionTick);

private:
    Collection<RenderbufferObject>          m_colorRenderBuffers;
    Collection<Texture>                     m_colorTextures;
    std::map<uint, Texture::CubeMapFace>    m_colorCubeMapFaces;

    ref<RenderbufferObject>                 m_depthRenderBuffer;
    ref<Texture>                            m_depthTexture2d;

    ref<RenderbufferObject>                 m_depthStencilRenderBuffer;
    ref<Texture>                            m_depthStencilTexture2d;

    std::vector<uint>                       m_colorAttachmentVersionTicks;
    uint                                    m_depthAttachmentVersionTick;

    ref<OglRcFramebuffer>                   m_oglRcBuffer;	
};

}

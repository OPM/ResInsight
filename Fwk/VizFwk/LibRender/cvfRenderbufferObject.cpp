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
#include "cvfRenderbufferObject.h"
#include "cvfOpenGL.h"
#include "cvfOglRc.h"
#include "cvfOpenGLResourceManager.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::RenderbufferObject
/// \ingroup Render
///
/// Encapsulates an OpenGL renderbuffer object
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderbufferObject::RenderbufferObject(InternalFormat format, uint width, uint height)
:   Object(),
    m_interalFormat(format),
    m_width(width),
    m_height(height),
    m_versionTick(1)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderbufferObject::~RenderbufferObject()
{
    // Just release our reference
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcBuffer.p()));
    m_oglRcBuffer = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OglId RenderbufferObject::renderbufferOglId() const
{
    return OglRc::safeOglId(m_oglRcBuffer.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RenderbufferObject::versionTick() const
{
    return m_versionTick;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderbufferObject::setSize(uint width, uint height)
{
    // Forget existing buffer
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcBuffer.p()));
    m_oglRcBuffer = NULL;

    m_width = width;
    m_height = height;
    m_versionTick++;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderbufferObject::create(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_CLEAR_OGL_ERROR(oglContext);

    m_oglRcBuffer = oglContext->resourceManager()->createOglRcRenderbuffer(oglContext);
    OglId myOglId = OglRc::safeOglId(m_oglRcBuffer.p());
    
    glBindRenderbuffer(GL_RENDERBUFFER, myOglId);
    glRenderbufferStorage(GL_RENDERBUFFER, intenalFormatOpenGL(), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));

    if (CVF_TEST_AND_REPORT_OPENGL_ERROR(oglContext, "Allocate render buffer storage"))
    {
        deleteRenderbuffer(oglContext);
        return false;
    }

    m_versionTick++;
    
    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderbufferObject::deleteRenderbuffer(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    if (m_oglRcBuffer.notNull())
    {
        m_oglRcBuffer->deleteResource(oglContext);
        CVF_CHECK_OGL(oglContext);

        CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcBuffer.p()));
        m_oglRcBuffer = NULL;
    }

    m_versionTick++;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum RenderbufferObject::intenalFormatOpenGL() const
{
    switch(m_interalFormat)
    {
        case RGBA:              return GL_RGBA;
        case DEPTH_COMPONENT16: return GL_DEPTH_COMPONENT16;
#ifndef CVF_OPENGL_ES
        case DEPTH_COMPONENT24: return GL_DEPTH_COMPONENT24;
        case DEPTH_COMPONENT32: return GL_DEPTH_COMPONENT32;
        case DEPTH24_STENCIL8:  return GL_DEPTH24_STENCIL8;
#endif
    }

    CVF_FAIL_MSG("Illegal renderbuffer format");
    return GL_RGBA;
}

} // namespace cvf

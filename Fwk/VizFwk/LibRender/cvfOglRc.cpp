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
#include "cvfOglRc.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::OglRc
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OglRc::OglRc()
:   m_openGLObjId(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OglRc::~OglRc()
{
    // The actual OpenGL object should be gone by now!
    CVF_ASSERT(m_openGLObjId == 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OglId OglRc::oglId() const
{
    return m_openGLObjId;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OglId OglRc::safeOglId(const OglRc* oglRcObj)
{
    if (oglRcObj)
    {
        return oglRcObj->oglId();
    }
    else
    {
        return 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// Is it safe to release and potentially trigger the destructor
/// 
/// The requirement is that the object either
///   * doesn't contain any OpenGL resource
///   * that someone else is holding a reference to the object (assuming they will do proper cleanup)
//--------------------------------------------------------------------------------------------------
bool OglRc::isSafeToRelease(const OglRc* oglRcObj)
{
    if (oglRcObj)
    {
        if (oglRcObj->refCount() == 1)
        {
            if (oglRcObj->oglId() != 0)
            {
                return false;
            }
        }
    }

    return true;
}



//==================================================================================================
///
/// \class cvf::OglRcShader
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcShader> OglRcShader::create(OpenGLContext* oglContext, cvfGLenum shaderType)
{
    CVF_CALLSITE_OPENGL(oglContext);

    ref<OglRcShader> rcObj = new OglRcShader;
    rcObj->m_openGLObjId = glCreateShader(shaderType);
    if (rcObj->m_openGLObjId != 0)
    {
        return rcObj;
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OglRcShader::deleteResource(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (m_openGLObjId != 0)
    {
        glDeleteShader(m_openGLObjId);
        m_openGLObjId = 0;
    }
}



//==================================================================================================
///
/// \class cvf::OglRcProgram
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcProgram> OglRcProgram::create(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    ref<OglRcProgram> rcObj = new OglRcProgram;
    rcObj->m_openGLObjId = glCreateProgram();
    if (rcObj->m_openGLObjId != 0)
    {
        return rcObj;
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OglRcProgram::deleteResource(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (m_openGLObjId != 0)
    {
        glDeleteProgram(m_openGLObjId);
        m_openGLObjId = 0;
    }
}



//==================================================================================================
///
/// \class cvf::OglRcTexture
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcTexture> OglRcTexture::create(OpenGLContext* /*oglContext*/)
{
    ref<OglRcTexture> rcObj = new OglRcTexture;
    glGenTextures(1, &rcObj->m_openGLObjId);
    if (rcObj->m_openGLObjId != 0)
    {
        return rcObj;
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OglRcTexture::deleteResource(OpenGLContext* /*oglContext*/)
{
    if (m_openGLObjId != 0)
    {
        glDeleteTextures(1, &m_openGLObjId);
        m_openGLObjId = 0;
    }
}



//==================================================================================================
///
/// \class cvf::OglRcRenderbuffer
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcRenderbuffer> OglRcRenderbuffer::create(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    ref<OglRcRenderbuffer> rcObj = new OglRcRenderbuffer;
    glGenRenderbuffers(1, &rcObj->m_openGLObjId);
    if (rcObj->m_openGLObjId != 0)
    {
        return rcObj;
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OglRcRenderbuffer::deleteResource(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (m_openGLObjId != 0)
    {
        glDeleteRenderbuffers(1, &m_openGLObjId);
        m_openGLObjId = 0;
    }
}



//==================================================================================================
///
/// \class cvf::OglRcFramebuffer
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<OglRcFramebuffer> OglRcFramebuffer::create(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    ref<OglRcFramebuffer> rcObj = new OglRcFramebuffer;
    glGenFramebuffers(1, &rcObj->m_openGLObjId);
    if (rcObj->m_openGLObjId != 0)
    {
        return rcObj;
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void OglRcFramebuffer::deleteResource(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (m_openGLObjId != 0)
    {
        glDeleteFramebuffers(1, &m_openGLObjId);
        m_openGLObjId = 0;
    }
}



} // namespace cvf


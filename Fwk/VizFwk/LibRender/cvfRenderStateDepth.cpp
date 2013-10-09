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
#include "cvfRenderStateDepth.h"
#include "cvfAssert.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateDepth
/// \ingroup Render
///
/// Encapsulate OpenGL glEnable(GL_DEPTH_TEST), glDepthFunc() and glDepthMask() functions.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glDepthFunc.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glDepthMask.xml
/// 
/// \todo
/// Add support for glDepthRange() if needed.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateDepth::RenderStateDepth(bool depthTest, Function func, bool depthWrite)
:   RenderState(DEPTH)
{
    m_enableDepthTest = depthTest;
    m_depthFunc = func;
    m_enableDepthWrite = depthWrite;
}


//--------------------------------------------------------------------------------------------------
/// Specifies the depth comparison function.
/// 
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glDepthFunc.xml
//--------------------------------------------------------------------------------------------------
void RenderStateDepth::setFunction(Function func)
{
    m_depthFunc = func;
}


//--------------------------------------------------------------------------------------------------
/// Enable or disable depth testing and updating of the depth buffer.
/// 
/// \param enableTest  Specify true to enable testing against and updating of depth buffer.
/// 
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml with GL_DEPTH_TEST
/// 
/// From OpenGL docs: 
/// If enabled, do depth comparisons and update the depth buffer. Note that even if the depth buffer 
/// exists and the depth mask is non-zero, the depth buffer is not updated if the depth test is disabled
//--------------------------------------------------------------------------------------------------
void RenderStateDepth::enableDepthTest(bool enableTest)
{
    m_enableDepthTest = enableTest;
}


//--------------------------------------------------------------------------------------------------
/// Enable or disable writing into the depth buffer
/// 
/// \param enableWrite  Specify true to enable writing to depth buffer, false to disable. The default is true.
/// 
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glDepthMask.xml
//--------------------------------------------------------------------------------------------------
void RenderStateDepth::enableDepthWrite(bool enableWrite)
{
    m_enableDepthWrite = enableWrite;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateDepth::Function RenderStateDepth::function() const
{
    return m_depthFunc;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateDepth::isDepthTestEnabled() const
{
    return m_enableDepthTest;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateDepth::isDepthWriteEnabled() const
{
    return m_enableDepthWrite;
}


//--------------------------------------------------------------------------------------------------
/// Specify the depth setting to OpenGL.
//--------------------------------------------------------------------------------------------------
void RenderStateDepth::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enableDepthTest)
    {
        GLenum depthFuncOGL = depthFuncOpenGL();
        glDepthFunc(depthFuncOGL);

        GLboolean enableDepthWrite = m_enableDepthWrite ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);
        glDepthMask(enableDepthWrite);

        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    
    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum RenderStateDepth::depthFuncOpenGL() const
{
    switch (m_depthFunc)
    {
        case NEVER:     return GL_NEVER;
        case LESS:      return GL_LESS;
        case EQUAL:     return GL_EQUAL;
        case LEQUAL:    return GL_LEQUAL;
        case GREATER:   return GL_GREATER;
        case NOTEQUAL:  return GL_NOTEQUAL;
        case GEQUAL:    return GL_GEQUAL;
        case ALWAYS:    return GL_ALWAYS;
    }

    CVF_FAIL_MSG("Unhandled depth func");
    return 0;
}




} // namespace cvf


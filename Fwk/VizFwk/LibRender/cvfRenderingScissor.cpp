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
#include "cvfRenderingScissor.h"
#include "cvfOpenGL.h"

namespace cvf {

//==================================================================================================
///
/// \class cvf::RenderingScissor
/// \ingroup Render
///
/// An OpenGL Scissor. 
/// 
/// Stores scissoring settings that are applied to the OpenGl state when calling applyOpenGL()
/// unApplyOpenGL will restore the scissoring settings to what they where before the call to applyOpenGl()
/// 
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderingScissor::RenderingScissor()
: m_x(0),
  m_y(0),
  m_width(0),
  m_height(0),
  m_scissorEnabledStateToRestore(false)
{
    m_scissorBoxToRestore[0] = 0;
    m_scissorBoxToRestore[1] = 0;
    m_scissorBoxToRestore[2] = -1;
    m_scissorBoxToRestore[3] = -1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderingScissor::setScissorRectangle(int x, int y, uint width, uint height)
{
    m_x = x; 
    m_y = y; 
    m_width = width; 
    m_height = height;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RenderingScissor::x() const
{
    return m_x;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RenderingScissor::y() const
{
    return m_y;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RenderingScissor::width() const
{
    return m_width;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RenderingScissor::height() const
{
    return m_height;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderingScissor::applyOpenGL(OpenGLContext* oglContext, Viewport::ClearMode clearMode, const Color4f& clearColor)
{
    CVF_CHECK_OGL(oglContext);

    m_scissorEnabledStateToRestore = glIsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_SCISSOR_BOX, m_scissorBoxToRestore);
    
    glScissor(static_cast<GLsizei>(m_x),
              static_cast<GLsizei>(m_y),
              static_cast<GLsizei>(m_width),
              static_cast<GLsizei>(m_height));
    glEnable(GL_SCISSOR_TEST);
    
    GLbitfield clearFlags = Viewport::clearFlagsOpenGL(clearMode);

    if (clearFlags != 0)
    {
        if ( clearFlags & GL_COLOR_BUFFER_BIT )
        {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClearColor(clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a());
        }

        if ( clearFlags & GL_DEPTH_BUFFER_BIT )
        {
            glDepthMask(GL_TRUE);
            #ifndef CVF_OPENGL_ES
            glClearDepth(1.0f);
            #endif  // CVF_OPENGL_ES
        }

        if ( clearFlags & GL_STENCIL_BUFFER_BIT )
        {
            glStencilMask(0xffffffff);
            glClearStencil(0);
        }

        glClear(clearFlags);
    }

    CVF_CHECK_OGL(oglContext);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderingScissor::unApplyOpenGL(OpenGLContext* oglContext)
{
    if ( m_scissorEnabledStateToRestore )
    {
        glEnable(GL_SCISSOR_TEST);
    }
    else
    {
        glDisable(GL_SCISSOR_TEST);
    }

    glScissor(m_scissorBoxToRestore[0], 
              m_scissorBoxToRestore[1], 
              m_scissorBoxToRestore[2], 
              m_scissorBoxToRestore[3]);
}

}
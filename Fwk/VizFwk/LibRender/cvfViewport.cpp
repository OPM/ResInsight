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
#include "cvfViewport.h"
#include "cvfOpenGL.h"
#include "cvfMath.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Viewport
/// \ingroup Render
///
/// An OpenGL viewport. 
/// 
/// Stores the viewport dimensions and the clear options (flags, color, depth, etc.). 
/// Use the activate() method to apply the settings to OpenGL. This will setup the viewport and 
/// clear it according to the clear flags.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Viewport::Viewport()
    :   m_x(0),
    m_y(0),
    m_width(0),
    m_height(0),
    m_clearColor(0.69f, 0.77f, 0.87f, 1.0f),
    m_clearDepth(1.0f),
    m_clearStencil(0)
{
}


//--------------------------------------------------------------------------------------------------
/// Specify the position and dimensions of the viewport
/// 
/// The window coordinates (x,y) are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
void Viewport::set(int x, int y, uint width, uint height) 
{ 
    m_x = x; 
    m_y = y; 
    m_width = width; 
    m_height = height; 
}


//--------------------------------------------------------------------------------------------------
/// Specify the clear color to use for the color buffer
//--------------------------------------------------------------------------------------------------
void Viewport::setClearColor(Color4f color)
{
    // Note: Clear of non-normalized buffers (e.g. RGBA32F) attached to an FBO allows clearing 
    // other values than [0-1]. The GL spec is a bit vague on this, but it seems to work fine.
    // As an alternative glClearBuffer() should maybe be used. Will be considered when refactoring
    // Viewport (moving clear color etc).
    // 
    // CVF_ASSERT(color.isValid());
    m_clearColor = color;
}


//--------------------------------------------------------------------------------------------------
/// Get the x coordinate of the lower left corner of the viewport
//--------------------------------------------------------------------------------------------------
int Viewport::x() const
{
    return m_x;
}


//--------------------------------------------------------------------------------------------------
/// Get the y coordinate of the lower left corner of the viewport
//--------------------------------------------------------------------------------------------------
int Viewport::y() const
{
    return m_y;
}


//--------------------------------------------------------------------------------------------------
/// Get the width of the viewport
//--------------------------------------------------------------------------------------------------
uint Viewport::width() const
{
    return m_width;
}


//--------------------------------------------------------------------------------------------------
/// Get the height of the viewport
//--------------------------------------------------------------------------------------------------
uint Viewport::height() const
{
    return m_height;
}


//--------------------------------------------------------------------------------------------------
/// Get the aspect ratio (width / height)
//--------------------------------------------------------------------------------------------------
double Viewport::aspectRatio() const 
{
    if (height() <= 0)
    {
        return 1.0;
    }

    return static_cast<double>(width())/static_cast<double>(height());
}


//--------------------------------------------------------------------------------------------------
/// Returns the current clear color used for the color buffer
//--------------------------------------------------------------------------------------------------
Color4f Viewport::clearColor() const
{
    return m_clearColor;
}


//--------------------------------------------------------------------------------------------------
/// Convert ClearMode to corresponding OpenGL GLbitfield
//--------------------------------------------------------------------------------------------------
cvfGLbitfield Viewport::clearFlagsOpenGL(ClearMode clearMode)
{
    switch (clearMode)
    {
        case DO_NOT_CLEAR:              return 0;
        case CLEAR_COLOR:               return GL_COLOR_BUFFER_BIT;
        case CLEAR_DEPTH:               return GL_DEPTH_BUFFER_BIT;
        case CLEAR_STENCIL:             return GL_STENCIL_BUFFER_BIT;
        case CLEAR_COLOR_DEPTH:         return (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        case CLEAR_COLOR_STENCIL:       return (GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        case CLEAR_DEPTH_STENCIL:       return (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        case CLEAR_COLOR_DEPTH_STENCIL: return (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        default:                        CVF_FAIL_MSG("Unhandled clear mode");
    }

    return 0;
}


//--------------------------------------------------------------------------------------------------
/// Specify the viewport setting to OpenGL and clear the view port according to the clear flags
//--------------------------------------------------------------------------------------------------
void Viewport::applyOpenGL(OpenGLContext* oglContext, ClearMode clearMode)
{
    CVF_CHECK_OGL(oglContext);

    glViewport(static_cast<GLsizei>(m_x), static_cast<GLsizei>(m_y), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
    CVF_CHECK_OGL(oglContext);

    GLbitfield clearFlags = clearFlagsOpenGL(clearMode);

    if (clearFlags != 0)
    {
        if (clearFlags & GL_COLOR_BUFFER_BIT)
        {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClearColor(m_clearColor.r(), m_clearColor.g(), m_clearColor.b(), m_clearColor.a());
        }

        if (clearFlags & GL_DEPTH_BUFFER_BIT)
        {
            glDepthMask(GL_TRUE);
            #ifndef CVF_OPENGL_ES
            glClearDepth(m_clearDepth);
            #endif  // CVF_OPENGL_ES
        }

        if (clearFlags & GL_STENCIL_BUFFER_BIT)
        {
            glStencilMask(0xffffffff);
            glClearStencil(m_clearStencil);
        }

        // Must setup a scissor since the clear calls disregard the viewport settings
        GLboolean scissorWasOn = glIsEnabled(GL_SCISSOR_TEST);
        int scissorBox[4] = {0, 0, -1, -1};
        glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
        glScissor(static_cast<GLsizei>(m_x), static_cast<GLsizei>(m_y), static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
        glEnable(GL_SCISSOR_TEST);

        // Do the actual clear
        glClear(clearFlags);

        //         {
        //             // Code to draw a full screen quad into color buffer using FF
        //             // Experimented with this code when seeing problems with rendering to texture on ATI Catalys 11.3 where
        //             // it seemed that lazy clearing of color buffer was causing problems with depth peeling and glGenerateMipmap()
        //             glMatrixMode(GL_PROJECTION);
        //             glLoadIdentity();
        //             glMatrixMode(GL_MODELVIEW);
        //             glLoadIdentity();
        // 
        //             glDisable(GL_DEPTH_TEST);
        //             glDepthMask(GL_FALSE);
        //             glDisable(GL_LIGHTING);
        //             glColor3f(0, 1, 0);
        // 
        //             glBegin(GL_QUADS);
        //             {
        //                 glVertex2f(-1.0, -1.0); 
        //                 glVertex2f( 1.0, -1.0);
        //                 glVertex2f( 1.0, 1.0);
        //                 glVertex2f(-1.0, 1.0);
        //             }
        //             glEnd();
        // 
        //             glEnable(GL_DEPTH_TEST);
        //             glDepthMask(GL_TRUE);
        //         }

        // Restore scissor settings
        if (!scissorWasOn) glDisable(GL_SCISSOR_TEST);
        glScissor(scissorBox[0], scissorBox[1], scissorBox[2], scissorBox[3]); 

        CVF_CHECK_OGL(oglContext);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String Viewport::debugString() const
{
    String str = "Viewport: ";

    str += "\n Pos:            x=" + String(m_x) + ", y=" + String(m_y);
    str += "\n Size:           x=" + String(m_width) + ", y=" + String(m_height);

    str += "\n ClearColor:     " + String(m_clearColor.r()) + ", " + String(m_clearColor.g()) + ", " + String(m_clearColor.b()) + ", " + String(m_clearColor.a());
    str += "\n m_clearDepth:   " + String(m_clearDepth);
    str += "\n m_clearStencil: " + String(m_clearStencil);

    return str;
}

} // namespace cvf


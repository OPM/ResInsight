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
#include "cvfRenderStateColorMask.h"
#include "cvfAssert.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateColorMask
/// \ingroup Render
///
/// Encapsulate OpenGL glColorMask() function.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glColorMask.xml
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateColorMask::RenderStateColorMask(bool writeAllComponents)
:   RenderState(COLOR_MASK),
    m_writeRed(writeAllComponents),
    m_writeGreen(writeAllComponents),
    m_writeBlue(writeAllComponents),
    m_writeAlpha(writeAllComponents)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateColorMask::enable(bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha)
{
    m_writeRed = writeRed;
    m_writeGreen = writeGreen;
    m_writeBlue = writeBlue;
    m_writeAlpha = writeAlpha;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateColorMask::enableWriteAllComponents(bool writeAllComponents)
{
    m_writeRed = writeAllComponents;
    m_writeGreen = writeAllComponents;
    m_writeBlue = writeAllComponents;
    m_writeAlpha = writeAllComponents;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateColorMask::isRedEnabled() const
{
    return m_writeRed;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateColorMask::isGreenEnabled() const
{
    return m_writeGreen;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateColorMask::isBlueEnabled() const
{
    return m_writeBlue;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateColorMask::isAlphaEnabled() const
{
    return m_writeAlpha;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateColorMask::applyOpenGL(OpenGLContext* oglContext) const
{
    GLboolean writeRed   = m_writeRed   ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);
    GLboolean writeGreen = m_writeGreen ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);
    GLboolean writeBlue  = m_writeBlue  ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);
    GLboolean writeAlpha = m_writeAlpha ? static_cast<GLboolean>(GL_TRUE) : static_cast<GLboolean>(GL_FALSE);

    glColorMask(writeRed, writeGreen, writeBlue, writeAlpha);

    CVF_CHECK_OGL(oglContext);
}



} // namespace cvf


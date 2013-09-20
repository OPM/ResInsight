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
#include "cvfRenderStatePolygonMode.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStatePolygonMode
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStatePolygonMode::RenderStatePolygonMode(Mode frontAndBackFaceMode)
:   RenderState(POLYGON_MODE),
    m_frontFaceMode(frontAndBackFaceMode),
    m_backFaceMode(frontAndBackFaceMode)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonMode::set(Mode frontAndBackMode)
{
    m_frontFaceMode = frontAndBackMode;
    m_backFaceMode = frontAndBackMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonMode::setFrontFace(Mode mode)
{
    m_frontFaceMode = mode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonMode::setBackFace(Mode mode)
{
    m_backFaceMode = mode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStatePolygonMode::Mode RenderStatePolygonMode::frontFace() const
{
    return m_frontFaceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStatePolygonMode::Mode RenderStatePolygonMode::backFace() const
{
    return m_backFaceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonMode::applyOpenGL(OpenGLContext* oglContext) const
{
#ifndef CVF_OPENGL_ES
    if (m_frontFaceMode == m_backFaceMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, polygonModeOpenGL(m_frontFaceMode)); 
    }
    else
    {
        glPolygonMode(GL_FRONT, polygonModeOpenGL(m_frontFaceMode)); 
        glPolygonMode(GL_BACK, polygonModeOpenGL(m_backFaceMode)); 
    }
#endif

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum RenderStatePolygonMode::polygonModeOpenGL(Mode mode)
{
    switch (mode)
    {
#ifndef CVF_OPENGL_ES
        case FILL:  return GL_FILL;
        case LINE:  return GL_LINE;
        case POINT: return GL_POINT;
        default:    CVF_FAIL_MSG("Unhandled polygon mode");
#endif
    }

    return 0;
}


} // namespace cvf


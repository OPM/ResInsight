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
#include "cvfRenderStatePoint.h"
#include "cvfAssert.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {




//==================================================================================================
///
/// \class cvf::RenderStatePoint
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStatePoint::RenderStatePoint(Mode sizeMode)
:   RenderState(POINT),
    m_sizeMode(sizeMode),
    m_pointSprite(false),
    m_pointSize(1.0f)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePoint::setMode(Mode sizeMode)
{
    m_sizeMode = sizeMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStatePoint::Mode RenderStatePoint::mode() const
{
    return m_sizeMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePoint::enablePointSprite(bool enable)
{
    m_pointSprite = enable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStatePoint::isPointSpriteEnabled() const
{
    return m_pointSprite;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePoint::setSize(float pointSize)
{
    m_pointSize = pointSize;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RenderStatePoint::size() const
{
    return m_pointSize;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePoint::applyOpenGL(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    // OpenGL ES does not support fixed point size
    // Point size is always specified using GLSL's gl_PointSize
#ifndef CVF_OPENGL_ES
    bool openGL2Support = oglContext->capabilities()->supportsOpenGL2();

    if (m_sizeMode == FIXED_SIZE)
    {
        if (openGL2Support)
        {
            glDisable(GL_PROGRAM_POINT_SIZE);
        }
    
        glPointSize(m_pointSize);
    }
    else
    {
        if (openGL2Support)
        {
            glEnable(GL_PROGRAM_POINT_SIZE);
        }
        else
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support program point size.");
        }
    }

    if (openGL2Support)
    {
        if (m_pointSprite)
        {
            glEnable(GL_POINT_SPRITE);
            glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
        }
        else               
        {
            glDisable(GL_POINT_SPRITE);
        }
    }
    else
    {
        if (m_pointSprite)
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support point sprites.");
        }
    }
#endif

    CVF_CHECK_OGL(oglContext);
}


} // namespace cvf


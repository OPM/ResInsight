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
#include "cvfAssert.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStatePolygonOffset
/// \ingroup Render
///
/// Encapsulate OpenGL glPolygonOffset() and glEnable()/glDisable() with GL_POLYGON_OFFSET_FILL/LINE/POINT
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glPolygonOffset.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
///
//==================================================================================================
RenderStatePolygonOffset::RenderStatePolygonOffset()
:   RenderState(POLYGON_OFFSET),
    m_factor(0.0f),
    m_units(0.0f),
    m_enableFillMode(false),
    m_enableLineMode(false),
    m_enablePointMode(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonOffset::enableFillMode(bool enableFill)
{
    m_enableFillMode = enableFill;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonOffset::enableLineMode(bool enableLine)
{
    m_enableLineMode = enableLine;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonOffset::enablePointMode(bool enablePoint)
{
    m_enablePointMode = enablePoint;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStatePolygonOffset::isFillModeEnabled() const
{
    return m_enableFillMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStatePolygonOffset::isLineModeEnabled() const
{
    return m_enableLineMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStatePolygonOffset::isPointModeEnabled() const
{
    return m_enablePointMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonOffset::setFactor(float factor)
{
    m_factor = factor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonOffset::setUnits(float units)
{
    m_units = units;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RenderStatePolygonOffset::factor() const
{
    return m_factor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RenderStatePolygonOffset::units() const
{
    return m_units;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonOffset::configurePolygonPositiveOffset()
{
    m_enableFillMode = true;
    m_enableLineMode = false;
    m_enablePointMode = false;
    m_factor = 1.0;
    m_units = 1.0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonOffset::configureLineNegativeOffset()
{
    m_enableFillMode = false;
    m_enableLineMode = true;
    m_enablePointMode = false;
    m_factor = -1.0;
    m_units = -1.0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStatePolygonOffset::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enableFillMode ||
        m_enableLineMode ||
        m_enablePointMode)
    {
        glPolygonOffset(m_factor, m_units);
    }

    if (m_enableFillMode)   glEnable(GL_POLYGON_OFFSET_FILL);
    else                    glDisable(GL_POLYGON_OFFSET_FILL);
        
#ifndef CVF_OPENGL_ES
    if (m_enableLineMode)   glEnable(GL_POLYGON_OFFSET_LINE);
    else                    glDisable(GL_POLYGON_OFFSET_LINE);

    if (m_enablePointMode)  glEnable(GL_POLYGON_OFFSET_POINT);
    else                    glDisable(GL_POLYGON_OFFSET_POINT);
#endif
    
    CVF_CHECK_OGL(oglContext);
}



} // namespace cvf


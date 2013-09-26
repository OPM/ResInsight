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
#include "cvfRenderStateCullFace.h"
#include "cvfAssert.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateCullFace
/// \ingroup Render
///
/// Encapsulate OpenGL glCullFace() and glEnable(GL_CULL_FACE) functions.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glCullFace.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateCullFace::RenderStateCullFace(bool enableCulling, Mode faceMode)
:   RenderState(CULL_FACE),
    m_enableCulling(enableCulling),
    m_faceMode(faceMode)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateCullFace::enable(bool enableCulling)
{
    m_enableCulling = enableCulling;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderStateCullFace::isEnabled() const
{
    return m_enableCulling;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateCullFace::setMode(Mode faceMode)
{
    m_faceMode = faceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateCullFace::Mode RenderStateCullFace::mode() const
{
    return m_faceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateCullFace::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enableCulling)
    {
        if (m_faceMode == BACK)    
        {
            glCullFace(GL_BACK);
        }
        else if (m_faceMode == FRONT)
        {
            glCullFace(GL_FRONT);
        }
        else
        {
            glCullFace(GL_FRONT_AND_BACK);
        }

        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    CVF_CHECK_OGL(oglContext);
}



} // namespace cvf


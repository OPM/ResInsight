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
#include "cvfRenderStateFrontFace.h"
#include "cvfAssert.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateFrontFace
/// \ingroup Render
///
/// Encapsulate OpenGL glFrontFace() used to specify polygon winding. Used together with RenderStateCullFace
/// render state and the gl_FrontFacing bultin shader input variable.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glFrontFace.xml
/// \sa http://www.opengl.org/sdk/docs/manglsl/xhtml/gl_FrontFacing.xml
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateFrontFace::RenderStateFrontFace(Mode faceMode)
:   RenderState(FRONT_FACE),
    m_faceMode(faceMode)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateFrontFace::setMode(Mode faceMode)
{
    m_faceMode = faceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateFrontFace::Mode RenderStateFrontFace::mode() const
{
    return m_faceMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateFrontFace::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_faceMode == CW)    
    {
        glFrontFace(GL_CW);
    }
    else 
    {
        glFrontFace(GL_CCW);
    }

    CVF_CHECK_OGL(oglContext);
}



} // namespace cvf


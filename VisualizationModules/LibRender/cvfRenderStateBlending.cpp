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
#include "cvfRenderStateBlending.h"
#include "cvfAssert.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateBlending
/// \ingroup Render
///
/// Encapsulate OpenGL blending functions: glEnable(GL_BLEND), glBlendEquation(), glBlendEquationSeparate()
/// glBlendFunc(), glBlendFuncSeparate(), glBlendColor()
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glEnable.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendEquation.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendEquationSeparate.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendFunc.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendFuncSeparate.xml
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glBlendColor.xml
/// 
/// \todo
/// Add support for enable/disable blending per drawbuffer: glEnablei(GL_BLEND, drawBufferIndex)
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateBlending::RenderStateBlending()
:   RenderState(BLENDING),
    m_enableBlending(false),
    m_funcSourceRGB(ONE),
    m_funcDestinationRGB(ZERO),
    m_funcSourceAlpha(ONE),
    m_funcDestinationAlpha(ZERO),
    m_equationRGB(FUNC_ADD),
    m_equationAlpha(FUNC_ADD),
    m_blendColor(0, 0, 0, 0)
{
}


//--------------------------------------------------------------------------------------------------
/// glEnable(GL_BLEND) / glDisable(GL_BLEND)
//--------------------------------------------------------------------------------------------------
void RenderStateBlending::enableBlending(bool blend/*, uint drawBufferIndex*/)
{
    m_enableBlending = blend;
}


//--------------------------------------------------------------------------------------------------
/// glBlendFunc()
//--------------------------------------------------------------------------------------------------
void RenderStateBlending::setFunction(Function source, Function destination)
{
    m_funcSourceRGB         = source;
    m_funcSourceAlpha       = source;
    m_funcDestinationRGB    = destination;
    m_funcDestinationAlpha  = destination;
}


//--------------------------------------------------------------------------------------------------
/// glBlendEquation(). Requires OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void RenderStateBlending::setEquation(Equation eq)
{
    m_equationRGB   = eq;
    m_equationAlpha = eq;
}


//--------------------------------------------------------------------------------------------------
/// glBlendFuncSeparate(). Requires OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void RenderStateBlending::setFunctionSeparate(Function sourceRGB, Function destinationRGB, Function sourceAlpha, Function destinationAlpha)
{
    m_funcSourceRGB         = sourceRGB;
    m_funcDestinationRGB    = destinationRGB;
    m_funcSourceAlpha       = sourceAlpha;
    m_funcDestinationAlpha  = destinationAlpha;
}


//--------------------------------------------------------------------------------------------------
/// glBlendEquationSeparate(). Requires OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void RenderStateBlending::setEquationSeparate(Equation equationRGB, Equation equationAlpha)
{
    m_equationRGB   = equationRGB;
    m_equationAlpha = equationAlpha;
}


//--------------------------------------------------------------------------------------------------
/// glBlendColor(). Requires OpenGL 2.0
//--------------------------------------------------------------------------------------------------
void RenderStateBlending::setBlendColor(Color4f blendColor)
{
    m_blendColor = blendColor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateBlending::configureTransparencyBlending()
{
    m_enableBlending = true;
    setFunction(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateBlending::applyOpenGL(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    /// As we do not care about specific support for OpenGL 1.4, 1.3 etc., everything that is not in 1.1
    /// will require at least support for our baseline (currently OpenGL 2.0)
    bool openGL2Support = oglContext->capabilities()->supportsOpenGL2();

    if (m_enableBlending)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if ((m_funcSourceRGB == m_funcSourceAlpha) && (m_funcDestinationRGB == m_funcDestinationAlpha))
    {
        glBlendFunc(blendFuncOpenGL(m_funcSourceRGB), blendFuncOpenGL(m_funcDestinationRGB));
    }
    else
    {
        if (openGL2Support)
        {
            glBlendFuncSeparate(blendFuncOpenGL(m_funcSourceRGB), blendFuncOpenGL(m_funcDestinationRGB), blendFuncOpenGL(m_funcSourceAlpha), blendFuncOpenGL(m_funcDestinationAlpha));
        }
        else
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support separate blend functions.");
        }
    }

    if (openGL2Support)
    {
        if (m_equationRGB == m_equationAlpha)
        {
            glBlendEquation(blendEquationOpenGL(m_equationRGB));
        }
        else
        {
            glBlendEquationSeparate(blendEquationOpenGL(m_equationRGB), blendEquationOpenGL(m_equationAlpha));
        }

        glBlendColor(m_blendColor.r(), m_blendColor.g(), m_blendColor.b(), m_blendColor.a());
    }
    else
    {
        // Only error reporting here
        if (m_equationRGB != FUNC_ADD ||
            m_equationRGB != m_equationAlpha)
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support blend equations.");
        }

        if (m_blendColor != Color4f(0, 0, 0, 0))
        {
            CVF_LOG_RENDER_ERROR(oglContext, "Context does not support blend color.");
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum RenderStateBlending::blendEquationOpenGL(Equation eq) const
{
    switch (eq)
    {
        case FUNC_ADD:              return GL_FUNC_ADD;
        case FUNC_SUBTRACT:         return GL_FUNC_SUBTRACT;
        case FUNC_REVERSE_SUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
#ifndef CVF_OPENGL_ES
        case MIN:                   return GL_MIN;
        case MAX:                   return GL_MAX;
#endif
    }

    CVF_FAIL_MSG("Unhandled blend equation");
    return 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfGLenum RenderStateBlending::blendFuncOpenGL(Function func) const
{
    switch (func)
    {
        case ZERO:                      return GL_ZERO;
        case ONE:                       return GL_ONE;
        case SRC_COLOR:                 return GL_SRC_COLOR;
        case ONE_MINUS_SRC_COLOR:       return GL_ONE_MINUS_SRC_COLOR;
        case DST_COLOR:                 return GL_DST_COLOR;
        case ONE_MINUS_DST_COLOR:       return GL_ONE_MINUS_DST_COLOR;
        case SRC_ALPHA:                 return GL_SRC_ALPHA;
        case ONE_MINUS_SRC_ALPHA:       return GL_ONE_MINUS_SRC_ALPHA;
        case DST_ALPHA:                 return GL_DST_ALPHA;
        case ONE_MINUS_DST_ALPHA:       return GL_ONE_MINUS_DST_ALPHA;
        case CONSTANT_COLOR:            return GL_CONSTANT_COLOR;
        case ONE_MINUS_CONSTANT_COLOR:  return GL_ONE_MINUS_CONSTANT_COLOR;
        case CONSTANT_ALPHA:            return GL_CONSTANT_ALPHA;
        case ONE_MINUS_CONSTANT_ALPHA:  return GL_ONE_MINUS_CONSTANT_ALPHA;
        case SRC_ALPHA_SATURATE:        return GL_SRC_ALPHA_SATURATE;
    }

    CVF_FAIL_MSG("Unhandled blend func");
    return 0;
}



} // namespace cvf


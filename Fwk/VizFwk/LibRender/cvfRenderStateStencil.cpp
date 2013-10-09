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
#include "cvfRenderStateStencil.h"
#include "cvfAssert.h"
#include "cvfOpenGL.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateStencil
/// \ingroup Render
///
/// glStencilFunc(), glStencilOp() and glEnable(GL_STENCIL_TEST) functions.
///
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glStencilFunc.xml
/// \sa http://www.opengl.org/sdk/docs/man/xhtml/glStencilOp.xml
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateStencil::RenderStateStencil()
:   RenderState(STENCIL),
    m_function(ALWAYS),
    m_functionRefValue(0),
    m_functionMask(0xffffffff),
    m_opStencilFails(KEEP),
    m_opStencilPassesDepthFails(KEEP),
    m_opStencilPassesDepthPasses(KEEP),
    m_enable(false)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateStencil::enableStencilTest(bool enableTest)
{
    m_enable = enableTest;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateStencil::setFunction(Function func, int refValue, uint mask)
{
    m_function = func;
    m_functionRefValue = refValue;
    m_functionMask = mask;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateStencil::setOperation(Operation stencilFails, Operation stencilPassesDepthFails, Operation stencilPassesDepthPasses)
{
    m_opStencilFails = stencilFails;
    m_opStencilPassesDepthFails = stencilPassesDepthFails;
    m_opStencilPassesDepthPasses = stencilPassesDepthPasses;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateStencil::applyOpenGL(OpenGLContext* oglContext) const
{
    if (m_enable)
    {
        GLenum funcOGL = functionOpenGL(m_function);
        glStencilFunc(funcOGL, m_functionRefValue, m_functionMask);

        const GLenum stencilFailsOGL          = operationOpenGL(m_opStencilFails);
        const GLenum stencilPassesDepthFails  = operationOpenGL(m_opStencilPassesDepthFails);
        const GLenum stencilPassesDepthPasses = operationOpenGL(m_opStencilPassesDepthPasses);        
        glStencilOp(stencilFailsOGL, stencilPassesDepthFails, stencilPassesDepthPasses);

        glEnable(GL_STENCIL_TEST);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
    
    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::cvfGLenum RenderStateStencil::functionOpenGL(Function func)
{
    switch (func)
    {
        case NEVER:     return GL_NEVER;
        case LESS:      return GL_LESS;
        case LEQUAL:    return GL_LEQUAL;
        case GREATER:   return GL_GREATER;
        case GEQUAL:    return GL_GEQUAL;
        case EQUAL:     return GL_EQUAL;
        case NOTEQUAL:  return GL_NOTEQUAL;
        case ALWAYS:    return GL_ALWAYS;
    }

    CVF_FAIL_MSG("Unhandled stencil func");
    return 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::cvfGLenum RenderStateStencil::operationOpenGL(Operation op)
{
    switch (op)
    {
        case KEEP:        return GL_KEEP;     
        case ZERO:        return GL_ZERO;     
        case REPLACE:     return GL_REPLACE;  
        case INCR:        return GL_INCR;     
        case INCR_WRAP:   return GL_INCR_WRAP;
        case DECR:        return GL_DECR;     
        case DECR_WRAP:   return GL_DECR_WRAP;
        case INVERT:      return GL_INVERT;   
    }

    CVF_FAIL_MSG("Unhandled stencil operation");
    return 0;
}


} // namespace cvf


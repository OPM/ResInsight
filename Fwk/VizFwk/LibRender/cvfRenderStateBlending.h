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


#pragma once

#include "cvfRenderState.h"
#include "cvfOpenGLTypes.h"
#include "cvfColor4.h"


namespace cvf {



//==================================================================================================
//
// Controls OpenGL blending
//
//==================================================================================================
class RenderStateBlending : public RenderState
{
public:
    enum Function
    {
        ZERO,
        ONE,
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        CONSTANT_COLOR,
        ONE_MINUS_CONSTANT_COLOR,
        CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA,
        SRC_ALPHA_SATURATE
    };

    enum Equation
    {
        FUNC_ADD, 
        FUNC_SUBTRACT,
        FUNC_REVERSE_SUBTRACT,
        MIN,                    // Unsupported on OpenGL ES
        MAX                     // Unsupported on OpenGL ES
    };

public:
    RenderStateBlending();

    void            enableBlending(bool blend);
    void            setFunction(Function source, Function destination);
    void            setEquation(Equation equation);

    void            setFunctionSeparate(Function sourceRGB, Function destinationRGB, Function sourceAlpha, Function destinationAlpha);
    void            setEquationSeparate(Equation equationRGB, Equation equationAlpha);

    void            setBlendColor(Color4f blendColor);

    void            configureTransparencyBlending();

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    cvfGLenum       blendFuncOpenGL(Function func) const;
    cvfGLenum       blendEquationOpenGL(Equation eq) const;

private:
    bool        m_enableBlending;
    Function    m_funcSourceRGB;
    Function    m_funcDestinationRGB;
    Function    m_funcSourceAlpha;
    Function    m_funcDestinationAlpha;
    Equation    m_equationRGB;
    Equation    m_equationAlpha;
    Color4f     m_blendColor;
};


}  // namespace cvf

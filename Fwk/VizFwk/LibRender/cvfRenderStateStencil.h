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

namespace cvf {



//==================================================================================================
//
// Encapsulate OpenGL glStencilFunc(), glStencilOp() and glEnable(GL_STENCIL_TEST) functions.
//
//==================================================================================================
class RenderStateStencil : public RenderState
{
public:
    enum Function
    {
        NEVER,      ///< Always fails.
        LESS,       ///< Passes if ( ref & mask ) <  ( stencil & mask ).
        LEQUAL,     ///< Passes if ( ref & mask ) <= ( stencil & mask ).
        GREATER,    ///< Passes if ( ref & mask ) >  ( stencil & mask ).
        GEQUAL,     ///< Passes if ( ref & mask ) >= ( stencil & mask ).
        EQUAL,      ///< Passes if ( ref & mask ) =  ( stencil & mask ).
        NOTEQUAL,   ///< Passes if ( ref & mask ) != ( stencil & mask ).
        ALWAYS      ///< Always passes.
    };

    enum Operation
    {
        KEEP,       ///< Keeps the current value.
        ZERO,       ///< Sets the stencil buffer value to 0.
        REPLACE,    ///< Sets the stencil buffer value to ref, as specified by glStencilFunc.
        INCR,       ///< Increments the current stencil buffer value. Clamps to the maximum representable unsigned value.
        INCR_WRAP,  ///< Increments the current stencil buffer value. Wraps stencil buffer value to zero when incrementing the maximum representable unsigned value.
        DECR,       ///< Decrements the current stencil buffer value. Clamps to 0.
        DECR_WRAP,  ///< Decrements the current stencil buffer value. Wraps stencil buffer value to the maximum representable unsigned value when decrementing a stencil buffer value of zero.
        INVERT      ///< Bitwise inverts the current stencil buffer value.
    };

public:
    RenderStateStencil();

    void                enableStencilTest(bool enableTest);
    void                setFunction(Function func, int refValue, uint mask);
    void                setOperation(Operation stencilFails, Operation stencilPassesDepthFails, Operation stencilPassesDepthPasses);

    virtual void        applyOpenGL(OpenGLContext* oglContext) const;

private:
private:
    static cvfGLenum    functionOpenGL(Function func);
    static cvfGLenum    operationOpenGL(Operation op);

private:
    Function        m_function;                     // Stencil test function. Initial value ALWAYS
    int             m_functionRefValue;             // Reference value for the stencil test. Initial value is 0
    uint            m_functionMask;                 // Mask that is ANDed with both the reference value and the stored stencil value when the test is done. Initial value is all 1's
    Operation       m_opStencilFails;               // Action to take when the stencil test fails. Initial value is KEEP.
    Operation       m_opStencilPassesDepthFails;   // Stencil action when the stencil test passes, but the depth test fails. Initial value is KEEP
    Operation       m_opStencilPassesDepthPasses;  // Stencil action when both the stencil test and the depth test passes. Initial value is KEEP
    bool            m_enable;                       // Enable/disable the stencil test
};
}  // namespace cvf

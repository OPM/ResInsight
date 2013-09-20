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
// Encapsulate OpenGL glEnable(GL_DEPTH_TEST), glDepthFunc() and glDepthMask() functions.
//
//==================================================================================================
class RenderStateDepth : public RenderState
{
public:
    enum Function
    {
        NEVER,      ///< Never passes
        LESS,       ///< Passes if the incoming depth value is less than the stored depth value. This is the OpenGL default.
        EQUAL,      ///< Passes if the incoming depth value is equal to the stored depth value.
        LEQUAL,     ///< Passes if the incoming depth value is less than or equal to the stored depth value.
        GREATER,    ///< Passes if the incoming depth value is greater than the stored depth value.
        NOTEQUAL,   ///< Passes if the incoming depth value is not equal to the stored depth value.
        GEQUAL,     ///< Passes if the incoming depth value is greater than or equal to the stored depth value.
        ALWAYS      ///< Always passes.
    };

public:
    RenderStateDepth(bool depthTest = true, Function func = LESS, bool depthWrite = true);

    void            setFunction(Function func);
    void            enableDepthTest(bool enableTest);
    void            enableDepthWrite(bool enableWrite);
    
    Function        function() const;
    bool            isDepthTestEnabled() const;
    bool            isDepthWriteEnabled() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    cvfGLenum       depthFuncOpenGL() const;

private:
    Function    m_depthFunc;
    bool        m_enableDepthTest;
    bool        m_enableDepthWrite;
};
}  // namespace cvf

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


#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfViewport.h"

namespace cvf {

class OpenGLContext;

//==================================================================================================
//
// OpenGL Scissoring
//
//==================================================================================================
class RenderingScissor : public Object
{
public:
    RenderingScissor();
    ~RenderingScissor(){}

    void        setScissorRectangle(int x, int y, uint width, uint height);

    int         x() const;
    int         y() const;
    uint        width() const;
    uint        height() const;

    void        applyOpenGL(OpenGLContext* oglContext, Viewport::ClearMode clearMode, const Color4f& clearColor);
    void        unApplyOpenGL(OpenGLContext* oglContext);

private:

    int         m_x;
    int         m_y;
    uint        m_width;
    uint        m_height;

    int         m_scissorBoxToRestore[4];
    bool        m_scissorEnabledStateToRestore;
};

}



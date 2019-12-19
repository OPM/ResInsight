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

#include "cvfObject.h"
#include "cvfColor4.h"
#include "cvfOpenGLTypes.h"
#include "cvfString.h"

namespace cvf {

class OpenGLContext;


//==================================================================================================
//
// An OpenGL viewport
//
//==================================================================================================
class Viewport : public Object
{
public:
    enum ClearMode
    {
        DO_NOT_CLEAR,
        CLEAR_COLOR,
        CLEAR_DEPTH,
        CLEAR_STENCIL,
        CLEAR_COLOR_DEPTH,
        CLEAR_COLOR_STENCIL,
        CLEAR_DEPTH_STENCIL,
        CLEAR_COLOR_DEPTH_STENCIL
    };

public:
    Viewport();

    void        set(int x, int y, uint width, uint height);

    int         x() const;
    int         y() const;
    uint        width() const;
    uint        height() const;
    double      aspectRatio() const;

    void        setClearColor(Color4f color);
    Color4f     clearColor() const;

    void        applyOpenGL(OpenGLContext* oglContext, ClearMode clearMode);

    String      debugString() const;

    static cvfGLbitfield clearFlagsOpenGL(ClearMode clearMode);

private:
    int         m_x;
    int         m_y;
    uint        m_width;
    uint        m_height;

    Color4f     m_clearColor;
    double      m_clearDepth;
    int         m_clearStencil;
};

}

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
// Controls OpenGL polygon rasterization mode, glPolygonMode() 
//
//==================================================================================================
class RenderStatePolygonMode : public RenderState
{
public:
    enum Mode
    {
        FILL,   ///< The interior of the polygons is filled
        LINE,   ///< Boundary edges of the polygons are drawn as line segments
        POINT   ///< Polygon vertices that are marked as the start of a boundary edge are drawn as points
    };

public:
    RenderStatePolygonMode(Mode frontAndBackFaceMode = FILL);

    void            set(Mode frontAndBackMode);
    void            setFrontFace(Mode mode);
    void            setBackFace(Mode mode);
    Mode            frontFace() const;
    Mode            backFace() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    static cvfGLenum    polygonModeOpenGL(Mode mode);

private:
    Mode    m_frontFaceMode;
    Mode    m_backFaceMode;
};


}  // namespace cvf

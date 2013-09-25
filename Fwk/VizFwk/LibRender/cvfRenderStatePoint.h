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

namespace cvf {



//==================================================================================================
//
// Controls OpenGL point size, glPointSize() and glEnable()/glDisable() with GL_PROGRAM_POINT_SIZE
//
//==================================================================================================
class RenderStatePoint : public RenderState
{
public:
    enum Mode
    {
        FIXED_SIZE,     ///< Fixed diameter of raserized points (as specified by point size/glPointSize())
        PROGRAM_SIZE    ///< Point size will be specified using GLSL and the gl_PointSize built-in variable
    };

public:
    RenderStatePoint(Mode sizeMode = FIXED_SIZE);

    void            setMode(Mode sizeMode);
    Mode            mode() const;
    void            enablePointSprite(bool enable);
    bool            isPointSpriteEnabled() const;
    void            setSize(float pointSize);
    float           size() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    Mode    m_sizeMode;
    bool    m_pointSprite;
    float   m_pointSize;
};


}  // namespace cvf

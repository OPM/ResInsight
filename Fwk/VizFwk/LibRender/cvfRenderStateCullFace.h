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
// Encapsulate OpenGL glCullFace() and glEnable(GL_CULL_FACE) functions.
//
//==================================================================================================
class RenderStateCullFace : public RenderState
{
public:
    enum Mode
    {
        BACK,           ///< Cull back facing polygons
        FRONT,          ///< Cull front facing polygons
        FRONT_AND_BACK  ///< No polygons are drawn, but other primitives such as points and lines are drawn
    };

public:
    RenderStateCullFace(bool enableCulling = true, Mode faceMode = BACK);

    void            enable(bool enableCulling);
    bool            isEnabled() const;

    void            setMode(Mode faceMode);
    Mode            mode() const;

    virtual void    applyOpenGL(OpenGLContext* oglContext) const;

private:
    bool        m_enableCulling;
    Mode        m_faceMode;
};


}  // namespace cvf

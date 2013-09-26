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
#include "cvfString.h"
#include "cvfOpenGLTypes.h"

namespace cvf {

class OpenGLContext;
class OglRcShader;


//==================================================================================================
//
// Encapsulates a GLSL shader (vertex, fragment or geometry shader)
//
//==================================================================================================
class Shader : public Object
{
public:
    enum ShaderType
    {
        VERTEX_SHADER,
        FRAGMENT_SHADER,
        GEOMETRY_SHADER
    };

public:
    Shader(ShaderType shaderType, const String& shaderName);
    Shader(ShaderType shaderType, const String& shaderName, const String& source);
    ~Shader();

    ShaderType      shaderType() const;
    String          shaderName() const;

    bool            compile(OpenGLContext* oglContext);
    int             compiledVersionTick() const;

    OglId           shaderOglId() const;
    void            deleteShader(OpenGLContext* oglContext);

    String          shaderInfoLog(OpenGLContext* oglContext) const;

private:
    const ShaderType    m_shaderType;           // Type is vertex, fragment or geometry shader
    String              m_shaderName;           // Name of this shader. Primarily used when reporting errors
    String              m_source;
    bool                m_lastCompileSucceeded; // Flag to indicate if the last attempt at compiling this shader succeeded. May be true even after shader is deleted wrt OpenGL
    int                 m_compiledVersionTick;  // Versioning of this shader
    ref<OglRcShader>    m_oglRcShader;			// 
};

}

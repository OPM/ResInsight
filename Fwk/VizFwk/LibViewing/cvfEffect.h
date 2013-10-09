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
#include "cvfRenderState.h"

namespace cvf {

class ShaderProgram;
class Uniform;
class UniformSet;
class RenderStateSet;
class OpenGLContext;


//==================================================================================================
//
// The effect class defines how the drawable should be rendered. (Shader program, uniforms, OpenGL state etc)
//
//==================================================================================================
class Effect : public Object
{
public:
    Effect();
    ~Effect();

    void                    setShaderProgram(ShaderProgram* shaderProgram);
    ShaderProgram*          shaderProgram();
    const ShaderProgram*    shaderProgram() const;

    void                    setUniform(Uniform* uniform);
    void                    setUniformsFromUniformSet(UniformSet* sourceUniformSet);

    void                    setUniformSet(UniformSet* uniformSet);
    UniformSet*             uniformSet();
    const UniformSet*       uniformSet() const;

    void                    setRenderState(RenderState* renderState);
    RenderState*            renderStateOfType(RenderState::Type type);
    const RenderState*      renderStateOfType(RenderState::Type type) const;
    void                    removeRenderState(RenderState::Type type);
    
    void                    setRenderStateSet(RenderStateSet* renderStateSet);
    RenderStateSet*         renderStateSet();
    const RenderStateSet*   renderStateSet() const;

    void                    deleteOrReleaseOpenGLResources(OpenGLContext* oglContext);

private:
    ref<ShaderProgram>      m_shaderProgram;
    ref<UniformSet>         m_uniformSet;
    ref<RenderStateSet>     m_renderStateSet;
};

}

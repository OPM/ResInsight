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

#include "cvfCollection.h"
#include "cvfPart.h"
#include "cvfShader.h"
#include "cvfUniformSet.h"
#include "cvfTexture.h"
#include "cvfSampler.h"
#include "cvfRendering.h"
#include "cvfRenderStateSet.h"

namespace cvf
{

//==================================================================================================
//
// SingleQuadRenderingGenerator
//
//==================================================================================================
class SingleQuadRenderingGenerator
{
public:
    SingleQuadRenderingGenerator(const String& renderingName = String());

    void addTexture(Texture* texture, Sampler* sampler, String samplerUniformName);
    void addFragmentShaderCode(String shaderCode);
    void setRenderState(RenderState* renderState);
    void setUniform(Uniform* uniform);

    ref<Rendering> generate();

private:
    String              m_renderingName;
    Collection<Texture> m_textures;
    Collection<Sampler> m_samplers;
    std::vector<String> m_samplerNames;
    std::vector<String> m_fragShadersCode;
    RenderStateSet      m_renderStateSet;
    UniformSet          m_uniformSet;
};

}  // cvf

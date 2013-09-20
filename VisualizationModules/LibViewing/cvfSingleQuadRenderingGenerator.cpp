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


#include "cvfBase.h"
#include "cvfSingleQuadRenderingGenerator.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfDrawableGeo.h"
#include "cvfEffect.h"
#include "cvfModelBasicList.h"
#include "cvfScene.h"
#include "cvfCamera.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfRenderStateDepth.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::SingleQuadRenderingGenerator
/// \ingroup Viewing
///
/// Utility class for generating renderings with a single full screen quad
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SingleQuadRenderingGenerator::SingleQuadRenderingGenerator(const String& renderingName)
:   m_renderingName(renderingName)
{
    if (m_renderingName.isEmpty())
    {
        m_renderingName = "SingleQuadRendering";
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SingleQuadRenderingGenerator::addTexture(Texture* texture, Sampler* sampler, String samplerUniformName)
{
    m_textures.push_back(texture);
    m_samplers.push_back(sampler);
    m_samplerNames.push_back(samplerUniformName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SingleQuadRenderingGenerator::addFragmentShaderCode(String fragShaderCode)
{
    m_fragShadersCode.push_back(fragShaderCode);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SingleQuadRenderingGenerator::setRenderState(RenderState* renderState)
{
    m_renderStateSet.setRenderState(renderState);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SingleQuadRenderingGenerator::setUniform(Uniform* uniform)
{
    m_uniformSet.setUniform(uniform);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Rendering> SingleQuadRenderingGenerator::generate()
{
    ref<PrimitiveSetIndexedUShort> quad = new PrimitiveSetIndexedUShort(PT_TRIANGLES);

    ref<UShortArray> triIndices = new UShortArray;
    triIndices->reserve(6);

    triIndices->add(0);
    triIndices->add(1);
    triIndices->add(2);
    triIndices->add(0);
    triIndices->add(2);
    triIndices->add(3);

    quad->setIndices(triIndices.p());

    ref<Vec3fArray> quadVerts = new Vec3fArray;
    quadVerts->reserve(4);

    // Must set a z value that is within the default camera's clipping volume in order for the texture to be shown
    float z = -1.0f;
    quadVerts->add(Vec3f(0, 0, z));
    quadVerts->add(Vec3f(1, 0, z));
    quadVerts->add(Vec3f(1, 1, z));
    quadVerts->add(Vec3f(0, 1, z));

    ref<DrawableGeo> geo1 = new DrawableGeo;
    geo1->addPrimitiveSet(quad.p());
    geo1->setVertexArray(quadVerts.p());
    geo1->computeNormals();

    ref<Part> part = new Part;
    part->setDrawable(geo1.p());

    ref<Effect> eff = new Effect;

    part->setEffect(eff.p());

//     ref<Sampler> sampler = new Sampler;
//     sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
//     sampler->setMinFilter(Sampler::NEAREST);
//     sampler->setMagFilter(Sampler::NEAREST);

    if (m_textures.size() > 0)
    {
        // Setup the texture binding render state
        ref<RenderStateTextureBindings> textureBindings = new RenderStateTextureBindings;
        eff->setRenderState(textureBindings.p());

        CVF_ASSERT((m_textures.size() == m_samplers.size()) && (m_textures.size() == m_samplerNames.size()));
        size_t i;
        for (i = 0; i < m_textures.size(); i++)
        {
            textureBindings->addBinding(m_textures[i].p(), m_samplers[i].p(), m_samplerNames[i].toAscii().ptr());
        }
    }

    // Shader program
    cvf::ShaderProgramGenerator gen(m_renderingName + "ShaderProg", cvf::ShaderSourceProvider::instance());

    gen.addVertexCode(cvf::ShaderSourceRepository::vs_FullScreenQuad);

    // Add custom fragment shaders
    size_t i;
    for (i = 0; i < m_fragShadersCode.size(); i++)
    {
        gen.addFragmentCode(m_fragShadersCode[i]);
    }

    ref<ShaderProgram> shaderProg = gen.generate();
    eff->setShaderProgram(shaderProg.p());
    eff->setRenderState(new RenderStateDepth(true, RenderStateDepth::LESS, false));

    uint rs;
    for (rs = 0; rs < m_renderStateSet.count(); rs++)
    {
        eff->setRenderState(m_renderStateSet.renderState(rs));
    }

    uint uf;
    for (uf = 0; uf < m_uniformSet.count(); uf++)
    {
        eff->setUniform(m_uniformSet.uniform(uf));
    }


    ref<Rendering> quadRendering = new Rendering;
    ref<ModelBasicList> quadModel = new ModelBasicList;
    ref<Scene> scene = new Scene;

    quadModel->addPart(part.p());
    quadModel->updateBoundingBoxesRecursive();
    scene->addModel(quadModel.p());
    quadRendering->setScene(scene.p());
    quadRendering->setRenderingName(m_renderingName);

    return quadRendering;
}

} // namespace cvf

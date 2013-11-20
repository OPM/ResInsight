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


#include "cvfLibCore.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "snipTransparentWeightedAverage.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TransparentWeightedAverage::TransparentWeightedAverage()
{
    m_opacity = 0.7f;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool TransparentWeightedAverage::onInitialize()
{
    setupShaders();
  
    // Add the transparent model
    {
        PartCompoundGenerator gen;
        gen.setPartDistribution(Vec3i(2, 2, 2));
        gen.setNumEffects(8);
        gen.useRandomEffectAssignment(false);
        gen.setExtent(Vec3f(3,3,3));
        gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

        Collection<Part> parts;
        gen.generateBoxes(&parts);

        m_transparentModel = new ModelBasicList;

        size_t i;
        for (i = 0; i < parts.size(); i++)
        {
            Part* part = parts[i].p();

            ref<RenderStateBlending> blending = new RenderStateBlending;
            blending->setFunction(RenderStateBlending::ONE, RenderStateBlending::ONE);
            blending->setEquation(RenderStateBlending::FUNC_ADD);
            blending->enableBlending(true);
            part->effect()->setRenderState(blending.p());

            ref<RenderStateDepth> depth = new RenderStateDepth;
            depth->enableDepthWrite(false);
            part->effect()->setRenderState(depth.p());

            part->effect()->setShaderProgram(m_progInit.p());
            part->setPriority(2);

            RenderStateMaterial_FF* mat = static_cast<RenderStateMaterial_FF*>(part->effect()->renderStateOfType(RenderState::MATERIAL_FF));
            if (mat)
            {
                part->effect()->setUniform(new UniformFloat("u_color", Color4f(mat->frontDiffuse(), m_opacity)));
            }

            m_transparentModel->addPart(part);
        }

        m_transparentModel->updateBoundingBoxesRecursive();

        m_renderSequence->rendering(0)->scene()->addModel(m_transparentModel.p());
    }

    // Add the solid model
    {
        PartCompoundGenerator gen;
        gen.setPartDistribution(Vec3i(5, 5, 5));
        gen.setNumEffects(8);
        gen.useRandomEffectAssignment(false);
        gen.setExtent(Vec3f(2,2,2));
        gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

        Collection<Part> parts;
        gen.generateSpheres(20,20, &parts);

        m_solidModel = new ModelBasicList;

        size_t i;
        for (i = 0; i < parts.size(); i++)
        {
            Part* part = parts[i].p();

            part->effect()->setShaderProgram(m_progInit.p());
            part->setPriority(1);

            m_solidModel->addPart(part);
        }

        m_solidModel->updateBoundingBoxesRecursive();

        m_renderSequence->rendering(0)->scene()->addModel(m_solidModel.p());
    }

    BoundingBox bb = m_renderSequence->rendering(0)->scene()->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    setupRenderings();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TransparentWeightedAverage::setupShaders()
{
    // Init shader
    // -------------------------------------------------------------------------
    ShaderProgramGenerator initGen("InitShader", ShaderSourceProvider::instance());
    initGen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    initGen.addFragmentCode(ShaderSourceRepository::src_Color);
    initGen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
    initGen.addFragmentCodeFromFile("TranspWA_InitFrag");

    m_progInit = initGen.generate();

    // Final shader
    m_finalFragShaderCode = ShaderSourceProvider::instance()->getSourceFromFile("TranspWA_FinalFrag");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TransparentWeightedAverage::setupRenderings()
{
    m_fbo = new FramebufferObject;
    m_renderSequence->firstRendering()->setTargetFramebuffer(m_fbo.p());
    m_renderSequence->firstRendering()->camera()->viewport()->setClearColor(Color4f(0,0,0,0));

    ref<RenderbufferObject> rbo = new RenderbufferObject(RenderbufferObject::DEPTH_COMPONENT24, 1, 1);
    m_fbo->attachDepthRenderbuffer(rbo.p());

    ref<Texture> colorSumTexture = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA32F);
    colorSumTexture->setSize(1, 1);
    m_fbo->attachColorTexture2d(0, colorSumTexture.p());

    ref<Texture> countTexture = new Texture(Texture::TEXTURE_RECTANGLE, Texture::R32F);
    countTexture->setSize(1, 1);
    m_fbo->attachColorTexture2d(1, countTexture.p());


    // Setup second rendering drawing the texture on the screen
    // -------------------------------------------------------------------------
    SingleQuadRenderingGenerator quadRenderGen;
    ref<Sampler> sampler = new Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    quadRenderGen.addTexture(colorSumTexture.p(), sampler.p(), "ColorTex0");
    quadRenderGen.addTexture(countTexture.p(), sampler.p(), "ColorTex1");
    quadRenderGen.addFragmentShaderCode(m_finalFragShaderCode);
    quadRenderGen.setUniform(new UniformFloat("BackgroundColor", Color3f(0.69f, 0.77f, 0.87f)));

    ref<Rendering> quadRendering = quadRenderGen.generate();
    m_renderSequence->addRendering(quadRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TransparentWeightedAverage::onResizeEvent(int width, int height)
{
    // Resize the FBO (and the rendering viewport)
    m_renderSequence->rendering(1)->camera()->viewport()->set(0, 0, width, height);
    m_fbo->resizeAttachedBuffers(width, height);

    // Finally, call base
    TestSnippet::onResizeEvent(width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TransparentWeightedAverage::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->character() == 'T')
    {
        m_opacity -= 0.1f;
        //m_alphaUniform->set(m_opacity);
    }
    if (keyEvent->character() == 't')
    {
        m_opacity += 0.1f;
        //m_alphaUniform->set(m_opacity);
    }

    cvf::Trace::show(String("Current setting: Transparency: %1 %%").arg(m_opacity*100.0f));

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> TransparentWeightedAverage::helpText() const
{
    std::vector<cvf::String> helpText;

    helpText.push_back("'T' to increase the transparency");
    helpText.push_back("'t' to decrease the transparency");

    return helpText;
}

} // namespace snip

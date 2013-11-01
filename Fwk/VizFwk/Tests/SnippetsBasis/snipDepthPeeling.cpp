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

#include "snipDepthPeeling.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DepthPeeling::DepthPeeling()
{
    m_numPasses = 4;
    m_opacity = 0.6f;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DepthPeeling::onInitialize()
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
        m_transparentModel->addPart(part);

        // Setup to be a transparent part
        // part->effect()->set
    }

    m_transparentModel->updateBoundingBoxesRecursive();

    m_renderSequence->rendering(0)->scene()->addModel(m_transparentModel.p());

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
        //gen.generateSpheres(70,70, &parts);

        m_solidModel = new ModelBasicList;

        size_t i;
        for (i = 0; i < parts.size(); i++)
        {
            m_solidModel->addPart(parts[i].p());
        }

        m_solidModel->updateBoundingBoxesRecursive();

        m_renderSequence->rendering(0)->scene()->addModel(m_solidModel.p());
    }

    BoundingBox bb = m_renderSequence->rendering(0)->scene()->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    setupShaders();
    setupRenderings(m_numPasses);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeeling::setupShaders()
{
    // Init shader
    // -------------------------------------------------------------------------
    ShaderProgramGenerator initGen("Init", ShaderSourceProvider::instance());
    initGen.addVertexCode(ShaderSourceRepository::vs_Minimal);
    initGen.addFragmentCodeFromFile("DepthPeeling_InitFrag");
    m_progInit = initGen.generate();

    // Peel shader
    // -------------------------------------------------------------------------
    ShaderProgramGenerator peelGen("Peel", ShaderSourceProvider::instance());
    peelGen.addVertexCode(ShaderSourceRepository::vs_Standard);
    peelGen.addFragmentCode(ShaderSourceRepository::src_Color);
    peelGen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
    peelGen.addFragmentCodeFromFile("DepthPeeling_PeelFrag");
    m_progPeel = peelGen.generate();
    
    // Blend shader
    // -------------------------------------------------------------------------
    m_fragShaderBlendCode = ShaderSourceProvider::instance()->getSourceFromFile("DepthPeeling_BlendFrag");

    // Final shader
    // -------------------------------------------------------------------------
    m_fragShaderFinalCode = ShaderSourceProvider::instance()->getSourceFromFile("DepthPeeling_FinalFrag");

    // OpenGL sanity check for shaders
    m_progInit->linkProgram(m_openGLContext.p());
    m_progPeel->linkProgram(m_openGLContext.p());
    CVF_CHECK_OGL(m_openGLContext.p());
    if (m_progInit->programInfoLog(m_openGLContext.p()).size() > 0)    cvf::Trace::show("m_progInit: \n" + m_progInit->programInfoLog(m_openGLContext.p()));
    if (m_progPeel->programInfoLog(m_openGLContext.p()).size() > 0)    cvf::Trace::show("m_progPeel: \n" + m_progPeel->programInfoLog(m_openGLContext.p()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeeling::onResizeEvent(int width, int height)
{
    resizeViewportsAndBuffers(width, height);

    // Finally, call base
    TestSnippet::onResizeEvent(width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeeling::resizeViewportsAndBuffers(cvf::uint width, cvf::uint height)
{
    // Resize FBOs
    m_fboSolid->resizeAttachedBuffers(width, height);
    m_fboInitBlend->resizeAttachedBuffers(width, height);

    cvf::uint i = 0;
    for (i = 0; i < 2; i++)
    {
        m_fboClearDepth[i]->resizeAttachedBuffers(width, height);
        m_fboClearBlendTextures[i]->resizeAttachedBuffers(width, height);
        m_fboDepthPeel[i]->resizeAttachedBuffers(width, height);
        m_fboFullScreenUpdate[i]->resizeAttachedBuffers(width, height);
    }

    // Resize all viewports
    for (i = 0; i < m_renderSequence->renderingCount(); i++)
    {
        m_renderSequence->rendering(i)->camera()->viewport()->set(0, 0, width, height);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeeling::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->key() == Key_Plus)
    {
        m_numPasses++;
    }
    if (keyEvent->key() == Key_Minus)
    {
        if (m_numPasses > 2) m_numPasses--;
    }
    if (keyEvent->character() == 'T')
    {
        m_opacity -= 0.1f;
    }
    if (keyEvent->character() == 't')
    {
        m_opacity += 0.1f;
    }

    cvf::uint width = m_renderSequence->firstRendering()->camera()->viewport()->width();
    cvf::uint height = m_renderSequence->firstRendering()->camera()->viewport()->height();

    setupRenderings(m_numPasses);
    resizeViewportsAndBuffers(width, height);

    cvf::Trace::show(String("Current setting: Num passes %1 Transparency: %2 %%").arg(m_numPasses).arg(m_opacity*100.0f));

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> DepthPeeling::helpText() const
{
    std::vector<cvf::String> helpText;

    helpText.push_back("'+' to increase num passes");
    helpText.push_back("'-' to decrease num passes");
    helpText.push_back(" ");
    helpText.push_back("'T' to increase the transparency");
    helpText.push_back("'t' to decrease the transparency");

    return helpText;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeeling::setupRenderings(cvf::uint numPasses)
{
    float MAX_DEPTH = 1.0;

    // Create scenes
    ref<Scene> solidScene = new Scene;
    solidScene->addModel(m_solidModel.p());
    ref<Scene> transparentScene = new Scene;
    transparentScene->addModel(m_transparentModel.p());

    // Setup FBOs & textures
    ref<Rendering> clearDepthRendering[2];
    ref<Rendering> clearBlendRendering[2];
    ref<Rendering> peelRendering[2];
    ref<Rendering> fullScreenUpdateRendering[2];

    ref<Texture> dualDepthTexure[2];
    ref<Texture> dualFrontBlendTexture[2];
    ref<Texture> dualBackBlendTexture[2];

    ref<Texture> solidDepthTexture  = new Texture(Texture::TEXTURE_RECTANGLE, Texture::DEPTH_COMPONENT24);
    ref<Texture> backBlendTexture   = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
    dualDepthTexure[0]              = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA32F);
    dualDepthTexure[1]              = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA32F);
    dualFrontBlendTexture[0]        = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
    dualFrontBlendTexture[1]        = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
    dualBackBlendTexture[0]         = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
    dualBackBlendTexture[1]         = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);

    ref<Sampler> sampler = new Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);


    m_fboSolid = new FramebufferObject;
    m_fboSolid->attachDepthTexture2d(solidDepthTexture.p());
    m_fboSolid->attachColorTexture2d(0, backBlendTexture.p());

    cvf::uint i;
    for (i = 0; i < 2; i++)
    {
        cvf::uint otherIdx = (i == 0) ? 1 : 0;

        m_fboClearDepth[i] = new FramebufferObject;
        m_fboClearDepth[i]->attachColorTexture2d(0, dualDepthTexure[i].p());

        m_fboClearBlendTextures[i] = new FramebufferObject;
        m_fboClearBlendTextures[i]->attachColorTexture2d(0, dualFrontBlendTexture[i].p());
        m_fboClearBlendTextures[i]->attachColorTexture2d(1, dualBackBlendTexture[i].p());

        m_fboDepthPeel[i] = new FramebufferObject;
        m_fboDepthPeel[i]->attachColorTexture2d(0, dualDepthTexure[i].p());
        m_fboDepthPeel[i]->attachColorTexture2d(1, dualFrontBlendTexture[i].p());
        m_fboDepthPeel[i]->attachColorTexture2d(2, dualBackBlendTexture[i].p());

        m_fboFullScreenUpdate[i] = new FramebufferObject;
        m_fboFullScreenUpdate[i]->attachColorTexture2d(0, backBlendTexture.p());

        // Setup renderings
        clearDepthRendering[i] = new Rendering;
        clearDepthRendering[i]->setTargetFramebuffer(m_fboClearDepth[i].p());
        ref<Camera> clearDepthCamera = new Camera;
        clearDepthCamera->viewport()->setClearColor(Color4f(-MAX_DEPTH, -MAX_DEPTH, 0, 0));
        clearDepthRendering[i]->setClearMode(Viewport::CLEAR_COLOR);
        clearDepthRendering[i]->setCamera(clearDepthCamera.p());

        clearBlendRendering[i] = new Rendering;
        clearBlendRendering[i]->setTargetFramebuffer(m_fboClearBlendTextures[i].p());
        ref<Camera> clearBlendCamera = new Camera;
        clearBlendCamera->viewport()->setClearColor(Color4f(0, 0, 0, 0));
        clearBlendRendering[i]->setClearMode(Viewport::CLEAR_COLOR);
        clearBlendRendering[i]->setCamera(clearBlendCamera.p());

        // Peel-rendering
        peelRendering[i] = new Rendering;

        // Want something like
        // HACK! To be discussed!
        /*
        peelRendeing[i]->addBufferClear(0, Vec4f(-MAX_DEPTH, -MAX_DEPTH, 0, 0));
        peelRendeing[i]->addBufferClear(1, Color4f(0, 0, 0, 0));
        peelRendeing[i]->addBufferClear(2, Color4f(0, 0, 0, 0));
        */

        peelRendering[i]->setTargetFramebuffer(m_fboDepthPeel[i].p());
        peelRendering[i]->setCamera(m_camera.p());
        peelRendering[i]->setClearMode(Viewport::DO_NOT_CLEAR);
        peelRendering[i]->setScene(transparentScene.p());

        ref<RenderStateBlending> peelBlending = new RenderStateBlending;
        peelBlending->enableBlending(true);
        peelBlending->setEquation(RenderStateBlending::MAX);

        // Setup the texture binding render state
        ref<RenderStateTextureBindings> peelTextureBindings = new RenderStateTextureBindings;
        peelTextureBindings->addBinding(dualDepthTexure[otherIdx].p(), sampler.p(), "DepthBlenderTex");
        peelTextureBindings->addBinding(dualFrontBlendTexture[otherIdx] .p(), sampler.p(), "FrontBlenderTex");
        peelTextureBindings->addBinding(solidDepthTexture.p(), sampler.p(), "SolidDepthTex");

        ref<Effect> peelEffect = new Effect;
        peelEffect->setShaderProgram(m_progPeel.p());
        peelEffect->setRenderState(peelBlending.p());
        peelEffect->setRenderState(peelTextureBindings.p());
        peelEffect->setRenderState(new RenderStateDepth(false));
        peelEffect->setUniform(new UniformFloat("u_color", Color4f(1,0,1,m_opacity)));
        peelRendering[i]->setEffectOverride(peelEffect.p());

        // Add peeling result to the output texture
        ref<RenderStateBlending> fullScreenBlending = new RenderStateBlending;
        fullScreenBlending->enableBlending(true);
        fullScreenBlending->setEquation(RenderStateBlending::FUNC_ADD);
        fullScreenBlending->setFunction(RenderStateBlending::SRC_ALPHA, RenderStateBlending::ONE_MINUS_SRC_ALPHA);

        SingleQuadRenderingGenerator gen;      
        gen.addTexture(backBlendTexture.p(), sampler.p(), "TempTex");
        gen.addFragmentShaderCode(m_fragShaderBlendCode);
        gen.setRenderState(fullScreenBlending.p());
        gen.setRenderState(new RenderStateDepth(false));
        fullScreenUpdateRendering[i] = gen.generate();
        fullScreenUpdateRendering[i]->setTargetFramebuffer(m_fboFullScreenUpdate[i].p());
        fullScreenUpdateRendering[i]->setClearMode(Viewport::DO_NOT_CLEAR);
    }


    m_fboInitBlend = new FramebufferObject;
    m_fboInitBlend->attachColorTexture2d(0, dualDepthTexure[0].p());

    // Setup renderings
    // -------------------------------------------------------------------------
    m_renderSequence->removeAllRenderings();

    // 1. Render solid model
    ref<Rendering> solidRendering = new Rendering;
    solidRendering->setTargetFramebuffer(m_fboSolid.p());
    solidRendering->setScene(solidScene.p());
    solidRendering->setCamera(m_camera.p());
    m_renderSequence->addRendering(solidRendering.p());

    // 2.  Initialize Min-Max RenderStateDepth Buffers
    m_renderSequence->addRendering(clearBlendRendering[0].p());
    m_renderSequence->addRendering(clearDepthRendering[0].p());

    ref<Rendering> initBlendRendering = new Rendering;
    initBlendRendering->setTargetFramebuffer(m_fboInitBlend.p());
    initBlendRendering->setScene(transparentScene.p());
    initBlendRendering->setCamera(m_camera.p());
    initBlendRendering->setClearMode(Viewport::DO_NOT_CLEAR);

    ref<RenderStateBlending> blending = new RenderStateBlending;
    blending->enableBlending(true);
    blending->setEquation(RenderStateBlending::MAX);

    ref<Effect> initBlendingEffect = new Effect;
    initBlendingEffect->setShaderProgram(m_progInit.p());
    initBlendingEffect->setRenderState(blending.p());
    initBlendingEffect->setRenderState(new RenderStateDepth(false));
    initBlendRendering->setEffectOverride(initBlendingEffect.p());
    m_renderSequence->addRendering(initBlendRendering.p());

    // 3. Peeling loop
    cvf::uint currId = 0;
    for (i = 1; i < numPasses; i++)
    {
        currId = i % 2;

        m_renderSequence->addRendering(clearBlendRendering[currId].p());
        m_renderSequence->addRendering(clearDepthRendering[currId].p());
        m_renderSequence->addRendering(peelRendering[currId].p());
        m_renderSequence->addRendering(fullScreenUpdateRendering[currId].p());
    }

    SingleQuadRenderingGenerator gen;
    gen.addTexture(dualFrontBlendTexture[currId].p(), sampler.p(), "FrontBlenderTex");
    gen.addTexture(backBlendTexture.p(), sampler.p(), "BackBlenderTex");
    gen.addFragmentShaderCode(m_fragShaderFinalCode);

    ref<Rendering> finalRendering = gen.generate();
    m_renderSequence->addRendering(finalRendering.p());
}

} // namespace snip



//     // TO DISPLAY TEXTURE
//     {
//         SingleQuadRenderingGenerator quadRenderGen;
// 
//         quadRenderGen.addTexture(dualDepthTexure[0].p(), sampler.p(), "u_texture2DRect");
//         quadRenderGen.addFragmentShader(ShaderFactory::instance()->createFromRepository(Shader::FRAGMENT_SHADER, ShaderSourceRepository::fs_Unlit).p());
//         quadRenderGen.addFragmentShader(ShaderFactory::instance()->createFromRepository(Shader::FRAGMENT_SHADER, ShaderSourceRepository::src_TextureRectFragCoord_v33).p());
// 
//         ref<Rendering> quadRendering = quadRenderGen.generate();
//         m_renderSequence->addRendering(quadRendering.p());
// 
//         return;
//     }



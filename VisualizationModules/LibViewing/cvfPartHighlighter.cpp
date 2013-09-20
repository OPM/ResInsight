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
#include "cvfPartHighlighter.h"
#include "cvfRenderSequence.h"
#include "cvfFramebufferObject.h"
#include "cvfRenderbufferObject.h"
#include "cvfSampler.h"
#include "cvfSingleQuadRenderingGenerator.h"
#include "cvfRenderStateDepth.h"
#include "cvfShaderSourceProvider.h"
#include "cvfCamera.h"
#include "cvfScene.h"
#include "cvfModelBasicList.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderProgram.h"
#include "cvfRenderStateStencil.h"
#include "cvfEffect.h"
#include "cvfUniform.h"
#include "cvfGaussianBlur.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStatePolygonMode.h"
#include "cvfRenderStateLine.h"
#include "cvfClipPlaneSet.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::PartHighlighter
/// \ingroup Viewing
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PartHighlighter::PartHighlighter(Model* highlightModel, Camera* mainCamera)
:   m_highlightModel(highlightModel),
    m_mainCamera(mainCamera),
    m_highlightColor(Color3::WHITE)
{
    m_highlightModel = highlightModel;
    m_mainCamera = mainCamera;

    //ref<Texture> selectedColorTexture = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
    ref<Texture> selectedColorTexture = new Texture(Texture::TEXTURE_2D, Texture::RGBA);
    selectedColorTexture->setSize(1, 1);
    m_drawFbo = new FramebufferObject;
    m_drawFbo->attachColorTexture2d(0, selectedColorTexture.p());

    // Note that we clear alpha to 0
    m_highlightCamera = new Camera;
    m_highlightCamera->viewport()->setClearColor(Color4f(Color3::GRAY, 0)); 

    // Main 'solid' rendering
    m_drawRendering = new Rendering;
    m_drawRendering->setTargetFramebuffer(m_drawFbo.p());
    m_drawRendering->setCamera(m_highlightCamera.p());
    m_drawRendering->setScene(new Scene);
    m_drawRendering->scene()->addModel(m_highlightModel.p());
    m_drawRendering->setRenderingName("PartHighlighter : Main 'solid' rendering (m_drawRendering)");

    // Draw geometry again using lines to increase footprint of slim details 
    bool growFootprintUsingLineDrawing = true;
    if (growFootprintUsingLineDrawing)
    {
        m_drawRenderingLines = new Rendering;
        m_drawRenderingLines->setTargetFramebuffer(m_drawFbo.p());
        m_drawRenderingLines->setCamera(m_highlightCamera.p());
        m_drawRenderingLines->setScene(new Scene);
        m_drawRenderingLines->scene()->addModel(m_highlightModel.p());
        m_drawRenderingLines->setClearMode(Viewport::DO_NOT_CLEAR);
        m_drawRenderingLines->setRenderingName("PartHighlighter : Main 'solid' rendering (lines) (m_drawRenderingLines)");
    }

    configureOverrideEffects(false);

    // Setup the blur helper
    // The sigma value her should be tuned to the line width used above
    // If sigma gets larger that the line width we will get artifacts during the blur pass
    m_blur = new GaussianBlur(selectedColorTexture.p(), 7, 2);

    // Mix rendering
    {
        ref<Sampler> sampler = new Sampler;
        sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
        sampler->setMinFilter(Sampler::LINEAR);
        sampler->setMagFilter(Sampler::LINEAR);

        ref<RenderStateBlending> rsBlending = new RenderStateBlending;
        rsBlending->configureTransparencyBlending();

        m_highlightClrUniform = new UniformFloat("u_highlightColor", m_highlightColor);

        SingleQuadRenderingGenerator quadRenderGen("MixRendering");
        //quadRenderGen.addTexture(selectedColorTexture.p(), sampler.p(), "u_texture2DRect");
        quadRenderGen.addTexture(selectedColorTexture.p(), sampler.p(), "u_texture2D");
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::fs_HighlightMix));
        quadRenderGen.setRenderState(rsBlending.p());
        quadRenderGen.setUniform(m_highlightClrUniform.p());
        m_mixRendering = quadRenderGen.generate();
        m_mixRendering->setClearMode(Viewport::DO_NOT_CLEAR);
        m_mixRendering->setRenderingName("PartHighlighter : Mix Rendering (m_mixRendering)");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighter::setHighlightColor(const Color3f& color)
{
    m_highlightColor = color;
    m_highlightClrUniform->set(m_highlightColor);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighter::addRenderingsToSequence(RenderSequence* renderSequence)
{
    CVF_ASSERT(renderSequence);
    
    if (m_drawRendering.notNull())  
    {
        renderSequence->addRendering(m_drawRendering.p());
    }

    if (m_drawRenderingLines.notNull())  
    {
        renderSequence->addRendering(m_drawRenderingLines.p());
    }

    if (m_blur.notNull())
    {
        m_blur->addRenderingsToSequence(renderSequence);
    }

    if (m_mixRendering.notNull())
    {
        renderSequence->addRendering(m_mixRendering.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighter::removeRenderingsFromSequence(RenderSequence* renderSequence)
{
    CVF_ASSERT(renderSequence);

    if (m_drawRendering.notNull())  renderSequence->removeRendering(m_drawRendering.p());
    if (m_drawRenderingLines.notNull())  renderSequence->removeRendering(m_drawRenderingLines.p());
    if (m_mixRendering.notNull())   renderSequence->removeRendering(m_mixRendering.p());

    if (m_blur.notNull())
    {
        m_blur->removeRenderingsFromSequence(renderSequence);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighter::resize(int x, int y, uint width, uint height)
{
    m_drawFbo->resizeAttachedBuffers(width, height);

    // Since draw and drawLines shares a camera, this call will set both
    m_drawRendering->camera()->setViewport(0, 0, width, height);

    // Mix rendering is the only "on-screen" rendering so here we do need to set the position of the viewport
    m_mixRendering->camera()->setViewport(x, y, width, height);

    if (m_blur.notNull())
    {
        m_blur->resizeFromTextureSize();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighter::prepareForRedraw()
{
    if (m_mainCamera->projection() == Camera::PERSPECTIVE)
    {
        m_highlightCamera->setProjectionAsPerspective(m_mainCamera->fieldOfViewYDeg(), m_mainCamera->nearPlane(), m_mainCamera->farPlane());
    }
    else
    {
        m_highlightCamera->setProjectionAsOrtho(m_mainCamera->frontPlaneFrustumHeight(), m_mainCamera->nearPlane(), m_mainCamera->farPlane());
    }

    m_highlightCamera->setViewMatrix(m_mainCamera->viewMatrix());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighter::setClipPlaneSet(ClipPlaneSet* clipPlaneSet)
{
    m_drawRendering->removeAllGlobalDynamicUniformSets();
    
    if (m_drawRenderingLines.notNull())
    {
        m_drawRenderingLines->removeAllGlobalDynamicUniformSets();
    }

    if (clipPlaneSet)
    {
        m_drawRendering->addGlobalDynamicUniformSet(clipPlaneSet);
        if (m_drawRenderingLines.notNull())
        {
            m_drawRenderingLines->addGlobalDynamicUniformSet(clipPlaneSet);
        }

        configureOverrideEffects(true);
    }
    else
    {
        configureOverrideEffects(false);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighter::configureOverrideEffects(bool useClipping)
{
    ShaderProgramGenerator spGen("DrawHighlightGeo", ShaderSourceProvider::instance());

    if (useClipping)
    {
        spGen.addVertexCode(cvf::ShaderSourceRepository::calcClipDistances);
        spGen.addFragmentCode(cvf::ShaderSourceRepository::checkDiscard_ClipDistances);
    }

    spGen.addVertexCode(ShaderSourceRepository::vs_Standard);
    spGen.addFragmentCode(ShaderSourceRepository::src_Color);
    spGen.addFragmentCode(ShaderSourceRepository::fs_Unlit);

    ref<ShaderProgram> shaderProg = spGen.generate();

    if (useClipping)
    {
        shaderProg->setDefaultUniform(new cvf::UniformInt(  "u_clipPlaneCount",   0));
        shaderProg->setDefaultUniform(new cvf::UniformFloat("u_ecClipPlanes",     cvf::Vec4f(0, 0, 1, 0)));
    }

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(shaderProg.p());
    // Use magenta to detect if something isn't working as it should
    eff->setUniform(new UniformFloat("u_color", Color4f(Color3::MAGENTA)));
    m_drawRendering->setEffectOverride(eff.p());

    if (m_drawRenderingLines.notNull())
    {
        ref<Effect> eff = new Effect;
        eff->setShaderProgram(shaderProg.p());
        eff->setRenderState(new RenderStatePolygonMode(RenderStatePolygonMode::LINE));
        eff->setRenderState(new RenderStateLine(3));
        // Use cyan to detect if something isn't working as it should
        eff->setUniform(new UniformFloat("u_color", Color4f(Color3::CYAN)));
        m_drawRenderingLines->setEffectOverride(eff.p());
    }
}



//==================================================================================================
///
/// \class cvf::PartHighlighterStencil
/// \ingroup Viewing
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PartHighlighterStencil::PartHighlighterStencil(Model* highlightModel, Camera* mainCamera)
:   m_highlightModel(highlightModel),
    m_mainCamera(mainCamera),
    m_highlightColor(Color3::WHITE)
{
    m_highlightModel = highlightModel;
    m_mainCamera = mainCamera;

    // Selected rendering
    {
        ref<RenderbufferObject> depthStencilRBO = new RenderbufferObject(RenderbufferObject::DEPTH24_STENCIL8, 1, 1);
        ref<Texture> selectedColorTexture = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
        selectedColorTexture->setSize(1, 1);
        m_drawFbo = new FramebufferObject;
        m_drawFbo->attachDepthStencilRenderbuffer(depthStencilRBO.p());
        m_drawFbo->attachColorTexture2d(0, selectedColorTexture.p());

        m_highlightCamera = new Camera;
        m_highlightCamera->viewport()->setClearColor(Color4f(0, 0, 0, 0));
        m_drawRendering = new Rendering;
        m_drawRendering->setTargetFramebuffer(m_drawFbo.p());
        m_drawRendering->setClearMode(Viewport::CLEAR_COLOR_DEPTH_STENCIL);
        m_drawRendering->setCamera(m_highlightCamera.p());
        m_drawRendering->setScene(new Scene);
        m_drawRendering->scene()->addModel(m_highlightModel.p());

        {
            ShaderProgramGenerator spGen("DrawSelected", ShaderSourceProvider::instance());
            spGen.addVertexCode(ShaderSourceRepository::vs_Standard);
            spGen.addFragmentCode(ShaderSourceRepository::fs_HighlightStencilDraw);
            ref<ShaderProgram> prog  = spGen.generate();

            ref<RenderStateStencil> rsStencil = new RenderStateStencil;
            rsStencil->setFunction(RenderStateStencil::ALWAYS, 1, 0xffffffff);
            rsStencil->setOperation(RenderStateStencil::KEEP, RenderStateStencil::REPLACE, RenderStateStencil::REPLACE);
            rsStencil->enableStencilTest(true);

            ref<Effect> eff = new Effect;
            eff->setShaderProgram(prog.p());
            eff->setUniform(new UniformFloat("u_color", Color4f(m_highlightColor)));
            eff->setRenderState(new RenderStateDepth(true, RenderStateDepth::LEQUAL));
            eff->setRenderState(rsStencil.p());
            m_drawRendering->setEffectOverride(eff.p());
        }


        // Blur rendering
        {
            ref<Texture> blurredColorTexture = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
            blurredColorTexture->setSize(1, 1);
            m_blurFbo = new FramebufferObject;
            m_blurFbo->attachDepthStencilRenderbuffer(depthStencilRBO.p());
            m_blurFbo->attachColorTexture2d(0, blurredColorTexture.p());

            SingleQuadRenderingGenerator quadRenderGen("BlurRendering");
            {
                quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::fs_HighlightStencilBlur_v33));

                ref<Sampler> sampler = new Sampler;
                sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
                sampler->setMinFilter(Sampler::NEAREST);
                sampler->setMagFilter(Sampler::NEAREST);
                quadRenderGen.addTexture(selectedColorTexture.p(), sampler.p(), "u_texture2DRect");

                ref<RenderStateStencil> s = new RenderStateStencil;
                s->setFunction(RenderStateStencil::EQUAL, 0, 0xffffffff);
                s->enableStencilTest(true);
                quadRenderGen.setRenderState(s.p());

                quadRenderGen.setRenderState(new RenderStateDepth(false, RenderStateDepth::LESS, false));
            }

            m_blurRendering = quadRenderGen.generate();
            m_blurRendering->setTargetFramebuffer(m_blurFbo.p());
            m_blurRendering->setClearMode(Viewport::CLEAR_COLOR);
            m_blurRendering->camera()->viewport()->setClearColor(Color4f(0.0, 0.0, 0.0, 0));

            // Mix rendering
            {
                ref<Sampler> sampler = new Sampler;
                sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
                sampler->setMinFilter(Sampler::NEAREST);
                sampler->setMagFilter(Sampler::NEAREST);

                SingleQuadRenderingGenerator quadRenderGen("MixRendering");
                quadRenderGen.addTexture(blurredColorTexture.p(), sampler.p(), "u_texture2DRect");
                quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::fs_HighlightStencilMix_v33));
                m_mixRendering = quadRenderGen.generate();
                m_mixRendering->setClearMode(Viewport::DO_NOT_CLEAR);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighterStencil::setHighlightColor(const Color3f& color)
{
    m_highlightColor = color;
    applyHighlightColor();
}

//--------------------------------------------------------------------------------------------------
/// Applies the currently set highlight color by modifying the override effect used in the draw pass
//--------------------------------------------------------------------------------------------------
void PartHighlighterStencil::applyHighlightColor()
{
    if (m_drawRendering.notNull())
    {
        ref<Effect> eff = m_drawRendering->effectOverride();
        if (eff.notNull())
        {
            eff->setUniform(new UniformFloat("u_color", Color4f(m_highlightColor)));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighterStencil::addRenderingsToSequence(RenderSequence* renderSequence)
{
    CVF_ASSERT(renderSequence);
    if (m_drawRendering.notNull())  renderSequence->addRendering(m_drawRendering.p());
    if (m_blurRendering.notNull())  renderSequence->addRendering(m_blurRendering.p());
    if (m_mixRendering.notNull())   renderSequence->addRendering(m_mixRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighterStencil::removeRenderingsFromSequence(RenderSequence* renderSequence)
{
    CVF_ASSERT(renderSequence);
    if (m_drawRendering.notNull())  renderSequence->removeRendering(m_drawRendering.p());
    if (m_blurRendering.notNull())  renderSequence->removeRendering(m_blurRendering.p());
    if (m_mixRendering.notNull())   renderSequence->removeRendering(m_mixRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighterStencil::resize(uint width, uint height)
{
    m_drawFbo->resizeAttachedBuffers(width, height);
    m_blurFbo->resizeAttachedBuffers(width, height);

    m_drawRendering->camera()->setViewport(0, 0, width, height);
    m_blurRendering->camera()->setViewport(0, 0, width, height);
    m_mixRendering->camera()->setViewport(0, 0, width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighterStencil::prepareForRedraw()
{
    m_highlightCamera->setProjectionAsPerspective(m_mainCamera->fieldOfViewYDeg(), m_mainCamera->nearPlane(), m_mainCamera->farPlane());
    m_highlightCamera->setViewMatrix(m_mainCamera->viewMatrix());
}



}

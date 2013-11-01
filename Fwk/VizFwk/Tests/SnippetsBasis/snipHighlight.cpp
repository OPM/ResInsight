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
#include "cvfLibViewing.h"
#include "cvfLibRender.h"

#include "snipHighlight.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"
#include "cvfuWavefrontObjImport.h"

namespace snip {


//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PartHighlighterExperiment::PartHighlighterExperiment(ModelBasicList* selectedModel, Camera* mainCamera)
{
    m_selectedModel = selectedModel;
    m_mainCamera = mainCamera;

    // Selected rendering
    {
        ref<RenderbufferObject> depthStencilRBO = new RenderbufferObject(RenderbufferObject::DEPTH24_STENCIL8, 1, 1);
        ref<Texture> selectedColorTexture = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
        selectedColorTexture->setSize(1, 1);
        m_selectedFbo = new FramebufferObject;
        m_selectedFbo->attachDepthStencilRenderbuffer(depthStencilRBO.p());
        m_selectedFbo->attachColorTexture2d(0, selectedColorTexture.p());

        m_selectedCamera = new Camera;
        m_selectedCamera->viewport()->setClearColor(Color4f(0, 0, 0, 0));
        m_selectedRendering = new Rendering;
        m_selectedRendering->setTargetFramebuffer(m_selectedFbo.p());
        m_selectedRendering->setClearMode(Viewport::CLEAR_COLOR_DEPTH_STENCIL);
        m_selectedRendering->setCamera(m_selectedCamera.p());
        m_selectedRendering->setScene(new Scene);
        m_selectedRendering->scene()->addModel(m_selectedModel.p());

        {
            ShaderProgramGenerator spGen("DrawSelected", cvf::ShaderSourceProvider::instance());
            spGen.addVertexCode(ShaderSourceRepository::vs_Standard);
            spGen.addFragmentCode(ShaderSourceProvider::instance()->getSourceFromFile("HighlightDraw_Frag"));
            ref<ShaderProgram> prog  = spGen.generate();

            ref<RenderStateStencil> rsStencil = new RenderStateStencil;
            rsStencil->setFunction(RenderStateStencil::ALWAYS, 1, 0xffffffff);
            rsStencil->setOperation(RenderStateStencil::KEEP, RenderStateStencil::REPLACE, RenderStateStencil::REPLACE);
            rsStencil->enableStencilTest(true);

            ref<Effect> eff = new Effect;
            eff->setShaderProgram(prog.p());
            eff->setUniform(new UniformFloat("u_color", Color4f(Color3::ORANGE)));
            eff->setRenderState(new cvf::RenderStateDepth(true, cvf::RenderStateDepth::LEQUAL));
            eff->setRenderState(rsStencil.p());
            m_selectedRendering->setEffectOverride(eff.p());
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
                quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromFile("HighlightBlur_Frag"));

                ref<Sampler> sampler = new Sampler;
                sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
                sampler->setMinFilter(cvf::Sampler::NEAREST);
                sampler->setMagFilter(cvf::Sampler::NEAREST);
                quadRenderGen.addTexture(selectedColorTexture.p(), sampler.p(), "u_texture2DRect");

                ref<RenderStateStencil> s = new RenderStateStencil;
                s->setFunction(RenderStateStencil::EQUAL, 0, 0xffffffff);
                s->enableStencilTest(true);
                quadRenderGen.setRenderState(s.p());

                quadRenderGen.setRenderState(new cvf::RenderStateDepth(false, RenderStateDepth::LESS, false));
            }

            m_blurRendering = quadRenderGen.generate();
            m_blurRendering->setTargetFramebuffer(m_blurFbo.p());
            m_blurRendering->setClearMode(Viewport::CLEAR_COLOR);
            m_blurRendering->camera()->viewport()->setClearColor(Color4f(0.0, 0.0, 0.0, 0));

            // Mix rendering
            {
                ref<Sampler> sampler = new Sampler;
                sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
                sampler->setMinFilter(cvf::Sampler::NEAREST);
                sampler->setMagFilter(cvf::Sampler::NEAREST);

                SingleQuadRenderingGenerator quadRenderGen("MixRendering");
                quadRenderGen.addTexture(blurredColorTexture.p(), sampler.p(), "u_texture2DRect");
                quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromFile("HighlightMix_Frag"));
                //quadRenderGen.setRenderState(new cvf::RenderStateDepth(true, RenderStateDepth::LESS, false));
                m_mixRendering = quadRenderGen.generate();
                m_mixRendering->setClearMode(Viewport::DO_NOT_CLEAR);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighterExperiment::addRenderingsToSequence(cvf::RenderSequence* renderSequence)
{
    renderSequence->addRendering(m_selectedRendering.p());
    renderSequence->addRendering(m_blurRendering.p());
    renderSequence->addRendering(m_mixRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighterExperiment::resize(cvf::uint width, cvf::uint height)
{
    int fboWidth  = width;
    int fboHeight = height;

    m_selectedFbo->resizeAttachedBuffers(fboWidth, fboHeight);
    m_blurFbo->resizeAttachedBuffers(fboWidth, fboHeight);

    m_selectedRendering->camera()->setViewport(0, 0, fboWidth, fboHeight);
    m_blurRendering->camera()->setViewport(0, 0, fboWidth, fboHeight);
    m_mixRendering->camera()->setViewport(0, 0, fboWidth, fboHeight);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartHighlighterExperiment::prepareForRedraw()
{
    m_selectedCamera->setViewMatrix(m_mainCamera->viewMatrix());
}



//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Highlight::onInitialize()
{
    // Create the main model
    {
        m_mainModel = new ModelBasicList;

        {
            ref<DrawableGeo> geo;
            WavefrontObjImport imp;
            //imp.readFile(m_testDataDir + "dragon_10k.obj");
            imp.readFile(m_testDataDir + "teapot.obj");
            GeometryBuilderDrawableGeo builder;
            imp.buildGeometry(&builder);
            geo = builder.drawableGeo();
            geo->weldVertices(0.00001);
            geo->computeNormals();

            ShaderProgramGenerator spGen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
            spGen.configureStandardHeadlightColor();
            ref<ShaderProgram> prog  = spGen.generate();
            ref<Effect> eff = new Effect;
            eff->setShaderProgram(prog.p());
            eff->setUniform(new UniformFloat("u_color", Color4f(Color3::YELLOW, 0)));

            ref<Part> part = new Part;
            part->setDrawable(geo.p());
            part->setEffect(eff.p());
            m_mainModel->addPart(part.p());
        }

        {
            PartCompoundGenerator gen;
            gen.setPartDistribution(Vec3i(4, 4, 4));
            gen.setNumEffects(8);
            gen.useRandomEffectAssignment(false);
            gen.setExtent(Vec3f(6,6,6));
            gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));
            gen.setUseShaders(true);

            Collection<Part> parts;
            //gen.generateSpheres(20, 20, &parts);
            gen.generateBoxes(&parts);

            size_t i;
            for (i = 0; i < parts.size(); i++)
            {
                Part* part = parts[i].p();
                m_mainModel->addPart(part);
            }
        }

        m_mainModel->updateBoundingBoxesRecursive();
    }

    {
        m_selectedModel = new ModelBasicList;
        m_selectedModel->addPart(m_mainModel->part(0));
        //m_selectedModel->addPart(m_mainModel->part(20));
        m_selectedModel->updateBoundingBoxesRecursive();
    }

    BoundingBox bb = m_mainModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }


    // Main rendering
    {
        m_mainRendering = new Rendering;
        m_mainRendering->setClearMode(Viewport::CLEAR_COLOR_DEPTH_STENCIL);
        m_mainRendering->setCamera(m_camera.p());
        m_mainRendering->setScene(new Scene);
        m_mainRendering->scene()->addModel(m_mainModel.p());
    }

    //m_highlighter = new PartHighlighterExperiment(m_selectedModel.p(), m_camera.p());
    //m_highlighter = new PartHighlighterStencil(m_selectedModel.p(), m_camera.p());
    m_highlighter = new PartHighlighter(m_selectedModel.p(), m_camera.p());
    m_highlighter->setHighlightColor(Color3::YELLOW_GREEN);

    m_renderSequence->removeAllRenderings();
    m_renderSequence->addRendering(m_mainRendering.p());
    m_highlighter->addRenderingsToSequence(m_renderSequence.p());

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Highlight::onPaintEvent(PostEventAction* postEventAction)
{
    CVF_UNUSED(postEventAction);
    
    m_highlighter->prepareForRedraw();
    TestSnippet::onPaintEvent(postEventAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Highlight::onResizeEvent(int width, int height)
{
    m_mainRendering->camera()->setViewport(0, 0, width, height);

    m_highlighter->resize(0, 0, width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Highlight::onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent)
{
    if (buttonPressed == LeftButton && 
        (mouseEvent->modifiers().testFlag(ControlModifier) || mouseEvent->modifiers().testFlag(ShiftModifier)))
    {
        // If control wasn't down, clear current selection 
        if (!mouseEvent->modifiers().testFlag(ControlModifier))
        {
            m_selectedModel->removeAllParts();
        }

        ref<Part> selectedPart;

        ref<cvf::RayIntersectSpec> ris = m_mainRendering->rayIntersectSpecFromWindowCoordinates(mouseEvent->x(), mouseEvent->y());
        cvf::HitItemCollection hic;
        m_mainRendering->rayIntersect(*ris, &hic);
        if (hic.count() > 0)
        {
            hic.sort();
            const cvf::HitItem* item = hic.firstItem();
            selectedPart = const_cast<Part*>(item->part());
        }

        if (selectedPart.notNull())
        {
            Collection<Part> currParts;
            m_selectedModel->allParts(&currParts);
            if (currParts.contains(selectedPart.p()))
            {
                m_selectedModel->removePart(selectedPart.p());
            }
            else
            {
                m_selectedModel->addPart(selectedPart.p());
            }
        }
        
        m_selectedModel->updateBoundingBoxesRecursive();

        mouseEvent->setRequestedAction(REDRAW);

        return;
    }

    TestSnippet::onMousePressEvent(buttonPressed, mouseEvent);
}


//     {
//         m_selectedRendering = new Rendering;
//         m_selectedRendering->setTargetFrameBuffer(m_fbo.p());
//         m_selectedRendering->setCamera(m_camera.p());
//         m_selectedRendering->setClearMode(Viewport::DO_NOT_CLEAR);
//         m_selectedRendering->setScene(m_mainRendering->scene());
// 
//         ShaderProgramGenerator spGen("BalloonGlow", cvf::ShaderSourceProvider::instance());
//         spGen.addVertexCodeFromFile("Highlight_Vert");
//         spGen.addFragmentCodeFromFile("Highlight_Frag");
//         ref<ShaderProgram> ballonProg  = spGen.generate();
// 
//         ref<RenderStateBlending> blending = new RenderStateBlending;
//         blending->setFunction(RenderStateBlending::SRC_ALPHA, RenderStateBlending::ONE_MINUS_SRC_ALPHA);
//         blending->enableBlending(true);
// 
//         ref<Effect> balloonEff = new Effect;
//         balloonEff->setRenderState(blending.p());
//         balloonEff->setShaderProgram(ballonProg.p());
//         balloonEff->setRenderState(new cvf::RenderStateDepth(true, cvf::RenderStateDepth::LEQUAL));
// 
//         m_selectedRendering->setEffectOverride(balloonEff.p());
// 
//         m_renderSequence->addRendering(m_selectedRendering.p());
//     }


} // namespace snip

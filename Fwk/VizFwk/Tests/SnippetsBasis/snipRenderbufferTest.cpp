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

#include "snipRenderbufferTest.h"

#include "cvfuSampleFactory.h"
#include "cvfuPartCompoundGenerator.h"


namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderbufferTest::RenderbufferTest()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderbufferTest::onInitialize()
{
    if (!m_openGLContext->capabilities()->hasCapability(OpenGLCapabilities::FRAMEBUFFER_OBJECT))
    {
        return false;
    }

    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);

    ref<ModelBasicList> myModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        myModel->addPart(parts[i].p());
    }

    myModel->updateBoundingBoxesRecursive();

    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    //  Setup first rendering with FBO
    // -------------------------------------------------------------------------
    // Offscreen rendering needs a separate camera as the viewport size is different
    ref<Camera> firstRenderingCamera = new Camera;
    m_renderSequence->firstRendering()->setCamera(firstRenderingCamera.p());

    m_fbo = new FramebufferObject;
    m_renderSequence->firstRendering()->setTargetFramebuffer(m_fbo.p());

    ref<RenderbufferObject> rbo = new RenderbufferObject(RenderbufferObject::DEPTH_COMPONENT24, 1, 1);
    m_fbo->attachDepthRenderbuffer(rbo.p());

    ref<Texture> texture = new Texture(Texture::TEXTURE_2D, Texture::RGBA);
    texture->setSize(1, 1);
    m_fbo->attachColorTexture2d(0, texture.p());

    // Setup second rendering drawing the texture on the screen
    // -------------------------------------------------------------------------
    SingleQuadRenderingGenerator quadRenderGen;
    ref<Sampler> sampler = new Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    quadRenderGen.addTexture(texture.p(), sampler.p(), "u_texture2D");
    quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::fs_Unlit));
    quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::src_Texture));

    ref<Rendering> quadRendering = quadRenderGen.generate();
    m_renderSequence->addRendering(quadRendering.p());

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderbufferTest::onPaintEvent(PostEventAction* postEventAction)
{
    // Offscreen rendering needs a separate camera as the viewport size is different
    ref<Rendering> offscreenRendering = m_renderSequence->firstRendering();
    offscreenRendering->camera()->setViewMatrix(m_camera->viewMatrix());

    TestSnippet::onPaintEvent(postEventAction);
    return;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderbufferTest::onResizeEvent(int width, int height)
{
    ref<Rendering> offscreenRendering = m_renderSequence->firstRendering();
    ref<Rendering> quadRendering = m_renderSequence->rendering(1);

    int fboWidth  = width/4;
    int fboHeight = height/4;

    offscreenRendering->camera()->setViewport(0, 0, fboWidth, fboHeight);
    m_fbo->resizeAttachedBuffers(fboWidth, fboHeight);

    quadRendering->camera()->viewport()->set(0, 0, width, height);

    TestSnippet::onResizeEvent(width, height);
}


} // namespace snip

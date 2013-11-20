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

#include "snipRenderToTexture.h"

#include "cvfuInputEvents.h"
#include "cvfuSampleFactory.h"
#include "cvfuImageJpeg.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderToTexture::createModels()
{
    // Create model to render to a texture
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);

    m_useShaders = false;

    ShaderProgramGenerator spGen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    spGen.configureStandardHeadlightColor();
    m_shaderProg  = spGen.generate();

    m_textureGenerationModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        Part* part = parts[i].p();

        ref<Effect> eff = part->effect();
        if (m_useShaders)
        {
            eff->setShaderProgram(m_shaderProg.p());
        }

        m_textureGenerationModel->addPart(parts[i].p());
    }

    m_textureGenerationModel->updateBoundingBoxesRecursive();


    // Use a common sampler for both textures
    m_sampler = new Sampler;
    m_sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    m_sampler->setMinFilter(cvf::Sampler::LINEAR_MIPMAP_LINEAR);
    m_sampler->setMagFilter(cvf::Sampler::LINEAR);

    // Create the texture to render onto and to show on the part
    m_renderedTexture = new Texture(Texture::TEXTURE_2D, Texture::RGBA);
    m_renderedTexture->enableMipmapGeneration(true);

    // Part with rendered texture
    m_geometryModel = new ModelBasicList;
    ref<Part> part = SampleFactory::createTexturedQuad(m_renderedTexture.p(), m_sampler.p(), 1.0f);
    part->effect()->setRenderState(new RenderStateCullFace(true, RenderStateCullFace::BACK));
    m_geometryModel->addPart(part.p());


    // Part with static texture
    ref<TextureImage> img = cvfu::ImageJpeg::loadImage(m_testDataDir + "Koala.jpg");
    m_staticTexture = new Texture(img.p());
    m_staticTexture->enableMipmapGeneration(true);

    ref<Part> staticTexturePart = SampleFactory::createTexturedQuad(m_staticTexture.p(), m_sampler.p(), 1.0f);
    staticTexturePart->effect()->setRenderState(new RenderStateCullFace(true, RenderStateCullFace::FRONT));
    m_geometryModel->addPart(staticTexturePart.p());

    m_geometryModel->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderToTexture::onInitialize()
{
    // Create models
    createModels();

    m_renderSequence->removeAllRenderings();

    // Setup first rendering
    {
        const cvf::uint offscreenWidth = 1024;
        const cvf::uint offscreenHeight = 1024;

        Scene* scene = new Scene;
        m_offscreenRendering = new Rendering;
        m_offscreenRendering->setScene(scene);
        m_renderSequence->addRendering(m_offscreenRendering.p());
        scene->addModel(m_textureGenerationModel.p());

        Camera* cam = m_offscreenRendering->camera();
        cam->setViewport(0, 0, offscreenWidth, offscreenHeight);
        cam->viewport()->setClearColor(Color4f(Color3::RED));

        BoundingBox bb = m_textureGenerationModel->boundingBox();
        if (bb.isValid())
        {
            m_offscreenRendering->camera()->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
        }

        // Setup FBO
        m_fbo = new FramebufferObject;
        m_offscreenRendering->setTargetFramebuffer(m_fbo.p());

        ref<RenderbufferObject> rbo = new RenderbufferObject(RenderbufferObject::DEPTH_COMPONENT24, offscreenWidth, offscreenHeight);
        m_fbo->attachDepthRenderbuffer(rbo.p());

        m_renderedTexture->setSize(offscreenWidth, offscreenHeight);
        m_fbo->attachColorTexture2d(0, m_renderedTexture.p());
    }

    // Setup second rendering
    {
        Scene* scene = new Scene;
        m_mainRendering = new Rendering;
        m_mainRendering->setScene(scene);
        m_mainRendering->setCamera(m_camera.p());
        m_renderSequence->addRendering(m_mainRendering.p());
        scene->addModel(m_geometryModel.p());

        m_mainRendering->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

        BoundingBox bb = m_geometryModel->boundingBox();
        if (bb.isValid())
        {
            m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
            m_trackball->setRotationPoint(bb.center());
        }
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderToTexture::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->character() == 's')
    {
        m_useShaders = true;
    }

    if (keyEvent->character() == 'f')
    {
        m_useShaders = false;
    }

    if (keyEvent->character() == 'm')
    {
        m_staticTexture->enableMipmapGeneration(true);
        m_renderedTexture->enableMipmapGeneration(true);
        m_sampler->setMinFilter(Sampler::LINEAR_MIPMAP_LINEAR);
    }

    if (keyEvent->character() == 'M')
    {
        m_staticTexture->enableMipmapGeneration(false);
        m_renderedTexture->enableMipmapGeneration(false);
        m_sampler->setMinFilter(Sampler::LINEAR);
    }

    Collection<Part> partCollection;
    m_textureGenerationModel->allParts(&partCollection);

    size_t i;
    for (i = 0; i < partCollection.size(); i++)
    {
        ref<Part> part = partCollection[i];
        CVF_ASSERT(part.notNull());

        part->effect()->setShaderProgram(m_useShaders ? m_shaderProg.p() : NULL);
    }

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> RenderToTexture::helpText() const
{
    std::vector<cvf::String> helpText;

    helpText.push_back("'s' to use a shader program for rendering");
    helpText.push_back("'f' to use fixed function pipeline for rendering");
    helpText.push_back("'m/M' enable/disable mipmaps");

    return helpText;

}


// void RenderToTexture::onPaintEvent(PostEventAction* postEventAction)
// {
//     TestSnippet::onPaintEvent(postEventAction);
// 
//     static int i = 0;
// 
//     if (i == 0)
//     {
//         m_renderedTexture->bind();
//         GLubyte* pixels = new GLubyte[1024*1024*4];
//         glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA,  GL_UNSIGNED_BYTE, pixels);
// 
//         ref<cvf::TextureImage> img = new cvf::TextureImage;
// 
//         img->setData(pixels, 1024, 1024);
// 
//         cvfu::ImageJpeg::saveImage(*img, "c:\\buf\\test.jpg");
//     }
// }

} // namespace snip

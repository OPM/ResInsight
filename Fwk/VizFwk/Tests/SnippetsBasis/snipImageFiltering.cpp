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

#include "snipImageFiltering.h"

#include "cvfuWavefrontObjImport.h"
#include "cvfuSnippetPropertyPublisher.h"

namespace snip {



//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
GaussianBlurExperiment::GaussianBlurExperiment(Texture* textureToBlur)
:   m_textureToBlur(textureToBlur)
{
    CVF_ASSERT(m_textureToBlur.notNull());

    createRenderings();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GaussianBlurExperiment::createRenderings()
{
    CVF_ASSERT(m_textureToBlur.notNull());

    // Determine if we should be using texture rectangle or normal texture
    // If the input texture's type is texture rectangle, we'll use texture rectangle
    bool useTextureRect = (m_textureToBlur->textureType() == Texture::TEXTURE_RECTANGLE);

    m_tempColorTexture = new Texture(useTextureRect ? Texture::TEXTURE_RECTANGLE : Texture::TEXTURE_2D, Texture::RGBA);
    m_tempColorTexture->setSize(1, 1);

    m_depthRbo = new RenderbufferObject(RenderbufferObject::DEPTH_COMPONENT24, 1, 1);


    // !!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!
    int kernelSize = 15;
    float sigma = 7;//4.95f;//static_cast<float>(kernelSize - 1)/6.0f;

    String defineString = String("#define KERNEL_SIZE %1\n").arg(kernelSize);
    if (useTextureRect)
    {
        defineString += "#define USE_TEXTURE_RECT\n";
    }


    // First blur pass (vertical)
    {
        ref<FramebufferObject> vertBlurFbo = new FramebufferObject;
        vertBlurFbo->attachDepthRenderbuffer(m_depthRbo.p());
        vertBlurFbo->attachColorTexture2d(0, m_tempColorTexture.p());

        ref<Sampler> sampler = new Sampler;
        sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
        sampler->setMinFilter(cvf::Sampler::NEAREST);
        sampler->setMagFilter(cvf::Sampler::NEAREST);

        SingleQuadRenderingGenerator quadRenderGen("VertBlurRendering");
        quadRenderGen.addTexture(m_textureToBlur.p(), sampler.p(), "u_blurSampler");
        quadRenderGen.addFragmentShaderCode(defineString);
        quadRenderGen.addFragmentShaderCode("#define VERTICAL_BLUR\n");
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromFile("ImageFilteringBlur_Frag"));
        quadRenderGen.setUniform(new UniformFloat("u_sigma", sigma));

        if (!useTextureRect)
        {
            m_vertBlurSize = new UniformFloat("u_blurSize", 1);
            quadRenderGen.setUniform(m_vertBlurSize.p());
        }

        m_vertBlurRendering = quadRenderGen.generate();
        m_vertBlurRendering->setTargetFramebuffer(vertBlurFbo.p());
    }

    // Second blur pass (horizontal)
    {
        ref<FramebufferObject> horBlurFbo = new FramebufferObject;
        horBlurFbo->attachDepthRenderbuffer(m_depthRbo.p());
        horBlurFbo->attachColorTexture2d(0, m_textureToBlur.p());

        ref<Sampler> sampler = new Sampler;
        sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
        sampler->setMinFilter(cvf::Sampler::NEAREST);
        sampler->setMagFilter(cvf::Sampler::NEAREST);

        SingleQuadRenderingGenerator quadRenderGen("HorBlurRendering");
        quadRenderGen.addTexture(m_tempColorTexture.p(), sampler.p(), "u_blurSampler");
        quadRenderGen.addFragmentShaderCode(defineString);
        quadRenderGen.addFragmentShaderCode("#define HORIZONTAL_BLUR\n");
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromFile("ImageFilteringBlur_Frag"));
        quadRenderGen.setUniform(new UniformFloat("u_sigma", sigma));

        if (!useTextureRect)
        {
            m_horBlurSize = new UniformFloat("u_blurSize", 1);
            quadRenderGen.setUniform(m_horBlurSize.p());
        }

        m_horBlurRendering = quadRenderGen.generate();
        m_horBlurRendering->setTargetFramebuffer(horBlurFbo.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GaussianBlurExperiment::addRenderingsToSequence(RenderSequence* renderSequence)
{
    CVF_ASSERT(renderSequence);
    CVF_ASSERT(m_vertBlurRendering.notNull());
    CVF_ASSERT(m_horBlurRendering.notNull());
    renderSequence->addRendering(m_vertBlurRendering.p());
    renderSequence->addRendering(m_horBlurRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// Resizes the internal worker objects based on the size of the texture that is to be blurred
///
/// Must be called whenever the size of the texture changes
//--------------------------------------------------------------------------------------------------
void GaussianBlurExperiment::resizeFromTextureSize()
{
    CVF_ASSERT(m_textureToBlur.notNull());
    cvf::uint width = m_textureToBlur->width();
    cvf::uint height = m_textureToBlur->height();

    m_tempColorTexture->setSize(width, height);
    m_depthRbo->setSize(width, height);

    m_vertBlurRendering->camera()->setViewport(0, 0, width, height);
    m_horBlurRendering->camera()->setViewport(0, 0, width, height);

    if (m_vertBlurSize.notNull() && m_horBlurSize.notNull())
    {
        // The size here should be the size of the texture that is read from in the pass
        m_vertBlurSize->set(1.0f/static_cast<float>(width));
        m_horBlurSize->set(1.0f/static_cast<float>(height));
    }
}




//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ImageFiltering::onInitialize()
{
    m_propGaussKernelSize = new PropertyInt("Gauss kernel size", 25);
    m_propGaussKernelSize->setRange(3, 999);
    m_propGaussKernelSize->setGuiStep(2);
    m_propertyPublisher->publishProperty(m_propGaussKernelSize.p());
    m_propGaussSigma = new PropertyDouble("Gauss sigma", 12);
    m_propGaussSigma->setRange(0.1, 9999.0);
    m_propGaussSigma->setGuiStep(1.0);
    m_propertyPublisher->publishProperty(m_propGaussSigma.p());


    // Create the main model
    {
        m_mainModel = new ModelBasicList;

        {
            ref<DrawableGeo> geo;
            WavefrontObjImport imp;
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
            eff->setUniform(new UniformFloat("u_color", Color4f(Color3::YELLOW_GREEN, 0)));

            ref<Part> part = new Part;
            part->setDrawable(geo.p());
            part->setEffect(eff.p());
            m_mainModel->addPart(part.p());
        }

        m_mainModel->updateBoundingBoxesRecursive();
    }

    BoundingBox bb = m_mainModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }


    m_mainColorTexture = new Texture(Texture::TEXTURE_2D, Texture::RGBA);
    //ref<Texture> mainColorTexture = new Texture(Texture::TEXTURE_RECTANGLE, Texture::RGBA);
    m_mainColorTexture->setSize(1, 1);
    m_mainFbo = new FramebufferObject;
    m_mainFbo->attachDepthRenderbuffer(new RenderbufferObject(RenderbufferObject::DEPTH_COMPONENT24, 1, 1));
    m_mainFbo->attachColorTexture2d(0, m_mainColorTexture.p());

    // Main rendering
    {
        m_mainRendering = new Rendering;
        m_mainRendering->setTargetFramebuffer(m_mainFbo.p());
        m_mainRendering->setCamera(m_camera.p());
        m_mainRendering->setScene(new Scene);
        m_mainRendering->scene()->addModel(m_mainModel.p());
    }


    // Sobel rendering
//     {
//         ref<Sampler> sampler = new Sampler;
//         sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
//         sampler->setMinFilter(cvf::Sampler::NEAREST);
//         sampler->setMagFilter(cvf::Sampler::NEAREST);
// 
//         SingleQuadRenderingGenerator quadRenderGen("FinalRendering");
//         quadRenderGen.addTexture(mainColorTexture.p(), sampler.p(), "u_texture2D");
//         quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromFile("ImageFiltering_Frag"));
//         quadRenderGen.setUniform(new UniformFloat("EdgeThreshold", 0.5f*0.5f));
//         m_finalRendering = quadRenderGen.generate();
//     }


    // Straight rendering to screen
    {
        ref<Sampler> sampler = new Sampler;
        sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
        sampler->setMinFilter(cvf::Sampler::NEAREST);
        sampler->setMagFilter(cvf::Sampler::NEAREST);

        SingleQuadRenderingGenerator quadRenderGen("FinalRendering");
        quadRenderGen.addTexture(m_mainColorTexture.p(), sampler.p(), "u_texture2D");
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::src_Texture));
        //quadRenderGen.addTexture(mainColorTexture.p(), sampler.p(), "u_texture2DRect");
        //quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::src_TextureRectFromFragCoord_v33));
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::fs_Unlit));
        m_finalRendering = quadRenderGen.generate();
    }

    m_renderSequence->removeAllRenderings();
    m_renderSequence->addRendering(m_mainRendering.p());

    //m_blur = new GaussianBlurExperiment(mainColorTexture.p());
    m_blur = new GaussianBlur(m_mainColorTexture.p(), m_propGaussKernelSize->value(), m_propGaussSigma->value());
    m_blur->addRenderingsToSequence(m_renderSequence.p());

    m_renderSequence->addRendering(m_finalRendering.p());

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ImageFiltering::onPaintEvent(PostEventAction* postEventAction)
{
    TestSnippet::onPaintEvent(postEventAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ImageFiltering::onResizeEvent(int width, int height)
{
    m_mainFbo->resizeAttachedBuffers(width, height);

    m_mainRendering->camera()->setViewport(0, 0, width, height);
    m_finalRendering->camera()->setViewport(0, 0, width, height);

    // Must be done after the texture that is to be blurred has been resized
    if (m_blur.notNull())
    {
        m_blur->resizeFromTextureSize();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ImageFiltering::onPropertyChanged(Property* property, PostEventAction* postEventAction)
{
    CVF_UNUSED(property);

    if (m_blur.notNull())
    {
        if (property == m_propGaussKernelSize)
        {
            m_blur->removeRenderingsFromSequence(m_renderSequence.p());
            m_blur = new GaussianBlur(m_mainColorTexture.p(), m_propGaussKernelSize->value(), m_propGaussSigma->value());
            m_blur->resizeFromTextureSize();
            
            RenderSequence dummySeq;
            m_blur->addRenderingsToSequence(&dummySeq);
            m_renderSequence->insertRendering(m_finalRendering.p(), dummySeq.rendering((0)));
            m_renderSequence->insertRendering(m_finalRendering.p(), dummySeq.rendering((1)));
        }

        if (property == m_propGaussSigma)
        {
            m_blur->setSigma(m_propGaussSigma->value());
        }
    }


    *postEventAction = REDRAW;
}




} // namespace snip

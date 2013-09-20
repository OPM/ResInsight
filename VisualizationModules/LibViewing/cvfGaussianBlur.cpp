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
#include "cvfGaussianBlur.h"
#include "cvfTexture.h"
#include "cvfSampler.h"
#include "cvfFramebufferObject.h"
#include "cvfRenderbufferObject.h"
#include "cvfString.h"
#include "cvfSingleQuadRenderingGenerator.h"
#include "cvfShaderSourceProvider.h"
#include "cvfUniform.h"
#include "cvfRenderSequence.h"
#include "cvfCamera.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::GaussianBlur
/// \ingroup Viewing
///
/// Helper to create and manage two pass Gaussian blur of a texture.
///
/// The sigma value is the standard deviation for the Gaussian distribution. A higher sigma value 
/// means more blur, but note that the sigma value should be tuned to the kernel size.
/// The gaussian distribution will have the vast majority of values in the interval [-3*sigma, 3*sigma], 
/// so a kernel width of 6*sigma should suffice. Note that this guideline is very defensive and will
/// require a large kernel for significant blur. In practice, you can get away with a larger sigma value
/// for many of our applications. See more documentation in fs_GaussianBlur.glsl
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
GaussianBlur::GaussianBlur(Texture* textureToBlur, int kernelSize, double sigma)
:   m_textureToBlur(textureToBlur),
    m_kernelSize(kernelSize),
    m_sigma(sigma)
{
    CVF_ASSERT(m_textureToBlur.notNull());
    CVF_ASSERT(m_kernelSize >= 3);
    CVF_ASSERT(sigma > 0);

    createRenderings();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GaussianBlur::createRenderings()
{
    CVF_ASSERT(m_textureToBlur.notNull());

    // Determine if we should be using texture rectangle or normal texture
    // If the input texture's type is texture rectangle, we'll use texture rectangle
    bool useTextureRect = (m_textureToBlur->textureType() == Texture::TEXTURE_RECTANGLE);

    m_tempColorTexture = new Texture(useTextureRect ? Texture::TEXTURE_RECTANGLE : Texture::TEXTURE_2D, Texture::RGBA);
    m_tempColorTexture->setSize(1, 1);

    String defineString = String("#define KERNEL_SIZE %1\n").arg(m_kernelSize);
    if (useTextureRect)
    {
        defineString += "#define USE_TEXTURE_RECT\n";
    }

    m_uniformSigma = new UniformFloat("u_sigma", static_cast<float>(m_sigma));

    ref<Sampler> sampler = new Sampler;
    sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(cvf::Sampler::NEAREST);
    sampler->setMagFilter(cvf::Sampler::NEAREST);

    // First blur pass (vertical)
    {
        ref<FramebufferObject> vertBlurFbo = new FramebufferObject;
        vertBlurFbo->attachColorTexture2d(0, m_tempColorTexture.p());

        SingleQuadRenderingGenerator quadRenderGen("VertBlurRendering");
        quadRenderGen.addTexture(m_textureToBlur.p(), sampler.p(), "u_blurSampler");
        quadRenderGen.addFragmentShaderCode(defineString);
        quadRenderGen.addFragmentShaderCode("#define VERTICAL_BLUR\n");
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::fs_GaussianBlur));
        quadRenderGen.setUniform(m_uniformSigma.p());

        if (!useTextureRect)
        {
            m_uniformVertBlurSize = new UniformFloat("u_blurSize", 1);
            quadRenderGen.setUniform(m_uniformVertBlurSize.p());
        }

        m_vertBlurRendering = quadRenderGen.generate();
        m_vertBlurRendering->setTargetFramebuffer(vertBlurFbo.p());
    }

    // Second blur pass (horizontal)
    {
        ref<FramebufferObject> horBlurFbo = new FramebufferObject;
        horBlurFbo->attachColorTexture2d(0, m_textureToBlur.p());

        SingleQuadRenderingGenerator quadRenderGen("HorBlurRendering");
        quadRenderGen.addTexture(m_tempColorTexture.p(), sampler.p(), "u_blurSampler");
        quadRenderGen.addFragmentShaderCode(defineString);
        quadRenderGen.addFragmentShaderCode("#define HORIZONTAL_BLUR\n");
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::fs_GaussianBlur));
        quadRenderGen.setUniform(m_uniformSigma.p());

        if (!useTextureRect)
        {
            m_uniformHorBlurSize = new UniformFloat("u_blurSize", 1);
            quadRenderGen.setUniform(m_uniformHorBlurSize.p());
        }

        m_horBlurRendering = quadRenderGen.generate();
        m_horBlurRendering->setTargetFramebuffer(horBlurFbo.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GaussianBlur::setSigma(double sigma)
{
    CVF_ASSERT(sigma > 0);
    m_sigma = sigma;

    CVF_ASSERT(m_uniformSigma.notNull());
    m_uniformSigma->set(static_cast<float>(sigma));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GaussianBlur::addRenderingsToSequence(RenderSequence* renderSequence)
{
    CVF_ASSERT(renderSequence);
    CVF_ASSERT(m_vertBlurRendering.notNull());
    CVF_ASSERT(m_horBlurRendering.notNull());
    renderSequence->addRendering(m_vertBlurRendering.p());
    renderSequence->addRendering(m_horBlurRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GaussianBlur::removeRenderingsFromSequence(RenderSequence* renderSequence)
{
    CVF_ASSERT(renderSequence);
    if (m_vertBlurRendering.notNull()) renderSequence->removeRendering(m_vertBlurRendering.p());
    if (m_horBlurRendering.notNull())  renderSequence->removeRendering(m_horBlurRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// Resizes the internal worker objects based on the size of the texture that is to be blurred
///
/// Must be called whenever the size of the texture changes
//--------------------------------------------------------------------------------------------------
void GaussianBlur::resizeFromTextureSize()
{
    CVF_ASSERT(m_textureToBlur.notNull());
    cvf::uint width = m_textureToBlur->width();
    cvf::uint height = m_textureToBlur->height();

    m_tempColorTexture->setSize(width, height);

    m_vertBlurRendering->camera()->setViewport(0, 0, width, height);
    m_horBlurRendering->camera()->setViewport(0, 0, width, height);

    if (m_uniformVertBlurSize.notNull() && m_uniformHorBlurSize.notNull())
    {
        // The size here should be the size of the texture that is read from in the pass
        m_uniformVertBlurSize->set(1.0f/static_cast<float>(height));
        m_uniformHorBlurSize->set(1.0f/static_cast<float>(width));
    }
}


}

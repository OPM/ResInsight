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
#include "cvfLibGeometry.h"
#include "cvfLibViewing.h"
#include "cvfLibRender.h"

#include "snipStencil.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Stencil::Stencil()
:   m_method(NORMAL_STENCIL)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Stencil::onInitialize()
{
    // Create the model that will populate the stencil buffer
    {
        m_stencilModel = new ModelBasicList;

        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createSphere(0.35, 50, 50, &builder);
        ref<DrawableGeo> geo = builder.drawableGeo();
        
        ref<RenderStateStencil> rsStencil = new RenderStateStencil;
        rsStencil->setFunction(RenderStateStencil::ALWAYS, 1, 0xffffffff);
        rsStencil->setOperation(RenderStateStencil::KEEP, RenderStateStencil::REPLACE, RenderStateStencil::REPLACE);
        rsStencil->enableStencilTest(true);

        ref<Effect> eff = new Effect;
        eff->setRenderState(rsStencil.p());
        eff->setRenderState(new RenderStateMaterial_FF(Color3f::MAGENTA));
        eff->setRenderState(new RenderStateColorMask(false));
        eff->setRenderState(new RenderStateDepth(true, RenderStateDepth::LESS, false));

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());

        m_stencilModel->addPart(part.p());
        m_stencilModel->updateBoundingBoxesRecursive();
    }

    // Create the main model
    {
        m_mainModel = new ModelBasicList;

        PartCompoundGenerator gen;
        gen.setPartDistribution(Vec3i(4, 4, 4));
        gen.setNumEffects(8);
        gen.useRandomEffectAssignment(false);
        gen.setExtent(Vec3f(3,3,3));
        gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));
        gen.setUseShaders(true);

        Collection<Part> parts;
        gen.generateSpheres(20, 20, &parts);

        size_t i;
        for (i = 0; i < parts.size(); i++)
        {
            ref<RenderStateStencil> s = new RenderStateStencil;
            s->setFunction(RenderStateStencil::EQUAL, 1, 0xffffffff);
            s->enableStencilTest(true);

            Part* part = parts[i].p();
            part->effect()->setRenderState(s.p());
            m_mainModel->addPart(part);
        }

        m_mainModel->updateBoundingBoxesRecursive();
    }

    createAndConfigureRenderings();

    BoundingBox bb = m_mainModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Stencil::createAndConfigureRenderings()
{
    CVF_ASSERT(m_stencilModel.notNull());
    CVF_ASSERT(m_mainModel.notNull());

    // Release these in case we don't use FBO
    m_fbo = NULL;
    m_fullScreenQuadRendering = NULL;

    // The colorbuffer texture in case we use FBO
    ref<Texture> colorTexture;

    if (m_method == FBO_RENDERBUFFER || m_method == FBO_TEXTURE)
    {
        // Set up the FBO to be used as target for stencil and main rendering
        m_fbo = new FramebufferObject;
        colorTexture = new Texture(Texture::TEXTURE_2D, Texture::RGBA);
        colorTexture->setSize(1, 1);
        m_fbo->attachColorTexture2d(0, colorTexture.p());

        if (m_method == FBO_RENDERBUFFER)
        {
            ref<RenderbufferObject> depthStencilRBO = new RenderbufferObject(RenderbufferObject::DEPTH24_STENCIL8, 1, 1);
            m_fbo->attachDepthStencilRenderbuffer(depthStencilRBO.p());
        }
        else
        {
            ref<Texture> depthStencilTexture = new Texture(Texture::TEXTURE_2D, Texture::DEPTH24_STENCIL8);
            m_fbo->attachDepthStencilTexture2d(depthStencilTexture.p());
        }
    }


    // Stencil rendering
    {
        m_stencilRendering = new Rendering;
        m_stencilRendering->setClearMode(Viewport::CLEAR_COLOR_DEPTH_STENCIL);
        m_stencilRendering->setCamera(new Camera);
        m_stencilRendering->camera()->setProjectionAsUnitOrtho();
        m_stencilRendering->setScene(new Scene);
        m_stencilRendering->scene()->addModel(m_stencilModel.p());

        if (m_fbo.notNull())
        {
            m_stencilRendering->setTargetFramebuffer(m_fbo.p());
        }
    }

    // Main rendering
    {
        m_mainRendering = new Rendering;
        m_mainRendering->setClearMode(Viewport::DO_NOT_CLEAR);
        m_mainRendering->setCamera(m_camera.p());
        m_mainRendering->setScene(new Scene);
        m_mainRendering->scene()->addModel(m_mainModel.p());
        if (m_fbo.notNull())
        {
            m_mainRendering->setTargetFramebuffer(m_fbo.p());
        }

        // Add a text box with current method
        {
            cvf::String txt = "Current method: ";
            if      (m_method == NORMAL_STENCIL)    txt += "NORMAL_STENCIL";
            else if (m_method == FBO_RENDERBUFFER)  txt += "FBO_RENDERBUFFER";
            else if (m_method == FBO_TEXTURE)       txt += "FBO_TEXTURE";

            ref<Font> font = new FixedAtlasFont(FixedAtlasFont::LARGE);
            ref<OverlayTextBox> textBox = new OverlayTextBox(font.p());
            textBox->setLayout(OverlayItem::VERTICAL, OverlayItem::TOP_LEFT);
            textBox->setText(txt);
            textBox->setSizeToFitText();
            m_mainRendering->addOverlayItem(textBox.p());
        }
    }

    // Setup rendering drawing the texture on the screen
    if (colorTexture.notNull())
    {
        ref<Sampler> sampler = new Sampler;
        sampler->setWrapMode(cvf::Sampler::CLAMP_TO_EDGE);
        sampler->setMinFilter(cvf::Sampler::NEAREST);
        sampler->setMagFilter(cvf::Sampler::NEAREST);

        SingleQuadRenderingGenerator quadRenderGen;
        quadRenderGen.addTexture(colorTexture.p(), sampler.p(), "u_texture2D");
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::fs_Unlit));
        quadRenderGen.addFragmentShaderCode(ShaderSourceProvider::instance()->getSourceFromRepository(ShaderSourceRepository::src_Texture));
        m_fullScreenQuadRendering = quadRenderGen.generate();
    }

    // Build the render sequence we want
    m_renderSequence->removeAllRenderings();
    m_renderSequence->addRendering(m_stencilRendering.p());
    m_renderSequence->addRendering(m_mainRendering.p());
    if (m_fullScreenQuadRendering.notNull())
    {
        m_renderSequence->addRendering(m_fullScreenQuadRendering.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Stencil::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->character() == 'n')
    {
        m_method = NORMAL_STENCIL;
    }
    if (keyEvent->character() == 'r')
    {
        m_method = FBO_RENDERBUFFER;
    }
    if (keyEvent->character() == 't')
    {
        m_method = FBO_TEXTURE;
    }

    createAndConfigureRenderings();

    // Trigger a resize
    onResizeEvent(m_camera->viewport()->width(), m_camera->viewport()->height());

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Stencil::onResizeEvent(int width, int height)
{
    if (m_fbo.notNull())
    {
        m_fbo->resizeAttachedBuffers(width, height);
    }

    m_stencilRendering->camera()->setViewport(0, 0, width, height);
    m_mainRendering->camera()->setViewport(0, 0, width, height);

    if (m_fullScreenQuadRendering.notNull())
    {
        m_fullScreenQuadRendering->camera()->viewport()->set(0, 0, width, height);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> Stencil::helpText() const
{
    std::vector<cvf::String> helpText;
    helpText.push_back("'n' NORMAL_STENCIL");
    helpText.push_back("'r' FBO_RENDERBUFFER");
    helpText.push_back("'t' FBO_TEXTURE");

    return helpText;
}


} // namespace snip

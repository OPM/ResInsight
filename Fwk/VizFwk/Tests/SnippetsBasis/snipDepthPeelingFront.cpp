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

#include "snipDepthPeelingFront.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DepthPeelingFront::DepthPeelingFront()
{
    m_numPasses = 4;
    m_opacity = 0.6f;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DepthPeelingFront::~DepthPeelingFront()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DepthPeelingFront::onInitialize()
{
#ifdef NODEF
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20,20, &parts);
    //gen.generateSpheres(70,70, &parts);

    m_transparentModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        m_transparentModel->addPart(parts[i].p());
    }

    m_transparentModel->updateBoundingBox();

    m_renderSequence->rendering(0)->scene()->addModel(m_transparentModel.p());

    BoundingBox bb = m_transparentModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }
#else
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(2, 2, 2));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateBoxes(&parts);
    //gen.generateSpheres(70,70, &parts);

    m_transparentModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        m_transparentModel->addPart(parts[i].p());
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
#endif

    setupShaders();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeelingFront::setupShaders()
{
    // Init shader
    // -------------------------------------------------------------------------
    ShaderProgramGenerator initGen("Init", ShaderSourceProvider::instance());
    initGen.addVertexCodeFromFile("DepthPeelingFront_InitVert");
    initGen.addFragmentCodeFromFile("DepthPeeling_ShadeFragment");
    initGen.addFragmentCodeFromFile("DepthPeelingFront_InitFrag");
    m_progInit = initGen.generate();

    // Peel shader
    // -------------------------------------------------------------------------
    ShaderProgramGenerator peelGen("Peel", ShaderSourceProvider::instance());
    peelGen.addVertexCodeFromFile("DepthPeelingFront_PeelVert");
    peelGen.addFragmentCodeFromFile("DepthPeeling_ShadeFragment");
    peelGen.addFragmentCodeFromFile("DepthPeelingFront_PeelFrag");
    m_progPeel = peelGen.generate();

    // Blend shader
    // -------------------------------------------------------------------------
    ShaderProgramGenerator blendGen("Blend", ShaderSourceProvider::instance());
    blendGen.addVertexCodeFromFile("DepthPeelingFront_BlendVert");
    blendGen.addFragmentCodeFromFile("DepthPeelingFront_BlendFrag");
    m_progBlend = blendGen.generate();

    // Final shader
    // -------------------------------------------------------------------------
    ShaderProgramGenerator finalGen("Final", ShaderSourceProvider::instance());
    finalGen.addVertexCodeFromFile("DepthPeelingFront_FinalVert");
    finalGen.addFragmentCodeFromFile("DepthPeelingFront_FinalFrag");
    m_progFinal = finalGen.generate();

    // Shader to use for opaque objects
    // -------------------------------------------------------------------------
    ShaderProgramGenerator opGen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    opGen.configureStandardHeadlightColor();
    m_progOpaque = opGen.generate();

    m_progInit->linkProgram(m_openGLContext.p());
    m_progPeel->linkProgram(m_openGLContext.p());
    m_progBlend->linkProgram(m_openGLContext.p());
    m_progFinal->linkProgram(m_openGLContext.p());
    m_progOpaque->linkProgram(m_openGLContext.p());

    CVF_CHECK_OGL(m_openGLContext.p());

    cvf::Trace::show("m_progInit: \n" + m_progInit->programInfoLog(m_openGLContext.p()));
    cvf::Trace::show("m_progPeel: \n" + m_progPeel->programInfoLog(m_openGLContext.p()));
    cvf::Trace::show("m_progBlend:\n" + m_progBlend->programInfoLog(m_openGLContext.p()));
    cvf::Trace::show("m_progFinal:\n" + m_progFinal->programInfoLog(m_openGLContext.p()));
    cvf::Trace::show("m_progOpaque:\n" + m_progOpaque->programInfoLog(m_openGLContext.p()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeelingFront::initRenderTargets(int width, int height)
{
    CVF_CALLSITE_OPENGL(m_openGLContext.p());

    glGenTextures(2, m_frontDepthTexId);
    glGenTextures(2, m_frontColorTexId);
    glGenFramebuffers(2, m_frontFboId);

    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_frontDepthTexId[i]);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        CVF_CHECK_OGL(m_openGLContext.p());

        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_frontColorTexId[i]);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
        CVF_CHECK_OGL(m_openGLContext.p());

        glBindFramebuffer(GL_FRAMEBUFFER, m_frontFboId[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE_ARB, m_frontDepthTexId[i], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, m_frontColorTexId[i], 0);
        CVF_CHECK_OGL(m_openGLContext.p());
    }

    glGenTextures(1, &m_frontColorBlenderTexId);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_frontColorBlenderTexId);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    CVF_CHECK_OGL(m_openGLContext.p());

    glGenFramebuffers(1, &m_frontColorBlenderFboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frontColorBlenderFboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE_ARB, m_frontDepthTexId[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, m_frontColorBlenderTexId, 0);


    // Setup textures and frame/depth buffer for the opaque model
    // -------------------------------------------------------------------------
    glGenTextures(1, &m_solidModelDepthTexId);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_solidModelDepthTexId);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    CVF_CHECK_OGL(m_openGLContext.p());

    glGenTextures(1, &m_solidModelColorTexId);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_solidModelColorTexId);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    CVF_CHECK_OGL(m_openGLContext.p());

    glGenFramebuffers(1, &m_solidModelFboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_solidModelFboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE_ARB, m_solidModelDepthTexId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, m_solidModelColorTexId, 0);

    CVF_CHECK_OGL(m_openGLContext.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeelingFront::onResizeEvent(int width, int height)
{
    // Finally, call base
    initRenderTargets(width, height);
    TestSnippet::onResizeEvent(width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeelingFront::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->key() == Key_Plus)
    {
        m_numPasses++;
    }
    if (keyEvent->key() == Key_Minus)
    {
        if (m_numPasses > 1) m_numPasses--;
    }
    if (keyEvent->character() == 'T')
    {
        m_opacity -= 0.1f;
    }
    if (keyEvent->character() == 't')
    {
        m_opacity += 0.1f;
    }

    cvf::Trace::show(String("Current setting: Num passes %1 Transparency: %2 %%").arg(m_numPasses).arg(m_opacity*100.0f));

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> DepthPeelingFront::helpText() const
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
void DepthPeelingFront::renderFrontToBackPeeling()
{
    CVF_CALLSITE_OPENGL(m_openGLContext.p());

    Color3f m_backgroundColor(1.0f, 1.0f, 1.0f);

    // Some Qt versions leave this on!!
    glDisable(GL_BLEND);

    // ---------------------------------------------------------------------
    // 0. Render solid model
    // ---------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, m_solidModelFboId);
    CVF_CHECK_OGL(m_openGLContext.p());
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glClearColor(m_backgroundColor.r(), m_backgroundColor.g(), m_backgroundColor.b(), 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    m_progOpaque->useProgram(m_openGLContext.p());
    drawModel(m_solidModel.p(), m_progOpaque.p());
    glUseProgram(0);

    // ---------------------------------------------------------------------
    // 1. Initialize Min Depth Buffer
    // ---------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, m_frontColorBlenderFboId);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    m_progInit->useProgram(m_openGLContext.p());
    //  Not accepted by llvm-gcc-4.2: m_progInit->applyUniform(UniformFloat("Alpha", m_opacity));
    UniformFloat uniform("Alpha", m_opacity);
    m_progInit->applyUniform(m_openGLContext.p(), uniform);
    bindTextureRECT(m_progInit.p(), "SolidDepthTex", m_solidModelDepthTexId, 0);
    drawModel(m_transparentModel.p(), m_progInit.p());

    glUseProgram(0);

    CVF_CHECK_OGL(m_openGLContext.p());

    // ---------------------------------------------------------------------
    // 2. Depth Peeling + Blending
    // ---------------------------------------------------------------------
    for (int layer = 1; layer < m_numPasses; layer++) 
    {
        // Peel one more layer
        // ---------------------------------------------------------------------
        int currId = layer % 2;
        int prevId = 1 - currId;

        glBindFramebuffer(GL_FRAMEBUFFER, m_frontFboId[currId]);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        m_progPeel->useProgram(m_openGLContext.p());
        bindTextureRECT(m_progPeel.p(), "SolidDepthTex", m_solidModelDepthTexId, 0);
        bindTextureRECT(m_progPeel.p(), "DepthTex", m_frontDepthTexId[prevId], 1);
        //  Not accepted by llvm-gcc-4.2: m_progPeel->applyUniform(UniformFloat("Alpha", m_opacity));
        UniformFloat uniform("Alpha", m_opacity);
        m_progPeel->applyUniform(m_openGLContext.p(), uniform);
        drawModel(m_transparentModel.p(), m_progPeel.p());
        glUseProgram(0);

        CVF_CHECK_OGL(m_openGLContext.p());

        // Blend result
        // ---------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, m_frontColorBlenderFboId);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

        m_progBlend->useProgram(m_openGLContext.p());
        bindTextureRECT(m_progBlend.p(), "TempTex", m_frontColorTexId[currId], 0);
        drawQuad();
        glUseProgram(0);

        glDisable(GL_BLEND);

        CVF_CHECK_OGL(m_openGLContext.p());
    }

    // ---------------------------------------------------------------------
    // 3. Final Pass
    // ---------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glDisable(GL_DEPTH_TEST);

    m_progFinal->useProgram(m_openGLContext.p());
    bindTextureRECT(m_progFinal.p(), "ColorTex", m_frontColorBlenderTexId, 0);
    bindTextureRECT(m_progFinal.p(), "BackgroundAndOpaqueTex", m_solidModelColorTexId, 1);
    drawQuad();
    glUseProgram(0);

    CVF_CHECK_OGL(m_openGLContext.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeelingFront::bindTextureRECT(ShaderProgram* shaderProgram, const char* texname, GLuint texid, int texunit)
{
    CVF_CALLSITE_OPENGL(m_openGLContext.p());

    glActiveTexture(GL_TEXTURE0 + texunit);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texid);

    //  Not accepted by llvm-gcc-4.2: Not accepted by llvm-gcc-4.2: shaderProgram->applyUniform(UniformInt(texname, texunit));
    UniformInt uniform(texname, texunit);
    shaderProgram->applyUniform(m_openGLContext.p(), uniform);

    glActiveTexture(GL_TEXTURE0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeelingFront::drawModel(ModelBasicList* model, ShaderProgram* shaderProg)
{
    //m_model->draw();

    MatrixState matrixState(*m_camera);

    Collection<Part> partCollection;
    model->allParts(&partCollection);

    size_t i;
    for (i = 0; i < partCollection.size(); i++)
    {
        ref<Part> part = partCollection[i];
        CVF_ASSERT(part.notNull());

        if (part->transform())
        {
            matrixState.setModelMatrix(part->transform()->worldTransform());
        }
        else
        {
            matrixState.clearModelMatrix();
        }
        
        shaderProg->applyFixedUniforms(m_openGLContext.p(), matrixState);
        shaderProg->applyUniforms(m_openGLContext.p(), *part->effect()->uniformSet());

        part->drawable()->render(m_openGLContext.p(), shaderProg, *m_camera);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeelingFront::drawQuad()
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    {
        glVertex2f(0.0, 0.0); 
        glVertex2f(1.0, 0.0);
        glVertex2f(1.0, 1.0);
        glVertex2f(0.0, 1.0);
    }
    glEnd();
    glPopMatrix();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DepthPeelingFront::onPaintEvent(PostEventAction* postEventAction)
{
	CVF_UNUSED(postEventAction);

    renderFrontToBackPeeling();

    CVF_CHECK_OGL(m_openGLContext.p());
}


} // namespace snip


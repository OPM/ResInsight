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
#include "cvfuInputTypes.h"
#include "cvfuInputEvents.h"

#include "snipIncrementalRenderer.h"

#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
IncrementalRenderer::IncrementalRenderer()
:   m_incrementalRendering(false),
    m_startPartIdx(0),
    m_renderBuffersDirty(true),
    m_dynamicTextureID(0),
    m_frameBuffer(0),
    m_depthRenderBuffer(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool IncrementalRenderer::onInitialize()
{
    CVF_CALLSITE_OPENGL(m_openGLContext.p());

    if (!m_openGLContext->capabilities()->hasCapability(OpenGLCapabilities::FRAMEBUFFER_OBJECT))
    {
        return false;
    }

    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(15, 15, 15));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(50, 50, &parts);

    m_model = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        m_model->addPart(parts[i].p());
    }

    m_model->updateBoundingBoxesRecursive();

    m_renderSequence->rendering(0)->scene()->addModel(m_model.p());

    BoundingBox bb = m_model->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    // Create a frame-buffer object and a render-buffer object...
    glGenFramebuffersEXT( 1, &m_frameBuffer );
    glGenRenderbuffersEXT( 1, &m_depthRenderBuffer );
    glGenTextures( 1, &m_dynamicTextureID);
    CVF_CHECK_OGL(m_openGLContext.p());

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void IncrementalRenderer::onPaintEvent(PostEventAction* postEventAction)
{
    CVF_CALLSITE_OPENGL(m_openGLContext.p());

    const int NUM_PARTS_PER_RENDER = 12500;
    //const int NUM_PARTS_PER_RENDER = 450;
    // Update masks
    if (m_renderBuffersDirty)
    {
        m_startPartIdx = 0;
    }


    size_t endIdx = CVF_MIN(m_startPartIdx + NUM_PARTS_PER_RENDER - 1, m_model->partCount() - 1);
    size_t i;
    for (i = 0; i < m_model->partCount(); i++)
    {
        unsigned int enableMask = 1;
        if (m_incrementalRendering)
        {
            if ((i < m_startPartIdx) || (i > endIdx))
            {
                enableMask = 0;
            }
        }

        Part* p = m_model->part(i);
        CVF_ASSERT(p);

        p->setEnableMask(enableMask);
    }

    m_startPartIdx += NUM_PARTS_PER_RENDER;
    if (m_startPartIdx < m_model->partCount() - 1)
    {
        *postEventAction = REDRAW;
    }

    Viewport* vp = m_camera->viewport();
    int vpWidth = vp->width();
    int vpHeight = vp->height();

    Rendering* mainRendering = m_renderSequence->firstRendering();
    CVF_ASSERT(mainRendering);

    if (m_renderBuffersDirty) 
    {
        mainRendering->setClearMode(Viewport::CLEAR_COLOR_DEPTH);
    }
    else
    {
        mainRendering->setClearMode(Viewport::DO_NOT_CLEAR);
    }

    // Initialize the render-buffer for usage as a depth buffer.
    // We don't really need this to render things into the frame-buffer object,
    // but without it the geometry will not be sorted properly.
    if (m_renderBuffersDirty)
    {
        glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, m_depthRenderBuffer );
        glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, vpWidth, vpHeight);
        CVF_CHECK_OGL(m_openGLContext.p());

        // Now, create our dynamic texture. It doesn't actually get loaded with any 
        // pixel data, but its texture ID becomes associated with the pixel data
        // contained in the frame-buffer object. This allows us to bind to this data
        // like we would any regular texture.
        glBindTexture( GL_TEXTURE_2D, m_dynamicTextureID);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, vpWidth, vpHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        CVF_CHECK_OGL(m_openGLContext.p());
    }

    // Bind the frame-buffer object and attach to it a render-buffer object set up as a depth-buffer.
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_frameBuffer );
    CVF_CHECK_OGL(m_openGLContext.p());
    glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthRenderBuffer );
    CVF_CHECK_OGL(m_openGLContext.p());
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_dynamicTextureID, 0 );
    CVF_CHECK_OGL(m_openGLContext.p());

// 
//     vp->set(0, 0, RENDERBUFFER_WIDTH, RENDERBUFFER_HEIGHT);

    m_renderSequence->render(m_openGLContext.p());
    CVF_CHECK_OGL(m_openGLContext.p());

    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    CVF_CHECK_OGL(m_openGLContext.p());

    //     vp->set(0, 0, vpWidth, vpHeight);
    vp->applyOpenGL(m_openGLContext.p(), Viewport::CLEAR_COLOR_DEPTH);



    //glViewport(0, 0, vpWidth, vpHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, vpWidth, 0, vpHeight);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    
    glBindTexture( GL_TEXTURE_2D, m_dynamicTextureID );
    glEnable(GL_TEXTURE_2D);
    CVF_CHECK_OGL(m_openGLContext.p());

    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    
        glTexCoord2f(0, 0);
        glVertex2f(0, 0);
      
        glTexCoord2f(1, 0);
        glVertex2f((GLfloat)vp->width(), 0);

        glTexCoord2f(1, 1);
        glVertex2f((GLfloat)vp->width(), (GLfloat)vp->height());

        glTexCoord2f(0, 1);
        glVertex2f(0, (GLfloat)vp->height());

    glEnd();

//    glDeleteTextures( 1, &dynamicTextureID );
//     glDeleteFramebuffersEXT( 1, &frameBuffer );
//     glDeleteRenderbuffersEXT( 1, &depthRenderBuffer );
    CVF_CHECK_OGL(m_openGLContext.p());
    
    m_renderBuffersDirty = false;

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void IncrementalRenderer::onMouseMoveEvent(MouseEvent* mouseEvent)
{
    TestSnippet::onMouseMoveEvent(mouseEvent);

    if (m_trackball->activeNavigation() != ManipulatorTrackball::NONE)
    {
        m_renderBuffersDirty = true;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void IncrementalRenderer::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->key() == Key_I)
    {
        m_incrementalRendering = !m_incrementalRendering;
        keyEvent->setRequestedAction(REDRAW);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void IncrementalRenderer::onResizeEvent(int width, int height)
{
    m_renderBuffersDirty = true;
    TestSnippet::onResizeEvent(width, height);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> IncrementalRenderer::helpText() const
{
    std::vector<cvf::String> help;

    help.push_back("'I' to toggle incremental rendering");

    return help;
}


} // namespace snip


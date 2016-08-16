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
#include "cvfRenderSequence.h"
#include "cvfRendering.h"
#include "cvfScene.h"
#include "cvfModel.h"
#include "cvfColor4.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderSequence
/// \ingroup Viewing
///
/// 
//==================================================================================================

//------------------------------------------------------------------------------------------------
/// 
//------------------------------------------------------------------------------------------------
RenderSequence::RenderSequence() 
    : m_defaultGlLightPosition(0.5, 5.0, 7.0, 1.0) // Positional Headlight right, up, back from camera
{
    
}


//------------------------------------------------------------------------------------------------
/// Get the number of render passes in the sequence
//------------------------------------------------------------------------------------------------
uint RenderSequence::renderingCount() const
{
    return static_cast<uint>(m_renderings.size());
}


//------------------------------------------------------------------------------------------------
/// Add a rendering pass
//------------------------------------------------------------------------------------------------
void RenderSequence::addRendering(Rendering* rendering)
{
    CVF_ASSERT(rendering);
    m_renderings.push_back(rendering);
}


//--------------------------------------------------------------------------------------------------
/// Insert a rendering
///
/// The rendering will be inserted in the sequence before \a beforeRendering. If \a beforeRendering
/// is NULL or isn't in the sequence, the rendering will be added at the end of the sequence
//--------------------------------------------------------------------------------------------------
void RenderSequence::insertRendering(const Rendering* beforeRendering, Rendering* rendering)
{
    size_t indexToInsertAt = m_renderings.indexOf(beforeRendering);
    if (indexToInsertAt == UNDEFINED_SIZE_T)
    {
        addRendering(rendering);
        return;
    }

    size_t numRenderings = m_renderings.size();
    CVF_ASSERT(numRenderings > 0);
    CVF_ASSERT(indexToInsertAt < numRenderings);
    m_renderings.resize(numRenderings + 1);
    
    for (size_t i = numRenderings; i > indexToInsertAt; i--)
    {
        m_renderings[i] = m_renderings[i - 1];
    }

    m_renderings[indexToInsertAt] = rendering;
}


//------------------------------------------------------------------------------------------------
/// Get a rendering pass by index
//------------------------------------------------------------------------------------------------
Rendering* RenderSequence::rendering(uint index)
{
    CVF_ASSERT(index < renderingCount());

    return m_renderings[index].p();
}


//------------------------------------------------------------------------------------------------
/// Get a rendering pass by index
//------------------------------------------------------------------------------------------------
const Rendering* RenderSequence::rendering(uint index) const
{
    CVF_ASSERT(index < renderingCount());

    return m_renderings[index].p();
}


//--------------------------------------------------------------------------------------------------
/// Get pointer to the first rendering pass if any.
/// 
/// This function is safe to call even if there are no renderings. In this case NULL will be returned
//--------------------------------------------------------------------------------------------------
Rendering* RenderSequence::firstRendering()
{
    if (m_renderings.empty())
    {
        return NULL;
    }
    else
    {
        return m_renderings[0].p();
    }
}


//--------------------------------------------------------------------------------------------------
/// Get pointer to the first rendering pass if any.
/// 
/// This function is safe to call even if there are no renderings. In this case NULL will be returned
//--------------------------------------------------------------------------------------------------
const Rendering* RenderSequence::firstRendering() const
{
    if (m_renderings.empty())
    {
        return NULL;
    }
    else
    {
        return m_renderings[0].p();
    }
}


//------------------------------------------------------------------------------------------------
/// Remove all rendering passes
//------------------------------------------------------------------------------------------------
void RenderSequence::removeAllRenderings()
{
    m_renderings.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderSequence::removeRendering(const Rendering* rendering)
{
    CVF_ASSERT(rendering);
    m_renderings.erase(rendering);
}


//------------------------------------------------------------------------------------------------
/// Draw all the rendering passes in the sequence
//------------------------------------------------------------------------------------------------
void RenderSequence::render(OpenGLContext* oglContext)
{
    m_performanceInfo.resetCurrentTimers();

    uint numPasses = renderingCount();

    preRenderApplyExpectedOpenGLState(oglContext);

    uint i;
    for (i = 0; i < numPasses; i++)
    {
        Rendering* rendering = m_renderings[i].p();
        CVF_ASSERT(rendering);

        rendering->render(oglContext);
        m_performanceInfo.update(rendering->performanceInfo());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BoundingBox RenderSequence::boundingBox() const
{
    BoundingBox bb;

    uint numPasses = renderingCount();

    uint i;
    for (i = 0; i < numPasses; i++)
    {
        const Rendering* rendering = m_renderings[i].p();
        CVF_ASSERT(rendering);

        bb.add(rendering->boundingBox());
    }

    return bb;
}


//--------------------------------------------------------------------------------------------------
/// Todo: Replace with some renderstate object etc.
//--------------------------------------------------------------------------------------------------
void RenderSequence::setDefaultFFLightPositional(const Vec3f& position)
{
    m_defaultGlLightPosition = Vec4f(position, 1.0);
}

//--------------------------------------------------------------------------------------------------
/// Todo: Replace with some renderstate object etc.
//--------------------------------------------------------------------------------------------------
void RenderSequence::setDefaultFFLightDirectional(const Vec3f& direction)
{
    // The fourth value= 0.0 makes the light become a directional one, 
    // with direction from the position towards origo.

    m_defaultGlLightPosition = Vec4f(-direction, 0.0);
}

//--------------------------------------------------------------------------------------------------
/// Get the performance info for the last rendering (last call to render()).
//--------------------------------------------------------------------------------------------------
const PerformanceInfo&RenderSequence::performanceInfo() const
{
    return m_performanceInfo;
}


//--------------------------------------------------------------------------------------------------
/// Delete or release all OpenGL resources used by all parts in all renderings.
/// 
/// Will iterate over all the contents of the rendering sequence and delete or release any OpenGL 
/// resources that are being held. 
/// 
/// \warning The OpenGL context in which the resources were created or a context that is being
///          shared must be current in the calling thread.
/// \warning Some resources are merely released (by unreferencing the object) so in order to assure 
///          that the actual OpenGL resources get deleted, you may have to do cleanup through the  
///          OpenGLResourceManager as afterwards (eg. deleteOrphanedManagedBufferObjects())
//--------------------------------------------------------------------------------------------------
void RenderSequence::deleteOrReleaseOpenGLResources(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    uint numRenderings = renderingCount();

    uint i;
    for (i = 0; i < numRenderings; i++)
    {
        Rendering* rendering = m_renderings.at(i);
        CVF_ASSERT(rendering);

        Scene* scene = rendering->scene();

        if (scene)
        {
            uint numModels = scene->modelCount();
            uint j;
            for (j = 0; j < numModels; j++)
            {
                Model* model = scene->model(j);
                model->deleteOrReleaseOpenGLResources(oglContext);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Apply the OpenGL states we expect before rendering
/// 
/// The OpenGL states we set here will be a moving target. The main focus is on states that are
/// not captured through our RenderState objects. We do try and set all possible OpenGL states
/// in this function, but rather the ones that are likely to have been set by our caller and that
/// are likely to affect our rendering.
//--------------------------------------------------------------------------------------------------
void RenderSequence::preRenderApplyExpectedOpenGLState(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_CHECK_OGL(oglContext);

    const OpenGLCapabilities* oglCaps = oglContext->capabilities();

    // The following settings match the OpenGL defaults
    // ------------------------------------------------
    if (oglCaps->supportsOpenGL2())
    {
        glActiveTexture(GL_TEXTURE0);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);

    CVF_CHECK_OGL(oglContext);

#ifndef CVF_OPENGL_ES
    if (oglCaps->supportsFixedFunction())
    {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glShadeModel(GL_SMOOTH);
        glDisable(GL_NORMALIZE);
        glClearDepth(1);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        CVF_CHECK_OGL(oglContext);


        // These settings differ from OpenGL defaults,
        // but form a suitable starting point for rendering
        // ------------------------------------------------

        // TODO Work out a proper solution for this
        // Should probably add some RenderState that encapsulates a light source

        glLightfv(GL_LIGHT0, GL_POSITION, m_defaultGlLightPosition.ptr());

        const Vec3f spotDirection(0.0f, 0.0f, -1.0f);
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION,	spotDirection.ptr());
        glLightf( GL_LIGHT0, GL_SPOT_EXPONENT,	0.0f);
        glLightf( GL_LIGHT0, GL_SPOT_CUTOFF,	180.0f);

        const Color4f amb (0.0f, 0.0f, 0.0f, 1.0f);
        const Color4f diff(1.0f, 1.0f, 1.0f, 1.0f);
        const Color4f spec(1.0f, 1.0f, 1.0f, 1.0f);
        glLightfv(GL_LIGHT0, GL_AMBIENT,	amb.ptr());
        glLightfv(GL_LIGHT0, GL_DIFFUSE,	diff.ptr());
        glLightfv(GL_LIGHT0, GL_SPECULAR,   spec.ptr());

        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,	1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,		0.0f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION,	0.0f);

        glEnable(GL_LIGHT0);

        CVF_CHECK_OGL(oglContext);
    }
#endif
}


} // namespace cvf


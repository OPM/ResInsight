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
#include "cvfRenderQueueBuilder.h"
#include "cvfRenderQueue.h"
#include "cvfPartRenderHintCollection.h"
#include "cvfPart.h"
#include "cvfDrawable.h"
#include "cvfEffect.h"
#include "cvfShaderProgram.h"
#include "cvfOpenGLContext.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfBufferObjectManaged.h"
#include "cvfCamera.h"
#include "cvfRenderStateTextureBindings.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderQueueBuilder
/// \ingroup Viewing
///
/// Populates a render queue based on a part collection. All items in the part collection will 
/// result in a corresponding RenderItem. 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor. Sets up the object with the part collection to use in populateRenderQueue()
//--------------------------------------------------------------------------------------------------
RenderQueueBuilder::RenderQueueBuilder(PartRenderHintCollection* srcPartCollection, OpenGLContext* oglContext, const Camera* camera)
{
    CVF_ASSERT(srcPartCollection);
    CVF_ASSERT(oglContext);
    CVF_ASSERT(camera);
    m_srcPartCollection = srcPartCollection;
    m_oglContext = oglContext;
    m_camera = camera;

    m_requireDistance = false;
    m_requirePixelArea = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderQueueBuilder::setFixedEffect(Effect* effect)
{
    m_fixedEffect = effect;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderQueueBuilder::setRequireDistance(bool requireDistance)
{
    m_requireDistance = requireDistance;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderQueueBuilder::setRequirePixelArea(bool requirePixelArea)
{
    m_requirePixelArea = requirePixelArea;
}


//--------------------------------------------------------------------------------------------------
/// Populate the render queue based on the input parts 
/// 
/// \param renderQueue The render queue that should be populated
/// 
/// Currently this function will:
/// - Populate the RenderQueue with RenderItem objects
/// - Link all referenced GLShaderProgram objects
/// 
/// Future tasks (?):
/// - Do LOD evaluation to determine LOD level for drawables and effects
/// - Perform VBO and display list setup
/// 
/// To be determined:
/// - What should happen to parts that end up having no effect or no drawable? 
/// - Currently, parts ending up with \b no drawable are \b not added to the queue.
/// - Parts with a drawable, but without any effect or with illegal effects (shader program 
///   that fails to link) will get added to the queue
//--------------------------------------------------------------------------------------------------
void RenderQueueBuilder::populateRenderQueue(RenderQueue* renderQueue)
{
    CVF_ASSERT(renderQueue);

    size_t srcPartCount = m_srcPartCollection->count();

    renderQueue->removeAll();
    renderQueue->hintNumEntriesToAdd(srcPartCount);

    // Extract raw pointer to any fixed effect
    Effect* fixedEffect = m_fixedEffect.p();

    OpenGLResourceManager* resourceManager = m_oglContext->resourceManager();
    resourceManager->evictOrphanedOpenGLResources(m_oglContext.p());

    // Does OpenGL support buffer objects?
    const bool oglSupportsBufferObjects = BufferObjectManaged::supportedOpenGL(m_oglContext.p());


    size_t i;
    for (i = 0; i < srcPartCount; i++)
    {
        Part* part = m_srcPartCollection->part(i);
        CVF_ASSERT(part);

        // TODO Must do LOD evaluation for geometry lod
        const int drawableLod = 0;
        Drawable* drawable = part->drawable(drawableLod);
        CVF_ASSERT(drawable);

        Effect* effect = fixedEffect;
        if (!effect)
        {
            // TODO Must do LOD evaluation for effect lod
            const int effectLod = 0;
            effect = part->effect(effectLod);
        }

        CVF_ASSERT(effect);

        bool canAddToQueue = true;

        // TODO rework this when we get uniforms in place
        ShaderProgram* shaderProg = effect->shaderProgram();
        if (shaderProg)
        {
            // For now, don't add parts where program fails to compile
            if (!shaderProg->linkProgram(m_oglContext.p()))
            {
                // TODO Should we do any error reporting here?
                // For now we just don't add the effect to the render queue
                canAddToQueue = false;
            }
        }

        // Create and upload data to buffer objects (VBOs)
        // Whether anything actually gets uploaded is controlled by the drawable itself
        if (oglSupportsBufferObjects)
        {
            drawable->createUploadBufferObjectsGPU(m_oglContext.p());
        }

        // Create/Setup the textures
        RenderStateTextureBindings* textureBindings = static_cast<RenderStateTextureBindings*>(effect->renderStateOfType(RenderState::TEXTURE_BINDINGS));
        if (textureBindings && canAddToQueue) 
        {
            textureBindings->setupTextures(m_oglContext.p());
        }

#ifndef CVF_OPENGL_ES
        RenderStateTextureMapping_FF* textureMapping = static_cast<RenderStateTextureMapping_FF*>(effect->renderStateOfType(RenderState::TEXTURE_MAPPING_FF));
        if (textureMapping && canAddToQueue) 
        {
            textureMapping->setupTexture(m_oglContext.p());
        }
#endif

        if (canAddToQueue)
        {
            float distance = -1.0f;
            float pixelArea = -1.0f;

            // Check if we need distance and pixel area. If it is provided by the PartRenderHint we use that.
            // If not, we compute it here if it is required
            PartRenderHint* hint = m_srcPartCollection->renderHint(i);
            if (hint)
            {
                distance = hint->centerDistance();
                pixelArea = hint->projectedAreaPixels();
            }

            if (m_requireDistance && (distance == -1.0f))
            {
                const BoundingBox& bb = part->boundingBox();
                CVF_ASSERT(bb.isValid());
                distance = static_cast<float>((m_camera->position() - bb.center()).length());
            }

            if (m_requirePixelArea && (pixelArea == -1.0f))
            {
                const BoundingBox& bb = part->boundingBox();
                CVF_ASSERT(bb.isValid());

                pixelArea = static_cast<float>(m_camera->computeProjectedBoundingSpherePixelArea(bb.center(), bb.radius()));
            }

            renderQueue->add(part, drawable, effect, pixelArea, distance);
        }
    }
}

} // namespace cvf

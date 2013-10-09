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
#include "cvfRenderEngine.h"
#include "cvfRenderQueue.h"
#include "cvfPart.h"
#include "cvfDrawable.h"
#include "cvfTransform.h"
#include "cvfEffect.h"
#include "cvfRenderStateSet.h"
#include "cvfShaderProgram.h"
#include "cvfOpenGL.h"
#include "cvfBufferObjectManaged.h"
#include "cvfMatrixState.h"
#include "cvfCamera.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfLogManager.h"

#include <memory.h>

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderEngine
/// \ingroup Viewing
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderEngine::RenderEngine()
:   m_renderedPartCount(0),
    m_renderedVertexCount(0),
    m_renderedTriangleCount(0),
    m_renderedOpenGLPrimitiveCount(0),
    m_disableDrawableRender(false),
    m_disableApplyEffects(false),
    m_forceImmediateMode(false),
    m_enableItemCountUpdate(false),
    m_logger(CVF_GET_LOGGER("cee.cvf"))
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderEngine::render(OpenGLContext* oglContext, RenderQueue* renderQueue, size_t maxNumPartsToDraw, const Camera& camera, const UniformSet* globalUniformSet)
{
    CVF_ASSERT(oglContext);
    CVF_ASSERT(renderQueue);

    m_renderedPartCount = 0;
    m_renderedVertexCount = 0;
    m_renderedTriangleCount = 0;
    m_renderedOpenGLPrimitiveCount = 0;
    m_applyRenderStateCount = 0;
    m_shaderProgramChangesCount = 0;

    m_renderStateTracker.resetAndApplyDefaultRenderStates(oglContext);

    // Use immediate mode either if forced or if buffer objects are not supported (OpenGL 1.5 and below)
    // This is because the current fixed function rendering relies on the buffer object binding points being present
    const bool renderUsingImmediateMode = (m_forceImmediateMode || !BufferObjectManaged::supportedOpenGL(oglContext));

    // Does the current OpenGL context support OpenGL shaders 
    const bool contextSupportsShaders = ShaderProgram::supportedOpenGL(oglContext);

    // Init matrix state tracker
    MatrixState matrixState(camera);
    uint lastAppliedMatrixStateVersionTick = matrixState.versionTick() - 1;
    uint lastAppliedMatrixStateVersionTickFixedFunction = matrixState.versionTick() - 1;

    const Effect* prevEffect = NULL;
    const RenderStateSet* prevRenderStateSet = NULL;
    const ShaderProgram* prevShaderProgram = NULL;

    // Track the current shader program being used. 
    // Will be NULL when effect has no shader program or if usage of shader program fails
    ShaderProgram* shaderProgramInUse = NULL;
    if (contextSupportsShaders) 
    {
        // Start out with fixed function
        ShaderProgram::useNoProgram(oglContext);
    }

    // Determine number of parts we'll draw
    size_t numPartsInQueue = renderQueue->count();
    size_t numPartsToDraw = std::min(numPartsInQueue, maxNumPartsToDraw);

    CVF_LOG_DEBUG(m_logger, "RenderEngine::render(), numParts=" + String(static_cast<uint>(numPartsInQueue)));

    size_t i;
    for (i = 0; i < numPartsToDraw; i++)
    {
        RenderItem* item = renderQueue->item(i);
        CVF_ASSERT(item);
        
        Drawable* drawable = item->drawable();
        Effect* effect = item->effect();
        Part* part = item->part();
        CVF_ASSERT(drawable);
        CVF_ASSERT(effect);
        CVF_ASSERT(part);
        
        CVF_LOG_DEBUG(m_logger, String("part#=%1, partName='%2'").arg(static_cast<uint>(i)).arg(part->name()));

        // Update matrix state to reflect any part transformations
        // Register if the pass modifies the view matrix so that we can reset it at the end of this part
        bool lastPartModifiedViewMatrix = false;
        if (part->transform())
        {
            const Transform* trans = part->transform();
            if (trans->eyeLiftFactor() != 0)
            {
                // Eye lifting done by scaling the vertex coordinates after transforming them to eye space. See Transform::setEyeLiftFactor()
                // An eye lift factor of 1.0 results in scaling by 0.995. Might have to tweak the value below somewhat
                double scaleFactor = 1 - (trans->eyeLiftFactor()*0.005);
                Mat4d scaleMatrix = Mat4d::fromScaling(Vec3d(scaleFactor, scaleFactor, scaleFactor));
                matrixState.setViewMatrix(scaleMatrix*camera.viewMatrix());
                lastPartModifiedViewMatrix = true;
            }

            matrixState.setModelMatrix(trans->worldTransform());
        }
        else
        {
            matrixState.clearModelMatrix();
        }

        if (effect != prevEffect && !m_disableApplyEffects)
        {
            const RenderStateSet* const renderStateSet = effect->renderStateSet();
            if (renderStateSet != prevRenderStateSet)
            {
                // This apply function is designed to work with NULL pointers
                m_renderStateTracker.applyRenderStates(oglContext, renderStateSet, prevRenderStateSet);
                prevRenderStateSet = renderStateSet;
                m_applyRenderStateCount++;
            }

            if (contextSupportsShaders)
            {
                ShaderProgram* shaderProgThisEffect = effect->shaderProgram();
                if (shaderProgThisEffect != prevShaderProgram)
                {
                    // Reset the 'in use' pointer, and only set it if we have a shader program AND we manage to use it
                    shaderProgramInUse = NULL;
                    if (shaderProgThisEffect)
                    {
                        // Will fail if program hasn't been linked
                        if (shaderProgThisEffect->useProgram(oglContext))
                        {
                            shaderProgramInUse = shaderProgThisEffect;
                        }
                    }

                    if (!shaderProgramInUse)
                    {
                        ShaderProgram::useNoProgram(oglContext);
                    }

                    prevShaderProgram = shaderProgThisEffect;
                    m_shaderProgramChangesCount++;
                }

                if (shaderProgramInUse)
                {
                    // Apply uniforms for this shader program
                    // Note order of application here.
                    //  - shader program default uniforms
                    //  - global dynamic uniforms
                    //  - (texture binding uniforms)
                    //  - effect uniforms
                    //  - fixed uniforms
                    // We would really like the effect's uniforms to be at the very end, but that would give inconsistent 
                    // behavior since the fixed uniforms may get applied also when the effect does NOT change.
                    shaderProgramInUse->clearUniformApplyTracking();

                    shaderProgramInUse->applyDefaultUniforms(oglContext);

                    if (globalUniformSet)
                    {
                        shaderProgramInUse->applyActiveUniformsOnly(oglContext, *globalUniformSet);
                    }

                    const RenderStateTextureBindings* textureBindings = static_cast<const RenderStateTextureBindings*>(effect->renderStateOfType(RenderState::TEXTURE_BINDINGS));
                    if (textureBindings)
                    {
                        textureBindings->applySamplerTextureUnitUniforms(oglContext, shaderProgramInUse);
                    }

                    // Apply the effect's uniform last so that they win in case of clashes
                    const UniformSet* uniformSet = effect->uniformSet();
                    if (uniformSet)
                    {
                        shaderProgramInUse->applyUniforms(oglContext, *uniformSet);
                    }

                    // Still, the fixed ones have to go at the end for consistency
                    shaderProgramInUse->applyFixedUniforms(oglContext, matrixState);
                    lastAppliedMatrixStateVersionTick = matrixState.versionTick();
                }
            }

            prevEffect = effect;
        }


        if (shaderProgramInUse)
        {
            if (lastAppliedMatrixStateVersionTick != matrixState.versionTick())
            {
                shaderProgramInUse->applyFixedUniforms(oglContext, matrixState);
                lastAppliedMatrixStateVersionTick = matrixState.versionTick();
            }
            
            // Will check that all active uniforms in shader program have been set since last call to >clearUniformApplyTracking()
            // Reports error to log if any uniforms are missing.
            shaderProgramInUse->checkReportAllUniformsApplied(oglContext);
        }
        else
        {
#ifndef CVF_OPENGL_ES
            if (lastAppliedMatrixStateVersionTickFixedFunction != matrixState.versionTick())
            {
                glLoadMatrixf(matrixState.modelViewMatrix().ptr());
                lastAppliedMatrixStateVersionTickFixedFunction = matrixState.versionTick();
            }
#endif // CVF_OPENGL_ES
        }


        // Do actual rendering
        if (!m_disableDrawableRender) 
        {
            if (renderUsingImmediateMode)
            {
                drawable->renderImmediateMode(oglContext, matrixState);
            }
            else
            {
                if (shaderProgramInUse)
                {
                    drawable->render(oglContext, shaderProgramInUse, matrixState);
                }
                else
                {
                    drawable->renderFixedFunction(oglContext, matrixState);
                }
            }

            m_renderedPartCount++;
        }

        // Must reset the view matrix if it was modified during drawing of this part
        if (lastPartModifiedViewMatrix)
        {
            matrixState.setViewMatrix(camera.viewMatrix());
        }
        
        // Update counters
        if (m_enableItemCountUpdate)
        {
            m_renderedVertexCount           += drawable->vertexCount();
            m_renderedTriangleCount         += drawable->triangleCount();
            m_renderedOpenGLPrimitiveCount  += drawable->faceCount();
        }
    }

    // Do an apply with an empty new render state set to get defaults back before leaving
    // This really isn't the optimal place to do this, but until we expose application of render states it's the only place
    m_renderStateTracker.applyRenderStates(oglContext, NULL, prevRenderStateSet);

    // For now, set back to fixed function
    if (contextSupportsShaders) 
    {
        ShaderProgram::useNoProgram(oglContext);
    }

    // Set back VBO settings (do we need this?)
    if (BufferObjectManaged::supportedOpenGL(oglContext))
    {
        BufferObjectManaged::unbindAllBuffers(oglContext);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RenderEngine::renderedPartCount() const
{
    return m_renderedPartCount;
}   


//--------------------------------------------------------------------------------------------------
/// Returns the number of vertices used for the last rendering
//--------------------------------------------------------------------------------------------------
size_t RenderEngine::renderedVertexCount() const
{
    return m_renderedVertexCount;
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of triangles (GL_TRIANGLE) rendered in the last rendering
//--------------------------------------------------------------------------------------------------
size_t RenderEngine::renderedTriangleCount() const
{
    return m_renderedTriangleCount;
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of OpenGL primitives (triangles, points, lines, etc) rendered in the last rendering
//--------------------------------------------------------------------------------------------------
size_t RenderEngine::renderedOpenGLPrimitiveCount() const
{
    return m_renderedOpenGLPrimitiveCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RenderEngine::applyRenderStateCount() const
{
    return m_applyRenderStateCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RenderEngine::shaderProgramChangesCount() const
{
    return m_shaderProgramChangesCount;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderEngine::enableForcedImmediateMode(bool enable)
{
    m_forceImmediateMode = enable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderEngine::disableRenderDrawables(bool disable)
{
    m_disableDrawableRender = disable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderEngine::disableApplyEffects(bool disable)
{
    m_disableApplyEffects = disable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderEngine::isForcedImmediateModeEnabled() const
{
    return m_forceImmediateMode;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderEngine::isRenderDrawableDisabled() const
{
    return m_disableDrawableRender;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderEngine::isApplyEffectsDisabled() const
{
    return m_disableApplyEffects;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderEngine::enableItemCountUpdate(bool enable)
{
    m_enableItemCountUpdate = enable;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RenderEngine::isItemCountUpdateEnabled() const
{
    return m_enableItemCountUpdate;
}


} // namespace cvf


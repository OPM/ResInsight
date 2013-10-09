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
#include "cvfRenderStateTracker.h"
#include "cvfRenderStateSet.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStateColorMask.h"
#include "cvfRenderStateCullFace.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateFrontFace.h"
#include "cvfRenderStateLine.h"
#include "cvfRenderStatePoint.h"
#include "cvfRenderStatePolygonMode.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfRenderStateStencil.h"
#include "cvfRenderStateTextureBindings.h"
#include "cvfOpenGLContext.h"
#include "cvfOpenGLCapabilities.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

#include <memory.h>

namespace cvf {



//==================================================================================================
///
/// \class cvf::RenderStateTracker
/// \ingroup Render
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RenderStateTracker::RenderStateTracker()
{
    memset(m_renderStateUsageTable, 0, sizeof(m_renderStateUsageTable));
    memset(m_currentRenderStates, 0, sizeof(m_currentRenderStates));

    setupDefaultRenderStates();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTracker::setupDefaultRenderStates()
{
    m_defaultRenderStates[RenderState::BLENDING]            = new RenderStateBlending;
    m_defaultRenderStates[RenderState::COLOR_MASK]          = new RenderStateColorMask;
    m_defaultRenderStates[RenderState::CULL_FACE]           = new RenderStateCullFace(false);
    m_defaultRenderStates[RenderState::DEPTH]               = new RenderStateDepth;
    m_defaultRenderStates[RenderState::FRONT_FACE]          = new RenderStateFrontFace;
    m_defaultRenderStates[RenderState::LINE]                = new RenderStateLine;
    m_defaultRenderStates[RenderState::POINT]               = new RenderStatePoint;
    m_defaultRenderStates[RenderState::POLYGON_MODE]        = new RenderStatePolygonMode;
    m_defaultRenderStates[RenderState::POLYGON_OFFSET]      = new RenderStatePolygonOffset;
    m_defaultRenderStates[RenderState::STENCIL]             = new RenderStateStencil;
    m_defaultRenderStates[RenderState::TEXTURE_BINDINGS]    = new RenderStateTextureBindings;

#ifndef CVF_OPENGL_ES
    m_defaultRenderStates[RenderState::LIGHTING_FF]         = new RenderStateLighting_FF;
    m_defaultRenderStates[RenderState::MATERIAL_FF]         = new RenderStateMaterial_FF;
    m_defaultRenderStates[RenderState::NORMALIZE_FF]        = new RenderStateNormalize_FF(false);
    m_defaultRenderStates[RenderState::TEXTURE_MAPPING_FF]  = new RenderStateTextureMapping_FF;
    m_defaultRenderStates[RenderState::CLIP_PLANES_FF]      = new RenderStateClipPlanes_FF;
#endif
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RenderStateTracker::resetAndApplyDefaultRenderStates(OpenGLContext* oglContext)
{
    memset(m_renderStateUsageTable, 0, sizeof(m_renderStateUsageTable));
    memset(m_currentRenderStates, 0, sizeof(m_currentRenderStates));

    bool contextSupportsFixedFunction = oglContext->capabilities()->supportsFixedFunction();

    int numRenderStates = RenderState::COUNT;
    int i;
    for (i = 0; i < numRenderStates; i++)
    {
        RenderState* rs = m_defaultRenderStates[i].p();
        CVF_ASSERT(rs);
        if (rs->isFixedFunction())
        {
            // Apply fixed function render states ONLY if the OpenGL context supports it 
            if (contextSupportsFixedFunction)
            {
                rs->applyOpenGL(oglContext);
            }
        }
        else
        {
            rs->applyOpenGL(oglContext);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Apply states in RenderStateSet
/// 
/// \param oglContext  The current OpenGLContext
/// \param newRSS      The new RenderStateSet that is to be applied
/// \param prevRSS     The previous RenderStateSet
//--------------------------------------------------------------------------------------------------
void RenderStateTracker::applyRenderStates(OpenGLContext* oglContext, const RenderStateSet* newRSS, const RenderStateSet* prevRSS)
{
    const int IS_DEFAULT  = 0;
    const int IN_USE      = 1;
    const int WILL_BE_SET = 2;

    // If there is no previous state set, assume all render states are at defaults
    if (!prevRSS)
    {
        memset(m_renderStateUsageTable, IS_DEFAULT, sizeof(m_renderStateUsageTable));
        memset(m_currentRenderStates, 0, sizeof(m_currentRenderStates));
    }


    uint numNewStates = newRSS ? newRSS->count() : 0;

    // Modify the usage flag for all the states that are present in the new state set.
    // These will be set later so there is no need to set them back to the defaults
    uint i;
    for (i = 0; i < numNewStates; i++)
    {
        m_renderStateUsageTable[newRSS->renderState(i)->type()] = WILL_BE_SET;
    }


    // Iterate over the previous state set
    // Apply the corresponding default state only if it won't be overwritten when we apply the new states later
    if (prevRSS)
    {
        uint numPrevStates = prevRSS->count();
        for (i = 0; i < numPrevStates; i++)
        {
            RenderState::Type stateType = prevRSS->renderState(i)->type();

            // Only need to apply the corresponding default if it is flagged as IN_USE
            // States with the WILL_BE_SET usage are handled in the final loop below
            if (m_renderStateUsageTable[stateType] == IN_USE)
            {
                const RenderState* defaultState = m_defaultRenderStates[stateType].p();
                
                defaultState->applyOpenGL(oglContext);
                m_currentRenderStates[stateType] = defaultState;
                m_renderStateUsageTable[stateType] = IS_DEFAULT;
            }
        }
    }


    // Iterate over the incoming new render states
    // Call apply function only if the new state is different from the current one 
    for (i = 0; i < numNewStates; i++)
    {
        const RenderState* newState = newRSS->renderState(i);
        CVF_ASSERT(newState);
        RenderState::Type stateType = newState->type();

        if (m_currentRenderStates[stateType] != newState)
        {
            newState->applyOpenGL(oglContext);
            m_currentRenderStates[stateType] = newState;
        }

        m_renderStateUsageTable[stateType] = IN_USE;
    }
}



} // namespace cvf


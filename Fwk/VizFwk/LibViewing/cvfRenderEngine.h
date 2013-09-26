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


#pragma once

#include "cvfMatrix4.h"
#include "cvfRenderStateTracker.h"

namespace cvf {

class RenderQueue;
class UniformSet;
class Camera;
class OpenGLContext;
class Logger;


//==================================================================================================
//
// RenderEngine
//
//==================================================================================================
class RenderEngine
{
public:
    RenderEngine();

    void    render(OpenGLContext* oglContext, RenderQueue* renderQueue, size_t maxNumPartsToDraw, const Camera& camera, const UniformSet* globalUniformSet);

    size_t  renderedPartCount() const;
    size_t  renderedVertexCount() const;
    size_t  renderedTriangleCount() const;
    size_t  renderedOpenGLPrimitiveCount() const;
    size_t  applyRenderStateCount() const;
    size_t  shaderProgramChangesCount() const;

    void    enableForcedImmediateMode(bool enable);
    void    disableRenderDrawables(bool disable);
    void    disableApplyEffects(bool disable);
    bool    isForcedImmediateModeEnabled() const;
    bool    isRenderDrawableDisabled() const;
    bool    isApplyEffectsDisabled() const;

    void    enableItemCountUpdate(bool enable);
    bool    isItemCountUpdateEnabled() const;

private:
    RenderStateTracker  m_renderStateTracker;

    size_t              m_renderedPartCount;
    size_t              m_renderedVertexCount;
    size_t              m_renderedTriangleCount;
    size_t              m_renderedOpenGLPrimitiveCount;
    size_t              m_applyRenderStateCount;
    size_t              m_shaderProgramChangesCount;

    bool                m_disableDrawableRender;
    bool                m_disableApplyEffects;
    bool                m_forceImmediateMode;           // Can be used to force immediate mode drawing for debugging purposes (good old glBegin()/glEnd())
    bool                m_enableItemCountUpdate;

    ref<Logger>         m_logger;
};

}

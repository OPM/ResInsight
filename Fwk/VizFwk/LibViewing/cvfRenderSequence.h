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

#include "cvfObject.h"
#include "cvfCollection.h"
#include "cvfBoundingBox.h"
#include "cvfPerformanceInfo.h"

namespace cvf {

class Rendering;
class OpenGLContext;


//==================================================================================================
//
// RenderSequence
//
//==================================================================================================
class RenderSequence : public Object
{
public:
    RenderSequence();

    uint			        renderingCount() const;
    void                    addRendering(Rendering* rendering);
    void                    insertRendering(const Rendering* beforeRendering, Rendering* rendering);
    Rendering*              firstRendering();
    const Rendering*        firstRendering() const;
    Rendering*              rendering(uint index);
    const Rendering*        rendering(uint index) const;
    void                    removeAllRenderings();
    void                    removeRendering(const Rendering* rendering);

    BoundingBox             boundingBox() const;

    void                    setDefaultFFLightPositional(const Vec3f& position);
    void                    setDefaultFFLightDirectional(const Vec3f& direction);

    void                    render(OpenGLContext* oglContext);
    const PerformanceInfo&  performanceInfo() const;

    void                    deleteOrReleaseOpenGLResources(OpenGLContext* oglContext);

private:
    void                    preRenderApplyExpectedOpenGLState(OpenGLContext* oglContext) const;

private:
    Collection<Rendering>   m_renderings;       // One rendering per render pass
    PerformanceInfo         m_performanceInfo;  // Performance summary for this view
    Vec4f                   m_defaultGlLightPosition; // Fixed function default setting for glLightfv(GL_LIGHT0, GL_POSITION, m_defaultGlLightPosition.ptr());
};

}

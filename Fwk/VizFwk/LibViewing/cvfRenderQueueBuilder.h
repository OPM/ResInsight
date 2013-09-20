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

namespace cvf {

class PartRenderHintCollection;
class RenderQueue;
class Effect;
class Camera;
class OpenGLContext;


//==================================================================================================
//
// Populates a render queue based on a part collection
//
//==================================================================================================
class RenderQueueBuilder 
{
public:
    RenderQueueBuilder(PartRenderHintCollection* srcPartCollection, OpenGLContext* oglContext, const Camera* camera);

    void    setFixedEffect(Effect* effect);
    void    setRequireDistance(bool requireDistance);
    void    setRequirePixelArea(bool requirePixelArea);

    void    populateRenderQueue(RenderQueue* renderQueue);

private:
    ref<PartRenderHintCollection>   m_srcPartCollection;    // The source part collection
    ref<OpenGLContext>              m_oglContext;           // OpenGLContext to use. This responsibility should be removed from the render queue builder
    cref<Camera>                    m_camera;               // Camera used when there is a need for computing distance and/or pixel area

    ref<Effect>                     m_fixedEffect;          // If specified, all render items in the queue will have this effect
    bool                            m_requireDistance;
    bool                            m_requirePixelArea;
};

}

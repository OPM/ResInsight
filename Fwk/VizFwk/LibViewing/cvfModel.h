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

#include "cvfCollection.h"
#include "cvfBoundingBox.h"

namespace cvf {

class Part;
class PartRenderHintCollection;
class Camera;
class CullSettings;
class RayIntersectSpec;
class HitItemCollection;
class OpenGLContext;
class Transform;


//==================================================================================================
//
// Abstract Model class
//
//==================================================================================================
class Model : public Object
{
public:
    Model();
    virtual ~Model();

    virtual String          name() const = 0;

    virtual void            findVisibleParts(PartRenderHintCollection* visibleParts, const Camera& camera, const CullSettings& cullSettings, uint enableMask) = 0;
    virtual void            allParts(Collection<Part>* partCollection) = 0;

    virtual void            updateBoundingBoxesRecursive() = 0;
    virtual BoundingBox     boundingBox() const = 0;

    virtual bool            rayIntersect(const RayIntersectSpec& rayIntersectSpec, HitItemCollection* hitItemCollection) = 0;

    void                    setPartEnableMask(uint partEnableMask);
    uint                    partEnableMask() const;

    void                    deleteOrReleaseOpenGLResources(OpenGLContext* oglContext);

    void                    setTransformTree(Transform* transform);
    Transform*              transformTree();

private:
    uint                    m_partEnableMask;   // Mask will be compared against our parts when determining visible parts
    ref<Transform>          m_tranformTree;
};

}

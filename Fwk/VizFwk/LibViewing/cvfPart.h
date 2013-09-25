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
#include "cvfOpenGLTypes.h"
#include "cvfCollection.h"
#include "cvfString.h"
#include "cvfBoundingBox.h"


namespace cvf {

class Drawable;
class Transform;
class Effect;
class Ray;
class HitItemCollection;
class OpenGLContext;


//==================================================================================================
//
// Part
//
//==================================================================================================
class Part : public Object
{
public:
    static const uint MAX_NUM_LOD_LEVELS = 2;

public:
    Part();
    Part(int64 id, String name);
    virtual ~Part();

    void                setName(const String& name);
    void                setId(int64 id);
    const String&       name() const;
    int64               id() const;

    void                setEnableMask(uint mask);
    uint                enableMask() const;
    void                setPriority(int priority);
    int                 priority() const;

    virtual ref<Part>   shallowCopy() const;

    void                setDrawable(Drawable* drawable);
    void                setDrawable(uint lodLevel, Drawable* drawable);
    Drawable*           drawable();
    const Drawable*     drawable() const;
    Drawable*           drawable(uint lodLevel);

    Effect*             effect();
    Effect*             effect(uint lodLevel);
    void                setEffect(Effect* effect);
    void                setEffect(uint lodLevel, Effect* effect);

    Transform*          transform();
    const Transform*    transform() const;
    void                setTransform(Transform* transform);

    void                updateBoundingBox();
    const BoundingBox&  boundingBox() const;

    bool                rayIntersect(const Ray& ray, HitItemCollection* hitItemCollection) const;

    void                deleteOrReleaseOpenGLResources(OpenGLContext* oglContext);

    void                setSourceInfo(Object* sourceInfo);
    Object*             sourceInfo();
    const Object*       sourceInfo() const;

protected:
    String              m_name;
    int64               m_id;
    uint                m_enableMask;           // The enable mask determines if the part should be visible and/or rendered depending on masks set in Model and/or Rendering
    int                 m_priority;             // The part's render priority. Parts with the highest priority (highest value) are drawn last. The default priority is 0.

    ref<Transform>      m_transform;
    ref<Drawable>       m_drawables[MAX_NUM_LOD_LEVELS];
    ref<Effect>         m_effects[MAX_NUM_LOD_LEVELS];
    BoundingBox         m_boundingBox;

    ref<Object>         m_sourceInfo;           // Source info object used to identify the origin of a given geometry entity. This usually needed during picking operations,
                                                // and the application needs to identify the domain specific entity from which the selected geometry entity was created
};

}

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
#include "cvfCollection.h"
#include "cvfCamera.h"

namespace cvf {


//==================================================================================================
//
// Transformation for use in a Part
//
//==================================================================================================
class Transform : public Object
{
public:
    Transform();
    virtual ~Transform();
    virtual void        updateWorldTransform(const cvf::Camera* camera); // camera optional, not used in base

    void                addChild(Transform* transform);
    void                removeAllChildren();
    size_t              childCount() const;
    Transform*          child(size_t index);
    const Transform*    child(size_t index) const;

    void                setLocalTransform(const Mat4d& transform);      // setter world og local
    const Mat4d&        localTransform() const;

    const Mat4d&        worldTransform() const;

    void                setEyeLiftFactor(double eyeLiftFactor);
    double              eyeLiftFactor() const;

protected:
    Transform*              m_parent;
    Collection<Transform>   m_children;
    Mat4d                   m_localMatrix;
    Mat4d                   m_worldMatrix;
    double                  m_eyeLiftFactor;    // Lift the part towards the eye. A factor of 0 does no lifting.
};

}

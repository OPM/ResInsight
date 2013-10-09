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
#include "cvfTransform.h"
#include "cvfBoundingBox.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Transform
/// \ingroup Viewing
///
/// Transformation for use with a Part
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
Transform::Transform()
:   m_parent(NULL),
    m_eyeLiftFactor(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Transform::~Transform()
{
    removeAllChildren();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Mat4d& Transform::worldTransform() const
{
    return m_worldMatrix;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Transform::updateWorldTransform(const cvf::Camera* camera)
{
    // Update our matrix
    if (m_parent)
    {
        m_worldMatrix = m_parent->worldTransform()*m_localMatrix;
    }
    else
    {
        m_worldMatrix = m_localMatrix;
    }

    // then update all the children
    for (size_t i = 0; i < m_children.size(); ++i)
    {
        m_children[i]->updateWorldTransform(camera);
    }
}


//--------------------------------------------------------------------------------------------------
/// Note: Sets both the local AND the world transform
//--------------------------------------------------------------------------------------------------
void Transform::setLocalTransform(const Mat4d& transform)
{
    m_localMatrix = transform;
    m_worldMatrix = transform;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Mat4d& Transform::localTransform() const
{
    return m_localMatrix;
}


//--------------------------------------------------------------------------------------------------
/// Sets the 'eye lift factor' 
///
/// Use this factor to move the vertices of the part towards the eye point. Useful for primitives
/// that lie in the same plane and would otherwise cause flickering. A lift factor of 0 disables all lifting
//--------------------------------------------------------------------------------------------------
void Transform::setEyeLiftFactor(double eyeLiftFactor)
{
    // The lift factor will be translated into an eye space scaling of the eye space vertex positions during rendering.
    // Since it is a scaling it moves the vertex more at the far-plane and less at the front-plane.
    // Inspired by the follwing shader code from:
    // http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=252937&Searchpage=6&Main=49189&Words=wireframe&Search=true#Post252937
    //
    // vec4 v = gl_ModelViewMatrix * gl_Vertex;
    // v.xyz = v.xyz * 0.99;
    // gl_Position = gl_ProjectionMatrix * v;
    m_eyeLiftFactor = eyeLiftFactor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Transform::eyeLiftFactor() const
{
    return m_eyeLiftFactor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Transform::addChild(Transform* transform)
{
    CVF_ASSERT(transform);

    transform->m_parent = this;
    m_children.push_back(transform);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Transform::removeAllChildren()
{
    // Detach from all children 
    for (size_t i = 0; i < m_children.size(); ++i)
    {
        m_children[i]->m_parent = NULL;
    }

    m_children.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t Transform::childCount() const
{
    return m_children.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Transform* Transform::child(size_t index)
{
    return m_children[index].p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Transform* Transform::child(size_t index) const
{
    return m_children[index].p();
}

} // namespace cvf

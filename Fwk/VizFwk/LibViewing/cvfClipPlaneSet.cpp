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
#include "cvfClipPlaneSet.h"
#include "cvfUniformSet.h"
#include "cvfCamera.h"
#include "cvfRendering.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::ClipPlaneSet
/// \ingroup Viewing
///
/// A dynamic uniform set used to control model clipping. 
/// 
/// The class is a DynamicUniformSet capable of controlling up to 6 clipping planes. The purpose 
/// of the class is provide two uniforms:
/// * u_clipPlaneCount : the number of active clipping planes
/// * u_ecClipPlanes : The clip planes in eye coordinates
/// 
/// A dynamic uniform is required as the clipping planes needs to be specified in eye coordinates, 
/// and thus needs to be updated before each rendering.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
ClipPlaneSet::ClipPlaneSet()
{
    m_planeCountUniform = new UniformInt("u_clipPlaneCount", 0);
    m_ecPlanesUniformArray = new UniformFloat("u_ecClipPlanes", Vec4f::ZERO);

    m_combinedUniformSet = new UniformSet;
    m_combinedUniformSet->setUniform(m_planeCountUniform.p());
    m_combinedUniformSet->setUniform(m_ecPlanesUniformArray.p());
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
ClipPlaneSet::~ClipPlaneSet()
{
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of planes currently specified
//--------------------------------------------------------------------------------------------------
size_t ClipPlaneSet::planeCount() const
{
    return m_wcPlanes.size();
}


//--------------------------------------------------------------------------------------------------
/// Add a clipping plane. Note that max 6 planes are supported
//--------------------------------------------------------------------------------------------------
void ClipPlaneSet::addPlane(const Plane& plane)
{
    CVF_ASSERT(planeCount() < 6);
    m_wcPlanes.push_back(plane);
}


//--------------------------------------------------------------------------------------------------
/// Remove all specified clipping planes
//--------------------------------------------------------------------------------------------------
void ClipPlaneSet::clear()
{
    m_wcPlanes.clear();
}


//--------------------------------------------------------------------------------------------------
/// Pre-render update to transform the clipping planes into the correct eye coordinates.
//--------------------------------------------------------------------------------------------------
void ClipPlaneSet::update(Rendering* rendering) 
{
    const cvf::Camera* cam = rendering->camera();
    CVF_ASSERT(cam);
    const Mat4d& vm = cam->viewMatrix();

    size_t planeCount = m_wcPlanes.size();
    Vec4fArray planeArray;
    if (planeCount > 0)
    {
        planeArray.resize(planeCount);
        size_t i;
        for (i = 0; i < planeCount; i++)
        {
            Plane ecPlane(m_wcPlanes[i]);
            ecPlane.transform(vm);

            Vec4d planeVec(ecPlane.A(), ecPlane.B(), ecPlane.C(), ecPlane.D());
            planeArray[i] = Vec4f(planeVec);
        }
    }
    else
    {
        // Insert a dummy
        planeArray.resize(1);
        planeArray[0] = Vec4f::ZERO;
    }

    m_planeCountUniform->set(static_cast<int>(planeCount));
    m_ecPlanesUniformArray->setArray(planeArray);
}


//--------------------------------------------------------------------------------------------------
/// Get the combined uniform set consisting of the two uniforms u_clipPlaneCount and u_ecClipPlanes
//--------------------------------------------------------------------------------------------------
UniformSet* ClipPlaneSet::uniformSet() 
{
    return m_combinedUniformSet.p();
}

} // namespace cvf

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
#include "cvfUniform.h"
#include "cvfUniformSet.h"
#include "cvfTransform.h"
#include "cvfCamera.h"
#include "cvfRendering.h"

#include "cvfuPointLight.h"

namespace cvfu {

using cvf::ref;
using cvf::Vec3d;
using cvf::Vec3f;
using cvf::Mat4d;


//==================================================================================================
///
/// \class cvfu::PointLight
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PointLight::PointLight()
:   m_position(0, 0, 0),
    m_uniformSet(new cvf::UniformSet),
    m_markerTransform(new cvf::Transform)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PointLight::~PointLight()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PointLight::setPosition(Vec3d position)
{
    m_position = position;

    Mat4d m;
    m.setTranslation(m_position);
    m_markerTransform->setLocalTransform(m);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d PointLight::position() const
{
    return m_position;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PointLight::update(cvf::Rendering* rendering)
{
    m_uniformSet->setUniform(new cvf::UniformFloat("u_wcLightPosition", Vec3f(m_position)));

    cvf::Camera* cam = rendering->camera();
    const Mat4d& vm = cam->viewMatrix();
    Vec3f ecPos = Vec3f(m_position.getTransformedPoint(vm));
    m_uniformSet->setUniform(new cvf::UniformFloat("u_ecLightPosition", ecPos));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::UniformSet* PointLight::uniformSet()
{
    return m_uniformSet.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Transform* PointLight::markerTransform()
{
    return m_markerTransform.p();
}


} // namespace cvfu





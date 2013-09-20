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
#include "cvfRayIntersectSpec.h"
#include "cvfRay.h"
#include "cvfRendering.h"
#include "cvfCamera.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RayIntersectSpec
/// \ingroup Viewing
///
/// Class is used for picking on Model and Rendering. If contains the Ray to use for picking and
/// any other filtering settings (eg. Camera or enableMask)
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RayIntersectSpec::RayIntersectSpec(const Ray* ray, const Rendering* rendering)
:   m_ray(ray)
{
    if (rendering)
    {
        m_camera = rendering->camera();
        m_cullSettings = rendering->cullSettings();
        m_enableMask = rendering->enableMask();
    }
    else
    {
        m_enableMask = 0xffffffff;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RayIntersectSpec::RayIntersectSpec(const Ray* ray, const Camera* camera, const CullSettings* cullSettings, uint enableMask)
:   m_ray(ray),
    m_camera(camera),
    m_cullSettings(cullSettings),
    m_enableMask(enableMask)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RayIntersectSpec::~RayIntersectSpec()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Ray* RayIntersectSpec::ray() const
{
    return m_ray.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Camera* RayIntersectSpec::camera() const
{
    return m_camera.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const CullSettings* RayIntersectSpec::cullSettings() const
{
    return m_cullSettings.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint RayIntersectSpec::enableMask() const
{
    return m_enableMask;
}


} // namespace cvf


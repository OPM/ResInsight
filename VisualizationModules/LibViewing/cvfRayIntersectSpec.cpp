//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cvfBase.h"
#include "cvfRayIntersectSpec.h"
#include "cvfRay.h"

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
RayIntersectSpec::RayIntersectSpec(const Ray* ray)
:   m_ray(ray)
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


} // namespace cvf


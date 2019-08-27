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
#include "cvfFixedSizeTransform.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::FixedSizeTransform
/// \ingroup Viewing
///
/// Fixed size parts
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FixedSizeTransform::FixedSizeTransform()
{
    m_fixedPixelSizeModelUnits = 1.0;
}


//--------------------------------------------------------------------------------------------------
/// Make the object a constant pixel size
//--------------------------------------------------------------------------------------------------
void FixedSizeTransform::updateWorldTransform(const cvf::Camera* camera)
{
    Transform::updateWorldTransform(camera);
    
    if (m_fixedPixelSizeModelUnits > 0.0)
    {
        cvf::Vec3d objectCenter(0,0,0);
        objectCenter.transformPoint(m_worldMatrix);

        double eyeDist = (objectCenter - camera->position())*camera->direction();

        if (eyeDist > 0)
        {
            double scaleFactor = 1.0;

            if (camera->projection() == Camera::PERSPECTIVE)
            {
                double nearPlane = camera->nearPlane();
                double frontPlanePixelHeigth = camera->frontPlanePixelHeight();
                scaleFactor = (fixedPixelSizeModelUnits()*frontPlanePixelHeigth*eyeDist)/nearPlane;
            }
            else
            {
                scaleFactor = fixedPixelSizeModelUnits()*camera->frontPlanePixelHeight();
            }

            Mat4d scaleMatrix = Mat4d::fromScaling(Vec3d(scaleFactor, scaleFactor, scaleFactor));
            m_worldMatrix = m_worldMatrix*scaleMatrix;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Set the pixel size of a unit size in world coords
/// E.g: Setting pixel size to 5 will make an object 1 units wide in WC 5 pixels
///      or setting this to 0.2 will make a 50 unit size object 10 pixels
//--------------------------------------------------------------------------------------------------
void FixedSizeTransform::setFixedPixelSizeInModelUnits(double pixelSize)
{
    m_fixedPixelSizeModelUnits = pixelSize;
}


//--------------------------------------------------------------------------------------------------
/// Return the pixel size of a unit size in world coords
//--------------------------------------------------------------------------------------------------
double FixedSizeTransform::fixedPixelSizeModelUnits() const
{
    return m_fixedPixelSizeModelUnits;
}

} // namespace cvf



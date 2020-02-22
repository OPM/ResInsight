/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaBoundingBoxTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RiaBoundingBoxTools::inflate( const cvf::BoundingBox& boundingBox, double factor )
{
    cvf::Vec3d center = boundingBox.center();
    cvf::Vec3d sizes  = boundingBox.extent() * factor;

    cvf::Vec3d newMin( center.x() - sizes.x() / 2.0, center.y() - sizes.y() / 2.0, center.z() - sizes.z() / 2.0 );
    cvf::Vec3d newMax( center.x() + sizes.x() / 2.0, center.y() + sizes.y() / 2.0, center.z() + sizes.z() / 2.0 );
    return cvf::BoundingBox( newMin, newMax );
}

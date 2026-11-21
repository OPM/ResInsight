/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#pragma once

#include "RiaViewDefines.h"

#include "cvfBoundingBox.h"
#include "cvfVector3.h"

class Rim3dView;
class RiuViewer;

//==================================================================================================
/// Camera operations on a 3D view. All operations require that the view has a viewer, and return
/// false if the camera was not changed.
//==================================================================================================
class Rim3dViewCameraOperations
{
public:
    // Look at the point of interest from a standard direction, keeping the distance to it
    static bool setViewDirection( Rim3dView& view, RiaDefines::ViewDirection direction );

    // Look at the center of the model along the given direction, at the distance where the whole model fits
    // divided by zoom
    static bool setViewDirectionAndFit( Rim3dView& view, const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection, double zoom );

    // Move the camera to the given bounding box, as done by the "Fly to Object" command
    static bool flyToBoundingBox( Rim3dView& view, const cvf::BoundingBox& boundingBoxInDomainCoords );

    // Returns true if the camera of the viewer is already oriented along the given view direction and up direction
    static bool isCameraOriented( RiuViewer* viewer, const cvf::Vec3d& viewDirection, const cvf::Vec3d& upDirection );
};

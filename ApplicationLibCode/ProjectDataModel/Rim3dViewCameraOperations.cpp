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

#include "Rim3dViewCameraOperations.h"

#include "RiaCameraTools.h"

#include "ContourMap/RimEclipseContourMapView.h"
#include "GeoMech/RimGeoMechContourMapView.h"
#include "Rim3dView.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"

#include "cvfCamera.h"

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dViewCameraOperations::setViewDirection( Rim3dView& view, RiaDefines::ViewDirection direction )
{
    RiuViewer* viewer = view.viewer();
    if ( !viewer ) return false;

    const auto orientation = RiaCameraTools::fromViewDirection( direction );
    viewer->setView( orientation.direction, orientation.up );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dViewCameraOperations::setViewDirectionAndFit( Rim3dView& view, const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection, double zoom )
{
    RiuViewer* viewer = view.viewer();
    if ( !viewer ) return false;

    return viewer->setViewDirectionAndFit( alongDirection, upDirection, zoom );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dViewCameraOperations::flyToBoundingBox( Rim3dView& view, const cvf::BoundingBox& boundingBoxInDomainCoords )
{
    RiuViewer* viewer = view.viewer();
    if ( !viewer || !boundingBoxInDomainCoords.isValid() ) return false;

    const bool isContourMap = ( dynamic_cast<RimEclipseContourMapView*>( &view ) != nullptr ) ||
                              ( dynamic_cast<RimGeoMechContourMapView*>( &view ) != nullptr );

    cvf::ref<caf::DisplayCoordTransform> transForm             = view.displayCoordTransform();
    cvf::Vec3d                           centerInDisplayCoords = transForm->transformToDisplayCoord( boundingBoxInDomainCoords.center() );

    cvf::Vec3d cameraEye;
    cvf::Vec3d cameraUp;

    if ( !isContourMap )
    {
        cvf::Vec3d directionNormalToLargestExtent = cvf::Vec3d::X_AXIS;

        const cvf::Vec3d extent        = boundingBoxInDomainCoords.extent();
        double           largestExtent = std::fabs( extent.y() );
        if ( std::fabs( extent.x() ) > largestExtent )
        {
            largestExtent                  = std::fabs( extent.x() );
            directionNormalToLargestExtent = cvf::Vec3d::Y_AXIS;
        }

        largestExtent = std::max( largestExtent, std::fabs( extent.z() ) * view.scaleZ() );

        // Scale to make the object fit in view
        largestExtent *= 2.0;
        cameraEye = centerInDisplayCoords + directionNormalToLargestExtent * std::max( largestExtent, 30.0 );
        cameraUp  = cvf::Vec3d::Z_AXIS;
    }
    else
    {
        // Keep the current height and up direction, and move to the center of the object
        cvf::Vec3d eye, vrp;
        viewer->mainCamera()->toLookAt( &eye, &vrp, &cameraUp );

        cameraEye     = centerInDisplayCoords;
        cameraEye.z() = eye.z();
    }

    viewer->mainCamera()->setFromLookAt( cameraEye, centerInDisplayCoords, cameraUp );
    viewer->setPointOfInterest( centerInDisplayCoords );

    view.updateDisplayModelForCurrentTimeStepAndRedraw();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dViewCameraOperations::isCameraOriented( RiuViewer* viewer, const cvf::Vec3d& viewDirection, const cvf::Vec3d& upDirection )
{
    if ( !viewer || !viewer->mainCamera() ) return false;

    const double tolerance = 1.0e-6;
    return ( viewer->mainCamera()->direction() - viewDirection ).length() < tolerance &&
           ( viewer->mainCamera()->up() - upDirection ).length() < tolerance;
}
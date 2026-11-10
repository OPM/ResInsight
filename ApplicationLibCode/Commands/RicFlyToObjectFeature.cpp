/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicFlyToObjectFeature.h"

#include "RiaApplication.h"

#include "ContourMap/RimEclipseContourMapView.h"
#include "GeoMech/RimGeoMechContourMapView.h"
#include "Rim3dPropertiesInterface.h"
#include "Rim3dView.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include "cvfCamera.h"

#include <QAction>

#include <algorithm>
#include <cmath>

CAF_CMD_SOURCE_INIT( RicFlyToObjectFeature, "RicFlyToObjectFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicFlyToObjectFeature::isCommandEnabled() const
{
    return RicFlyToObjectFeature::boundingBoxForSelectedObjects().isValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFlyToObjectFeature::onActionTriggered( bool isChecked )
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if ( !activeView ) return;

    bool isContourMap = ( dynamic_cast<RimEclipseContourMapView*>( activeView ) != nullptr ) ||
                        ( dynamic_cast<RimGeoMechContourMapView*>( activeView ) != nullptr );

    RiuViewer* destinationViewer = activeView->viewer();
    if ( !destinationViewer ) return;

    cvf::BoundingBox bb = RicFlyToObjectFeature::boundingBoxForSelectedObjects();
    if ( !bb.isValid() ) return;

    if ( !isContourMap )
    {
        cvf::ref<caf::DisplayCoordTransform> transForm = activeView->displayCoordTransform();

        cvf::Vec3d centerInDisplayCoords = transForm->transformToDisplayCoord( bb.center() );

        cvf::Vec3d directionNormalToLargesExtent = cvf::Vec3d::X_AXIS;
        double     largesExtent                  = fabs( bb.extent().y() );
        if ( fabs( bb.extent().x() ) > largesExtent )
        {
            largesExtent                  = fabs( bb.extent().x() );
            directionNormalToLargesExtent = cvf::Vec3d::Y_AXIS;
        }

        double zExtent = fabs( bb.extent().z() ) * activeView->scaleZ();
        largesExtent   = std::max( largesExtent, zExtent );

        // Scale to make the object fit in view
        largesExtent *= 2.0;
        cvf::Vec3d cameraEye          = centerInDisplayCoords + directionNormalToLargesExtent * std::max( largesExtent, 30.0 );
        cvf::Vec3d cameraViewRefPoint = centerInDisplayCoords;
        cvf::Vec3d cameraUp           = cvf::Vec3d::Z_AXIS;

        destinationViewer->mainCamera()->setFromLookAt( cameraEye, cameraViewRefPoint, cameraUp );
        destinationViewer->setPointOfInterest( cameraViewRefPoint );
    }
    else
    {
        cvf::ref<caf::DisplayCoordTransform> transForm             = activeView->displayCoordTransform();
        cvf::Vec3d                           centerInDisplayCoords = transForm->transformToDisplayCoord( bb.center() );

        // get current camera settings
        cvf::Vec3d eye, vrp, up;
        destinationViewer->mainCamera()->toLookAt( &eye, &vrp, &up );

        cvf::Vec3d cameraEye = centerInDisplayCoords;
        cameraEye.z()        = eye.z(); // keep current zoom level

        // move camera to look at the center of the selected object(s), keeping current height
        destinationViewer->mainCamera()->setFromLookAt( cameraEye, centerInDisplayCoords, up );
        destinationViewer->setPointOfInterest( centerInDisplayCoords );
    }

    activeView->updateDisplayModelForCurrentTimeStepAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFlyToObjectFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Fly to Object" );
    // actionToSetup->setIcon(QIcon(":/3DView16x16.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RicFlyToObjectFeature::boundingBoxForSelectedObjects()
{
    cvf::BoundingBox bb;

    for ( auto obj : caf::SelectionManager::instance()->objectsByType<Rim3dPropertiesInterface>() )
    {
        if ( obj )
        {
            auto candidate = obj->boundingBoxInDomainCoords();
            if ( candidate.isValid() )
            {
                bb.add( candidate );
            }
        }
    }

    return bb;
}

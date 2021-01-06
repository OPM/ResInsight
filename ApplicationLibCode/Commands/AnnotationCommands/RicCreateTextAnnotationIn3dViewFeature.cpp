/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicCreateTextAnnotationIn3dViewFeature.h"

#include "RiaApplication.h"

#include "RimAnnotationInViewCollection.h"
#include "RimCase.h"
#include "RimEclipseContourMapView.h"
#include "RimGridView.h"
#include "RimTextAnnotation.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include "cvfBoundingBox.h"
#include "cvfCamera.h"

#include <QAction>

#include <cmath>

CAF_CMD_SOURCE_INIT( RicCreateTextAnnotationIn3dViewFeature, "RicCreateTextAnnotationIn3dViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateTextAnnotationIn3dViewFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTextAnnotationIn3dViewFeature::onActionTriggered( bool isChecked )

{
    RimGridView*              activeView  = RiaApplication::instance()->activeGridView();
    RimEclipseContourMapView* contMapView = dynamic_cast<RimEclipseContourMapView*>( activeView );

    if ( activeView )
    {
        RimGridView* viewOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();

        cvf::Vec3d       domainCoord = activeView->viewer()->viewerCommands()->lastPickPositionInDomainCoords();
        cvf::BoundingBox bbox        = viewOrComparisonView->ownerCase()->activeCellsBoundingBox();

        if ( contMapView ) domainCoord[2] = bbox.max().z() - bbox.extent().z() * 0.2;

        auto coll = viewOrComparisonView->annotationCollection();

        if ( coll )
        {
            auto newAnnotation = new RimTextAnnotation();
            newAnnotation->setAnchorPoint( domainCoord );
            cvf::Vec3d labelPos = domainCoord;

            cvf::Camera* viewCamera = activeView->viewer()->mainCamera();

            if ( viewCamera->direction().z() <= 0 )
            {
                labelPos.z() = bbox.max().z();
            }
            else
            {
                labelPos.z() = bbox.min().z();
            }

            cvf::Vec3d horizontalRight = viewCamera->direction() ^ cvf::Vec3d::Z_AXIS;
            cvf::Vec3d horizontalUp = viewCamera->up() - ( cvf::Vec3d::Z_AXIS * ( viewCamera->up() * cvf::Vec3d::Z_AXIS ) );

            bool isOk = horizontalRight.normalize();
            if ( !isOk ) horizontalRight = { 1.0, 0.0, 0.0 };

            double height = fabs( labelPos.z() - domainCoord.z() );
            newAnnotation->setLabelPoint( labelPos + 2.0 * height * ( horizontalRight + horizontalUp ) );

            coll->addAnnotation( newAnnotation );
            coll->scheduleRedrawOfRelevantViews();
            coll->updateConnectedEditors();

            RiuMainWindow::instance()->selectAsCurrentItem( newAnnotation, false );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTextAnnotationIn3dViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/TextAnnotation16x16.png" ) );
    actionToSetup->setText( "Create Text Annotation" );
}

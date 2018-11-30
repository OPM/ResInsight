/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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

#include "RimTextAnnotation.h"
#include "RimAnnotationInViewCollection.h"
#include "RimGridView.h"
#include "RimCase.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include <QAction>

#include "cvfBoundingBox.h"
#include "cvfCamera.h"


CAF_CMD_SOURCE_INIT(RicCreateTextAnnotationIn3dViewFeature, "RicCreateTextAnnotationIn3dViewFeature");

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
void RicCreateTextAnnotationIn3dViewFeature::onActionTriggered(bool isChecked)

{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if ( activeView )
    {
        cvf::Vec3d domainCoord = activeView->viewer()->lastPickPositionInDomainCoords();
        cvf::BoundingBox bbox = activeView->ownerCase()->activeCellsBoundingBox();

        auto coll =  activeView->annotationCollection();
       
        if ( coll )
        {
            auto newAnnotation = new RimTextAnnotation();
            newAnnotation->setAnchorPoint(domainCoord);
            cvf::Vec3d labelPos = domainCoord;
            labelPos.z() = bbox.max().z();
            double height = labelPos.z() - domainCoord.z();
            cvf::Vec3d horizontalRight = activeView->viewer()->mainCamera()->direction() ^ cvf::Vec3d::Z_AXIS;
            bool isOk = horizontalRight.normalize();
            if (!isOk) horizontalRight = {1.0, 0.0, 0.0};
            newAnnotation->setLabelPoint(labelPos + horizontalRight*0.5*height);

            coll->addAnnotation(newAnnotation);
            coll->scheduleRedrawOfRelevantViews();
            coll->updateConnectedEditors();

            RiuMainWindow::instance()->selectAsCurrentItem(newAnnotation);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTextAnnotationIn3dViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/TextAnnotation16x16.png"));
    actionToSetup->setText("Create Text Annotation");
}


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

CAF_CMD_SOURCE_INIT(RicFlyToObjectFeature, "RicFlyToObjectFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicFlyToObjectFeature::isCommandEnabled()
{
    if (RicFlyToObjectFeature::boundingBoxForSelectedObjects().isValid())
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFlyToObjectFeature::onActionTriggered(bool isChecked)
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    RiuViewer* destinationViewer = activeView->viewer();
    if (!destinationViewer) return;

    cvf::BoundingBox bb = RicFlyToObjectFeature::boundingBoxForSelectedObjects();
    CVF_ASSERT(bb.isValid());

    cvf::ref<caf::DisplayCoordTransform> transForm = activeView->displayCoordTransform();

    cvf::Vec3d centerInDisplayCoords = transForm->transformToDisplayCoord(bb.center());

    cvf::Vec3d directionNormalToLargesExtent = cvf::Vec3d::X_AXIS;
    double largesExtent = fabs(bb.extent().y());
    if (fabs(bb.extent().x()) > largesExtent)
    {
        largesExtent = fabs(bb.extent().x());
        directionNormalToLargesExtent = cvf::Vec3d::Y_AXIS;
    }

    cvf::Vec3d cameraEye = centerInDisplayCoords + directionNormalToLargesExtent * std::max(largesExtent, 30.0);
    cvf::Vec3d cameraViewRefPoint = centerInDisplayCoords;
    cvf::Vec3d cameraUp = cvf::Vec3d::Z_AXIS;

    destinationViewer->mainCamera()->setFromLookAt(cameraEye, cameraViewRefPoint, cameraUp);

    destinationViewer->setPointOfInterest(cameraViewRefPoint);
    
    activeView->updateCurrentTimeStepAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFlyToObjectFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Fly to Object");
    //actionToSetup->setIcon(QIcon(":/3DView16x16.png"));
}    

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RicFlyToObjectFeature::boundingBoxForSelectedObjects()
{
    cvf::BoundingBox bb;

    std::vector<Rim3dPropertiesInterface*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    for (auto obj : objects)
    {
        if (obj)
        {
            bb.add(obj->boundingBoxInDomainCoords());
        }
    }

    return bb;
}


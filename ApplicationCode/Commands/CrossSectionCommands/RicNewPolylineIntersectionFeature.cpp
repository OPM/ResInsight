/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewPolylineIntersectionFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimView.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"
#include "RiuViewer.h"

#include "cafCmdExecCommandManager.h"
#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewPolylineIntersectionFeature, "RicNewPolylineIntersectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewPolylineIntersectionFeature::RicNewPolylineIntersectionFeature()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPolylineIntersectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineIntersectionFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;
   
    RicNewPolylineIntersectionFeatureCmd* cmd = new RicNewPolylineIntersectionFeatureCmd(activeView->crossSectionCollection);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineIntersectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Polyline Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPolylineIntersectionFeature::handleEvent(cvf::Object* eventObject)
{
    std::vector<RimIntersection*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() == 1)
    {
        RicViewerEventObject* polylineUiEvent = dynamic_cast<RicViewerEventObject*>(eventObject);
        if (polylineUiEvent)
        {
            RimIntersection* intersection = selection[0];

            RimView* rimView = nullptr;
            intersection->firstAncestorOrThisOfType(rimView);
            CVF_ASSERT(rimView);

            cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
            cvf::Vec3d domainCoord = transForm->transformToDomainCoord(polylineUiEvent->globalIntersectionPoint);

            if (intersection->inputPolyLineFromViewerEnabled())
            {
                intersection->appendPointToPolyLine(domainCoord);

                // Further Ui processing is stopped when true is returned
                return true;
            }
            else if (intersection->inputExtrusionPointsFromViewerEnabled())
            {
                intersection->appendPointToExtrusionDirection(domainCoord);

                // Further Ui processing is stopped when true is returned
                return true;
            }
            else if (intersection->inputTwoAzimuthPointsFromViewerEnabled())
            {
                intersection->appendPointToAzimuthLine(domainCoord);

                // Further Ui processing is stopped when true is returned
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewPolylineIntersectionFeatureCmd::RicNewPolylineIntersectionFeatureCmd(RimIntersectionCollection* intersectionCollection)
    : CmdExecuteCommand(NULL),
    m_intersectionCollection(intersectionCollection)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewPolylineIntersectionFeatureCmd::~RicNewPolylineIntersectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewPolylineIntersectionFeatureCmd::name()
{
    return "Start Polyline Intersection";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineIntersectionFeatureCmd::redo()
{
    CVF_ASSERT(m_intersectionCollection);

    RimIntersection* intersection = new RimIntersection();
    intersection->name = "Polyline";
    intersection->type = RimIntersection::CS_POLYLINE;
    intersection->inputPolyLineFromViewerEnabled = true;

    m_intersectionCollection->appendIntersection(intersection);

    RiuSelectionManager::instance()->deleteAllItems();

    RiuMainWindow::instance()->selectAsCurrentItem(intersection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineIntersectionFeatureCmd::undo()
{
}

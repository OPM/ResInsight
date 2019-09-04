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
#include "RimGridView.h"
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
#include "RiuViewer.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewPolylineIntersectionFeature, "RicNewPolylineIntersectionFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewPolylineIntersectionFeature::RicNewPolylineIntersectionFeature() {}

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
    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if (!activeView) return;

    RicNewPolylineIntersectionFeatureCmd* cmd = new RicNewPolylineIntersectionFeatureCmd(activeView->crossSectionCollection());
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolylineIntersectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("Polyline Intersection");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewPolylineIntersectionFeatureCmd::RicNewPolylineIntersectionFeatureCmd(RimIntersectionCollection* intersectionCollection)
    : CmdExecuteCommand(nullptr)
    , m_intersectionCollection(intersectionCollection)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewPolylineIntersectionFeatureCmd::~RicNewPolylineIntersectionFeatureCmd() {}

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

    RimIntersection* intersection                = new RimIntersection();
    intersection->name                           = "Polyline";
    intersection->type                           = RimIntersection::CS_POLYLINE;
    intersection->inputPolyLineFromViewerEnabled = true;

    m_intersectionCollection->appendIntersectionAndUpdate(intersection);

    Riu3dSelectionManager::instance()->deleteAllItems();

    Riu3DMainWindowTools::selectAsCurrentItem(intersection);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolylineIntersectionFeatureCmd::undo() {}

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

#include "RicNewWellPathIntersectionFeature.h"

#include "RiaApplication.h"

#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimWellPath.h"
#include "Rim3dView.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewWellPathIntersectionFeature, "RicNewWellPathIntersectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewWellPathIntersectionFeature::RicNewWellPathIntersectionFeature()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathIntersectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathIntersectionFeature::onActionTriggered(bool isChecked)
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if (!activeView) return;

    std::vector<RimWellPath*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    CVF_ASSERT(collection.size() == 1);

    RimWellPath* wellPath = collection[0];
    
    RicNewWellPathIntersectionFeatureCmd* cmd = new RicNewWellPathIntersectionFeatureCmd(activeView->crossSectionCollection, wellPath);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathIntersectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewWellPathIntersectionFeatureCmd::RicNewWellPathIntersectionFeatureCmd(RimIntersectionCollection* intersectionCollection, RimWellPath* wellPath)
    : CmdExecuteCommand(NULL),
    m_intersectionCollection(intersectionCollection),
    m_wellPath(wellPath)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewWellPathIntersectionFeatureCmd::~RicNewWellPathIntersectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewWellPathIntersectionFeatureCmd::name()
{
    return "Create Intersection From Well Path";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathIntersectionFeatureCmd::redo()
{
    CVF_ASSERT(m_intersectionCollection);
    CVF_ASSERT(m_wellPath);

    RimIntersection* intersection = new RimIntersection();
    intersection->name = m_wellPath->name();
    intersection->type = RimIntersection::CS_WELL_PATH;
    intersection->wellPath = m_wellPath;

    m_intersectionCollection->appendIntersection(intersection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathIntersectionFeatureCmd::undo()
{
}

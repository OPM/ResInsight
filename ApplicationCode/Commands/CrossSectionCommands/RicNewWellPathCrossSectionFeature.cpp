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

#include "RicNewWellPathCrossSectionFeature.h"

#include "RiaApplication.h"

#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimWellPath.h"
#include "RimView.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewWellPathCrossSectionFeature, "RicNewWellPathCrossSectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewWellPathCrossSectionFeature::RicNewWellPathCrossSectionFeature()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathCrossSectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCrossSectionFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;

    std::vector<RimWellPath*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    CVF_ASSERT(collection.size() == 1);

    RimWellPath* wellPath = collection[0];
    
    RicNewWellPathCrossSectionFeatureCmd* cmd = new RicNewWellPathCrossSectionFeatureCmd(activeView->crossSectionCollection, wellPath);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCrossSectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewWellPathCrossSectionFeatureCmd::RicNewWellPathCrossSectionFeatureCmd(RimIntersectionCollection* crossSectionCollection, RimWellPath* wellPath)
    : CmdExecuteCommand(NULL),
    m_crossSectionCollection(crossSectionCollection),
    m_wellPath(wellPath)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewWellPathCrossSectionFeatureCmd::~RicNewWellPathCrossSectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewWellPathCrossSectionFeatureCmd::name()
{
    return "Create Intersection From Well Path";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCrossSectionFeatureCmd::redo()
{
    CVF_ASSERT(m_crossSectionCollection);
    CVF_ASSERT(m_wellPath);

    RimIntersection* crossSection = new RimIntersection();
    crossSection->name = m_wellPath->name;
    crossSection->type = RimIntersection::CS_WELL_PATH;
    crossSection->wellPath = m_wellPath;

    m_crossSectionCollection->appendCrossSection(crossSection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCrossSectionFeatureCmd::undo()
{
}

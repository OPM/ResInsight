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

#include "RimCrossSection.h"
#include "RimCrossSectionCollection.h"
#include "RimWellPath.h"

#include "RiuMainWindow.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewWellPathCrossSectionFeature, "RicNewWellPathCrossSectionFeature");

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
/*
    std::vector<RimEclipseWell*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    CVF_ASSERT(collection.size() == 1);

    RimEclipseWell* eclWell = collection[0];
    
    RimEclipseView* eclView = NULL;
    eclWell->firstAnchestorOrThisOfType(eclView);
    CVF_ASSERT(eclView);

    RicNewWellPathCrossSectionFeatureCmd* cmd = new RicNewWellPathCrossSectionFeatureCmd(eclView->crossSectionCollection, eclWell);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCrossSectionFeature::setupActionLook(QAction* actionToSetup)
{
//    actionToSetup->setIcon(QIcon(":/CellFilter_Values.png"));
    actionToSetup->setText("New Cross Section");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewWellPathCrossSectionFeatureCmd::RicNewWellPathCrossSectionFeatureCmd(RimCrossSectionCollection* crossSectionCollection, RimWellPath* wellPath)
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
    return "Create Cross Section From Well Path";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCrossSectionFeatureCmd::redo()
{
    CVF_ASSERT(m_crossSectionCollection);
    CVF_ASSERT(m_wellPath);

    RimCrossSection* crossSection = new RimCrossSection();
    crossSection->name = m_wellPath->name;
    crossSection->crossSectionType = RimCrossSection::CS_WELL_PATH;
    crossSection->wellPath = m_wellPath;

    m_crossSectionCollection->crossSections.push_back(crossSection);

    m_crossSectionCollection->updateConnectedEditors();
    RiuMainWindow::instance()->setCurrentObjectInTreeView(crossSection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathCrossSectionFeatureCmd::undo()
{
}

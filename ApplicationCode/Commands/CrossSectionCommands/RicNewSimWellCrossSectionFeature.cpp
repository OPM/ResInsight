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

#include "RicNewSimWellCrossSectionFeature.h"

#include "RimCrossSection.h"
#include "RimCrossSectionCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewSimWellCrossSectionFeature, "RicNewSimWellCrossSectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSimWellCrossSectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellCrossSectionFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimEclipseWell*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    CVF_ASSERT(collection.size() == 1);

    RimEclipseWell* eclWell = collection[0];
    
    RimEclipseView* eclView = NULL;
    eclWell->firstAnchestorOrThisOfType(eclView);
    CVF_ASSERT(eclView);

    RicNewSimWellCrossSectionCmd* cmd = new RicNewSimWellCrossSectionCmd(eclView->crossSectionCollection, eclWell);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellCrossSectionFeature::setupActionLook(QAction* actionToSetup)
{
//    actionToSetup->setIcon(QIcon(":/CellFilter_Values.png"));
    actionToSetup->setText("New Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewSimWellCrossSectionCmd::RicNewSimWellCrossSectionCmd(RimCrossSectionCollection* crossSectionCollection, RimEclipseWell* simWell)
    : CmdExecuteCommand(NULL),
    m_crossSectionCollection(crossSectionCollection),
    m_wellPath(simWell)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewSimWellCrossSectionCmd::~RicNewSimWellCrossSectionCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewSimWellCrossSectionCmd::name()
{
    return "Create Intersection From Well";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellCrossSectionCmd::redo()
{
    CVF_ASSERT(m_crossSectionCollection);
    CVF_ASSERT(m_wellPath);

    RimCrossSection* crossSection = new RimCrossSection();
    crossSection->name = m_wellPath->name;
    crossSection->type = RimCrossSection::CS_SIMULATION_WELL;
    crossSection->simulationWell = m_wellPath;

    m_crossSectionCollection->appendCrossSection(crossSection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellCrossSectionCmd::undo()
{
}

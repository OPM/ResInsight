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

#include "RicNewSimWellIntersectionFeature.h"

#include "RimEclipseView.h"
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimSimWellInView.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewSimWellIntersectionFeature, "RicNewSimWellIntersectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSimWellIntersectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellIntersectionFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    CVF_ASSERT(collection.size() == 1);

    RimSimWellInView* simWell = collection[0];
    
    RimEclipseView* eclView = NULL;
    simWell->firstAncestorOrThisOfType(eclView);
    CVF_ASSERT(eclView);

    RicNewSimWellIntersectionCmd* cmd = new RicNewSimWellIntersectionCmd(eclView->crossSectionCollection(), simWell);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellIntersectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewSimWellIntersectionCmd::RicNewSimWellIntersectionCmd(RimIntersectionCollection* intersectionCollection, RimSimWellInView* simWell)
    : CmdExecuteCommand(NULL),
    m_intersectionCollection(intersectionCollection),
    m_simWell(simWell)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewSimWellIntersectionCmd::~RicNewSimWellIntersectionCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewSimWellIntersectionCmd::name()
{
    return "Create Intersection From Well";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellIntersectionCmd::redo()
{
    CVF_ASSERT(m_intersectionCollection);
    CVF_ASSERT(m_simWell);

    RimIntersection* intersection = new RimIntersection();
    intersection->name = m_simWell->name;
    intersection->type = RimIntersection::CS_SIMULATION_WELL;
    intersection->simulationWell = m_simWell;

    m_intersectionCollection->appendIntersectionAndUpdate(intersection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellIntersectionCmd::undo()
{
}

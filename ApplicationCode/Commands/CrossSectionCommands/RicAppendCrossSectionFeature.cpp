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

#include "RicAppendCrossSectionFeature.h"

#include "RimCrossSection.h"
#include "RimCrossSectionCollection.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicAppendCrossSectionFeature, "RicAppendCrossSectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAppendCrossSectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendCrossSectionFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimCrossSectionCollection*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    CVF_ASSERT(collection.size() == 1);

    RicAppendCrossSectionFeatureCmd* cmd = new RicAppendCrossSectionFeatureCmd(collection[0]);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendCrossSectionFeature::setupActionLook(QAction* actionToSetup)
{
//    actionToSetup->setIcon(QIcon(":/CellFilter_Values.png"));
    actionToSetup->setText("New Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicAppendCrossSectionFeatureCmd::RicAppendCrossSectionFeatureCmd(RimCrossSectionCollection* crossSectionCollection)
    : CmdExecuteCommand(NULL),
    m_crossSectionCollection(crossSectionCollection)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicAppendCrossSectionFeatureCmd::~RicAppendCrossSectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicAppendCrossSectionFeatureCmd::name()
{
    return "New Intersection";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendCrossSectionFeatureCmd::redo()
{
    CVF_ASSERT(m_crossSectionCollection);

    RimCrossSection* crossSection = new RimCrossSection();
    crossSection->name = QString("Intersection");
    m_crossSectionCollection->appendCrossSection(crossSection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendCrossSectionFeatureCmd::undo()
{
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicAppendIntersectionBoxFeature.h"

#include "RimIntersectionBox.h"
#include "RimIntersectionBoxCollection.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicAppendIntersectionBoxFeature, "RicAppendIntersectionBoxFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAppendIntersectionBoxFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionBoxFeature::onActionTriggered(bool isChecked)
{
    std::vector<caf::PdmObjectHandle*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    CVF_ASSERT(collection.size() == 1);

    RimIntersectionBoxCollection* crossSectionCollection = NULL;
    collection[0]->firstAnchestorOrThisOfType(crossSectionCollection);

    CVF_ASSERT(crossSectionCollection);

    crossSectionCollection->appendIntersectionBox();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionBoxFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/IntersectionBox16x16.png"));
    actionToSetup->setText("New Intersection Box");
}


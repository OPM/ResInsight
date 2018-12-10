/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RicDeleteWellPathAttributeFeature.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include <QAction>

CAF_CMD_SOURCE_INIT(RicDeleteWellPathAttributeFeature, "RicDeleteWellPathAttributeFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeleteWellPathAttributeFeature::isCommandEnabled()
{
    {
        std::vector<RimWellPathAttribute*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects, caf::SelectionManager::FIRST_LEVEL);

        if ( objects.size() > 0 )
        {
            return true;
        }
    }
    {
        if (caf::SelectionManager::instance()->selectedItemOfType<RimWellPathAttributeCollection>())
        {
            return true;
        }

    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteWellPathAttributeFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPathAttribute*> attributes;
    caf::SelectionManager::instance()->objectsByType(&attributes, caf::SelectionManager::FIRST_LEVEL);
    RimWellPathAttributeCollection* wellPathAttributeCollection = nullptr;
    if (attributes.size() > 0)
    {               
        attributes[0]->firstAncestorOrThisOfTypeAsserted(wellPathAttributeCollection);
        for (RimWellPathAttribute* attributeToDelete : attributes)
        {
            wellPathAttributeCollection->deleteAttribute(attributeToDelete);
        }
        wellPathAttributeCollection->updateAllRequiredEditors();
    }
    else
    {
        wellPathAttributeCollection = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathAttributeCollection>();
        if (wellPathAttributeCollection)
        {
            wellPathAttributeCollection->deleteAllAttributes();
        }
    }

    if (wellPathAttributeCollection)
    {
        if (wellPathAttributeCollection->attributes().empty())
        {
            RimWellPath* wellPath = nullptr;
            wellPathAttributeCollection->firstAncestorOrThisOfTypeAsserted(wellPath);
            wellPath->updateConnectedEditors();
            Riu3DMainWindowTools::selectAsCurrentItem(wellPath);
        }

        RimProject* proj = nullptr;
        wellPathAttributeCollection->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            proj->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteWellPathAttributeFeature::setupActionLook(QAction* actionToSetup)
{
    std::vector<RimWellPathAttribute*> attributes;
    caf::SelectionManager::instance()->objectsByType(&attributes, caf::SelectionManager::FIRST_LEVEL);
    if (attributes.size() > 0)
    {
        actionToSetup->setText("Delete Attribute");
        actionToSetup->setIcon(QIcon(":/Erase.png"));
    }
    else if (caf::SelectionManager::instance()->selectedItemOfType<RimWellPathAttributeCollection>())
    {
        actionToSetup->setText("Delete Casing Design");
        actionToSetup->setIcon(QIcon(":/Erase.png"));
    }
}

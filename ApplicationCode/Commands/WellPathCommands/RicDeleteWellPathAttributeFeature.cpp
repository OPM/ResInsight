/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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

#include "RimWellPathAttributeCollection.h"
#include "RimWellPath.h"

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

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteWellPathAttributeFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPathAttribute*> attributes;
    caf::SelectionManager::instance()->objectsByType(&attributes, caf::SelectionManager::FIRST_LEVEL);
    if (attributes.size() > 0)
    {       
        RimWellPathAttributeCollection* wellPathAttributeCollection = nullptr;
        attributes[0]->firstAncestorOrThisOfTypeAsserted(wellPathAttributeCollection);
        RimWellPath* wellPath = nullptr;
        wellPathAttributeCollection->firstAncestorOrThisOfTypeAsserted(wellPath);
        for (RimWellPathAttribute* attributeToDelete : attributes)
        {
            wellPathAttributeCollection->deleteAttribute(attributeToDelete);
        }
        wellPath->updateConnectedEditors();        
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteWellPathAttributeFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Attribute");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

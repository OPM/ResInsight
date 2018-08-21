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
#include "RicNewWellPathAttributeFeature.h"

#include "RimWellPathAttributeCollection.h"
#include "RimWellPathTarget.h"
#include "RimModeledWellPath.h"
#include "cafSelectionManager.h"
#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewWellPathAttributeFeature, "RicNewWellPathAttributeFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathAttributeFeature::isCommandEnabled()
{
    {
        std::vector<RimWellPath*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects);

        if ( objects.size() > 0 )
        {
            return true;
        }
    }
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
void RicNewWellPathAttributeFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPathAttribute*> attributes;
    caf::SelectionManager::instance()->objectsByType(&attributes, caf::SelectionManager::FIRST_LEVEL);
    if (attributes.size() > 0)
    {
        RimWellPathAttributeCollection* attributeCollection = nullptr;
        attributes[0]->firstAncestorOrThisOfTypeAsserted(attributeCollection);
        RimWellPath* wellPath = nullptr;
        attributeCollection->firstAncestorOrThisOfTypeAsserted(wellPath);
        attributes[0]->updateConnectedEditors();
        attributeCollection->insertAttribute(attributes[0], new RimWellPathAttribute);
        wellPath->updateConnectedEditors();
        return;
    }

    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByType(&wellPaths);

    if (wellPaths.size() > 0)
    {
        std::vector<RimWellPathAttributeCollection*> attributeCollections;
        wellPaths[0]->descendantsIncludingThisOfType(attributeCollections);
        if (attributeCollections.size() > 0)
        {
            attributeCollections[0]->insertAttribute(nullptr, new RimWellPathAttribute);
        }
        wellPaths[0]->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathAttributeFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Attribute");
    actionToSetup->setIcon(QIcon(":/Well.png"));
}



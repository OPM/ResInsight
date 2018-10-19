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
#include "RicNewWellPathAttributeFeature.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewWellPathAttributeFeature, "RicNewWellPathAttributeFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathAttributeFeature::isCommandEnabled()
{
    {
        std::vector<RimWellPathAttribute*> objects;
        caf::SelectionManager::instance()->objectsByType(&objects, caf::SelectionManager::FIRST_LEVEL);

        if (objects.size() > 0)
        {
            return true;
        }
    }

    {
        if (caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>())
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
    if (attributes.size() == 1u)
    {
        RimWellPathAttributeCollection* attributeCollection = nullptr;
        attributes[0]->firstAncestorOrThisOfTypeAsserted(attributeCollection);

        RimWellPathAttribute* attribute = new RimWellPathAttribute;
        RimWellPath* wellPath = nullptr;
        attributeCollection->firstAncestorOrThisOfTypeAsserted(wellPath);

        attribute->setDepthsFromWellPath(wellPath);
        attributeCollection->insertAttribute(attributes[0], attribute);

        attributeCollection->updateAllRequiredEditors();
        return;
    }

    RimWellPath* wellPath = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>();
    if (wellPath)
    {
        std::vector<RimWellPathAttributeCollection*> attributeCollections;
        wellPath->descendantsIncludingThisOfType(attributeCollections);
        if (!attributeCollections.empty())
        {
            RimWellPathAttribute* attribute = new RimWellPathAttribute;
            attribute->setDepthsFromWellPath(wellPath);

            attributeCollections[0]->insertAttribute(nullptr, attribute);
            attributeCollections[0]->updateAllRequiredEditors();

            wellPath->updateConnectedEditors();
            Riu3DMainWindowTools::selectAsCurrentItem(attributeCollections[0]);

        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathAttributeFeature::setupActionLook(QAction* actionToSetup)
{
    std::vector<RimWellPathAttribute*> attributes;
    caf::SelectionManager::instance()->objectsByType(&attributes, caf::SelectionManager::FIRST_LEVEL);
    if (attributes.size() == 1u)
    {
        actionToSetup->setText(QString("Insert New Attribute before %1").arg(attributes[0]->componentTypeLabel()));
        actionToSetup->setIcon(QIcon(":/Well.png"));
    }    
    else if (caf::SelectionManager::instance()->selectedItemOfType<RimWellPathAttributeCollection>())
    {
        actionToSetup->setText("Append New Attribute");
        actionToSetup->setIcon(QIcon(":/Well.png"));
    }
    else if(caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>())
    {
        actionToSetup->setText("Create Casing Design");
        actionToSetup->setIcon(QIcon(":/Well.png"));
    }
}



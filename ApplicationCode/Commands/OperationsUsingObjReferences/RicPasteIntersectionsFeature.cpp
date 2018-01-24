/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicPasteIntersectionsFeature.h"

#include "RicPasteFeatureImpl.h"

#include "RimIntersection.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicPasteIntersectionsFeature, "RicPasteIntersectionsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteIntersectionsFeature::isCommandEnabled()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimIntersection>> intersectionObjects;
    objectGroup.objectsByType(&intersectionObjects);

    std::vector<caf::PdmPointer<RimIntersectionBox>> intersectionBoxObjects;
    objectGroup.objectsByType(&intersectionBoxObjects);

    if (intersectionObjects.empty() && intersectionBoxObjects.empty())
    {
        return false;
    }

    caf::PdmObjectHandle* destinationObject =
        dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    if (findIntersectionCollection(destinationObject))
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteIntersectionsFeature::onActionTriggered(bool isChecked)
{
    caf::PdmObjectHandle* destinationObject =
        dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimIntersectionCollection* intersectionCollection =
        RicPasteIntersectionsFeature::findIntersectionCollection(destinationObject);

    CAF_ASSERT(intersectionCollection);

    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    if (objectGroup.objects.size() == 0) return;

    std::vector<caf::PdmPointer<RimIntersection>> intersectionObjects;
    objectGroup.objectsByType(&intersectionObjects);

    for (size_t i = 0; i < intersectionObjects.size(); i++)
    {
        RimIntersection* intersection = dynamic_cast<RimIntersection*>(
            intersectionObjects[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));

        QString nameOfCopy = QString("Copy of ") + intersection->name;
        intersection->name = nameOfCopy;

        if (i == intersectionObjects.size() - 1)
        {
            intersectionCollection->appendIntersectionAndUpdate(intersection);
        }
        else
        {
            intersectionCollection->appendIntersectionNoUpdate(intersection);
        }
    }

    std::vector<caf::PdmPointer<RimIntersectionBox>> intersectionBoxObjects;
    objectGroup.objectsByType(&intersectionBoxObjects);

    for (size_t i = 0; i < intersectionBoxObjects.size(); i++)
    {
        RimIntersectionBox* intersectionBox = dynamic_cast<RimIntersectionBox*>(
            intersectionBoxObjects[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));

        QString nameOfCopy    = QString("Copy of ") + intersectionBox->name;
        intersectionBox->name = nameOfCopy;

        if (i == intersectionBoxObjects.size() - 1)
        {
            intersectionCollection->appendIntersectionBoxAndUpdate(intersectionBox);
        }
        else
        {
            intersectionCollection->appendIntersectionBoxNoUpdate(intersectionBox);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteIntersectionsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste (Intersections)");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection* RicPasteIntersectionsFeature::findIntersectionCollection(caf::PdmObjectHandle* objectHandle)
{
    RimIntersectionCollection* intersectionCollection = dynamic_cast<RimIntersectionCollection*>(objectHandle);
    if (intersectionCollection)
    {
        return intersectionCollection;
    }

    RimIntersection* intersection = dynamic_cast<RimIntersection*>(objectHandle);
    if (intersection)
    {
        intersection->firstAncestorOrThisOfType(intersectionCollection);
        return intersectionCollection;
    }

    RimIntersectionBox* intersectionBox = dynamic_cast<RimIntersectionBox*>(objectHandle);
    if (intersectionBox)
    {
        intersectionBox->firstAncestorOrThisOfType(intersectionCollection);
        return intersectionCollection;
    }

    return nullptr;
}

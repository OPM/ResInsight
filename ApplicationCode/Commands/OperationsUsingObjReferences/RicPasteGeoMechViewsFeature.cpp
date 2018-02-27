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

#include "RicPasteGeoMechViewsFeature.h"

#include "RicPasteFeatureImpl.h"

#include "Riu3DMainWindowTools.h"

#include "RimGeoMechView.h"
#include "RimGeoMechCase.h"

#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>
#include "Rim2dIntersectionViewCollection.h"


CAF_CMD_SOURCE_INIT(RicPasteGeoMechViewsFeature, "RicPasteGeoMechViewsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteGeoMechViewsFeature::isCommandEnabled()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimGeoMechView> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    if (typedObjects.size() == 0)
    {
        return false;
    }

    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimGeoMechCase* geoMechCase = RicPasteFeatureImpl::findGeoMechCase(destinationObject);
    if (geoMechCase) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteGeoMechViewsFeature::onActionTriggered(bool isChecked)
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimGeoMechCase* geomCase = RicPasteFeatureImpl::findGeoMechCase(destinationObject);
    assert(geomCase);

    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    if (objectGroup.objects.size() == 0) return;

    std::vector<caf::PdmPointer<RimGeoMechView> > geomViews;
    objectGroup.objectsByType(&geomViews);

    RimGeoMechView* lastViewCopy = nullptr;

    // Add cases to case group
    for (size_t i = 0; i < geomViews.size(); i++)
    {
        RimGeoMechView* rimReservoirView = dynamic_cast<RimGeoMechView*>(geomViews[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
        QString nameOfCopy = QString("Copy of ") + rimReservoirView->name;
        rimReservoirView->name = nameOfCopy;
        geomCase->geoMechViews().push_back(rimReservoirView);

        rimReservoirView->setGeoMechCase(geomCase);

        // Resolve references after reservoir view has been inserted into Rim structures
        // Intersections referencing a well path requires this
        rimReservoirView->resolveReferencesRecursively();
        rimReservoirView->initAfterReadRecursively();

        caf::PdmDocument::updateUiIconStateRecursively(rimReservoirView);

        rimReservoirView->loadDataAndUpdate();

        geomCase->intersectionViewCollection()->syncFromExistingIntersections(false);
        geomCase->updateConnectedEditors();

        lastViewCopy = rimReservoirView;
    }

    if (lastViewCopy) Riu3DMainWindowTools::selectAsCurrentItem(lastViewCopy);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteGeoMechViewsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste (Geo Mech Views)");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

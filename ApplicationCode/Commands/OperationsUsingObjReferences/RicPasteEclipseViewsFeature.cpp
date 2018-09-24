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

#include "RicPasteEclipseViewsFeature.h"

#include "RiaApplication.h"

#include "Riu3DMainWindowTools.h"

#include "RicPasteFeatureImpl.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "Rim2dIntersectionViewCollection.h"

#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicPasteEclipseViewsFeature, "RicPasteEclipseViewsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteEclipseViewsFeature::isCommandEnabled()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimEclipseView> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    if (typedObjects.size() == 0)
    {
        return false;
    }

    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimIdenticalGridCaseGroup* gridCaseGroup = RicPasteFeatureImpl::findGridCaseGroup(destinationObject);
    if (gridCaseGroup) return false;

    RimEclipseCase* eclipseCase = RicPasteFeatureImpl::findEclipseCase(destinationObject);
    if (eclipseCase) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteEclipseViewsFeature::onActionTriggered(bool isChecked)
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimEclipseCase* eclipseCase = RicPasteFeatureImpl::findEclipseCase(destinationObject);
    assert(eclipseCase);

    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    if (objectGroup.objects.size() == 0) return;

    std::vector<caf::PdmPointer<RimEclipseView> > eclipseViews;
    objectGroup.objectsByType(&eclipseViews);

    RimEclipseView* lastViewCopy = nullptr;

    // Add cases to case group
    for (size_t i = 0; i < eclipseViews.size(); i++)
    {
        RimEclipseView* rimReservoirView = dynamic_cast<RimEclipseView*>(eclipseViews[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
        CVF_ASSERT(rimReservoirView);

        QString nameOfCopy = QString("Copy of ") + rimReservoirView->name();
        rimReservoirView->setName(nameOfCopy);
        eclipseCase->reservoirViews().push_back(rimReservoirView);

        rimReservoirView->setEclipseCase(eclipseCase);

        // Resolve references after reservoir view has been inserted into Rim structures
        // Intersections referencing a well path/ simulation well requires this
        rimReservoirView->resolveReferencesRecursively();
        rimReservoirView->initAfterReadRecursively();

        eclipseCase->intersectionViewCollection()->syncFromExistingIntersections(false);
        rimReservoirView->loadDataAndUpdate();

        caf::PdmDocument::updateUiIconStateRecursively(rimReservoirView);

        eclipseCase->updateConnectedEditors();
        lastViewCopy = rimReservoirView;
    }

    if (lastViewCopy) Riu3DMainWindowTools::selectAsCurrentItem(lastViewCopy);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteEclipseViewsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste (Eclipse Views)");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

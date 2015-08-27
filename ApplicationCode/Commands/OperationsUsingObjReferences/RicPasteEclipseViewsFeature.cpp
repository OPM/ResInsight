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

#include "RicPasteFeatureImpl.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"

#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>

namespace caf
{

CAF_CMD_SOURCE_INIT(RicPasteEclipseViewsFeature, "RicPasteEclipseViewsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteEclipseViewsFeature::isCommandEnabled()
{
    PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimEclipseView> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    if (typedObjects.size() == 0)
    {
        return false;
    }

    PdmObjectHandle* destinationObject = dynamic_cast<PdmObjectHandle*>(SelectionManager::instance()->selectedItem());

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
    PdmObjectHandle* destinationObject = dynamic_cast<PdmObjectHandle*>(SelectionManager::instance()->selectedItem());

    RimEclipseCase* eclipseCase = RicPasteFeatureImpl::findEclipseCase(destinationObject);
    assert(eclipseCase);

    PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    if (objectGroup.objects.size() == 0) return;

    std::vector<caf::PdmPointer<RimEclipseView> > eclipseViews;
    objectGroup.createCopyByType(&eclipseViews, PdmDefaultObjectFactory::instance());

    if (eclipseViews.size() != 0)
    {
        // Add cases to case group
        for (size_t i = 0; i < eclipseViews.size(); i++)
        {
            RimEclipseView* rimReservoirView = eclipseViews[i];
            QString nameOfCopy = QString("Copy of ") + rimReservoirView->name;
            rimReservoirView->name = nameOfCopy;
            eclipseCase->reservoirViews().push_back(rimReservoirView);

            // Delete all wells to be able to copy/paste between cases, as the wells differ between cases
            rimReservoirView->wellCollection()->wells().deleteAllChildObjects();

            rimReservoirView->initAfterReadRecursively();
            rimReservoirView->setEclipseCase(eclipseCase);

            caf::PdmDocument::updateUiIconStateRecursively(rimReservoirView);

            rimReservoirView->loadDataAndUpdate();

            eclipseCase->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteEclipseViewsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste (Eclipse Views)");
}


} // end namespace caf

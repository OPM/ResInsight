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

#include "RicPasteEclipseCasesFeature.h"

#include "RiaApplication.h"

#include "RicPasteFeatureImpl.h"

#include "RigGridManager.h"

#include "RimCaseCollection.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMimeData.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QString>

namespace caf
{

CAF_CMD_SOURCE_INIT(RicPasteEclipseCasesFeature, "RicPasteEclipseCasesFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteEclipseCasesFeature::isCommandEnabled()
{
    PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimEclipseResultCase> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    if (typedObjects.size() == 0)
    {
        return false;
    }

    PdmObjectHandle* destinationObject = dynamic_cast<PdmObjectHandle*>(SelectionManager::instance()->selectedItem());

    RimIdenticalGridCaseGroup* gridCaseGroup = RicPasteFeatureImpl::findGridCaseGroup(destinationObject);
    if (gridCaseGroup) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteEclipseCasesFeature::onActionTriggered(bool isChecked)
{
    PdmObjectHandle* destinationObject = dynamic_cast<PdmObjectHandle*>(SelectionManager::instance()->selectedItem());

    RimIdenticalGridCaseGroup* gridCaseGroup = RicPasteFeatureImpl::findGridCaseGroup(destinationObject);
    if (!gridCaseGroup) return;

    PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    if (objectGroup.objects.size() == 0) return;

    addCasesToGridCaseGroup(objectGroup, gridCaseGroup);
    
    return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteEclipseCasesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste (Eclipse Cases)");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteEclipseCasesFeature::addCasesToGridCaseGroup(PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup)
{
    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    std::vector<RimEclipseResultCase*> resultCases;

    for (size_t i = 0; i < objectGroup.objects.size(); i++)
    {
        RimEclipseResultCase* eclCase = dynamic_cast<RimEclipseResultCase*>(objectGroup.objects[i]);
        if (eclCase)
        {
            RimEclipseResultCase* eclCaseCopy = new RimEclipseResultCase();
            eclCaseCopy->setCaseInfo(eclCase->caseUserDescription(), eclCase->gridFileName());
            resultCases.push_back(eclCaseCopy);
        }
    }

    if (resultCases.size() == 0)
    {
        return;
    }

    RimEclipseResultCase* mainResultCase = NULL;
    std::vector< std::vector<int> > mainCaseGridDimensions;

    // Read out main grid and main grid dimensions if present in case group
    if (gridCaseGroup->mainCase())
    {
        mainResultCase = dynamic_cast<RimEclipseResultCase*>(gridCaseGroup->mainCase());
        CVF_ASSERT(mainResultCase);

        mainResultCase->readGridDimensions(mainCaseGridDimensions);
    }

    std::vector<RimEclipseResultCase*> insertedCases;

    // Add cases to case group
    for (size_t i = 0; i < resultCases.size(); i++)
    {
        RimEclipseResultCase* rimResultReservoir = resultCases[i];

        proj->assignCaseIdToCase(rimResultReservoir);

        if (gridCaseGroup->contains(rimResultReservoir))
        {
            continue;
        }

        insertedCases.push_back(rimResultReservoir);
    }

    // Load stuff 
    for (size_t i = 0; i < insertedCases.size(); i++)
    {
        RimEclipseResultCase* rimResultReservoir = insertedCases[i];

        if (!mainResultCase)
        {
            rimResultReservoir->openEclipseGridFile();
            rimResultReservoir->readGridDimensions(mainCaseGridDimensions);

            mainResultCase = rimResultReservoir;
        }
        else
        {
            std::vector< std::vector<int> > caseGridDimensions;
            rimResultReservoir->readGridDimensions(caseGridDimensions);

            bool identicalGrid = RigGridManager::isGridDimensionsEqual(mainCaseGridDimensions, caseGridDimensions);
            if (!identicalGrid)
            {
                continue;
            }

            if (!rimResultReservoir->openAndReadActiveCellData(mainResultCase->eclipseCaseData()))
            {
                CVF_ASSERT(false);
            }
        }

        RimOilField* activeOilField = proj ? proj->activeOilField() : NULL;
        RimEclipseCaseCollection* analysisModels = (activeOilField) ? activeOilField->analysisModels() : NULL;
        if (analysisModels) analysisModels->insertCaseInCaseGroup(gridCaseGroup, rimResultReservoir);

        caf::PdmDocument::updateUiIconStateRecursively(rimResultReservoir);

        gridCaseGroup->updateConnectedEditors();

        for (size_t rvIdx = 0; rvIdx < rimResultReservoir->reservoirViews.size(); rvIdx++)
        {
            RimEclipseView* riv = rimResultReservoir->reservoirViews()[rvIdx];
            riv->loadDataAndUpdate();
        }
    }
}


} // end namespace caf

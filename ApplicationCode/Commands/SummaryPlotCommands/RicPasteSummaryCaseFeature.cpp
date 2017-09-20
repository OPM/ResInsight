/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicPasteSummaryCaseFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicPasteSummaryCaseFeature, "RicPasteSummaryCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteSummaryCaseFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());
    if (!destinationObject) return false;

    RimSummaryCaseCollection* summaryCaseCollection = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryCaseCollection);

    RimSummaryCaseMainCollection* summaryCaseMainCollection = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryCaseMainCollection);

    if (!(summaryCaseCollection || summaryCaseMainCollection))
    {
        return false;
    }

    std::vector<caf::PdmPointer<RimSummaryCase> > summaryCases = RicPasteSummaryCaseFeature::summaryCases();
    
    if (summaryCases.size() == 0)
    {
        return false;
    }

    for (RimSummaryCase* summaryCase : summaryCases)
    {
        if (summaryCase->isObservedData())
        {
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCaseFeature::onActionTriggered(bool isChecked)
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    std::vector<caf::PdmPointer<RimSummaryCase> > sourceObjects = RicPasteSummaryCaseFeature::summaryCases();

    RimSummaryCaseCollection* summaryCaseCollection = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryCaseCollection);

    if (summaryCaseCollection)
    {
        for (size_t i = 0; i < sourceObjects.size(); i++)
        {
            RicPasteSummaryCaseFeature::removeFromSourceCollection(sourceObjects[i]);
            summaryCaseCollection->addCase(sourceObjects[i]);
        }

        summaryCaseCollection->updateConnectedEditors();
        RicPasteFeatureImpl::clearClipboard();
        return;
    }

    RimSummaryCaseMainCollection* summaryCaseMainCollection = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryCaseMainCollection);

    if (summaryCaseMainCollection)
    {
        for (size_t i = 0; i < sourceObjects.size(); i++)
        {
            RicPasteSummaryCaseFeature::removeFromSourceCollection(sourceObjects[i]);
            summaryCaseMainCollection->addCase(sourceObjects[i]);
        }

        RicPasteFeatureImpl::clearClipboard();
        summaryCaseMainCollection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCaseFeature::setupActionLook(QAction* action)
{
    action->setText("Paste Summary Case");
    action->setIcon(QIcon(":/clipboard.png"));
    action->setShortcut(QKeySequence::Paste);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimSummaryCase> > RicPasteSummaryCaseFeature::summaryCases()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimSummaryCase> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCaseFeature::removeFromSourceCollection(RimSummaryCase* summaryCase)
{
    RimSummaryCaseCollection* sourceSummaryCaseCollection = nullptr;
    summaryCase->firstAncestorOrThisOfType(sourceSummaryCaseCollection);

    if (sourceSummaryCaseCollection)
    {
        sourceSummaryCaseCollection->removeCase(summaryCase);
        sourceSummaryCaseCollection->updateConnectedEditors();
        return;
    }

    RimSummaryCaseMainCollection* sourceSummaryCaseMainCollection = nullptr;
    summaryCase->firstAncestorOrThisOfType(sourceSummaryCaseMainCollection);

    if (sourceSummaryCaseMainCollection)
    {
        sourceSummaryCaseMainCollection->removeCase(summaryCase);
        sourceSummaryCaseMainCollection->updateConnectedEditors();
    }
}
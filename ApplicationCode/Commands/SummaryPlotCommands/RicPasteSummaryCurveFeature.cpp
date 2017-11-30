/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicPasteSummaryCurveFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicPasteSummaryCurveFeature, "RicPasteSummaryCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteSummaryCurveFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);
    if (!summaryPlot)
    {
        return false;
    }

    return RicPasteSummaryCurveFeature::summaryCurves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCurveFeature::onActionTriggered(bool isChecked)
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);
    if (!summaryPlot)
    {
        return;
    }

    std::vector<caf::PdmPointer<RimSummaryCurve> > sourceObjects = RicPasteSummaryCurveFeature::summaryCurves();

    for (size_t i = 0; i < sourceObjects.size(); i++)
    {
        RimSummaryCurve* newObject = dynamic_cast<RimSummaryCurve*>(sourceObjects[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
        CVF_ASSERT(newObject);

        summaryPlot->addCurveAndUpdate(newObject);

        // Resolve references after object has been inserted into the project data model
        newObject->resolveReferencesRecursively();

        // If source curve is part of a curve filter, resolve of references to the summary case does not
        // work when pasting the new curve into a plot. Must set summary case manually.
        newObject->setSummaryCaseY(sourceObjects[i]->summaryCaseY());

        newObject->initAfterReadRecursively();

        newObject->loadDataAndUpdate(true);
        newObject->updateConnectedEditors();

        summaryPlot->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Summary Curve");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimSummaryCurve> > RicPasteSummaryCurveFeature::summaryCurves()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimSummaryCurve> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}

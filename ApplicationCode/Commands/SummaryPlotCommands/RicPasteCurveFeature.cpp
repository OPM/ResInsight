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

#include "RicPasteCurveFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimSummaryPlot.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include "RimSummaryCurve.h"


CAF_CMD_SOURCE_INIT(RicPasteCurveFeature, "RicPasteCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteCurveFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);
    if (!summaryPlot)
    {
        return false;
    }

    return RicPasteCurveFeature::summaryCurves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteCurveFeature::onActionTriggered(bool isChecked)
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);
    if (!summaryPlot)
    {
        return;
    }

    std::vector<caf::PdmPointer<RimSummaryCurve> > sourceObjects = RicPasteCurveFeature::summaryCurves();

    for (size_t i = 0; i < sourceObjects.size(); i++)
    {
        RimSummaryCurve* newObject = dynamic_cast<RimSummaryCurve*>(sourceObjects[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
        CVF_ASSERT(newObject);

        summaryPlot->addCurve(newObject);

        // Resolve references after object has been inserted into the project data model
        newObject->resolveReferencesRecursively();
        newObject->initAfterReadRecursively();

        newObject->loadDataAndUpdate();
        newObject->updateConnectedEditors();

        summaryPlot->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Summary Curve");
    actionToSetup->setIcon(QIcon(":/SummaryCurve16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimSummaryCurve> > RicPasteCurveFeature::summaryCurves()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimSummaryCurve> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}


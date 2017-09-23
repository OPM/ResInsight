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

#include "RicPasteAsciiDataCurveFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

#include "RimSummaryPlot.h"
#include "RimAsciiDataCurve.h"


CAF_CMD_SOURCE_INIT(RicPasteAsciiDataCurveFeature, "RicPasteAsciiDataCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteAsciiDataCurveFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    if (!destinationObject)
    {
        return false;
    }

    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);
    if (!summaryPlot)
    {
        return false;
    }

    return RicPasteAsciiDataCurveFeature::asciiDataCurves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataCurveFeature::onActionTriggered(bool isChecked)
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());
    CVF_ASSERT(destinationObject);

    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);
    if (!summaryPlot)
    {
        return;
    }

    std::vector<caf::PdmPointer<RimAsciiDataCurve> > sourceObjects = RicPasteAsciiDataCurveFeature::asciiDataCurves();

    for (size_t i = 0; i < sourceObjects.size(); i++)
    {
        RimAsciiDataCurve* newObject = dynamic_cast<RimAsciiDataCurve*>(sourceObjects[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
        CVF_ASSERT(newObject);

        summaryPlot->addAsciiDataCruve(newObject);

        // Resolve references after object has been inserted into the project data model
        newObject->resolveReferencesRecursively();

        // If source curve is part of a curve filter, resolve of references to the summary case does not
        // work when pasting the new curve into a plot. Must set summary case manually.
        //newObject->setSummaryCase(sourceObjects[i]->summaryCase());

        newObject->initAfterReadRecursively();

        newObject->loadDataAndUpdate(true);
        newObject->updateConnectedEditors();

        summaryPlot->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste ASCII Data Curve");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimAsciiDataCurve> > RicPasteAsciiDataCurveFeature::asciiDataCurves()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimAsciiDataCurve> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}


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

#include "RiaSummaryTools.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryCrossPlot.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicPasteSummaryCurveFeature, "RicPasteSummaryCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicPasteSummaryCurveFeature::copyCurveAndAddToPlot(RimSummaryCurve *sourceCurve)
{
    RimSummaryPlot* summaryPlot = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryPlot*>();

    RimSummaryCurve* newCurve = dynamic_cast<RimSummaryCurve*>(sourceCurve->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    CVF_ASSERT(newCurve);

    summaryPlot->addCurveAndUpdate(newCurve);

    // Resolve references after object has been inserted into the project data model
    newCurve->resolveReferencesRecursively();

    // If source curve is part of a curve filter, resolve of references to the summary case does not
    // work when pasting the new curve into a plot. Must set summary case manually.
    newCurve->setSummaryCaseY(sourceCurve->summaryCaseY());

    newCurve->initAfterReadRecursively();

    newCurve->loadDataAndUpdate(true);
    newCurve->updateConnectedEditors();

    summaryPlot->updateConnectedEditors();

    return newCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteSummaryCurveFeature::isCommandEnabled()
{
    caf::PdmObject* destinationObject = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);
    if(!RiaSummaryTools::parentSummaryPlot(destinationObject))
    {
        return false;
    }

    if (summaryCurvesOnClipboard().size() == 0)
    {
        return false;
    }

    for (caf::PdmPointer<RimSummaryCurve> curve : summaryCurvesOnClipboard())
    {
        // Check that owner plot is correct type
        RimSummaryPlot* ownerPlot = RiaSummaryTools::parentSummaryPlot(curve);
        
        if (!ownerPlot) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCurveFeature::onActionTriggered(bool isChecked)
{
    std::vector<caf::PdmPointer<RimSummaryCurve> > sourceObjects = RicPasteSummaryCurveFeature::summaryCurvesOnClipboard();

    for (size_t i = 0; i < sourceObjects.size(); i++)
    {
        copyCurveAndAddToPlot(sourceObjects[i]);
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
std::vector<caf::PdmPointer<RimSummaryCurve> > RicPasteSummaryCurveFeature::summaryCurvesOnClipboard()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimSummaryCurve> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}

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

#include "RicPasteSummaryCrossPlotCurveFeature.h"

#include "RiaSummaryTools.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryCrossPlot.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicPasteSummaryCrossPlotCurveFeature, "RicPasteSummaryCrossPlotCurveFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteSummaryCrossPlotCurveFeature::isCommandEnabled()
{
    caf::PdmObject* destinationObject = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());

    if(!RiaSummaryTools::parentCrossPlot(destinationObject))
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
        RimSummaryCrossPlot* ownerPlot = nullptr;
        curve->firstAncestorOrThisOfType(ownerPlot);

        if (!ownerPlot) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//void RicPasteSummaryCrossPlotCurveFeature::onActionTriggered(bool isChecked)
//{
//    std::vector<caf::PdmPointer<RimSummaryCurve> > sourceObjects = RicPasteSummaryCurveFeature::summaryCurves();
//
//    for (size_t i = 0; i < sourceObjects.size(); i++)
//    {
//        copyCurveAndAddToPlot(sourceObjects[i]);
//    }
//}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCrossPlotCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Summary Cross Plot Curve");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

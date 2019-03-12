/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicPasteGridCrossPlotCurveSetFeature.h"

#include "RiaApplication.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurveSet.h"
#include "RiuPlotMainWindowTools.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicPasteGridCrossPlotCurveSetFeature, "RicPasteGridCrossPlotCurveSetFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPasteGridCrossPlotCurveSetFeature::isCommandEnabled()
{
    auto curvesOnClipboard = gridCrossPlotCurveSetsOnClipboard();
    if (curvesOnClipboard.empty()) return false;

    return caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGridCrossPlot>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteGridCrossPlotCurveSetFeature::onActionTriggered(bool isChecked)
{
    RimGridCrossPlot* crossPlot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimGridCrossPlot>();

    if (crossPlot)
    {
        auto curvesOnClipboard = gridCrossPlotCurveSetsOnClipboard();
        if (!curvesOnClipboard.empty())
        {
            RimGridCrossPlotCurveSet* objectToSelect = nullptr;

            for (RimGridCrossPlotCurveSet* crossPlotCurveSet : gridCrossPlotCurveSetsOnClipboard())
            {
                RimGridCrossPlotCurveSet* newCurveSet = dynamic_cast<RimGridCrossPlotCurveSet*>(
                    crossPlotCurveSet->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));

                crossPlot->addCurveSet(newCurveSet);
                newCurveSet->resolveReferencesRecursively();
                newCurveSet->initAfterReadRecursively();

                objectToSelect = newCurveSet;
            }


            RiaApplication::instance()->getOrCreateMainPlotWindow();
            crossPlot->updateAllRequiredEditors();
            crossPlot->loadDataAndUpdate();

            RiuPlotMainWindowTools::selectAsCurrentItem(objectToSelect);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPasteGridCrossPlotCurveSetFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Grid Cross Plot Curve Set");
    // actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimGridCrossPlotCurveSet>> RicPasteGridCrossPlotCurveSetFeature::gridCrossPlotCurveSetsOnClipboard()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimGridCrossPlotCurveSet>> typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}

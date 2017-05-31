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

#include "RicPasteSummaryPlotFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicPasteSummaryPlotFeature, "RicPasteSummaryPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteSummaryPlotFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryPlotCollection* plotColl = nullptr;
    destinationObject->firstAncestorOrThisOfType(plotColl);
    if (!plotColl)
    {
        return false;
    }

    return RicPasteSummaryPlotFeature::summaryPlots().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryPlotFeature::onActionTriggered(bool isChecked)
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryPlotCollection* plotColl = nullptr;
    destinationObject->firstAncestorOrThisOfType(plotColl);
    if (!plotColl)
    {
        return;
    }

    std::vector<caf::PdmPointer<RimSummaryPlot> > sourceObjects = RicPasteSummaryPlotFeature::summaryPlots();

    for (size_t i = 0; i < sourceObjects.size(); i++)
    {
        RimSummaryPlot* newSummaryPlot = dynamic_cast<RimSummaryPlot*>(sourceObjects[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
        CVF_ASSERT(newSummaryPlot);

        plotColl->summaryPlots.push_back(newSummaryPlot);

        // Resolve references after object has been inserted into the data model
        newSummaryPlot->resolveReferencesRecursively();
        newSummaryPlot->initAfterReadRecursively();

        QString nameOfCopy = QString("Copy of ") + newSummaryPlot->description();
        newSummaryPlot->setDescription(nameOfCopy);

        plotColl->updateConnectedEditors();

        newSummaryPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Summary Plot");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimSummaryPlot> > RicPasteSummaryPlotFeature::summaryPlots()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimSummaryPlot> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}


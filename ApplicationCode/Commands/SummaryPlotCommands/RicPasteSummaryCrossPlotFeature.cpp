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

#include "RicPasteSummaryCrossPlotFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManagerTools.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicPasteSummaryCrossPlotFeature, "RicPasteSummaryCrossPlotFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCrossPlotFeature::copyPlotAndAddToCollection(RimSummaryCrossPlot *sourcePlot)
{
    RimSummaryCrossPlotCollection* plotColl = caf::firstAncestorOfTypeFromSelectedObject<RimSummaryCrossPlotCollection*>();

    if (plotColl)
    {
        RimSummaryCrossPlot* newSummaryPlot = dynamic_cast<RimSummaryCrossPlot*>(sourcePlot->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
        CVF_ASSERT(newSummaryPlot);

        plotColl->addSummaryPlot(newSummaryPlot);

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
bool RicPasteSummaryCrossPlotFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryCrossPlotCollection* plotColl = nullptr;
    destinationObject->firstAncestorOrThisOfType(plotColl);
    if (!plotColl)
    {
        return false;
    }

    return RicPasteSummaryCrossPlotFeature::summaryPlots().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCrossPlotFeature::onActionTriggered(bool isChecked)
{
    std::vector<caf::PdmPointer<RimSummaryCrossPlot> > sourceObjects = RicPasteSummaryCrossPlotFeature::summaryPlots();

    for (size_t i = 0; i < sourceObjects.size(); i++)
    {
        copyPlotAndAddToCollection(sourceObjects[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteSummaryCrossPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Summary Cross Plot");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimSummaryCrossPlot> > RicPasteSummaryCrossPlotFeature::summaryPlots()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimSummaryCrossPlot> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}


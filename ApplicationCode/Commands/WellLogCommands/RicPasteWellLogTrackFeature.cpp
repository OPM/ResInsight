/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RicPasteWellLogTrackFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellRftPlot.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmObjectHandle.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicPasteWellLogTrackFeature, "RicPasteWellLogTrackFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteWellLogTrackFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;

    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimWellLogPlot* wellLogPlot = nullptr;
    RimWellRftPlot* rftPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(wellLogPlot);
    destinationObject->firstAncestorOrThisOfType(rftPlot);
    if (!wellLogPlot || rftPlot)
    {
        return false;
    }

    return RicPasteWellLogTrackFeature::tracks().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteWellLogTrackFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    RimWellLogPlot* wellLogPlot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellLogPlot>();
    if (!wellLogPlot)
    {
        return;
    }

    std::vector<caf::PdmPointer<RimWellLogTrack> > sourceObjects = RicPasteWellLogTrackFeature::tracks();

    for (size_t i = 0; i < sourceObjects.size(); i++)
    {
        RimWellLogTrack* fileCurve = sourceObjects[i];
        if (fileCurve)
        {
            RimWellLogTrack* newObject = dynamic_cast<RimWellLogTrack*>(fileCurve->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
            CVF_ASSERT(newObject);

            wellLogPlot->addTrack(newObject);

            newObject->setDescription(QString("Track %1").arg(wellLogPlot->trackCount()));

            // Resolve references after object has been inserted into the project data model
            newObject->resolveReferencesRecursively();
            newObject->initAfterReadRecursively();

            newObject->loadDataAndUpdate();

            wellLogPlot->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteWellLogTrackFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Well Log Track");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimWellLogTrack> > RicPasteWellLogTrackFeature::tracks()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimWellLogTrack> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}

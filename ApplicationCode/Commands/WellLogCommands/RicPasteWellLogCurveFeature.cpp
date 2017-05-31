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

#include "RicPasteWellLogCurveFeature.h"
#include "RicWellLogPlotCurveFeatureImpl.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "RimWellLogCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogTrack.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmObjectHandle.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicPasteWellLogCurveFeature, "RicPasteWellLogCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteWellLogCurveFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;

    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimWellLogTrack* wellLogTrack = nullptr;
    destinationObject->firstAncestorOrThisOfType(wellLogTrack);
    if (!wellLogTrack)
    {
        return false;
    }

    return RicPasteWellLogCurveFeature::curves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteWellLogCurveFeature::onActionTriggered(bool isChecked)
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimWellLogTrack* wellLogTrack = nullptr;
    destinationObject->firstAncestorOrThisOfType(wellLogTrack);
    if (!wellLogTrack)
    {
        return;
    }

    std::vector<caf::PdmPointer<RimWellLogCurve> > sourceObjects = RicPasteWellLogCurveFeature::curves();

    for (size_t i = 0; i < sourceObjects.size(); i++)
    {
        RimWellLogFileCurve* fileCurve = dynamic_cast<RimWellLogFileCurve*>(sourceObjects[i].p());
        if (fileCurve)
        {
            RimWellLogFileCurve* newObject = dynamic_cast<RimWellLogFileCurve*>(sourceObjects[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
            CVF_ASSERT(newObject);

            wellLogTrack->addCurve(newObject);

            // Resolve references after object has been inserted into the project data model
            newObject->resolveReferencesRecursively();
            newObject->initAfterReadRecursively();

            newObject->loadDataAndUpdate();

            wellLogTrack->updateConnectedEditors();
        }

        RimWellLogExtractionCurve* extractionCurve = dynamic_cast<RimWellLogExtractionCurve*>(sourceObjects[i].p());
        if (extractionCurve)
        {
            RimWellLogExtractionCurve* newObject = dynamic_cast<RimWellLogExtractionCurve*>(sourceObjects[i]->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
            CVF_ASSERT(newObject);

            wellLogTrack->addCurve(newObject);

            // Resolve references after object has been inserted into the project data model
            newObject->resolveReferencesRecursively();
            newObject->initAfterReadRecursively();

            newObject->loadDataAndUpdate();

            wellLogTrack->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteWellLogCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Well Log Curve");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<RimWellLogCurve> > RicPasteWellLogCurveFeature::curves()
{
    caf::PdmObjectGroup objectGroup;
    RicPasteFeatureImpl::findObjectsFromClipboardRefs(&objectGroup);

    std::vector<caf::PdmPointer<RimWellLogCurve> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    return typedObjects;
}

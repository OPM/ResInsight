/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicDeleteItemFeature.h"
#include "RicDeleteItemExec.h"
#include "RicDeleteItemExecData.h"

#include "RimCellRangeFilter.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseView.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechView.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimIntersection.h"
#include "RimIntersectionBox.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimViewController.h"
#include "RimWellAllocationPlot.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimFishboneWellPath.h"
#include "RimPerforationInterval.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdSelectionHelper.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicDeleteItemFeature, "RicDeleteItemFeature");

bool isDeletable(caf::PdmUiItem* uiItem)
{
    // Enable delete of well allocation plots
    if (dynamic_cast<RimWellAllocationPlot*>(uiItem)) return true;

    // Disable delete of all sub objects of a well allocation plot
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(uiItem);
    if (destinationObject)
    {
        RimWellAllocationPlot* wellAllocationPlot = nullptr;
        destinationObject->firstAncestorOrThisOfType(wellAllocationPlot);

        if (wellAllocationPlot)
        {
            return false;
        }
    }

    if (dynamic_cast<RimGeoMechView*>(uiItem))               return true;
    if (dynamic_cast<RimEclipseView*>(uiItem))               return true;
    if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem))    return true;
    if (dynamic_cast<RimEclipseInputProperty*>(uiItem))      return true;
    if (dynamic_cast<RimCellRangeFilter*>(uiItem))           return true;
    if (dynamic_cast<RimEclipsePropertyFilter*>(uiItem))     return true;
    if (dynamic_cast<RimGeoMechPropertyFilter*>(uiItem))     return true;
    if (dynamic_cast<RimViewController*>(uiItem))            return true;
    if (dynamic_cast<RimWellLogPlot*>(uiItem))               return true;
    if (dynamic_cast<RimWellLogCurve*>(uiItem))              return true;
    if (dynamic_cast<RimSummaryPlot*>(uiItem))               return true;
    if (dynamic_cast<RimSummaryCurve*>(uiItem))              return true;
    if (dynamic_cast<RimGridTimeHistoryCurve*>(uiItem))      return true;
    if (dynamic_cast<RimSummaryCurveFilter*>(uiItem))        return true;
    if (dynamic_cast<RimIntersection*>(uiItem))              return true;
    if (dynamic_cast<RimIntersectionBox*>(uiItem))           return true;
    if (dynamic_cast<RimFormationNames*>(uiItem))            return true;
    if (dynamic_cast<RimFormationNamesCollection*>(uiItem))  return true;
    if (dynamic_cast<RimFishboneWellPath*>(uiItem))        return true;
    if (dynamic_cast<RimFishbonesMultipleSubs*>(uiItem))     return true;
    if (dynamic_cast<RimPerforationInterval*>(uiItem))       return true;
    if (dynamic_cast<RimWellPathFractureCollection*>(uiItem))        return true;
    if (dynamic_cast<RimWellPathFracture*>(uiItem))                  return true;
    if (dynamic_cast<RimEllipseFractureTemplate*>(uiItem))        return true;
    if (dynamic_cast<RimStimPlanFractureTemplate*>(uiItem))        return true;
    if (dynamic_cast<RimSimWellFractureCollection*>(uiItem))        return true;
    if (dynamic_cast<RimSimWellFracture*>(uiItem))                  return true;


    return false;    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeleteItemFeature::isCommandEnabled() 
{
    std::vector<caf::PdmUiItem*> items;
    caf::SelectionManager::instance()->selectedItems(items);
    
    if (items.empty() ) return false;

    for (caf::PdmUiItem* item : items)
    {
        if (!isDeletable(item)) return false;

        caf::PdmObject* currentPdmObject = dynamic_cast<caf::PdmObject*>(item);
        if ( !currentPdmObject ) return false;

        caf::PdmChildArrayFieldHandle* childArrayFieldHandle = dynamic_cast<caf::PdmChildArrayFieldHandle*>(currentPdmObject->parentField());
        if ( !childArrayFieldHandle ) return false;
    }
    
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteItemFeature::onActionTriggered(bool isChecked)
{
    std::vector<caf::PdmUiItem*> items;
    caf::SelectionManager::instance()->selectedItems(items);
    assert(items.size() > 0);

    for (caf::PdmUiItem* item: items)
    {
        if (!isDeletable(item)) continue;

        caf::PdmObject* currentPdmObject = dynamic_cast<caf::PdmObject*>(item);
        if ( !currentPdmObject) continue;

        caf::PdmChildArrayFieldHandle* childArrayFieldHandle = dynamic_cast<caf::PdmChildArrayFieldHandle*>(currentPdmObject->parentField());
        if ( !childArrayFieldHandle) continue;

        int indexAfter = -1;

        std::vector<caf::PdmObjectHandle*> childObjects;
        childArrayFieldHandle->childObjects(&childObjects);

        for ( size_t i = 0; i < childObjects.size(); i++ )
        {
            if ( childObjects[i] == currentPdmObject )
            {
                indexAfter = static_cast<int>(i);
            }
        }

        // Did not find currently selected pdm object in the current list field
        assert(indexAfter != -1);

        RicDeleteItemExec* executeCmd = new RicDeleteItemExec(caf::SelectionManager::instance()->notificationCenter());

        RicDeleteItemExecData* data = executeCmd->commandData();
        data->m_rootObject = caf::PdmReferenceHelper::findRoot(childArrayFieldHandle);
        data->m_pathToField = caf::PdmReferenceHelper::referenceFromRootToField(data->m_rootObject, childArrayFieldHandle);
        data->m_indexToObject = indexAfter;


        caf::CmdExecCommandManager::instance()->processExecuteCommand(executeCmd);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteItemFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

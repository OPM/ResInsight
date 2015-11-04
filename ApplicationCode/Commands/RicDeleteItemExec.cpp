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


#include "RicDeleteItemExec.h"
#include "RicDeleteItemExecData.h"

#include "RimCellRangeFilterCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimProject.h"
#include "RimView.h"
#include "RimViewLinkerCollection.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogPlotTrack.h"
#include "RimWellPathCollection.h"

#include "cafNotificationCenter.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmDocument.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiFieldHandle.h"
#include "cafSelectionManager.h"


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicDeleteItemExec::name()
{
    return m_commandData->classKeyword();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteItemExec::redo()
{
    PdmFieldHandle* field = PdmReferenceHelper::fieldFromReference(m_commandData->m_rootObject, m_commandData->m_pathToField);

    PdmChildArrayFieldHandle* listField = dynamic_cast<PdmChildArrayFieldHandle*>(field);
    if (listField)
    {
        std::vector<PdmObjectHandle*> children;
        listField->childObjects(&children);

        PdmObjectHandle* obj = children[m_commandData->m_indexToObject];
        caf::SelectionManager::instance()->removeObjectFromAllSelections(obj);

        if (m_commandData->m_deletedObjectAsXml().isEmpty())
        {
            m_commandData->m_deletedObjectAsXml = xmlObj(obj)->writeObjectToXmlString();
        }

        delete obj;

        listField->erase(m_commandData->m_indexToObject);

        caf::PdmObjectHandle* parentObj = listField->ownerObject();
        parentObj->uiCapability()->updateConnectedEditors();
        
        RimView* view = NULL;
        parentObj->firstAnchestorOrThisOfType(view);

        RimCellRangeFilterCollection* rangeFilterColl;
        parentObj->firstAnchestorOrThisOfType(rangeFilterColl);

        if (rangeFilterColl)
        {
            rangeFilterColl->updateDisplayModeNotifyManagedViews(NULL);
        }

        RimEclipsePropertyFilterCollection* eclipsePropColl;
        parentObj->firstAnchestorOrThisOfType(eclipsePropColl);
        
        RimGeoMechPropertyFilterCollection* geoMechPropColl;
        parentObj->firstAnchestorOrThisOfType(geoMechPropColl);

        if (view && (eclipsePropColl || geoMechPropColl))
        {
            view->scheduleGeometryRegen(PROPERTY_FILTERED);
            view->scheduleCreateDisplayModelAndRedraw();
        }

        RimWellPathCollection* wellPathColl;
        parentObj->firstAnchestorOrThisOfType(wellPathColl);

        if (wellPathColl)
        {
            wellPathColl->scheduleGeometryRegenAndRedrawViews();
        }

        // Update due to deletion of curves (not tracks, handled separatly)

        RimWellLogPlot* wellLogPlot;
        parentObj->firstAnchestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            wellLogPlot->calculateAvailableDepthRange();
            wellLogPlot->zoomAllDepth();
        }

        RimWellLogTrack* wellLogPlotTrack;
        parentObj->firstAnchestorOrThisOfType(wellLogPlotTrack);
        if (wellLogPlotTrack)
        {
            wellLogPlotTrack->zoomAllXAxis();
        }
        
        // Update due to delete plots
        // Make sure the plot collection disappears with the last plot

        RimWellLogPlotCollection* wellLogPlotCollection = dynamic_cast<RimWellLogPlotCollection*>(parentObj);
        if (wellLogPlotCollection)
        {
            if (wellLogPlotCollection->wellLogPlots.empty())
            {
                RimProject* project = NULL;
                parentObj->firstAnchestorOrThisOfType(project);
                if (project)
                {
                    project->updateConnectedEditors();
                }
            }
        }
        
        RimViewLinkerCollection* viewLinkerCollection = NULL;
        parentObj->firstAnchestorOrThisOfType(viewLinkerCollection);
        if (viewLinkerCollection)
        {
            viewLinkerCollection->uiCapability()->updateConnectedEditors();

            RimProject* project = NULL;
            parentObj->firstAnchestorOrThisOfType(project);
            if (project)
            {
                // Update visibility of top level Linked Views item in the project tree
                // Not visible if no views are linked
                project->uiCapability()->updateConnectedEditors();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteItemExec::undo()
{
    PdmFieldHandle* field = PdmReferenceHelper::fieldFromReference(m_commandData->m_rootObject, m_commandData->m_pathToField);

    PdmChildArrayFieldHandle* listField = dynamic_cast<PdmChildArrayFieldHandle*>(field);
    if (listField)
    {
        PdmObjectHandle* obj = PdmXmlObjectHandle::readUnknownObjectFromXmlString(m_commandData->m_deletedObjectAsXml(), PdmDefaultObjectFactory::instance());

        listField->insertAt(m_commandData->m_indexToObject, obj);

        obj->xmlCapability()->initAfterReadRecursively();

        listField->uiCapability()->updateConnectedEditors();
        listField->ownerObject()->uiCapability()->updateConnectedEditors();

        if (m_notificationCenter) m_notificationCenter->notifyObservers();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicDeleteItemExec::RicDeleteItemExec(NotificationCenter* notificationCenter)
    : CmdExecuteCommand(notificationCenter)
{
    m_commandData = new RicDeleteItemExecData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicDeleteItemExecData* RicDeleteItemExec::commandData()
{
    return m_commandData;
}

} // end namespace caf

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

#include "cafPdmChildArrayField.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiFieldHandle.h"

#include "cafNotificationCenter.h"
#include "cafSelectionManager.h"
#include "cafPdmDocument.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimWellPathCollection.h"
#include "RimView.h"


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

        listField->erase(m_commandData->m_indexToObject);

        delete obj;

        caf::PdmObjectHandle* parentObj = listField->ownerObject();
        parentObj->uiCapability()->updateConnectedEditors();
        
        RimView* view = NULL;
        parentObj->firstAnchestorOrThisOfType(view);

        RimCellRangeFilterCollection* rangeFilterColl;
        parentObj->firstAnchestorOrThisOfType(rangeFilterColl);

        if (rangeFilterColl)
        {
            rangeFilterColl->updateUiUpdateDisplayModel();
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

        PdmDocument::initAfterReadTraversal(obj);

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

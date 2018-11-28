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

#include "RimPropertyFilterCollection.h"
#include "Rim3dView.h"
#include "RimPropertyFilter.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimPropertyFilterCollection, "RimPropertyFilterCollection"); // Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPropertyFilterCollection::RimPropertyFilterCollection()
{
    CAF_PDM_InitField(&isActive,                   "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPropertyFilterCollection::~RimPropertyFilterCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPropertyFilterCollection::updateDisplayModelNotifyManagedViews(RimPropertyFilter* changedFilter) const
{
    Rim3dView* view = nullptr;
    this->firstAncestorOrThisOfType(view);
    CVF_ASSERT(view);
    if (!view) return;

    if (view->isMasterView())
    {
        RimViewLinker* viewLinker = view->assosiatedViewLinker();
        if (viewLinker)
        {
            // Update data for property filter
            // Update of display model is handled by view->scheduleGeometryRegen, also for managed views
            viewLinker->updatePropertyFilters(changedFilter);
        }
    }

    view->scheduleGeometryRegen(PROPERTY_FILTERED);
    view->scheduleCreateDisplayModelAndRedraw();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPropertyFilterCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    updateIconState();
    uiCapability()->updateConnectedEditors();

    updateDisplayModelNotifyManagedViews(nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPropertyFilterCollection::objectToggleField()
{
    return &isActive;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPropertyFilterCollection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    PdmObject::defineUiTreeOrdering(uiTreeOrdering, uiConfigName);

    Rim3dView* rimView = nullptr;
    this->firstAncestorOrThisOfType(rimView);
    RimViewController* viewController = rimView->viewController();
    if (viewController && (viewController->isPropertyFilterOveridden() 
                           || viewController->isVisibleCellsOveridden()))
    {
        isActive.uiCapability()->setUiReadOnly(true, uiConfigName);
    }
    else
    {
        isActive.uiCapability()->setUiReadOnly(false, uiConfigName);
    }

    updateIconState();
}

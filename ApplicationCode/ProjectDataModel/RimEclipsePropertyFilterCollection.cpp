/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipsePropertyFilterCollection.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cafPdmUiEditorHandle.h"


CAF_PDM_SOURCE_INIT(RimEclipsePropertyFilterCollection, "CellPropertyFilters");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection::RimEclipsePropertyFilterCollection()
{
    CAF_PDM_InitObject("Property Filters", ":/CellFilter_Values.png", "", "");

    CAF_PDM_InitFieldNoDefault(&propertyFilters, "PropertyFilters", "Property Filters",         "", "", "");
    propertyFilters.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isActive,                  "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection::~RimEclipsePropertyFilterCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipsePropertyFilterCollection::reservoirView()
{
    RimEclipseView* eclipseView = NULL;
    firstAnchestorOrThisOfType(eclipseView);

    return eclipseView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    updateIconState();

    updateDisplayModelNotifyManagedViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::loadAndInitializePropertyFilters()
{
    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimEclipsePropertyFilter* propertyFilter = propertyFilters[i];
        propertyFilter->initAfterRead();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::initAfterRead()
{
    updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilterCollection::hasActiveFilters() const
{
    if (!isActive) return false;

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimEclipsePropertyFilter* propertyFilter = propertyFilters[i];
        if (propertyFilter->isActive() && propertyFilter->resultDefinition->hasResult()) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether any of the active property filters are based on a dynamic result
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilterCollection::hasActiveDynamicFilters() const
{
    if (!isActive) return false;

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimEclipsePropertyFilter* propertyFilter = propertyFilters[i];
        if (propertyFilter->isActive() && propertyFilter->resultDefinition->hasDynamicResult()) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEclipsePropertyFilterCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::updateDisplayModelNotifyManagedViews()
{
    RimEclipseView* view = NULL;
    this->firstAnchestorOrThisOfType(view);
    CVF_ASSERT(view);

    view->scheduleGeometryRegen(PROPERTY_FILTERED);
    view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::updateIconState()
{
    bool activeIcon = true;

    RimEclipseView* view = NULL;
    this->firstAnchestorOrThisOfType(view);
    RimViewController* viewController = view->viewController();
    if (viewController && (viewController->isPropertyFilterOveridden() 
                           || viewController->isVisibleCellsOveridden()))
    {
        activeIcon = false;
    }

    if (!isActive)
    {
        activeIcon = false;
    }

    updateUiIconFromState(activeIcon);

    uiCapability()->updateConnectedEditors();
}

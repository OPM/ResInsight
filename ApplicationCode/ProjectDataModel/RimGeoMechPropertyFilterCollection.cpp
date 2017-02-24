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

#include "RimGeoMechPropertyFilterCollection.h"

#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "cvfAssert.h"


CAF_PDM_SOURCE_INIT(RimGeoMechPropertyFilterCollection, "GeoMechPropertyFilters");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection::RimGeoMechPropertyFilterCollection()
{
    CAF_PDM_InitObject("Property Filters", ":/CellFilter_Values.png", "", "");
    
    CAF_PDM_InitFieldNoDefault(&propertyFilters, "PropertyFilters", "Property Filters",         "", "", "");
    propertyFilters.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection::~RimGeoMechPropertyFilterCollection()
{
    propertyFilters.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechPropertyFilterCollection::reservoirView()
{
    RimGeoMechView* geoMechView = NULL;
    firstAncestorOrThisOfType(geoMechView);
    
    return geoMechView;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilterCollection::loadAndInitializePropertyFilters()
{
    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimGeoMechPropertyFilter* propertyFilter = propertyFilters[i];
        propertyFilter->resultDefinition->setGeoMechCase(reservoirView()->geoMechCase());
        propertyFilter->resultDefinition->loadResult();
        propertyFilter->computeResultValueRange();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilterCollection::initAfterRead()
{
    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimGeoMechPropertyFilter* propertyFilter = propertyFilters[i];

        propertyFilter->setParentContainer(this);
        propertyFilter->resultDefinition->setGeoMechCase(reservoirView()->geoMechCase());
        propertyFilter->updateIconState();
    }

    updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilterCollection::hasActiveFilters() const
{
    if (!isActive) return false;

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimGeoMechPropertyFilter* propertyFilter = propertyFilters[i];
        if (propertyFilter->isActive() && propertyFilter->resultDefinition->hasResult()) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether any of the active property filters are based on a dynamic result
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilterCollection::hasActiveDynamicFilters() const
{
    return hasActiveFilters();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilterCollection::isUsingFormationNames() const
{
    if ( !isActive ) return false;

    for ( size_t i = 0; i < propertyFilters.size(); i++ )
    {
        RimGeoMechPropertyFilter* propertyFilter = propertyFilters[i];
        if (   propertyFilter->isActive() 
            && propertyFilter->resultDefinition->resultPositionType() == RIG_FORMATION_NAMES 
            && propertyFilter->resultDefinition->resultFieldName() != "") return true;
    }

    return false;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilterCollection::updateIconState()
{
    bool activeIcon = true;

    RimGeoMechView* view = NULL;
    this->firstAncestorOrThisOfType(view);
    if (view)
    {
        RimViewController* viewController = view->viewController();
        if (viewController && ( viewController->isPropertyFilterOveridden() 
                                || viewController->isVisibleCellsOveridden()))
        {
            activeIcon = false;
        }
    }

    if (!isActive)
    {
        activeIcon = false;
    }

    updateUiIconFromState(activeIcon);

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimGeoMechPropertyFilter* propFilter = propertyFilters[i];
        propFilter->updateActiveState();
        propFilter->updateIconState();
    }
}


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
#include "RimGeoMechView.h"
#include "RimGeoMechCellColors.h"

#include "cvfAssert.h"


CAF_PDM_SOURCE_INIT(RimGeoMechPropertyFilterCollection, "GeoMechPropertyFilters");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection::RimGeoMechPropertyFilterCollection()
{
    CAF_PDM_InitObject("GeoMech Property Filters", ":/CellFilter_Values.png", "", "");
    
    CAF_PDM_InitFieldNoDefault(&propertyFilters, "PropertyFilters", "Property Filters",         "", "", "");
    CAF_PDM_InitField(&active,                   "Active", true, "Active", "", "", "");
    active.capability<caf::PdmUiFieldHandle>()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection::~RimGeoMechPropertyFilterCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilterCollection::setReservoirView(RimGeoMechView* reservoirView)
{
    m_reservoirView = reservoirView;

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimGeoMechPropertyFilter* propertyFilter = propertyFilters[i];
        propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechPropertyFilterCollection::reservoirView()
{
    CVF_ASSERT(!m_reservoirView.isNull());
    return m_reservoirView;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilterCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->updateUiIconFromToggleField();

    ((RimView*)m_reservoirView)->scheduleGeometryRegen(PROPERTY_FILTERED);
    m_reservoirView->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilter* RimGeoMechPropertyFilterCollection::createAndAppendPropertyFilter()
{
    RimGeoMechPropertyFilter* propertyFilter = new RimGeoMechPropertyFilter();
    
    propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());

    propertyFilter->setParentContainer(this);
    propertyFilters.push_back(propertyFilter);

    propertyFilter->resultDefinition->setResultAddress(m_reservoirView->cellResult()->resultAddress());
    propertyFilter->resultDefinition->loadResult();
    propertyFilter->setToDefaultValues();
    propertyFilter->updateFilterName();
 
    return propertyFilter;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilterCollection::loadAndInitializePropertyFilters()
{
    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimGeoMechPropertyFilter* propertyFilter = propertyFilters[i];

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
        propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());
        propertyFilter->updateIconState();
    }

    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilterCollection::remove(RimGeoMechPropertyFilter* propertyFilter)
{
    propertyFilters.removeChildObject(propertyFilter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechPropertyFilterCollection::hasActiveFilters() const
{
    if (!active) return false;

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
caf::PdmFieldHandle* RimGeoMechPropertyFilterCollection::objectToggleField()
{
    return &active;
}

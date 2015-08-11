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

#include "RimEclipseView.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseCellColors.h"


CAF_PDM_SOURCE_INIT(RimEclipsePropertyFilterCollection, "CellPropertyFilters");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilterCollection::RimEclipsePropertyFilterCollection()
{
    CAF_PDM_InitObject("Property Filters", ":/CellFilter_Values.png", "", "");

    CAF_PDM_InitFieldNoDefault(&propertyFilters, "PropertyFilters", "Property Filters",         "", "", "");
    propertyFilters.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&active,                  "Active", true, "Active", "", "", "");
    active.uiCapability()->setUiHidden(true);
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
void RimEclipsePropertyFilterCollection::setReservoirView(RimEclipseView* reservoirView)
{
    m_reservoirView = reservoirView;

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimEclipsePropertyFilter* propertyFilter = propertyFilters[i];

        propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipsePropertyFilterCollection::reservoirView()
{
    CVF_ASSERT(!m_reservoirView.isNull());
    return m_reservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->updateUiIconFromToggleField();

    m_reservoirView->fieldChangedByUi(&(m_reservoirView->propertyFilterCollection), oldValue, newValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter* RimEclipsePropertyFilterCollection::createAndAppendPropertyFilter()
{
    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    
    propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());

    propertyFilters.push_back(propertyFilter);

    propertyFilter->resultDefinition->setResultVariable(m_reservoirView->cellResult->resultVariable());
    propertyFilter->resultDefinition->setPorosityModel(m_reservoirView->cellResult->porosityModel());
    propertyFilter->resultDefinition->setResultType(m_reservoirView->cellResult->resultType());
    propertyFilter->resultDefinition->loadResult();
    propertyFilter->setToDefaultValues();
    propertyFilter->updateFilterName();

    return propertyFilter;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::loadAndInitializePropertyFilters()
{
    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimEclipsePropertyFilter* propertyFilter = propertyFilters[i];

        propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());
        propertyFilter->resultDefinition->loadResult();
        propertyFilter->updateIconState();
        propertyFilter->computeResultValueRange();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::initAfterRead()
{
    loadAndInitializePropertyFilters();

    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipsePropertyFilterCollection::remove(RimEclipsePropertyFilter* propertyFilter)
{
    propertyFilters.removeChildObject(propertyFilter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipsePropertyFilterCollection::hasActiveFilters() const
{
    if (!active) return false;

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
    if (!active) return false;

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
    return &active;
}

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

#include "RimCellPropertyFilterCollection.h"

#include "RimEclipseView.h"
#include "RimResultDefinition.h"
#include "RimResultSlot.h"


CAF_PDM_SOURCE_INIT(RimCellPropertyFilterCollection, "CellPropertyFilters");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellPropertyFilterCollection::RimCellPropertyFilterCollection()
{
    CAF_PDM_InitObject("Cell Property Filters", ":/CellFilter_Values.png", "", "");

    CAF_PDM_InitFieldNoDefault(&propertyFilters, "PropertyFilters", "Property Filters",         "", "", "");
    CAF_PDM_InitField(&active,                  "Active", true, "Active", "", "", "");
    active.setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellPropertyFilterCollection::~RimCellPropertyFilterCollection()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilterCollection::setReservoirView(RimEclipseView* reservoirView)
{
    m_reservoirView = reservoirView;

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimCellPropertyFilter* propertyFilter = propertyFilters[i];

        propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimCellPropertyFilterCollection::reservoirView()
{
    CVF_ASSERT(!m_reservoirView.isNull());
    return m_reservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilterCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    this->updateUiIconFromToggleField();

    m_reservoirView->fieldChangedByUi(&(m_reservoirView->propertyFilterCollection), oldValue, newValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellPropertyFilter* RimCellPropertyFilterCollection::createAndAppendPropertyFilter()
{
    RimCellPropertyFilter* propertyFilter = new RimCellPropertyFilter();
    
    propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());

    propertyFilter->setParentContainer(this);
    propertyFilters.push_back(propertyFilter);

    propertyFilter->resultDefinition->setResultVariable(m_reservoirView->cellResult->resultVariable());
    propertyFilter->resultDefinition->setPorosityModel(m_reservoirView->cellResult->porosityModel());
    propertyFilter->resultDefinition->setResultType(m_reservoirView->cellResult->resultType());
    propertyFilter->resultDefinition->loadResult();
    propertyFilter->setDefaultValues();

    propertyFilter->name = QString("New Filter (%1)").arg(propertyFilters().size());


    return propertyFilter;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilterCollection::loadAndInitializePropertyFilters()
{
    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimCellPropertyFilter* propertyFilter = propertyFilters[i];

        propertyFilter->setParentContainer(this);

        propertyFilter->resultDefinition->setReservoirView(m_reservoirView.p());
        propertyFilter->resultDefinition->loadResult();
        propertyFilter->updateIconState();
        propertyFilter->computeResultValueRange();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilterCollection::initAfterRead()
{
    loadAndInitializePropertyFilters();

    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellPropertyFilterCollection::remove(RimCellPropertyFilter* propertyFilter)
{
    propertyFilters.removeChildObject(propertyFilter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCellPropertyFilterCollection::hasActiveFilters() const
{
    if (!active) return false;

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimCellPropertyFilter* propertyFilter = propertyFilters[i];
        if (propertyFilter->isActive() && propertyFilter->resultDefinition->hasResult()) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether any of the active property filters are based on a dynamic result
//--------------------------------------------------------------------------------------------------
bool RimCellPropertyFilterCollection::hasActiveDynamicFilters() const
{
    if (!active) return false;

    for (size_t i = 0; i < propertyFilters.size(); i++)
    {
        RimCellPropertyFilter* propertyFilter = propertyFilters[i];
        if (propertyFilter->isActive() && propertyFilter->resultDefinition->hasDynamicResult()) return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellPropertyFilterCollection::objectToggleField()
{
    return &active;
}

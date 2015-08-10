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

#include "RicEclipsePropertyFilterNewExec.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseCellColors.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterNewExec::RicEclipsePropertyFilterNewExec(RimEclipsePropertyFilterCollection* propertyFilterCollection)
    : CmdExecuteCommand(NULL)
{
    assert(propertyFilterCollection);
    m_propertyFilterCollection = propertyFilterCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterNewExec::~RicEclipsePropertyFilterNewExec()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicEclipsePropertyFilterNewExec::name()
{
    return "Create Property Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewExec::redo()
{ 
    RimEclipsePropertyFilter* propertyFilter = new RimEclipsePropertyFilter();
    propertyFilter->setParentContainer(m_propertyFilterCollection);

    RimEclipseView* reservoirView = m_propertyFilterCollection->reservoirView();
    assert(reservoirView);

    propertyFilter->resultDefinition->setReservoirView(reservoirView);
    propertyFilter->resultDefinition->setResultVariable(reservoirView->cellResult->resultVariable());
    propertyFilter->resultDefinition->setPorosityModel(reservoirView->cellResult->porosityModel());
    propertyFilter->resultDefinition->setResultType(reservoirView->cellResult->resultType());
    propertyFilter->resultDefinition->loadResult();
    propertyFilter->setToDefaultValues();
    propertyFilter->updateFilterName();

    m_propertyFilterCollection->propertyFilters.push_back(propertyFilter);
    m_propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);

    caf::PdmUiFieldHandle::updateConnectedUiEditors(m_propertyFilterCollection->parentField());
    caf::PdmUiFieldHandle::updateConnectedUiEditors(&m_propertyFilterCollection->propertyFilters);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewExec::undo()
{
    m_propertyFilterCollection->propertyFilters.erase(m_propertyFilterCollection->propertyFilters.size() - 1);
    caf::PdmUiFieldHandle::updateConnectedUiEditors(m_propertyFilterCollection->parentField());
}

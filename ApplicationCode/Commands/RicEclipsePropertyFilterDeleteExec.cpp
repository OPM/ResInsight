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

#include "RicEclipsePropertyFilterDeleteExec.h"


#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterDeleteExec::RicEclipsePropertyFilterDeleteExec(RimEclipsePropertyFilter* propertyFilter)
    : CmdExecuteCommand(NULL)
{
    CVF_ASSERT(propertyFilter);
    m_propertyFilter = propertyFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterDeleteExec::~RicEclipsePropertyFilterDeleteExec()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicEclipsePropertyFilterDeleteExec::name()
{
    return "Delete Property Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterDeleteExec::redo()
{ 
    RimEclipsePropertyFilterCollection* propertyFilterCollection = m_propertyFilter->parentContainer();
    CVF_ASSERT(propertyFilterCollection);

    bool wasFilterActive = m_propertyFilter->isActive();
    bool wasSomeFilterActive = propertyFilterCollection->hasActiveFilters();

    propertyFilterCollection->remove(m_propertyFilter);
    delete m_propertyFilter;

    if (wasFilterActive)
    {
        propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);
    }

    if (wasSomeFilterActive)
    {
        propertyFilterCollection->reservoirView()->createDisplayModelAndRedraw();
    }

    caf::PdmUiFieldHandle::updateConnectedUiEditors(propertyFilterCollection->parentField());
    caf::PdmUiFieldHandle::updateConnectedUiEditors(&propertyFilterCollection->propertyFilters);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterDeleteExec::undo()
{
    // TODO
    CVF_ASSERT(0);
}

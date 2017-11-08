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

#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimView.h"


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
    return "New Property Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewExec::redo()
{ 
    RicEclipsePropertyFilterFeatureImpl::addPropertyFilter(m_propertyFilterCollection);

    RimView* view = nullptr;
    m_propertyFilterCollection->firstAncestorOrThisOfTypeAsserted(view);

    //Enable display of grid cells, to be able to show generated property filter
    view->showGridCells(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewExec::undo()
{
    m_propertyFilterCollection->propertyFilters.erase(m_propertyFilterCollection->propertyFilters.size() - 1);

    m_propertyFilterCollection->updateConnectedEditors();
}

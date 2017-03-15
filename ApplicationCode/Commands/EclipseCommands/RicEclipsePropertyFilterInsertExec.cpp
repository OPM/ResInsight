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

#include "RicEclipsePropertyFilterInsertExec.h"


#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterInsertExec::RicEclipsePropertyFilterInsertExec(RimEclipsePropertyFilter* propertyFilter)
    : CmdExecuteCommand(NULL)
{
    CVF_ASSERT(propertyFilter);
    m_propertyFilter = propertyFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterInsertExec::~RicEclipsePropertyFilterInsertExec()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicEclipsePropertyFilterInsertExec::name()
{
    return "Insert Property Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterInsertExec::redo()
{ 
    RimEclipsePropertyFilterCollection* propertyFilterCollection = nullptr;
    m_propertyFilter->firstAncestorOrThisOfTypeAsserted(propertyFilterCollection);

    size_t index = propertyFilterCollection->propertyFilters.index(m_propertyFilter);
    CVF_ASSERT(index < propertyFilterCollection->propertyFilters.size());

    RicEclipsePropertyFilterFeatureImpl::insertPropertyFilter(propertyFilterCollection, index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterInsertExec::undo()
{
    // TODO
    CVF_ASSERT(0);
}

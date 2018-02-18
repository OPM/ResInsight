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

#include "RicGeoMechPropertyFilterInsertExec.h"


#include "RicGeoMechPropertyFilterFeatureImpl.h"

#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicGeoMechPropertyFilterInsertExec::RicGeoMechPropertyFilterInsertExec(RimGeoMechPropertyFilter* propertyFilter)
    : CmdExecuteCommand(nullptr)
{
    CVF_ASSERT(propertyFilter);
    m_propertyFilter = propertyFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicGeoMechPropertyFilterInsertExec::~RicGeoMechPropertyFilterInsertExec()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicGeoMechPropertyFilterInsertExec::name()
{
    return "Insert Property Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterInsertExec::redo()
{ 
    RimGeoMechPropertyFilterCollection* propertyFilterCollection = m_propertyFilter->parentContainer();
    CVF_ASSERT(propertyFilterCollection);

    size_t index = propertyFilterCollection->propertyFilters.index(m_propertyFilter);
    CVF_ASSERT(index < propertyFilterCollection->propertyFilters.size());

    RicGeoMechPropertyFilterFeatureImpl::insertPropertyFilter(propertyFilterCollection, index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterInsertExec::undo()
{
    // TODO
    CVF_ASSERT(0);
}

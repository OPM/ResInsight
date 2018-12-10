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

#include "RicGeoMechPropertyFilterNewExec.h"

#include "RicGeoMechPropertyFilterFeatureImpl.h"

#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicGeoMechPropertyFilterNewExec::RicGeoMechPropertyFilterNewExec(RimGeoMechPropertyFilterCollection* propertyFilterCollection)
    : CmdExecuteCommand(nullptr)
{
    assert(propertyFilterCollection);
    m_propertyFilterCollection = propertyFilterCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicGeoMechPropertyFilterNewExec::~RicGeoMechPropertyFilterNewExec()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicGeoMechPropertyFilterNewExec::name()
{
    return "New Property Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterNewExec::redo()
{ 
    RicGeoMechPropertyFilterFeatureImpl::addPropertyFilter(m_propertyFilterCollection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterNewExec::undo()
{
    m_propertyFilterCollection->propertyFilters.erase(m_propertyFilterCollection->propertyFilters.size() - 1);

    m_propertyFilterCollection->updateConnectedEditors();
}

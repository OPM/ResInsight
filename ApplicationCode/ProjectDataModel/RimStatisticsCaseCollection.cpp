/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimStatisticsCaseCollection.h"

#include "RimIdenticalGridCaseGroup.h"


CAF_PDM_SOURCE_INIT(RimStatisticsCaseCollection, "RimStatisticalCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticsCaseCollection::RimStatisticsCaseCollection()
    : PdmObject()
{
    CAF_PDM_InitObject("Derived Statistics", "", "", "");

    CAF_PDM_InitFieldNoDefault(&cases, "Reservoirs", "",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticsCaseCollection::~RimStatisticsCaseCollection()
{
    cases.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimStatisticsCaseCollection::parentCaseGroup()
{
    std::vector<RimIdenticalGridCaseGroup*> parentObjects;
    this->parentObjectsOfType(parentObjects);

    if (parentObjects.size() > 0)
    {
        return parentObjects[0];
    }

    return NULL;
}


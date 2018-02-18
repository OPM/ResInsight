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

#include "RimCaseCollection.h"

#include "RimEclipseCase.h"
#include "RimIdenticalGridCaseGroup.h"


CAF_PDM_SOURCE_INIT(RimCaseCollection, "RimCaseCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCaseCollection::RimCaseCollection()
{
    CAF_PDM_InitObject("Derived Statistics", "", "", "");

    CAF_PDM_InitFieldNoDefault(&reservoirs, "Reservoirs", "Reservoirs ChildArrayField",  "", "", "");
    reservoirs.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCaseCollection::~RimCaseCollection()
{
    reservoirs.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimCaseCollection::parentCaseGroup()
{
    return dynamic_cast<RimIdenticalGridCaseGroup*>(this->parentField()->ownerObject());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimCaseCollection::findByDescription(const QString& caseDescription) const
{
    for (size_t i = 0; i < reservoirs.size(); i++)
    {
        if (caseDescription == reservoirs[i]->caseUserDescription())
        {
            return reservoirs[i];
        }
    }

    return nullptr;
}

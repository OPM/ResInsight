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

#include "RIStdInclude.h"

#include "RimReservoirView.h"

#include "RimStatisticalCollection.h"
#include "RimIdenticalGridCaseGroup.h"


CAF_PDM_SOURCE_INIT(RimStatisticalCollection, "RimStatisticalCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCollection::RimStatisticalCollection()
    : PdmObject()
{
    CAF_PDM_InitObject("Derived Statistics", "", "", "");

    CAF_PDM_InitFieldNoDefault(&reservoirs, "Reservoirs", "",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCollection::~RimStatisticalCollection()
{
    reservoirs.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCalculation* RimStatisticalCollection::createAndAppendStatisticalCalculation()
{
    RimStatisticalCalculation* newObject = new RimStatisticalCalculation;
    RimIdenticalGridCaseGroup* gridCaseGroup = parent();
    
    CVF_ASSERT(gridCaseGroup);
    CVF_ASSERT(gridCaseGroup->mainGrid());

    newObject->setMainGrid(gridCaseGroup->mainGrid());

    newObject->caseName = QString("Statistics ") + QString::number(reservoirs.size()+1);

    reservoirs.push_back(newObject);

    return newObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimStatisticalCollection::parent()
{
    std::vector<caf::PdmObject*> parentObjects;
    this->parentObjects(parentObjects);

    RimIdenticalGridCaseGroup* gridCaseGroup = NULL;
    for (size_t i = 0; i < parentObjects.size(); i++)
    {
        if (gridCaseGroup) continue;

        caf::PdmObject* obj = parentObjects[i];
        gridCaseGroup = dynamic_cast<RimIdenticalGridCaseGroup*>(obj);
    }

    return gridCaseGroup;

}


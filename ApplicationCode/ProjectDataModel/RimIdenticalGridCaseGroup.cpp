/////////////////////////////////////////////////////////////////////////////////
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

#include "RimIdenticalGridCaseGroup.h"
#include "RimReservoir.h"
#include "RimReservoirView.h"
#include "RigEclipseCase.h"

#include "RimStatisticalCalculation.h"
#include "RimStatisticalCollection.h"


CAF_PDM_SOURCE_INIT(RimIdenticalGridCaseGroup, "RimIdenticalGridCaseGroup");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup::RimIdenticalGridCaseGroup()
{
    CAF_PDM_InitObject("Grid Case Group", "", "", "");

    CAF_PDM_InitField(&name,    "UserDescription",  QString("Identical Grid Case Group"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&caseCollection, "CaseCollection", "Cases", "", "", "");
    CAF_PDM_InitFieldNoDefault(&statisticalReservoirCollection, "StatisticalReservoirCollection", "Derived Statistics", "", "", "");

    caseCollection = new RimCaseCollection;
    statisticalReservoirCollection = new RimStatisticalCollection;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup::~RimIdenticalGridCaseGroup()
{
    m_mainGrid = NULL;

    delete caseCollection;
    caseCollection = NULL;

    delete statisticalReservoirCollection;
    statisticalReservoirCollection = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::addCase(RimReservoir* reservoir)
{
    CVF_ASSERT(reservoir);

    if (!reservoir) return;

    RigMainGrid* incomingMainGrid = reservoir->reservoirData()->mainGrid();

    if (m_mainGrid.isNull())
    {
        m_mainGrid = incomingMainGrid;
    }

    CVF_ASSERT(m_mainGrid.p() == incomingMainGrid);
 
    caseCollection()->reservoirs().push_back(reservoir);

    if (statisticalReservoirCollection->reservoirs().size() == 0)
    {
        statisticalReservoirCollection->createAndAppendStatisticalCalculation();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimIdenticalGridCaseGroup::mainGrid()
{
    if (m_mainGrid.notNull()) return m_mainGrid.p();

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIdenticalGridCaseGroup::userDescriptionField()
{
    return &name;
}


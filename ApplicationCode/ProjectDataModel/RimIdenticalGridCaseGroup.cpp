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
#include "RimResultReservoir.h"
#include "cafProgressInfo.h"


CAF_PDM_SOURCE_INIT(RimIdenticalGridCaseGroup, "RimIdenticalGridCaseGroup");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup::RimIdenticalGridCaseGroup()
{
    CAF_PDM_InitObject("Grid Case Group", ":/GridCaseGroup16x16.png", "", "");

    CAF_PDM_InitField(&name,    "UserDescription",  QString("Grid Case Group"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&statisticalReservoirCollection, "StatisticalReservoirCollection", "Derived Statistics", ":/Histograms16x16.png", "", "");
    CAF_PDM_InitFieldNoDefault(&caseCollection, "CaseCollection", "Cases", ":/Cases16x16.png", "", "");
 
    caseCollection = new RimCaseCollection;
    statisticalReservoirCollection = new RimStatisticalCollection;

    m_mainGrid = NULL;
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

    if (!m_mainGrid)
    {
        m_mainGrid = incomingMainGrid;
    }

    CVF_ASSERT(m_mainGrid == incomingMainGrid);
 
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
    if (m_mainGrid) return m_mainGrid;

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIdenticalGridCaseGroup::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
///  Make sure changes in this functions is validated to RIApplication::addEclipseCases()
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::initAfterRead()
{
    if (caseCollection()->reservoirs().size() == 0)
    {
        return;
    }

    // First file is read completely including grid.
    // The main grid from the first case is reused directly in for the other cases. 
    // When reading active cell info, only the total cell count is tested for consistency
    RigEclipseCase* mainEclipseCase = NULL;

    RimResultReservoir* rimReservoir = dynamic_cast<RimResultReservoir*>(caseCollection()->reservoirs()[0]);
    CVF_ASSERT(rimReservoir);
        
    rimReservoir->openEclipseGridFile();

    mainEclipseCase = rimReservoir->reservoirData();
    CVF_ASSERT(mainEclipseCase);

    
    // Read active cell info from all source cases
    caf::ProgressInfo info(caseCollection()->reservoirs().size(), "Case group - Reading Active Cell data");
    for (size_t i = 1; i < caseCollection()->reservoirs().size(); i++)
    {
        RimResultReservoir* rimReservoir = dynamic_cast<RimResultReservoir*>(caseCollection()->reservoirs()[i]);
        CVF_ASSERT(rimReservoir);

        if (!rimReservoir->openAndReadActiveCellData(mainEclipseCase))
        {
            CVF_ASSERT(false);
        }

        info.setProgress(i);
    }


    // Set main grid for statistical calculations
    for (size_t i = 0; i < statisticalReservoirCollection()->reservoirs().size(); i++)
    {
        RimStatisticalCalculation* rimReservoir = dynamic_cast<RimStatisticalCalculation*>(statisticalReservoirCollection()->reservoirs()[i]);
        CVF_ASSERT(rimReservoir);

        rimReservoir->setMainGrid(mainEclipseCase->mainGrid());
    }

    m_mainGrid = mainEclipseCase->mainGrid();
}


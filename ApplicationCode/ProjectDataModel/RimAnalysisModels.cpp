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

#include "RiaStdInclude.h"

#include "RimAnalysisModels.h"
#include "RiaApplication.h"
#include "RimProject.h"
#include "cafAppEnum.h"
#include "RimReservoirView.h"

#include "RimIdenticalGridCaseGroup.h"

#include "RiaApplication.h"

#include "RigGridManager.h"
#include "RigCaseData.h"
#include "RimResultCase.h"
#include "RimWellPathCollection.h"


#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimWellCollection.h"
#include "RimCaseCollection.h"
#include "RimResultSlot.h"
#include "RimStatisticsCase.h"
#include "RimOilField.h"
#include "RimScriptCollection.h"

CAF_PDM_SOURCE_INIT(RimAnalysisModels, "ResInsightAnalysisModels");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnalysisModels::RimAnalysisModels(void)
{
    CAF_PDM_InitObject("Grid Models", ":/Cases16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&cases, "Reservoirs", "",  "", "", "");
    CAF_PDM_InitFieldNoDefault(&caseGroups, "CaseGroups", "",  "", "", "");

    m_gridCollection = new RigGridManager;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnalysisModels::~RimAnalysisModels(void)
{
    close();    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnalysisModels::close()
{
    m_gridCollection->clear();

    cases.deleteAllChildObjects();
    caseGroups.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimAnalysisModels::createIdenticalCaseGroupFromMainCase(RimCase* mainCase)
{
    CVF_ASSERT(mainCase);

    RigCaseData* rigEclipseCase = mainCase->reservoirData();
    RigMainGrid* equalGrid = registerCaseInGridCollection(rigEclipseCase);
    CVF_ASSERT(equalGrid);

    RimIdenticalGridCaseGroup* group = new RimIdenticalGridCaseGroup;
    assert(RiaApplication::instance()->project());
    RiaApplication::instance()->project()->assignIdToCaseGroup(group);

    RimCase* createdCase = group->createAndAppendStatisticsCase();
    RiaApplication::instance()->project()->assignCaseIdToCase(createdCase);

    group->addCase(mainCase);
    caseGroups().push_back(group);

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnalysisModels::moveEclipseCaseIntoCaseGroup(RimCase* rimReservoir)
{
    CVF_ASSERT(rimReservoir);

    RigCaseData* rigEclipseCase = rimReservoir->reservoirData();
    RigMainGrid* equalGrid = registerCaseInGridCollection(rigEclipseCase);
    CVF_ASSERT(equalGrid);

    // Insert in identical grid group
    bool foundGroup = false;

    for (size_t i = 0; i < caseGroups.size(); i++)
    {
        RimIdenticalGridCaseGroup* cg = caseGroups()[i];

        if (cg->mainGrid() == equalGrid)
        {
            cg->addCase(rimReservoir);
            foundGroup = true;
        }
    }

    if (!foundGroup)
    {
        RimIdenticalGridCaseGroup* group = new RimIdenticalGridCaseGroup;
        assert(RiaApplication::instance()->project());
        RiaApplication::instance()->project()->assignIdToCaseGroup(group);

        group->addCase(rimReservoir);

        caseGroups().push_back(group);
    }

    // Remove reservoir from main container
    cases().removeChildObject(rimReservoir);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnalysisModels::removeCaseFromAllGroups(RimCase* reservoir)
{
    m_gridCollection->removeCase(reservoir->reservoirData());

    for (size_t i = 0; i < caseGroups.size(); i++)
    {
        RimIdenticalGridCaseGroup* cg = caseGroups()[i];

        cg->removeCase(reservoir);
    }

    cases().removeChildObject(reservoir);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimAnalysisModels::registerCaseInGridCollection(RigCaseData* rigEclipseCase)
{
    CVF_ASSERT(rigEclipseCase);

    RigMainGrid* equalGrid = m_gridCollection->findEqualGrid(rigEclipseCase->mainGrid());

    if (equalGrid)
    {
        // Replace the grid with an already registered grid
        rigEclipseCase->setMainGrid(equalGrid);
    }
    else
    {
        // This is the first insertion of this grid, compute cached data
        rigEclipseCase->mainGrid()->computeCachedData();

        std::vector<RigGridBase*> grids;
        rigEclipseCase->allGrids(&grids);

        size_t i;
        for (i = 0; i < grids.size(); i++)
        {
            grids[i]->computeFaults();
        }

        equalGrid = rigEclipseCase->mainGrid();
    }

    m_gridCollection->addCase(rigEclipseCase);

    return equalGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnalysisModels::insertCaseInCaseGroup(RimIdenticalGridCaseGroup* caseGroup, RimCase* rimReservoir)
{
    CVF_ASSERT(rimReservoir);

    RigCaseData* rigEclipseCase = rimReservoir->reservoirData();
    registerCaseInGridCollection(rigEclipseCase);

    caseGroup->addCase(rimReservoir);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnalysisModels::recomputeStatisticsForAllCaseGroups()
{
    const size_t numCaseGroups = caseGroups.size();
    for (size_t caseGrpIdx = 0; caseGrpIdx < numCaseGroups; ++caseGrpIdx)
    {
        RimIdenticalGridCaseGroup* caseGroup = caseGroups[caseGrpIdx];
        RimCaseCollection* statisticsCaseCollection = caseGroup->statisticsCaseCollection;
        const size_t numStatisticsCases = statisticsCaseCollection->reservoirs.size();
        for (size_t caseIdx = 0; caseIdx < numStatisticsCases; caseIdx++)
        {
            RimStatisticsCase* statisticsCase = dynamic_cast<RimStatisticsCase*>(statisticsCaseCollection->reservoirs[caseIdx]);
            if (statisticsCase)
            {
                statisticsCase->clearComputedStatistics();
                statisticsCase->computeStatistics();
            }
        }
    }
}


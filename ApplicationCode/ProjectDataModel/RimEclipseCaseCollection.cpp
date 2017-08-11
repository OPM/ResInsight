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

#include "RimEclipseCaseCollection.h"

#include "RiaApplication.h"

#include "RigEclipseCaseData.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimProject.h"


CAF_PDM_SOURCE_INIT(RimEclipseCaseCollection, "ResInsightAnalysisModels");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCaseCollection::RimEclipseCaseCollection(void)
{
    CAF_PDM_InitObject("Grid Models", ":/Cases16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&cases, "Reservoirs", "",  "", "", "");
    cases.uiCapability()->setUiHidden(true);
    
    CAF_PDM_InitFieldNoDefault(&caseGroups, "CaseGroups", "",  "", "", "");
    caseGroups.uiCapability()->setUiHidden(true);

    m_gridCollection = new RigGridManager;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCaseCollection::~RimEclipseCaseCollection(void)
{
    close();    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseCollection::close()
{
    m_gridCollection->clear();

    cases.deleteAllChildObjects();
    caseGroups.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimEclipseCaseCollection::createIdenticalCaseGroupFromMainCase(RimEclipseCase* mainCase)
{
    CVF_ASSERT(mainCase);

    RigEclipseCaseData* rigEclipseCase = mainCase->eclipseCaseData();
    RigMainGrid* equalGrid = registerCaseInGridCollection(rigEclipseCase);
    CVF_ASSERT(equalGrid);

    RimIdenticalGridCaseGroup* group = new RimIdenticalGridCaseGroup;
    assert(RiaApplication::instance()->project());
    RiaApplication::instance()->project()->assignIdToCaseGroup(group);

    group->addCase(mainCase);
    RimEclipseCase* createdCase = group->createAndAppendStatisticsCase();

    RiaApplication::instance()->project()->assignCaseIdToCase(createdCase);

    caseGroups().push_back(group);

    return group;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseCollection::moveEclipseCaseIntoCaseGroup(RimEclipseCase* rimReservoir)
{
    CVF_ASSERT(rimReservoir);

    RigEclipseCaseData* rigEclipseCase = rimReservoir->eclipseCaseData();
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
void RimEclipseCaseCollection::removeCaseFromAllGroups(RimEclipseCase* reservoir)
{
    m_gridCollection->removeCase(reservoir->eclipseCaseData());

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
RigMainGrid* RimEclipseCaseCollection::registerCaseInGridCollection(RigEclipseCaseData* rigEclipseCase)
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

        rigEclipseCase->mainGrid()->calculateFaults(rigEclipseCase->activeCellInfo(RiaPorosityModel::MATRIX_MODEL));
  
        equalGrid = rigEclipseCase->mainGrid();
    }

    m_gridCollection->addCase(rigEclipseCase);

    return equalGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseCollection::insertCaseInCaseGroup(RimIdenticalGridCaseGroup* caseGroup, RimEclipseCase* rimReservoir)
{
    CVF_ASSERT(rimReservoir);

    RigEclipseCaseData* rigEclipseCase = rimReservoir->eclipseCaseData();
    registerCaseInGridCollection(rigEclipseCase);

    caseGroup->addCase(rimReservoir);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCaseCollection::recomputeStatisticsForAllCaseGroups()
{
    const size_t numCaseGroups = caseGroups.size();
    for (size_t caseGrpIdx = 0; caseGrpIdx < numCaseGroups; ++caseGrpIdx)
    {
        RimIdenticalGridCaseGroup* caseGroup = caseGroups[caseGrpIdx];
        RimCaseCollection* statisticsCaseCollection = caseGroup->statisticsCaseCollection;
        const size_t numStatisticsCases = statisticsCaseCollection->reservoirs.size();
        for (size_t caseIdx = 0; caseIdx < numStatisticsCases; caseIdx++)
        {
            RimEclipseStatisticsCase* statisticsCase = dynamic_cast<RimEclipseStatisticsCase*>(statisticsCaseCollection->reservoirs[caseIdx]);
            if (statisticsCase)
            {
                statisticsCase->clearComputedStatistics();
                statisticsCase->computeStatistics();
            }
        }
    }
}


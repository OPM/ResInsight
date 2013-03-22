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
#include "RigReservoirCellResults.h"

#include "RimStatisticalCalculation.h"
#include "RimStatisticalCollection.h"
#include "RimResultReservoir.h"
#include "cafProgressInfo.h"
#include "RigActiveCellInfo.h"


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

    m_unionOfMatrixActiveCells = new RigActiveCellInfo;
    m_unionOfFractureActiveCells = new RigActiveCellInfo;
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
        createAndAppendStatisticalCalculation();
    }

    clearActiveCellUnions();
    clearStatisticsResults();
    updateMainGridAndActiveCellsForStatisticsCases();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::removeCase(RimReservoir* reservoir)
{
    caseCollection()->reservoirs().removeChildObject(reservoir);

    if (caseCollection()->reservoirs().size() == 0)
    {
        m_mainGrid = NULL;
    }
    
    clearActiveCellUnions();
    clearStatisticsResults();
    updateMainGridAndActiveCellsForStatisticsCases();
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
void RimIdenticalGridCaseGroup::loadMainCaseAndActiveCellInfo()
{
    if (caseCollection()->reservoirs().size() == 0)
    {
        return;
    }

    // Read the main case completely including grid.
    // The mainGrid from the first case is reused directly in for the other cases. 
    // When reading active cell info, only the total cell count is tested for consistency

    RimReservoir* mainCase = caseCollection()->reservoirs[0];
    mainCase->openEclipseGridFile();
    RigEclipseCase* mainEclipseCase = mainCase->reservoirData();
    CVF_ASSERT(mainEclipseCase);

    // Read active cell info from all source cases
    
    caf::ProgressInfo info(caseCollection()->reservoirs.size(), "Case group - Reading Active Cell data");
    for (size_t i = 1; i < caseCollection()->reservoirs.size(); i++)
    {
        RimResultReservoir* rimReservoir = dynamic_cast<RimResultReservoir*>(caseCollection()->reservoirs[i]);
        if(!rimReservoir) continue; // Input reservoir

        if (!rimReservoir->openAndReadActiveCellData(mainEclipseCase))
        {
            CVF_ASSERT(false);
        }

        info.incrementProgress();
    }

    m_mainGrid = mainEclipseCase->mainGrid();

    // Check if we need to calculate the union of the active cells

    bool foundResultsInCache = false;
    for (size_t i = 0; i < statisticalReservoirCollection()->reservoirs.size(); i++)
    {
        RimStatisticalCalculation* rimReservoir = statisticalReservoirCollection()->reservoirs[i];

        // Check if any results are stored in cache
        if (rimReservoir->results(RifReaderInterface::MATRIX_RESULTS)->storedResultsCount() > 0 ||
            rimReservoir->results(RifReaderInterface::FRACTURE_RESULTS)->storedResultsCount() > 0)
        {
            foundResultsInCache = true;
            break;
        }
    }

    if (foundResultsInCache)
    {
        computeUnionOfActiveCells();
    }

    // "Load" the statistical cases

    for (size_t i = 0; i < statisticalReservoirCollection()->reservoirs.size(); i++)
    {
        RimStatisticalCalculation* rimReservoir = statisticalReservoirCollection()->reservoirs[i];

        rimReservoir->openEclipseGridFile();

        if (i == 0)
        {
            rimReservoir->reservoirData()->computeActiveCellBoundingBoxes();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::computeUnionOfActiveCells()
{
    if (m_unionOfMatrixActiveCells->globalMatrixModelActiveCellCount() > 0)
    {
        return;
    }

    if (caseCollection->reservoirs.size() == 0 || !m_mainGrid)
    {
        this->clearActiveCellUnions();

        return;
    }

    m_unionOfMatrixActiveCells->setGlobalCellCount(m_mainGrid->cells().size());
    m_unionOfFractureActiveCells->setGlobalCellCount(m_mainGrid->cells().size());
    m_unionOfMatrixActiveCells->setGridCount(m_mainGrid->gridCount());
    m_unionOfFractureActiveCells->setGridCount(m_mainGrid->gridCount());

    size_t globalActiveMatrixIndex = 0;
    size_t globalActiveFractureIndex = 0;

    for (size_t gridIdx = 0; gridIdx < m_mainGrid->gridCount(); gridIdx++)
    {
        RigGridBase* grid = m_mainGrid->gridByIndex(gridIdx);

        std::vector<char> activeM(grid->cellCount(), 0);
        std::vector<char> activeF(grid->cellCount(), 0);

        for (size_t localGridCellIdx = 0; localGridCellIdx < grid->cellCount(); localGridCellIdx++)
        {
            for (size_t caseIdx = 0; caseIdx < caseCollection->reservoirs.size(); caseIdx++)
            {
                size_t globalCellIdx = grid->globalGridCellIndex(localGridCellIdx);

                if (activeM[localGridCellIdx] == 0)
                {
                    if (caseCollection->reservoirs[caseIdx]->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->isActiveInMatrixModel(globalCellIdx))
                    {
                        activeM[localGridCellIdx] = 1;
                    }
                }

                if (activeF[localGridCellIdx] == 0)
                {
                    if (caseCollection->reservoirs[caseIdx]->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS)->isActiveInMatrixModel(globalCellIdx))
                    {
                        activeF[localGridCellIdx] = 1;
                    }
                }
            }
        }

        size_t activeMatrixIndex = 0;
        size_t activeFractureIndex = 0;

        for (size_t localGridCellIdx = 0; localGridCellIdx < grid->cellCount(); localGridCellIdx++)
        {
            size_t globalCellIdx = grid->globalGridCellIndex(localGridCellIdx);

            if (activeM[localGridCellIdx] != 0)
            {
                m_unionOfMatrixActiveCells->setActiveIndexInMatrixModel(globalCellIdx, globalActiveMatrixIndex++);
                activeMatrixIndex++;
            }

            if (activeF[localGridCellIdx] != 0)
            {
                m_unionOfFractureActiveCells->setActiveIndexInMatrixModel(globalCellIdx, globalActiveFractureIndex++);
                activeFractureIndex++;
            }
        }

        m_unionOfMatrixActiveCells->setGridActiveCellCounts(gridIdx, activeMatrixIndex);
        m_unionOfFractureActiveCells->setGridActiveCellCounts(gridIdx, activeFractureIndex);
    }

    m_unionOfMatrixActiveCells->computeDerivedData();
    m_unionOfFractureActiveCells->computeDerivedData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCalculation* RimIdenticalGridCaseGroup::createAndAppendStatisticalCalculation()
{
    RimStatisticalCalculation* newObject = new RimStatisticalCalculation;

    newObject->caseName = QString("Statistics ") + QString::number(statisticalReservoirCollection()->reservoirs.size()+1);
    statisticalReservoirCollection()->reservoirs.push_back(newObject);

    newObject->openEclipseGridFile();

    return newObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::updateMainGridAndActiveCellsForStatisticsCases()
{
    for (size_t i = 0; i < statisticalReservoirCollection->reservoirs().size(); i++)
    {
        RimStatisticalCalculation* rimStaticsCase = statisticalReservoirCollection->reservoirs[i];

        rimStaticsCase->reservoirData()->setMainGrid(this->mainGrid());

        if (i == 0)
        {
            rimStaticsCase->reservoirData()->computeActiveCellBoundingBoxes();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::clearStatisticsResults()
{
    for (size_t i = 0; i < statisticalReservoirCollection->reservoirs().size(); i++)
    {
        RimStatisticalCalculation* rimStaticsCase = statisticalReservoirCollection->reservoirs[i];
        rimStaticsCase->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->clearAllResults();
        rimStaticsCase->results(RifReaderInterface::FRACTURE_RESULTS)->cellResults()->clearAllResults();

        for (size_t j = 0; j < rimStaticsCase->reservoirViews.size(); j++)
        {
            RimReservoirView* rimReservoirView = rimStaticsCase->reservoirViews[j];
            rimReservoirView->cellResult()->resultVariable = RimDefines::undefinedResultName();
            rimReservoirView->cellEdgeResult()->resultVariable = RimDefines::undefinedResultName();
            rimReservoirView->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::clearActiveCellUnions()
{
    m_unionOfMatrixActiveCells->clear();
    m_unionOfFractureActiveCells->clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimIdenticalGridCaseGroup::contains(RimReservoir* reservoir) const
{
    CVF_ASSERT(reservoir);

    for (size_t i = 0; i < caseCollection()->reservoirs().size(); i++)
    {
        RimReservoir* rimReservoir = caseCollection()->reservoirs()[i];
        if (reservoir->caseName == reservoir->caseName)
        {
            return true;
        }
    }
    
    return false;
}

RigActiveCellInfo* RimIdenticalGridCaseGroup::unionOfActiveCells(RifReaderInterface::PorosityModelResultType porosityType)
{
    if (porosityType == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_unionOfMatrixActiveCells.p();
    }
    else
    {
        return m_unionOfFractureActiveCells.p();
    }
}

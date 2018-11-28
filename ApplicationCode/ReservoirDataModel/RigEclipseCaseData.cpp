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

#include "RigEclipseCaseData.h"

#include "RiaApplication.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellPath.h"

#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData::RigEclipseCaseData(RimEclipseCase* ownerCase)
{
    m_mainGrid = new RigMainGrid();
    m_ownerCase = ownerCase;

    m_matrixModelResults = new RigCaseCellResultsData(this);
    m_fractureModelResults = new RigCaseCellResultsData(this);

    m_activeCellInfo = new RigActiveCellInfo;
    m_fractureActiveCellInfo = new RigActiveCellInfo;

    m_matrixModelResults->setActiveCellInfo(m_activeCellInfo.p());
    m_fractureModelResults->setActiveCellInfo(m_fractureActiveCellInfo.p());

    m_unitsType = RiaEclipseUnitTools::UNITS_METRIC;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData::~RigEclipseCaseData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RigEclipseCaseData::mainGrid()
{
    return m_mainGrid.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigMainGrid* RigEclipseCaseData::mainGrid() const
{
    return m_mainGrid.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setMainGrid(RigMainGrid* mainGrid)
{
    m_mainGrid = mainGrid;

    m_matrixModelResults->setMainGrid(m_mainGrid.p());
    m_fractureModelResults->setMainGrid(m_mainGrid.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::allGrids(std::vector<RigGridBase*>* grids)
{
    CVF_ASSERT(grids);

    if (m_mainGrid.isNull())
    {
        return;
    }

    size_t i;
    for (i = 0; i < m_mainGrid->gridCount(); i++)
    {
        grids->push_back(m_mainGrid->gridByIndex(i));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::allGrids(std::vector<const RigGridBase*>* grids) const
{
    CVF_ASSERT(grids);

    if (m_mainGrid.isNull())
    {
        return;
    }

    size_t i;
    for (i = 0; i < m_mainGrid->gridCount(); i++)
    {
        grids->push_back(m_mainGrid->gridByIndex(i));
    }
}

//--------------------------------------------------------------------------------------------------
/// Get grid by index. The main grid has index 0, so the first lgr has index 1
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigEclipseCaseData::grid(size_t index) const
{
    CVF_ASSERT(m_mainGrid.notNull());
    return m_mainGrid->gridByIndex(index);
}


//--------------------------------------------------------------------------------------------------
/// Get grid by index. The main grid has index 0, so the first lgr has index 1
//--------------------------------------------------------------------------------------------------
RigGridBase* RigEclipseCaseData::grid(size_t index) 
{
    CVF_ASSERT(m_mainGrid.notNull());
    return m_mainGrid->gridByIndex(index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigEclipseCaseData::grid(const QString& gridName) const
{
    if (m_mainGrid.isNull())
    {
        return nullptr;
    }

    if (gridName.isEmpty())
    {
        return m_mainGrid.p();
    }

    size_t i;
    for (i = 0; i < m_mainGrid->gridCount(); i++)
    {
        const RigGridBase* grid = m_mainGrid->gridByIndex(i);
        if (QString::fromStdString(grid->gridName()) == gridName) return grid;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigEclipseCaseData::gridCount() const
{
    CVF_ASSERT(m_mainGrid.notNull());
    return m_mainGrid->gridCount();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeWellCellsPrGrid()
{
    // If we have computed this already, return
    if (m_wellCellsInGrid.size()) return; 

    std::vector<RigGridBase*> grids;
    this->allGrids(&grids);

    // Debug code used to display grid names and grid sizes
/*
    size_t totCellCount = 0;
    for (auto g : grids)
    {
        qDebug() << g->gridName().data();
        qDebug() << g->cellCountI() << " " << g->cellCountJ() << " " << g->cellCountK() << " ";

        size_t cellCount = g->cellCount();
        totCellCount += cellCount;
        qDebug() << cellCount;

        qDebug() << "\n";
    }

    qDebug() << "\nTotal cell count " << totCellCount;
*/

    size_t gIdx;

    //  Allocate and initialize the arrays

    m_wellCellsInGrid.resize(grids.size());
    m_gridCellToResultWellIndex.resize(grids.size());

    for (gIdx = 0; gIdx < grids.size(); ++gIdx)
    {
        if (m_wellCellsInGrid[gIdx].isNull() || m_wellCellsInGrid[gIdx]->size() != grids[gIdx]->cellCount())
        {
            m_wellCellsInGrid[gIdx] = new cvf::UByteArray;
            m_wellCellsInGrid[gIdx]->resize(grids[gIdx]->cellCount());

            m_gridCellToResultWellIndex[gIdx] = new cvf::UIntArray;
            m_gridCellToResultWellIndex[gIdx]->resize(grids[gIdx]->cellCount());
        }
        m_wellCellsInGrid[gIdx]->setAll(false);
        m_gridCellToResultWellIndex[gIdx]->setAll(cvf::UNDEFINED_UINT);
    }

    // Fill arrays with data
    size_t wIdx;
    for (wIdx = 0; wIdx < m_simWellData.size(); ++wIdx)
    {
        size_t tIdx;
        for (tIdx = 0; tIdx < m_simWellData[wIdx]->m_wellCellsTimeSteps.size(); ++tIdx)
        {
            RigWellResultFrame& wellCells =  m_simWellData[wIdx]->m_wellCellsTimeSteps[tIdx];

            // Well result branches
            for (size_t sIdx = 0; sIdx < wellCells.m_wellResultBranches.size(); ++sIdx)
            {
                RigWellResultBranch& wellSegment = wellCells.m_wellResultBranches[sIdx];

                size_t cdIdx;
                for (cdIdx = 0; cdIdx < wellSegment.m_branchResultPoints.size(); ++cdIdx)
                {
                    size_t gridIndex     = wellSegment.m_branchResultPoints[cdIdx].m_gridIndex;
                    size_t gridCellIndex = wellSegment.m_branchResultPoints[cdIdx].m_gridCellIndex;

                    if(gridIndex < m_wellCellsInGrid.size() && gridCellIndex < m_wellCellsInGrid[gridIndex]->size())
                    {
                        // NOTE : We do not check if the grid cell is active as we do for well head.
                        // If we add test for active cell, thorough testing and verification of the new behaviour must be adressed

                        m_wellCellsInGrid[gridIndex]->set(gridCellIndex, true);
                        m_gridCellToResultWellIndex[gridIndex]->set(gridCellIndex, static_cast<cvf::uint>(wIdx));
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setSimWellData(const cvf::Collection<RigSimWellData>& data)
{
    m_simWellData = data;
    m_wellCellsInGrid.clear();
    m_gridCellToResultWellIndex.clear();

    computeWellCellsPrGrid();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<QString> RigEclipseCaseData::findSortedWellNames() const
{
    std::set<QString> sortedWellNames;

    const cvf::Collection<RigSimWellData>& simWellData = wellResults();

    for (size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx)
    {
        sortedWellNames.insert(simWellData[wIdx]->m_wellName);
    }

    return sortedWellNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigSimWellData* RigEclipseCaseData::findSimWellData(QString wellName) const
{
    for (size_t wIdx = 0; wIdx < m_simWellData.size(); ++wIdx)
    {
        if (m_simWellData[wIdx]->m_wellName == wellName)
        {
            return m_simWellData[wIdx].p();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::UByteArray* RigEclipseCaseData::wellCellsInGrid(size_t gridIndex)
{
    computeWellCellsPrGrid();
    CVF_ASSERT(gridIndex < m_wellCellsInGrid.size());

    return m_wellCellsInGrid[gridIndex].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::UIntArray* RigEclipseCaseData::gridCellToResultWellIndex(size_t gridIndex)
{
    computeWellCellsPrGrid();
    CVF_ASSERT(gridIndex < m_gridCellToResultWellIndex.size());

    return m_gridCellToResultWellIndex[gridIndex].p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigCell& RigEclipseCaseData::cellFromWellResultCell(const RigWellResultPoint& wellResultCell) const
{
    CVF_ASSERT(wellResultCell.isCell());

    size_t gridIndex     = wellResultCell.m_gridIndex;
    size_t gridCellIndex = wellResultCell.m_gridCellIndex;

    std::vector<const RigGridBase*> grids;
    allGrids(&grids);

    return grids[gridIndex]->cell(gridCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigEclipseCaseData::findSharedSourceFace(cvf::StructGridInterface::FaceType& sharedSourceFace, 
                                              const RigWellResultPoint& sourceWellCellResult, 
                                              const RigWellResultPoint& otherWellCellResult) const
{
    size_t gridIndex = sourceWellCellResult.m_gridIndex;
    size_t gridCellIndex = sourceWellCellResult.m_gridCellIndex;

    size_t otherGridIndex = otherWellCellResult.m_gridIndex;
    size_t otherGridCellIndex = otherWellCellResult.m_gridCellIndex;

    if (gridIndex != otherGridIndex) return false;

    std::vector<const RigGridBase*> grids;
    allGrids(&grids);

    const RigGridBase* grid = grids[gridIndex];
    size_t i, j, k;
    grid->ijkFromCellIndex(gridCellIndex, &i, &j, &k);

    size_t faceIdx;
    for (faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        cvf::StructGridInterface::FaceType sourceFace = static_cast<cvf::StructGridInterface::FaceType>(faceIdx);

        size_t ni, nj, nk;
        grid->neighborIJKAtCellFace(i, j, k, sourceFace, &ni, &nj, &nk);

        if (grid->isCellValid(ni, nj, nk))
        {

            size_t neighborCellIndex = grid->cellIndexFromIJK(ni, nj, nk);

            if (neighborCellIndex == otherGridCellIndex)
            {
                sharedSourceFace = sourceFace;
                return true;
            }
        }
    }

    return false;
}



//--------------------------------------------------------------------------------------------------
/// Helper class used to find min/max range for valid and active cells
//--------------------------------------------------------------------------------------------------
class CellRangeBB
{
public:
    CellRangeBB()
        : m_min(cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T),
        m_max(cvf::Vec3st::ZERO)
    {

    }

    void add(size_t i, size_t j, size_t k)
    {
        if (i < m_min.x()) m_min.x() = i;
        if (j < m_min.y()) m_min.y() = j;
        if (k < m_min.z()) m_min.z() = k;

        if (i > m_max.x()) m_max.x() = i;
        if (j > m_max.y()) m_max.y() = j;
        if (k > m_max.z()) m_max.z() = k;
    }

public:
    cvf::Vec3st m_min;
    cvf::Vec3st m_max;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeActiveCellIJKBBox()
{
    if (m_mainGrid.notNull() && m_activeCellInfo.notNull() && m_fractureActiveCellInfo.notNull())
    {
        CellRangeBB matrixModelActiveBB;
        CellRangeBB fractureModelActiveBB;

        size_t idx;
        for (idx = 0; idx < m_mainGrid->cellCount(); idx++)
        {
            size_t i, j, k;
            m_mainGrid->ijkFromCellIndex(idx, &i, &j, &k);

            if (m_activeCellInfo->isActive(idx))
            {
                matrixModelActiveBB.add(i, j, k);
            }

            if (m_fractureActiveCellInfo->isActive(idx))
            {
                fractureModelActiveBB.add(i, j, k);
            }
        }
        m_activeCellInfo->setIJKBoundingBox(matrixModelActiveBB.m_min, matrixModelActiveBB.m_max);
        m_fractureActiveCellInfo->setIJKBoundingBox(fractureModelActiveBB.m_min, fractureModelActiveBB.m_max);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeActiveCellBoundingBoxes()
{
    computeActiveCellIJKBBox();
    computeActiveCellsGeometryBoundingBox();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigEclipseCaseData::simulationWellNames() const
{
    std::vector<QString> wellNames;
    for (const auto& wellResult : wellResults())
    {
        wellNames.push_back(wellResult->m_wellName);
    }
    return wellNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigEclipseCaseData::hasSimulationWell(const QString& simWellName) const
{
    const auto wellNames = simulationWellNames();
    return std::find(wellNames.begin(), wellNames.end(), simWellName) != wellNames.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigWellPath*> RigEclipseCaseData::simulationWellBranches(const QString& simWellName,
                                                                           bool           includeAllCellCenters,
                                                                           bool           useAutoDetectionOfBranches) const
{
    std::vector<const RigWellPath*> branches;

    if (simWellName.isEmpty() || simWellName.toUpper() == "NONE")
    {
        return branches;
    }

    const RigSimWellData* simWellData = findSimWellData(simWellName);
    if (!simWellData) return branches;

    std::tuple<QString, bool, bool> simWellSeachItem =
        std::make_tuple(simWellName, includeAllCellCenters, useAutoDetectionOfBranches);

    if (m_simWellBranchCache.find(simWellSeachItem) == m_simWellBranchCache.end())
    {
        std::vector<std::vector<cvf::Vec3d>>         pipeBranchesCLCoords;
        std::vector<std::vector<RigWellResultPoint>> pipeBranchesCellIds;

        RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineFromWellFrame(
            this, simWellData, -1, useAutoDetectionOfBranches, includeAllCellCenters, pipeBranchesCLCoords, pipeBranchesCellIds);

        m_simWellBranchCache.insert(std::make_pair(simWellSeachItem, cvf::Collection<RigWellPath>()));

        for (size_t brIdx = 0; brIdx < pipeBranchesCLCoords.size(); ++brIdx)
        {
            auto wellMdCalculator = RigSimulationWellCoordsAndMD(pipeBranchesCLCoords[brIdx]);

            cvf::ref<RigWellPath> newWellPath = new RigWellPath();
            newWellPath->m_measuredDepths     = wellMdCalculator.measuredDepths();
            newWellPath->m_wellPathPoints     = wellMdCalculator.wellPathPoints();

            m_simWellBranchCache[simWellSeachItem].push_back(newWellPath.p());
        }
    }

    for (const auto& branch : m_simWellBranchCache[simWellSeachItem])
    {
        branches.push_back(branch.p());
    }

    return branches;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setVirtualPerforationTransmissibilities(RigVirtualPerforationTransmissibilities* virtualPerforationTransmissibilities)
{
    m_virtualPerforationTransmissibilities = virtualPerforationTransmissibilities;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigVirtualPerforationTransmissibilities* RigEclipseCaseData::virtualPerforationTransmissibilities() const
{
    return m_virtualPerforationTransmissibilities.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RigEclipseCaseData::activeCellInfo(RiaDefines::PorosityModelType porosityModel)
{
    if (porosityModel == RiaDefines::MATRIX_MODEL)
    {
        return m_activeCellInfo.p();
    }

    return m_fractureActiveCellInfo.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigActiveCellInfo* RigEclipseCaseData::activeCellInfo(RiaDefines::PorosityModelType porosityModel) const
{
    if (porosityModel == RiaDefines::MATRIX_MODEL)
    {
        return m_activeCellInfo.p();
    }

    return m_fractureActiveCellInfo.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setActiveCellInfo(RiaDefines::PorosityModelType porosityModel, RigActiveCellInfo* activeCellInfo)
{
    if (porosityModel == RiaDefines::MATRIX_MODEL)
    {
        m_activeCellInfo = activeCellInfo;
        m_matrixModelResults->setActiveCellInfo(m_activeCellInfo.p());
    }
    else
    {
        m_fractureActiveCellInfo = activeCellInfo;
        m_fractureModelResults->setActiveCellInfo(m_fractureActiveCellInfo.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigEclipseCaseData::hasFractureResults() const
{
    if (activeCellInfo(RiaDefines::FRACTURE_MODEL)
     && activeCellInfo(RiaDefines::FRACTURE_MODEL)->reservoirActiveCellCount() > 0)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeActiveCellsGeometryBoundingBox()
{
    if (m_activeCellInfo.isNull() || m_fractureActiveCellInfo.isNull())
    {
        return;
    }

    if (m_mainGrid.isNull())
    {
        cvf::BoundingBox bb;
        m_activeCellInfo->setGeometryBoundingBox(bb);
        m_fractureActiveCellInfo->setGeometryBoundingBox(bb);
        return;
    }

    RigActiveCellInfo* activeInfos[2];
    activeInfos[0] = m_fractureActiveCellInfo.p();
    activeInfos[1] = m_activeCellInfo.p(); // Last, to make this bb.min become display offset

    cvf::BoundingBox bb;
    for (int acIdx = 0; acIdx < 2; ++acIdx)
    {
        bb.reset();
        if (m_mainGrid->nodes().size() == 0)
        {
            bb.add(cvf::Vec3d::ZERO);
        }
        else
        {
            std::array<cvf::Vec3d, 8> hexCorners;
            for (size_t i = 0; i < m_mainGrid->cellCount(); i++)
            {
                if (activeInfos[acIdx]->isActive(i))
                {
                    m_mainGrid->cellCornerVertices(i, hexCorners.data());
                    for (const auto& corner : hexCorners)
                    {
                        bb.add(corner);
                    }
                }
            }
        }

        activeInfos[acIdx]->setGeometryBoundingBox(bb);
    }

    m_mainGrid->setDisplayModelOffset(bb.min());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setActiveFormationNames(RigFormationNames* activeFormationNames)
{
    m_activeFormationNamesData  = activeFormationNames;

    size_t totalGlobCellCount = m_mainGrid->globalCellArray().size();
    size_t resIndex = m_matrixModelResults->addStaticScalarResult(RiaDefines::FORMATION_NAMES, 
                                                                  RiaDefines::activeFormationNamesResultName(), 
                                                                  false, 
                                                                  totalGlobCellCount);

    std::vector<double>& fnData =  m_matrixModelResults->cellScalarResults(resIndex,0);

    if (m_activeFormationNamesData.isNull())
    {
        for ( size_t cIdx = 0; cIdx < totalGlobCellCount; ++cIdx )
        {
            fnData[cIdx] = HUGE_VAL;
        }

        return;
    }

    size_t localCellCount = m_mainGrid->cellCount();
    for (size_t cIdx = 0; cIdx < localCellCount; ++cIdx)
    {
        size_t i (cvf::UNDEFINED_SIZE_T), j(cvf::UNDEFINED_SIZE_T), k(cvf::UNDEFINED_SIZE_T);

        if(!m_mainGrid->ijkFromCellIndex(cIdx, &i, &j, &k)) continue;

        int formNameIdx = activeFormationNames->formationIndexFromKLayerIdx(k);
        if (formNameIdx != -1)
        {
            fnData[cIdx] = formNameIdx;
        }
        else
        {
            fnData[cIdx] = HUGE_VAL;
        }
    }

    for (size_t cIdx = localCellCount; cIdx < totalGlobCellCount; ++cIdx)
    {
        size_t mgrdCellIdx =  m_mainGrid->globalCellArray()[cIdx].mainGridCellIndex();

        size_t i (cvf::UNDEFINED_SIZE_T), j(cvf::UNDEFINED_SIZE_T), k(cvf::UNDEFINED_SIZE_T);

        if(!m_mainGrid->ijkFromCellIndex(mgrdCellIdx, &i, &j, &k)) continue;

        int formNameIdx = activeFormationNames->formationIndexFromKLayerIdx(k);
        if (formNameIdx != -1)
        {
            fnData[cIdx] = formNameIdx;
        }
        else
        {
            fnData[cIdx] = HUGE_VAL;
        }
    }

    RimProject* project = RiaApplication::instance()->project();
    if (project)
    {
        if (project->mainPlotCollection())
        {
            project->mainPlotCollection->updatePlotsWithFormations();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFormationNames* RigEclipseCaseData::activeFormationNames()
{
    return m_activeFormationNamesData.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData* RigEclipseCaseData::results(RiaDefines::PorosityModelType porosityModel)
{
    if (porosityModel == RiaDefines::MATRIX_MODEL)
    {
        return m_matrixModelResults.p();
    }

    return m_fractureModelResults.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigCaseCellResultsData* RigEclipseCaseData::results(RiaDefines::PorosityModelType porosityModel) const
{
    if (porosityModel == RiaDefines::MATRIX_MODEL)
    {
        return m_matrixModelResults.p();
    }

    return m_fractureModelResults.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigEclipseCaseData::resultValues(RiaDefines::PorosityModelType porosityModel,
                                                            RiaDefines::ResultCatType type, 
                                                            const QString& resultName, 
                                                            size_t timeStepIndex)
{
    RigCaseCellResultsData* gridCellResults = this->results(porosityModel);
    size_t scalarResultIndex = gridCellResults->findOrLoadScalarResult(type, resultName);

    const std::vector<double>* swatResults = nullptr;
    if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
    {
        swatResults = &(gridCellResults->cellScalarResults(scalarResultIndex, timeStepIndex));
    }

    return swatResults;
}

/*
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseData::closeReaderInterface()
{
    RifReaderInterface* readerInterface = m_matrixModelResults->readerInterface();

    if (readerInterface)
    {
        readerInterface->close();
    }
}
*/

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

#include "RigCaseData.h"
#include "RigMainGrid.h"
#include "RigCaseCellResultsData.h"
#include "RigGridScalarDataAccess.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseData::RigCaseData()
{
    m_mainGrid = new RigMainGrid();

    m_matrixModelResults = new RigCaseCellResultsData(m_mainGrid.p());
    m_fractureModelResults = new RigCaseCellResultsData(m_mainGrid.p());

    m_activeCellInfo = new RigActiveCellInfo;
    m_fractureActiveCellInfo = new RigActiveCellInfo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseData::~RigCaseData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseData::setMainGrid(RigMainGrid* mainGrid)
{
    m_mainGrid = mainGrid;

    m_matrixModelResults->setMainGrid(m_mainGrid.p());
    m_fractureModelResults->setMainGrid(m_mainGrid.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseData::allGrids(std::vector<RigGridBase*>* grids)
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
void RigCaseData::allGrids(std::vector<const RigGridBase*>* grids) const
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
const RigGridBase* RigCaseData::grid(size_t index) const
{
    CVF_ASSERT(m_mainGrid.notNull());
    return m_mainGrid->gridByIndex(index);
}


//--------------------------------------------------------------------------------------------------
/// Get grid by index. The main grid has index 0, so the first lgr has index 1
//--------------------------------------------------------------------------------------------------
RigGridBase* RigCaseData::grid(size_t index) 
{
    CVF_ASSERT(m_mainGrid.notNull());
    return m_mainGrid->gridByIndex(index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigCaseData::gridCount() const
{
    CVF_ASSERT(m_mainGrid.notNull());
    return m_mainGrid->gridCount();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseData::computeWellCellsPrGrid()
{
    // If we have computed this already, return
    if (m_wellCellsInGrid.size()) return; 

    std::vector<RigGridBase*> grids;
    this->allGrids(&grids);
    size_t gIdx;

    //  Allocate and initialize the arrays

    m_wellCellsInGrid.resize(grids.size());
    m_gridCellToWellIndex.resize(grids.size());

    for (gIdx = 0; gIdx < grids.size(); ++gIdx)
    {
        if (m_wellCellsInGrid[gIdx].isNull() || m_wellCellsInGrid[gIdx]->size() != grids[gIdx]->cellCount())
        {
            m_wellCellsInGrid[gIdx] = new cvf::UByteArray;
            m_wellCellsInGrid[gIdx]->resize(grids[gIdx]->cellCount());

            m_gridCellToWellIndex[gIdx] = new cvf::UIntArray;
            m_gridCellToWellIndex[gIdx]->resize(grids[gIdx]->cellCount());
        }
        m_wellCellsInGrid[gIdx]->setAll(false);
        m_gridCellToWellIndex[gIdx]->setAll(cvf::UNDEFINED_UINT);
    }

    // Fill arrays with data
    size_t wIdx;
    for (wIdx = 0; wIdx < m_wellResults.size(); ++wIdx)
    {
        size_t tIdx;
        for (tIdx = 0; tIdx < m_wellResults[wIdx]->m_wellCellsTimeSteps.size(); ++tIdx)
        {
            RigWellResultFrame& wellCells =  m_wellResults[wIdx]->m_wellCellsTimeSteps[tIdx];

            size_t gridIndex        =  wellCells.m_wellHead.m_gridIndex;
            size_t gridCellIndex    =  wellCells.m_wellHead.m_gridCellIndex;

            CVF_ASSERT(gridIndex < m_wellCellsInGrid.size() && gridCellIndex < m_wellCellsInGrid[gridIndex]->size());
            m_wellCellsInGrid[gridIndex]->set(gridCellIndex, true);
            m_gridCellToWellIndex[gridIndex]->set(gridCellIndex, static_cast<cvf::uint>(wIdx));

            size_t sIdx;
            for (sIdx = 0; sIdx < wellCells.m_wellResultBranches.size(); ++sIdx)
            {
                RigWellResultBranch& wellSegment = wellCells.m_wellResultBranches[sIdx];
                size_t cdIdx;
                for (cdIdx = 0; cdIdx < wellSegment.m_branchResultPoints.size(); ++cdIdx)
                {
                    gridIndex     = wellSegment.m_branchResultPoints[cdIdx].m_gridIndex;
                    gridCellIndex = wellSegment.m_branchResultPoints[cdIdx].m_gridCellIndex;

                    if(gridIndex < m_wellCellsInGrid.size() && gridCellIndex < m_wellCellsInGrid[gridIndex]->size())
                    {
                        m_wellCellsInGrid[gridIndex]->set(gridCellIndex, true);
                        m_gridCellToWellIndex[gridIndex]->set(gridCellIndex, static_cast<cvf::uint>(wIdx));
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseData::setWellResults(const cvf::Collection<RigSingleWellResultsData>& data)
{
    m_wellResults = data;
    m_wellCellsInGrid.clear();
    m_gridCellToWellIndex.clear();

    computeWellCellsPrGrid();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::UByteArray* RigCaseData::wellCellsInGrid(size_t gridIndex)
{
    computeWellCellsPrGrid();
    CVF_ASSERT(gridIndex < m_wellCellsInGrid.size());

    return m_wellCellsInGrid[gridIndex].p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::UIntArray* RigCaseData::gridCellToWellIndex(size_t gridIndex)
{
    computeWellCellsPrGrid();
    CVF_ASSERT(gridIndex < m_gridCellToWellIndex.size());

    return m_gridCellToWellIndex[gridIndex].p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCell& RigCaseData::cellFromWellResultCell(const RigWellResultPoint& wellResultCell)
{
    CVF_ASSERT(wellResultCell.isCell());

    size_t gridIndex     = wellResultCell.m_gridIndex;
    size_t gridCellIndex = wellResultCell.m_gridCellIndex;

    std::vector<RigGridBase*> grids;
    allGrids(&grids);

    return grids[gridIndex]->cell(gridCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigCaseData::findSharedSourceFace(cvf::StructGridInterface::FaceType& sharedSourceFace,const RigWellResultPoint& sourceWellCellResult, const RigWellResultPoint& otherWellCellResult) const
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
void RigCaseData::computeActiveCellIJKBBox()
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
void RigCaseData::computeActiveCellBoundingBoxes()
{
    computeActiveCellIJKBBox();
    computeActiveCellsGeometryBoundingBox();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RigCaseData::activeCellInfo(RifReaderInterface::PorosityModelResultType porosityModel)
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_activeCellInfo.p();
    }

    return m_fractureActiveCellInfo.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigActiveCellInfo* RigCaseData::activeCellInfo(RifReaderInterface::PorosityModelResultType porosityModel) const
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_activeCellInfo.p();
    }

    return m_fractureActiveCellInfo.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseData::setActiveCellInfo(RifReaderInterface::PorosityModelResultType porosityModel, RigActiveCellInfo* activeCellInfo)
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        m_activeCellInfo = activeCellInfo;
    }
    else
    {
        m_fractureActiveCellInfo = activeCellInfo;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseData::computeActiveCellsGeometryBoundingBox()
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
            for (size_t i = 0; i < m_mainGrid->cellCount(); i++)
            {
                if (activeInfos[acIdx]->isActive(i))
                {
                    const RigCell& c = m_mainGrid->cells()[i];
                    const caf::SizeTArray8& indices = c.cornerIndices();

                    size_t idx;
                    for (idx = 0; idx < 8; idx++)
                    {
                        bb.add(m_mainGrid->nodes()[indices[idx]]);
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
RigCaseCellResultsData* RigCaseData::results(RifReaderInterface::PorosityModelResultType porosityModel)
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_matrixModelResults.p();
    }

    return m_fractureModelResults.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigCaseCellResultsData* RigCaseData::results(RifReaderInterface::PorosityModelResultType porosityModel) const
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_matrixModelResults.p();
    }

    return m_fractureModelResults.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::StructGridScalarDataAccess> RigCaseData::dataAccessObject(const RigGridBase* grid, 
                                                                           RifReaderInterface::PorosityModelResultType porosityModel, 
                                                                           size_t timeStepIndex, 
                                                                           size_t scalarSetIndex)
{
    if (timeStepIndex != cvf::UNDEFINED_SIZE_T && 
        scalarSetIndex != cvf::UNDEFINED_SIZE_T)
    {
        cvf::ref<cvf::StructGridScalarDataAccess> dataAccess = RigGridScalarDataAccessFactory::createPerGridDataAccessObject( this, grid->gridIndex(), porosityModel, timeStepIndex, scalarSetIndex);
        return dataAccess;
    }

    return NULL;

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

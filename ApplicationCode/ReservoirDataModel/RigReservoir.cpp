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
#include "RigReservoir.h"
#include "RigMainGrid.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigReservoir::RigReservoir()
{
    m_mainGrid = new RigMainGrid();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigReservoir::~RigReservoir()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoir::computeFaults()
{
    std::vector<RigGridBase*> grids;
    allGrids(&grids);

    size_t i;
    for (i = 0; i < grids.size(); i++)
    {
        grids[i]->computeFaults();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoir::allGrids(std::vector<RigGridBase*>* grids)
{
    CVF_ASSERT(grids);

    size_t i;
    for (i = 0; i < m_mainGrid->gridCount(); i++)
    {
        grids->push_back(m_mainGrid->gridByIndex(i));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoir::allGrids(std::vector<const RigGridBase*>* grids) const
{
    CVF_ASSERT(grids);
    size_t i;
    for (i = 0; i < m_mainGrid->gridCount(); i++)
    {
        grids->push_back(m_mainGrid->gridByIndex(i));
    }
}

//--------------------------------------------------------------------------------------------------
/// Get grid by index. The main grid has index 0, so the first lgr has index 1
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigReservoir::grid(size_t index) const
{
    return m_mainGrid->gridByIndex(index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoir::computeWellCellsPrGrid()
{
    // If we have computed this already, return
    if (m_wellCellsInGrid.size()) return; 

    std::vector<RigGridBase*> grids;
    this->allGrids(&grids);
    size_t gIdx;

    //  Allocate and initialize the arrays

    m_wellCellsInGrid.resize(grids.size());

    for (gIdx = 0; gIdx < grids.size(); ++gIdx)
    {
        if (m_wellCellsInGrid[gIdx].isNull() || m_wellCellsInGrid[gIdx]->size() != grids[gIdx]->cellCount())
        {
            m_wellCellsInGrid[gIdx] = new cvf::UByteArray;
            m_wellCellsInGrid[gIdx]->resize(grids[gIdx]->cellCount());

        }
        m_wellCellsInGrid[gIdx]->setAll(false);
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
            grids[gridIndex]->cell(gridCellIndex).setAsWellCell(true);
            m_wellCellsInGrid[gridIndex]->set(gridCellIndex, true);

            size_t sIdx;
            for (sIdx = 0; sIdx < wellCells.m_wellResultBranches.size(); ++sIdx)
            {
                RigWellResultBranch& wellSegment = wellCells.m_wellResultBranches[sIdx];
                size_t cdIdx;
                for (cdIdx = 0; cdIdx < wellSegment.m_wellCells.size(); ++cdIdx)
                {
                    gridIndex     = wellSegment.m_wellCells[cdIdx].m_gridIndex;
                    gridCellIndex = wellSegment.m_wellCells[cdIdx].m_gridCellIndex;

                    CVF_ASSERT(gridIndex < m_wellCellsInGrid.size() && gridCellIndex < m_wellCellsInGrid[gridIndex]->size());

                    grids[gridIndex]->cell(gridCellIndex).setAsWellCell(true);
                    m_wellCellsInGrid[gridIndex]->set(gridCellIndex, true);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoir::setWellResults(const cvf::Collection<RigWellResults>& data)
{
    m_wellResults = data;
    m_wellCellsInGrid.clear();
    computeWellCellsPrGrid();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::UByteArray* RigReservoir::wellCellsInGrid(size_t gridIndex)
{
    computeWellCellsPrGrid();
    CVF_ASSERT(gridIndex < m_wellCellsInGrid.size());

    return m_wellCellsInGrid[gridIndex].p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCell& RigReservoir::cellFromWellResultCell(const RigWellResultCell& wellResultCell)
{
    size_t gridIndex     = wellResultCell.m_gridIndex;
    size_t gridCellIndex = wellResultCell.m_gridCellIndex;

    std::vector<RigGridBase*> grids;
    allGrids(&grids);

    return grids[gridIndex]->cell(gridCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigReservoir::findSharedSourceFace(cvf::StructGridInterface::FaceType& sharedSourceFace,const RigWellResultCell& sourceWellCellResult, const RigWellResultCell& otherWellCellResult) const
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
        size_t neighborCellIndex = grid->cellIndexFromIJK(ni, nj, nk);

        if (neighborCellIndex == otherGridCellIndex)
        {
            sharedSourceFace = sourceFace;
            return true;
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
void RigReservoir::computeActiveCellData()
{
    CellRangeBB matrixModelActiveBB;
    CellRangeBB fractureModelActiveBB;

    size_t idx;
    for (idx = 0; idx < m_mainGrid->cellCount(); idx++)
    {
        const RigCell& c = m_mainGrid->cell(idx);

        size_t i, j, k;
        m_mainGrid->ijkFromCellIndex(idx, &i, &j, &k);

        if (c.isActiveInMatrixModel())
        {
            matrixModelActiveBB.add(i, j, k);
        }

        if (c.isActiveInFractureModel())
        {
            fractureModelActiveBB.add(i, j, k);
        }
    }

    m_activeCellInfo.setMatrixModelActiveCellsBoundingBox(matrixModelActiveBB.m_min, matrixModelActiveBB.m_max);
    m_activeCellInfo.setFractureModelActiveCellsBoundingBox(fractureModelActiveBB.m_min, fractureModelActiveBB.m_max);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigReservoir::computeCachedData()
{
    computeFaults();
    computeActiveCellData();

    //TODO Set display model offset
    /*
    if (m_mainGrid.notNull())
    {
        m_mainGrid->setDisplayModelOffset(m_activeCellInfo.m_activeCellPositionMin);
    }
    */
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RigReservoir::activeCellInfo()
{
    return &m_activeCellInfo;
}

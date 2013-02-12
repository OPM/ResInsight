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

#include "RigMainGrid.h"
#include "RigReservoirCellResults.h"

#include "cvfAssert.h"

RigMainGrid::RigMainGrid(void)
    : RigGridBase(this),
        m_activeCellPositionMin(cvf::Vec3st::UNDEFINED),
        m_activeCellPositionMax(cvf::Vec3st::UNDEFINED),
        m_validCellPositionMin(cvf::Vec3st::UNDEFINED),
        m_validCellPositionMax(cvf::Vec3st::UNDEFINED)
{
    m_matrixModelResults = new RigReservoirCellResults(this);
	m_fractureModelResults = new RigReservoirCellResults(this);

    m_activeCellsBoundingBox.add(cvf::Vec3d::ZERO);
    m_gridIndex = 0;
}


RigMainGrid::~RigMainGrid(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::addLocalGrid(RigLocalGrid* localGrid)
{
    m_localGrids.push_back(localGrid);
    localGrid->setGridIndex(m_localGrids.size()); // Maingrid itself has grid index 0
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::initAllSubGridsParentGridPointer()
{
    initSubGridParentPointer();
    size_t i;
    for (i = 0; i < m_localGrids.size(); ++i)
    {
       m_localGrids[i]->initSubGridParentPointer(); 
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::initAllSubCellsMainGridCellIndex()
{
    initSubCellsMainGridCellIndex();
    size_t i;
    for (i = 0; i < m_localGrids.size(); ++i)
    {
        m_localGrids[i]->initSubCellsMainGridCellIndex(); 
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::matrixModelActiveCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const
{
    min = m_activeCellPositionMin;
    max = m_activeCellPositionMax;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigMainGrid::matrixModelActiveCellsBoundingBox() const
{
    return m_activeCellsBoundingBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::validCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const
{
    min = m_validCellPositionMin;
    max = m_validCellPositionMax;
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
void RigMainGrid::computeActiveAndValidCellRanges()
{
    CellRangeBB validBB;
    CellRangeBB activeBB;

    size_t idx;
    for (idx = 0; idx < cellCount(); idx++)
    {
        const RigCell& c = cell(idx);
        
        size_t i, j, k;
        ijkFromCellIndex(idx, &i, &j, &k);

        if (!c.isInvalid())
        {
            validBB.add(i, j, k);
        }

        if (c.isActiveInMatrixModel())
        {
            activeBB.add(i, j, k);
        }
    }

    m_validCellPositionMin = validBB.m_min;
    m_validCellPositionMax = validBB.m_max;

    m_activeCellPositionMin = activeBB.m_min;
    m_activeCellPositionMax = activeBB.m_max;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigMainGrid::displayModelOffset() const
{
    return m_activeCellsBoundingBox.min();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::computeBoundingBox()
{
    m_activeCellsBoundingBox.reset();

    if (m_nodes.size() == 0)
    {
        m_activeCellsBoundingBox.add(cvf::Vec3d::ZERO);
    }
    else
    {
        size_t i;
        for (i = 0; i < cellCount(); i++)
        {
            const RigCell& c = cell(i);
            if (c.isActiveInMatrixModel())
            {
                const caf::SizeTArray8& indices = c.cornerIndices();

                size_t idx;
                for (idx = 0; idx < 8; idx++)
                {
                    m_activeCellsBoundingBox.add(m_nodes[indices[idx]]);
                }
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Initialize pointers from grid to parent grid
/// Compute cell ranges for active and valid cells
/// Compute bounding box in world coordinates based on node coordinates
//--------------------------------------------------------------------------------------------------
void RigMainGrid::computeCachedData()
{
    initAllSubGridsParentGridPointer();
    initAllSubCellsMainGridCellIndex();
    computeActiveAndValidCellRanges();
    computeBoundingBox();
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with index \a localGridIndex. Main Grid itself has index 0. First LGR starts on 1
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridByIndex(size_t localGridIndex)
{
    if (localGridIndex == 0) return this;
    CVF_ASSERT(localGridIndex - 1 < m_localGrids.size()) ;
    return m_localGrids[localGridIndex-1].p();
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with index \a localGridIndex. Main Grid itself has index 0. First LGR starts on 1
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigMainGrid::gridByIndex(size_t localGridIndex) const
{
    if (localGridIndex == 0) return this;
    CVF_ASSERT(localGridIndex - 1 < m_localGrids.size()) ;
    return m_localGrids[localGridIndex-1].p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigReservoirCellResults* RigMainGrid::results(RifReaderInterface::PorosityModelResultType porosityModel)
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
const RigReservoirCellResults* RigMainGrid::results(RifReaderInterface::PorosityModelResultType porosityModel) const
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_matrixModelResults.p();
    }

    return m_fractureModelResults.p();
}


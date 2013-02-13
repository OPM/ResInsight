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
    : RigGridBase(this)
{
    m_matrixModelResults = new RigReservoirCellResults(this);
	m_fractureModelResults = new RigReservoirCellResults(this);
    
    m_displayModelOffset = cvf::Vec3d::ZERO;
    
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
cvf::Vec3d RigMainGrid::displayModelOffset() const
{
    return m_displayModelOffset;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setDisplayModelOffset(cvf::Vec3d offset)
{
    m_displayModelOffset = offset;
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


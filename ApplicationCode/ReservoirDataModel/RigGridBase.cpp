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

#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigCell.h"
#include "RigCaseCellResultsData.h"
#include "RigResultAccessorFactory.h"

#include "cvfAssert.h"


RigGridBase::RigGridBase(RigMainGrid* mainGrid):
    m_gridPointDimensions(0,0,0),
    m_mainGrid(mainGrid),
    m_indexToStartOfCells(0)
{
    if (mainGrid == nullptr)
    {
        m_gridIndex = 0;
        m_gridId    = 0;
    }
    else
    {
        m_gridIndex = cvf::UNDEFINED_SIZE_T;
        m_gridId = cvf::UNDEFINED_INT;
    }
}


RigGridBase::~RigGridBase(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridBase::setGridName(const std::string& gridName)
{
    m_gridName = gridName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RigGridBase::gridName() const
{
    return m_gridName;
}

//--------------------------------------------------------------------------------------------------
/// Do we need this ?
//--------------------------------------------------------------------------------------------------
RigCell& RigGridBase::cell(size_t gridLocalCellIndex)
{
     CVF_ASSERT(m_mainGrid);

     CVF_ASSERT(m_indexToStartOfCells + gridLocalCellIndex < m_mainGrid->globalCellArray().size());

     return m_mainGrid->globalCellArray()[m_indexToStartOfCells + gridLocalCellIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigCell& RigGridBase::cell(size_t gridLocalCellIndex) const
{
    CVF_ASSERT(m_mainGrid);

    return m_mainGrid->globalCellArray()[m_indexToStartOfCells + gridLocalCellIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridBase::initSubGridParentPointer()
{
    RigGridBase* grid = this;

    size_t cellIdx;
    for (cellIdx = 0; cellIdx < grid->cellCount(); ++cellIdx)
    {
        RigCell& cell = grid->cell(cellIdx);
        if (cell.subGrid())
        {
            cell.subGrid()->setParentGrid(grid);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Find the cell index to the maingrid cell containing this cell, and store it as 
/// m_mainGridCellIndex in each cell.
//--------------------------------------------------------------------------------------------------
void RigGridBase::initSubCellsMainGridCellIndex()
{
    RigGridBase* grid = this;
    if (grid->isMainGrid())
    {
        size_t cellIdx;
        for (cellIdx = 0; cellIdx < grid->cellCount(); ++cellIdx)
        {
            RigCell& cell = grid->cell(cellIdx);
            cell.setMainGridCellIndex(cellIdx);
        }
    }
    else
    {
        size_t cellIdx;
        for (cellIdx = 0; cellIdx < grid->cellCount(); ++cellIdx)
        {
            RigLocalGrid* localGrid = static_cast<RigLocalGrid*>(grid);
            RigGridBase* parentGrid = localGrid->parentGrid();

            RigCell& cell = localGrid->cell(cellIdx);
            size_t parentCellIndex = cell.parentCellIndex();

            while (!parentGrid->isMainGrid())
            {
                const RigCell& parentCell = parentGrid->cell(parentCellIndex);
                parentCellIndex = parentCell.parentCellIndex();

                localGrid = static_cast<RigLocalGrid*>(parentGrid);
                parentGrid = localGrid->parentGrid();
            }

            cell.setMainGridCellIndex(parentCellIndex);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// For main grid, this will work with reservoirCellIndices retreiving the correct lgr cells as well.
/// the cell() call retreives correct cell, because main grid has offset of 0, and we access the global 
/// cell array in main grid.
//--------------------------------------------------------------------------------------------------
void RigGridBase::cellCornerVertices(size_t cellIndex, cvf::Vec3d vertices[8]) const
{
    const caf::SizeTArray8& indices = cell(cellIndex).cornerIndices();
    
    vertices[0].set(m_mainGrid->nodes()[indices[0]]);
    vertices[1].set(m_mainGrid->nodes()[indices[1]]);
    vertices[2].set(m_mainGrid->nodes()[indices[2]]);
    vertices[3].set(m_mainGrid->nodes()[indices[3]]);
    vertices[4].set(m_mainGrid->nodes()[indices[4]]);
    vertices[5].set(m_mainGrid->nodes()[indices[5]]);
    vertices[6].set(m_mainGrid->nodes()[indices[6]]);
    vertices[7].set(m_mainGrid->nodes()[indices[7]]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::cellIndexFromIJK(size_t i, size_t j, size_t k) const
{
    CVF_TIGHT_ASSERT(i != cvf::UNDEFINED_SIZE_T && j != cvf::UNDEFINED_SIZE_T && k != cvf::UNDEFINED_SIZE_T );
    CVF_TIGHT_ASSERT(i < m_gridPointDimensions.x() && j < m_gridPointDimensions.y() && k < m_gridPointDimensions.z()  );

    size_t ci = i + j*(m_gridPointDimensions.x() - 1) + k*((m_gridPointDimensions.x() - 1)*(m_gridPointDimensions.y() - 1));
    return ci;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridBase::cellMinMaxCordinates(size_t cellIndex, cvf::Vec3d* minCoordinate, cvf::Vec3d* maxCoordinate) const
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridBase::ijkFromCellIndex(size_t cellIndex, size_t* i, size_t* j, size_t* k) const
{
    CVF_TIGHT_ASSERT(cellIndex < cellCount());

    size_t index = cellIndex;

    if (cellCountI() <= 0 || cellCountJ() <= 0)
    {
        return false;
    }

    *k      = index/(cellCountI()*cellCountJ());
    index   -= (*k)*(cellCountI()*cellCountJ());
    *j      = index/cellCountI();
    index   -= (*j)*cellCountI();
    *i      = index;


    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::gridPointIndexFromIJK(size_t i, size_t j, size_t k) const
{
    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridBase::cellIJKFromCoordinate(const cvf::Vec3d& coord, size_t* i, size_t* j, size_t* k) const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGridBase::gridPointCoordinate(size_t i, size_t j, size_t k) const
{
    cvf::Vec3d pos;

    return pos;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGridBase::minCoordinate() const
{
    cvf::Vec3d v;

    return v;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::gridPointCountI() const
{
    return m_gridPointDimensions.x();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::gridPointCountJ() const
{
    return m_gridPointDimensions.y();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::gridPointCountK() const
{
    return m_gridPointDimensions.z();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGridBase::cellCentroid(size_t cellIndex) const
{
    cvf::Vec3d v;

    return v;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGridBase::maxCoordinate() const
{
    cvf::Vec3d v;

    return v;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridBase::isCellValid(size_t i, size_t j, size_t k) const
{
    if (i >= cellCountI() || j >= cellCountJ() || k >= cellCountK())
    {
        return false;
    }

    size_t idx = cellIndexFromIJK(i, j, k);
    const RigCell& c = cell(idx);
    return !c.isInvalid();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridBase::cellIJKNeighbor(size_t i, size_t j, size_t k, FaceType face, size_t* neighborCellIndex) const
{
    size_t ni, nj, nk;
    neighborIJKAtCellFace(i, j, k, face, &ni, &nj, &nk);

    if (!isCellValid(ni, nj, nk))
    {
        return false;
    }

    if (neighborCellIndex)
    {
        *neighborCellIndex = cellIndexFromIJK(ni, nj, nk);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridBase::isMainGrid() const
{
    return this == m_mainGrid;
}

//--------------------------------------------------------------------------------------------------
/// Models with large absolute values for coordinate scalars will often end up with z-fighting due
/// to numerical limits in float used by OpenGL to represent a position. displayModelOffset() is intended
//  to be subtracted from domain model coordinate when building geometry for visualization
//
//  Vec3d domainModelCoord
//  Vec3d coordForVisualization
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGridBase::displayModelOffset() const
{
    return m_mainGrid->displayModelOffset();
}

//--------------------------------------------------------------------------------------------------
/// Returns the min size of the I and J charactristic cell sizes
//--------------------------------------------------------------------------------------------------
double RigGridBase::characteristicIJCellSize() const
{
    double characteristicCellSize = HUGE_VAL;

    double cellSizeI, cellSizeJ, cellSizeK;
    this->characteristicCellSizes(&cellSizeI, &cellSizeJ, &cellSizeK);

    if (cellSizeI < characteristicCellSize) characteristicCellSize = cellSizeI;
    if (cellSizeJ < characteristicCellSize) characteristicCellSize = cellSizeJ;
   

    return characteristicCellSize;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::reservoirCellIndex(size_t gridLocalCellIndex) const
{
    return m_indexToStartOfCells + gridLocalCellIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::addCoarseningBox(size_t i1, size_t i2, size_t j1, size_t j2, size_t k1, size_t k2)
{
    caf::SizeTArray6 box;
    box[0] = i1;
    box[1] = i2;
    box[2] = j1;
    box[3] = j2;
    box[4] = k1;
    box[5] = k2;

    m_coarseningBoxInfo.push_back(box);

    size_t coarseningBoxIndex = m_coarseningBoxInfo.size() - 1;

    for (size_t k = k1; k <= k2; k++)
    {
        for (size_t j = j1; j <= j2; j++)
        {
            for (size_t i = i1; i <= i2; i++)
            {
                size_t cellIdx = this->cellIndexFromIJK(i, j, k);

                RigCell& c = this->cell(cellIdx);
                CVF_ASSERT(c.coarseningBoxIndex() == cvf::UNDEFINED_SIZE_T);

                c.setCoarseningBoxIndex(coarseningBoxIndex);
            }
        }
    }

    return coarseningBoxIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridBase::coarseningBox(size_t coarseningBoxIndex, size_t* i1, size_t* i2, size_t* j1, size_t* j2, size_t* k1, size_t* k2) const
{
    CVF_ASSERT(coarseningBoxIndex < m_coarseningBoxInfo.size());

    CVF_ASSERT(i1 && i2 && j1 && j2 && k1 && k2);

    caf::SizeTArray6 box = m_coarseningBoxInfo[coarseningBoxIndex];
    *i1 = box[0];
    *i2 = box[1];
    *j1 = box[2];
    *j2 = box[3];
    *k1 = box[4];
    *k2 = box[5];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigGridBase::boundingBox()
{
    if (!m_boundingBox.isValid())
    {
        cvf::Vec3d cornerVerts[8];

        for (size_t i = 0; i < cellCount(); i++)
        {
            cellCornerVertices(i, cornerVerts);

            for (size_t j = 0; j < 8; j++)
            {
                m_boundingBox.add(cornerVerts[j]);
            }
        }
    }
    
    return m_boundingBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridCellFaceVisibilityFilter::isFaceVisible(size_t i, size_t j, size_t k, cvf::StructGridInterface::FaceType face, const cvf::UByteArray* cellVisibility) const
{
    CVF_TIGHT_ASSERT(m_grid);

    size_t cellIndex = m_grid->cellIndexFromIJK(i, j, k);
    if (m_grid->cell(cellIndex).subGrid())
    {
        // Do not show any faces in the place where a LGR is present
        return false; 
    }

    size_t ni, nj, nk;
    cvf::StructGridInterface::neighborIJKAtCellFace(i, j, k, face, &ni, &nj, &nk);

    // If the cell is on the edge of the grid, Interpret as having an invisible neighbour
    if (ni >= m_grid->cellCountI() || nj >= m_grid->cellCountJ() || nk >= m_grid->cellCountK())
    {
        return true;
    }
       
    size_t neighborCellIndex = m_grid->cellIndexFromIJK(ni, nj, nk);

    // Do show the faces in the boarder between this grid and a possible LGR. Some of the LGR cells
    // might not be visible.
    if (m_grid->cell(neighborCellIndex).subGrid())
    {
        return true;
    }

    // Do not show cell geometry if a fault is present to avoid z fighting between surfaces
    // It will always be a better solution to avoid geometry creation instead of part priority and polygon offset
    size_t nativeResvCellIndex = m_grid->reservoirCellIndex(cellIndex);
    const RigFault* fault = m_grid->mainGrid()->findFaultFromCellIndexAndCellFace(nativeResvCellIndex, face);
    if (fault)
    {
        return false;
    }

    // If the neighbour cell is invisible, we need to draw the face
    if ((cellVisibility != nullptr) && !(*cellVisibility)[neighborCellIndex])
    {
        return true;
    }

    return false;
}

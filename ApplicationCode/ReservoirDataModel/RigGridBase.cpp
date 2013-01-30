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
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigCell.h"
#include "RigReservoirCellResults.h"

#include "cvfAssert.h"


RigGridBase::RigGridBase(RigMainGrid* mainGrid):
    m_gridPointDimensions(0,0,0),
    m_mainGrid(mainGrid),
    m_indexToStartOfCells(0),
    m_matrixModelActiveCellCount(cvf::UNDEFINED_SIZE_T),
    m_fractureModelActiveCellCount(cvf::UNDEFINED_SIZE_T)
{
    if (mainGrid == NULL)
    {
        m_gridIndex = 0;
    }
    else
    {
        m_gridIndex = cvf::UNDEFINED_SIZE_T;
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
RigCell& RigGridBase::cell(size_t gridCellIndex)
{
     CVF_ASSERT(m_mainGrid);

     CVF_ASSERT(m_indexToStartOfCells + gridCellIndex < m_mainGrid->cells().size());

     return m_mainGrid->cells()[m_indexToStartOfCells + gridCellIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigCell& RigGridBase::cell(size_t gridCellIndex) const
{
    CVF_ASSERT(m_mainGrid);

    return m_mainGrid->cells()[m_indexToStartOfCells + gridCellIndex];
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
            RigLocalGrid* localGrid = NULL;
            RigGridBase* parentGrid = NULL;

            localGrid = static_cast<RigLocalGrid*>(grid);
            parentGrid = localGrid->parentGrid();

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
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::scalarSetCount() const
{
    return m_mainGrid->results()->resultCount();
}

//--------------------------------------------------------------------------------------------------
/// 
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
void RigGridBase::cellCornerScalars(size_t timeStepIndex, size_t scalarSetIndex, size_t i, size_t j, size_t k, double scalars[8]) const
{
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::vectorSetCount() const
{
    return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridBase::cellScalar(size_t timeStepIndex, size_t scalarSetIndex, size_t i, size_t j, size_t k) const
{
    size_t cellIndex = cellIndexFromIJK(i, j, k);

    return cellScalar(timeStepIndex, scalarSetIndex, cellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridBase::cellScalar(size_t timeStepIndex, size_t scalarSetIndex, size_t cellIndex) const
{
    size_t resultValueIndex = cellIndex;
    
    bool useGlobalActiveIndex = m_mainGrid->results()->isUsingGlobalActiveIndex(scalarSetIndex);
    if (useGlobalActiveIndex)
    {
        resultValueIndex = cell(cellIndex).activeIndexInMatrixModel();
        if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;
    }
    
    return m_mainGrid->results()->cellScalarResult(timeStepIndex, scalarSetIndex, resultValueIndex);
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
bool RigGridBase::isCellActive(size_t i, size_t j, size_t k) const
{
    size_t idx = cellIndexFromIJK(i, j, k);
    const RigCell& c = cell(idx);
    if (!c.isActiveInMatrixModel() || c.isInvalid())
    {
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: Use structgrid::neighborIJKAtCellFace
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
const cvf::Vec3d* RigGridBase::cellVector(size_t vectorSetIndex, size_t i, size_t j, size_t k) const
{
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridBase::computeFaults()
{
  //size_t k;
#pragma omp parallel for 
    for (int k = 0; k < static_cast<int>(cellCountK()); k++)
    {
        size_t j;
        for (j = 0; j < cellCountJ(); j++)
        {
            size_t i;
            for (i = 0; i < cellCountI(); i++)
            {
                size_t idx = cellIndexFromIJK(i, j, k);

                RigCell& currentCell = cell(idx);

                if (currentCell.isInvalid())
                {
                    continue;
                }
        
                size_t faceIdx;
                for (faceIdx = 0; faceIdx < 6; faceIdx++)
                {
                    cvf::StructGridInterface::FaceType face = static_cast<cvf::StructGridInterface::FaceType>(faceIdx);

                    size_t cellNeighbourIdx = 0;
                    if (!cellIJKNeighbor(i, j, k, face, &cellNeighbourIdx))
                    {
                        continue;
                    }

                    const RigCell& neighbourCell = cell(cellNeighbourIdx);
                    if (neighbourCell.isInvalid())
                    {
                        continue;
                    }

                    cvf::Vec3d currentCellFaceVertices[4];
                    {
                        cvf::ubyte faceVertexIndices[4];
                        cellFaceVertexIndices(face, faceVertexIndices);
                        const caf::SizeTArray8& cornerIndices = currentCell.cornerIndices();
        
                        currentCellFaceVertices[0].set(m_mainGrid->nodes()[cornerIndices[faceVertexIndices[0]]]);
                        currentCellFaceVertices[1].set(m_mainGrid->nodes()[cornerIndices[faceVertexIndices[1]]]);
                        currentCellFaceVertices[2].set(m_mainGrid->nodes()[cornerIndices[faceVertexIndices[2]]]);
                        currentCellFaceVertices[3].set(m_mainGrid->nodes()[cornerIndices[faceVertexIndices[3]]]);
                    }

                    cvf::Vec3d neighbourCellFaceVertices[4];
                    {
                        cvf::ubyte faceVertexIndices[4];
                        StructGridInterface::FaceType opposite = StructGridInterface::oppositeFace(face);
                        cellFaceVertexIndices(opposite, faceVertexIndices);
                        const caf::SizeTArray8& cornerIndices = neighbourCell.cornerIndices();

                        neighbourCellFaceVertices[0].set(m_mainGrid->nodes()[cornerIndices[faceVertexIndices[0]]]);
                        neighbourCellFaceVertices[1].set(m_mainGrid->nodes()[cornerIndices[faceVertexIndices[3]]]);
                        neighbourCellFaceVertices[2].set(m_mainGrid->nodes()[cornerIndices[faceVertexIndices[2]]]);
                        neighbourCellFaceVertices[3].set(m_mainGrid->nodes()[cornerIndices[faceVertexIndices[1]]]);
                    }

                    bool sharedFaceVertices = true;

                    // Check if vertices are matching
                    double tolerance = 1e-6;
                    size_t i;
                    for (i = 0; i < 4; i++)
                    {
                        if (currentCellFaceVertices[i].pointDistance(neighbourCellFaceVertices[i]) > tolerance )
                        {
                            sharedFaceVertices = false;
                        }
                    }

                    if (!sharedFaceVertices)
                    {
                        currentCell.setCellFaceFault(face);
                    }
                }
            }
        }
    }
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
/// Returns the max size of the charactristic cell sizes
//--------------------------------------------------------------------------------------------------
double RigGridBase::characteristicCellSize()
{
    double characteristicCellSize = 0;

    double cellSizeI, cellSizeJ, cellSizeK;
    this->characteristicCellSizes(&cellSizeI, &cellSizeJ, &cellSizeK);

    if (cellSizeI > characteristicCellSize) characteristicCellSize = cellSizeI;
    if (cellSizeJ > characteristicCellSize) characteristicCellSize = cellSizeJ;
    if (cellSizeK > characteristicCellSize) characteristicCellSize = cellSizeK;

    return characteristicCellSize;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::matrixModelActiveCellCount()
{
    if (m_matrixModelActiveCellCount == cvf::UNDEFINED_SIZE_T)
    {
        computeMatrixAndFractureModelActiveCellCount();
    }

    CVF_ASSERT(m_matrixModelActiveCellCount != cvf::UNDEFINED_SIZE_T);

    return m_matrixModelActiveCellCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigGridBase::fractureModelActiveCellCount()
{
    if (m_fractureModelActiveCellCount == cvf::UNDEFINED_SIZE_T)
    {
        computeMatrixAndFractureModelActiveCellCount();
    }

    CVF_ASSERT(m_fractureModelActiveCellCount != cvf::UNDEFINED_SIZE_T);

    return m_fractureModelActiveCellCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridBase::computeMatrixAndFractureModelActiveCellCount()
{
    m_matrixModelActiveCellCount = 0;
    m_fractureModelActiveCellCount = 0;

    for (size_t i = 0; i < cellCount(); i++)
    {
        const RigCell& c = cell(i);
        if (c.isActiveInMatrixModel())
        {
            m_matrixModelActiveCellCount++;
        }

        if (c.isActiveInFractureModel())
        {
            m_fractureModelActiveCellCount++;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridCellFaceVisibilityFilter::isFaceVisible(size_t i, size_t j, size_t k, cvf::StructGridInterface::FaceType face, const cvf::UByteArray* cellVisibility) const
{
    CVF_TIGHT_ASSERT(m_grid);

    if (m_showFaultFaces)
    {
        size_t cellIndex = m_grid->cellIndexFromIJK(i, j, k);
        if (m_grid->cell(cellIndex).isCellFaceFault(face))
        {
            return true;
        }
    }

    if (m_showExternalFaces)
    {
        size_t ni, nj, nk;
        cvf::StructGridInterface::neighborIJKAtCellFace(i, j, k, face, &ni, &nj, &nk);

        if (ni >= m_grid->cellCountI() || nj >= m_grid->cellCountJ() || nk >= m_grid->cellCountK())
        {
            // Detected cell on edge of grid
            return true;
        }

        size_t neighborCellIndex = m_grid->cellIndexFromIJK(ni, nj, nk);
        const RigCell& neighborCell = m_grid->cell(neighborCellIndex);
        if (neighborCell.subGrid())
        {
            // Do not show face with a LGR neighbor. The subgrid will be responsible for displaying the face from the opposite side
            return false;
        }

        if ((cellVisibility != NULL) && !(*cellVisibility)[neighborCellIndex])
        {
            // Neighbor cell is not part of visible cells
            return true;
        }
    }

    return false;
}


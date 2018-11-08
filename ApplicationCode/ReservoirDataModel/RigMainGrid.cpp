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

#include "RigMainGrid.h"

#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RigActiveCellInfo.h"
#include "RigHexIntersectionTools.h"

#include "cvfAssert.h"
#include "cvfBoundingBoxTree.h"

RigMainGrid::RigMainGrid()
    : RigGridBase(this)
{
    m_displayModelOffset = cvf::Vec3d::ZERO;

    m_gridIndex = 0;
    m_gridId    = 0;
    m_gridIdToIndexMapping.push_back(0);

    m_flipXAxis = false;
    m_flipYAxis = false;
}

RigMainGrid::~RigMainGrid() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d>& RigMainGrid::nodes()
{
    return m_nodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigMainGrid::nodes() const
{
    return m_nodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCell>& RigMainGrid::globalCellArray()
{
    return m_cells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigCell>& RigMainGrid::globalCellArray() const
{
    return m_cells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridAndGridLocalIdxFromGlobalCellIdx(size_t globalCellIdx, size_t* gridLocalCellIdx)
{
    CVF_ASSERT(globalCellIdx < m_cells.size());

    RigCell&     cell     = m_cells[globalCellIdx];
    RigGridBase* hostGrid = cell.hostGrid();
    CVF_ASSERT(hostGrid);

    *gridLocalCellIdx = cell.gridLocalCellIndex();
    return hostGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigMainGrid::gridAndGridLocalIdxFromGlobalCellIdx(size_t globalCellIdx, size_t* gridLocalCellIdx) const
{
    CVF_ASSERT(globalCellIdx < m_cells.size());

    const RigCell&     cell     = m_cells[globalCellIdx];
    const RigGridBase* hostGrid = cell.hostGrid();
    CVF_ASSERT(hostGrid);

    *gridLocalCellIdx = cell.gridLocalCellIndex();
    return hostGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCell& RigMainGrid::cellByGridAndGridLocalCellIdx(size_t gridIdx, size_t gridLocalCellIdx) const
{
    return gridByIndex(gridIdx)->cell(gridLocalCellIdx);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::reservoirCellIndexByGridAndGridLocalCellIndex(size_t gridIdx, size_t gridLocalCellIdx) const
{
    return gridByIndex(gridIdx)->reservoirCellIndex(gridLocalCellIdx);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::findReservoirCellIndexFromPoint(const cvf::Vec3d& point) const
{
    size_t cellContainingPoint = cvf::UNDEFINED_SIZE_T;

    cvf::BoundingBox pointBBox;
    pointBBox.add(point);

    std::vector<size_t> cellIndices;
    m_mainGrid->findIntersectingCells(pointBBox, &cellIndices);

    cvf::Vec3d hexCorners[8];
    for (size_t cellIndex : cellIndices)
    {
        m_mainGrid->cellCornerVertices(cellIndex, hexCorners);

        if (RigHexIntersectionTools::isPointInCell(point, hexCorners))
        {
            cellContainingPoint = cellIndex;
            break;
        }
    }

    return cellContainingPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigMainGrid::findAllReservoirCellIndicesMatching2dPoint(const cvf::Vec2d& point2d) const
{
    cvf::BoundingBox gridBoundingVox = boundingBox();
    cvf::Vec3d       highestPoint(point2d, gridBoundingVox.max().z());
    cvf::Vec3d       lowestPoint(point2d, gridBoundingVox.min().z());

    cvf::BoundingBox rayBBox;
    rayBBox.add(highestPoint);
    rayBBox.add(lowestPoint);

    std::vector<size_t> cellIndices;
    m_mainGrid->findIntersectingCells(rayBBox, &cellIndices);

    cvf::Vec3d hexCorners[8];
    for (size_t cellIndex : cellIndices)
    {
        m_mainGrid->cellCornerVertices(cellIndex, hexCorners);

        if (RigHexIntersectionTools::lineIntersectsHexCell(highestPoint, lowestPoint, hexCorners))
        {
            cellIndices.push_back(cellIndex);
        }
    }

    return cellIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::addLocalGrid(RigLocalGrid* localGrid)
{
    CVF_ASSERT(localGrid && localGrid->gridId() != cvf::UNDEFINED_INT); // The grid ID must be set.
    CVF_ASSERT(localGrid->gridId() >= 0); // We cant handle negative ID's if they exist.

    m_localGrids.push_back(localGrid);
    localGrid->setGridIndex(m_localGrids.size()); // Maingrid itself has grid index 0

    if (m_gridIdToIndexMapping.size() <= static_cast<size_t>(localGrid->gridId()))
    {
        m_gridIdToIndexMapping.resize(localGrid->gridId() + 1, cvf::UNDEFINED_SIZE_T);
    }

    m_gridIdToIndexMapping[localGrid->gridId()] = localGrid->gridIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::gridCountOnFile() const
{
    size_t gridCount = 1;

    for (const auto& grid : m_localGrids)
    {
        if (!grid->isTempGrid())
        {
            gridCount++;
        }
    }

    return gridCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::gridCount() const
{
    return m_localGrids.size() + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::initAllSubGridsParentGridPointer()
{
    if (m_localGrids.size() && m_localGrids[0]->parentGrid() == nullptr)
    {
        initSubGridParentPointer();
        size_t i;
        for (i = 0; i < m_localGrids.size(); ++i)
        {
            m_localGrids[i]->initSubGridParentPointer();
        }
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

    m_cellSearchTree = nullptr;
    buildCellSearchTree();
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with index \a localGridIndex. Main Grid itself has index 0. First LGR starts on 1
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridByIndex(size_t localGridIndex)
{
    if (localGridIndex == 0) return this;
    CVF_ASSERT(localGridIndex - 1 < m_localGrids.size());
    return m_localGrids[localGridIndex - 1].p();
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with index \a localGridIndex. Main Grid itself has index 0. First LGR starts on 1
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigMainGrid::gridByIndex(size_t localGridIndex) const
{
    if (localGridIndex == 0) return this;
    CVF_ASSERT(localGridIndex - 1 < m_localGrids.size());
    return m_localGrids[localGridIndex - 1].p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setFlipAxis(bool flipXAxis, bool flipYAxis)
{
    bool needFlipX = false;
    bool needFlipY = false;

    if (m_flipXAxis != flipXAxis)
    {
        needFlipX = true;
    }

    if (m_flipYAxis != flipYAxis)
    {
        needFlipY = true;
    }

    if (needFlipX || needFlipY)
    {
        for (size_t i = 0; i < m_nodes.size(); i++)
        {
            if (needFlipX)
            {
                m_nodes[i].x() *= -1.0;
            }

            if (needFlipY)
            {
                m_nodes[i].y() *= -1.0;
            }
        }

        m_flipXAxis = flipXAxis;
        m_flipYAxis = flipYAxis;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridById(int localGridId)
{
    CVF_ASSERT(localGridId >= 0 && static_cast<size_t>(localGridId) < m_gridIdToIndexMapping.size());
    return this->gridByIndex(m_gridIdToIndexMapping[localGridId]);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::totalTemporaryGridCellCount() const
{
    size_t cellCount = 0;

    for (const auto& grid : m_localGrids)
    {
        if (grid->isTempGrid())
        {
            cellCount += grid->cellCount();
        }
    }

    return cellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNNCData* RigMainGrid::nncData()
{
    if (m_nncData.isNull())
    {
        m_nncData = new RigNNCData;
    }

    return m_nncData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setFaults(const cvf::Collection<RigFault>& faults)
{
    m_faults = faults;

#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(m_faults.size()); i++)
    {
        m_faults[i]->computeFaultFacesFromCellRanges(this->mainGrid());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Collection<RigFault>& RigMainGrid::faults()
{
    return m_faults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMainGrid::hasFaultWithName(const QString& name) const
{
    for (auto fault : m_faults)
    {
        if (fault->name() == name)
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::calculateFaults(const RigActiveCellInfo* activeCellInfo)
{
    if (hasFaultWithName(RiaDefines::undefinedGridFaultName()) &&
        hasFaultWithName(RiaDefines::undefinedGridFaultWithInactiveName()))
    {
        // RiaLogging::debug(QString("Calculate faults already run for grid."));

        return;
    }

    m_faultsPrCellAcc = new RigFaultsPrCellAccumulator(m_cells.size());

    // Spread fault idx'es on the cells from the faults
    for (size_t fIdx = 0; fIdx < m_faults.size(); ++fIdx)
    {
        m_faults[fIdx]->accumulateFaultsPrCell(m_faultsPrCellAcc.p(), static_cast<int>(fIdx));
    }

    // Find the geometrical faults that is in addition: Has no user defined (eclipse) fault assigned.
    // Separate the grid faults that has an inactive cell as member

    RigFault* unNamedFault = new RigFault;
    unNamedFault->setName(RiaDefines::undefinedGridFaultName());
    int unNamedFaultIdx = static_cast<int>(m_faults.size());
    m_faults.push_back(unNamedFault);

    RigFault* unNamedFaultWithInactive = new RigFault;
    unNamedFaultWithInactive->setName(RiaDefines::undefinedGridFaultWithInactiveName());
    int unNamedFaultWithInactiveIdx = static_cast<int>(m_faults.size());
    m_faults.push_back(unNamedFaultWithInactive);

    const std::vector<cvf::Vec3d>& vxs = m_mainGrid->nodes();

    for (int gcIdx = 0; gcIdx < static_cast<int>(m_cells.size()); ++gcIdx)
    {
        if (m_cells[gcIdx].isInvalid())
        {
            continue;
        }

        size_t neighborReservoirCellIdx;
        size_t neighborGridCellIdx;
        size_t i = 0;
        size_t j = 0;
        size_t k = 0;

        RigGridBase* hostGrid                 = nullptr;
        bool         firstNO_FAULTFaceForCell = true;
        bool         isCellActive             = true;

        char upperLimitForFaceType = cvf::StructGridInterface::FaceType::POS_K;

        // Compare only I and J faces
        for (char faceIdx = 0; faceIdx < upperLimitForFaceType; ++faceIdx)
        {
            cvf::StructGridInterface::FaceType face = cvf::StructGridInterface::FaceType(faceIdx);

            // For faces that has no used defined Fault assigned:

            if (m_faultsPrCellAcc->faultIdx(gcIdx, face) == RigFaultsPrCellAccumulator::NO_FAULT)
            {
                // Find neighbor cell
                if (firstNO_FAULTFaceForCell) // To avoid doing this for every face, and only when detecting a NO_FAULT
                {
                    size_t gridLocalCellIndex;
                    hostGrid = this->gridAndGridLocalIdxFromGlobalCellIdx(gcIdx, &gridLocalCellIndex);

                    hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);
                    isCellActive = activeCellInfo->isActive(gcIdx);

                    firstNO_FAULTFaceForCell = false;
                }

                if (!hostGrid->cellIJKNeighbor(i, j, k, face, &neighborGridCellIdx))
                {
                    continue;
                }

                neighborReservoirCellIdx = hostGrid->reservoirCellIndex(neighborGridCellIdx);
                if (m_cells[neighborReservoirCellIdx].isInvalid())
                {
                    continue;
                }

                bool isNeighborCellActive = activeCellInfo->isActive(neighborReservoirCellIdx);

                double tolerance = 1e-6;

                std::array<size_t, 4> faceIdxs;
                m_cells[gcIdx].faceIndices(face, &faceIdxs);
                std::array<size_t, 4> nbFaceIdxs;
                m_cells[neighborReservoirCellIdx].faceIndices(StructGridInterface::oppositeFace(face), &nbFaceIdxs);

                bool sharedFaceVertices = true;
                if (sharedFaceVertices && vxs[faceIdxs[0]].pointDistance(vxs[nbFaceIdxs[0]]) > tolerance)
                    sharedFaceVertices = false;
                if (sharedFaceVertices && vxs[faceIdxs[1]].pointDistance(vxs[nbFaceIdxs[3]]) > tolerance)
                    sharedFaceVertices = false;
                if (sharedFaceVertices && vxs[faceIdxs[2]].pointDistance(vxs[nbFaceIdxs[2]]) > tolerance)
                    sharedFaceVertices = false;
                if (sharedFaceVertices && vxs[faceIdxs[3]].pointDistance(vxs[nbFaceIdxs[1]]) > tolerance)
                    sharedFaceVertices = false;

                if (sharedFaceVertices)
                {
                    continue;
                }

                // To avoid doing this calculation for the opposite face
                int faultIdx = unNamedFaultIdx;
                if (!(isCellActive && isNeighborCellActive)) faultIdx = unNamedFaultWithInactiveIdx;

                m_faultsPrCellAcc->setFaultIdx(gcIdx, face, faultIdx);
                m_faultsPrCellAcc->setFaultIdx(neighborReservoirCellIdx, StructGridInterface::oppositeFace(face), faultIdx);

                // Add as fault face only if the grid index is less than the neighbors

                if (static_cast<size_t>(gcIdx) < neighborReservoirCellIdx)
                {
                    RigFault::FaultFace ff(gcIdx, cvf::StructGridInterface::FaceType(faceIdx), neighborReservoirCellIdx);
                    if (isCellActive && isNeighborCellActive)
                    {
                        unNamedFault->faultFaces().push_back(ff);
                    }
                    else
                    {
                        unNamedFaultWithInactive->faultFaces().push_back(ff);
                    }
                }
                else
                {
                    CVF_FAIL_MSG(
                        "Found fault with global neighbor index less than the native index. "); // Should never occur. because we
                                                                                                // flag the opposite face in the
                                                                                                // faultsPrCellAcc
                }
            }
        }
    }

    distributeNNCsToFaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::distributeNNCsToFaults()
{
    const std::vector<RigConnection>& nncs = this->nncData()->connections();
    for (size_t nncIdx = 0; nncIdx < nncs.size(); ++nncIdx)
    {
        // Find the fault for each side of the nnc
        const RigConnection& conn  = nncs[nncIdx];
        int                  fIdx1 = RigFaultsPrCellAccumulator::NO_FAULT;
        int                  fIdx2 = RigFaultsPrCellAccumulator::NO_FAULT;

        if (conn.m_c1Face != StructGridInterface::NO_FACE)
        {
            fIdx1 = m_faultsPrCellAcc->faultIdx(conn.m_c1GlobIdx, conn.m_c1Face);
            fIdx2 = m_faultsPrCellAcc->faultIdx(conn.m_c2GlobIdx, StructGridInterface::oppositeFace(conn.m_c1Face));
        }

        if (fIdx1 < 0 && fIdx2 < 0)
        {
            cvf::String lgrString("Same Grid");
            if (m_cells[conn.m_c1GlobIdx].hostGrid() != m_cells[conn.m_c2GlobIdx].hostGrid())
            {
                lgrString = "Different Grid";
            }

            // cvf::Trace::show("NNC: No Fault for NNC C1: " + cvf::String((int)conn.m_c1GlobIdx) + " C2: " +
            // cvf::String((int)conn.m_c2GlobIdx) + " Grid: " + lgrString);
        }

        if (fIdx1 >= 0)
        {
            // Add the connection to both, if they are different.
            m_faults[fIdx1]->connectionIndices().push_back(nncIdx);
        }

        if (fIdx2 != fIdx1)
        {
            if (fIdx2 >= 0)
            {
                m_faults[fIdx2]->connectionIndices().push_back(nncIdx);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// The cell is normally inverted due to Depth becoming -Z at import,
/// but if (only) one of the flipX/Y is done, the cell is back to normal
//--------------------------------------------------------------------------------------------------
bool RigMainGrid::isFaceNormalsOutwards() const
{
    for (int gcIdx = 0; gcIdx < static_cast<int>(m_cells.size()); ++gcIdx)
    {
        if (!m_cells[gcIdx].isInvalid())
        {
            cvf::Vec3d cellCenter = m_cells[gcIdx].center();
            cvf::Vec3d faceCenter = m_cells[gcIdx].faceCenter(StructGridInterface::POS_I);
            cvf::Vec3d faceNormal = m_cells[gcIdx].faceNormalWithAreaLenght(StructGridInterface::POS_I);

            double typicalIJCellSize = characteristicIJCellSize();
            double dummy, dummy2, typicalKSize;
            characteristicCellSizes(&dummy, &dummy2, &typicalKSize);

            if ((faceCenter - cellCenter).length() > 0.2 * typicalIJCellSize &&
                (faceNormal.length() > (0.2 * typicalIJCellSize * 0.2 * typicalKSize)))
            {
                // Cell is assumed ok to use, so calculate whether the normals are outwards or inwards

                if ((faceCenter - cellCenter) * faceNormal >= 0)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFault* RigMainGrid::findFaultFromCellIndexAndCellFace(size_t                             reservoirCellIndex,
                                                               cvf::StructGridInterface::FaceType face) const
{
    CVF_TIGHT_ASSERT(m_faultsPrCellAcc.notNull());

    if (face == cvf::StructGridInterface::NO_FACE) return nullptr;

    int faultIdx = m_faultsPrCellAcc->faultIdx(reservoirCellIndex, face);
    if (faultIdx != RigFaultsPrCellAccumulator::NO_FAULT)
    {
        return m_faults.at(faultIdx);
    }

#if 0
    for (size_t i = 0; i < m_faults.size(); i++)
    {
        const RigFault* rigFault = m_faults.at(i);
        const std::vector<RigFault::FaultFace>& faultFaces = rigFault->faultFaces();

        for (size_t fIdx = 0; fIdx < faultFaces.size(); fIdx++)
        {
            if (faultFaces[fIdx].m_nativeReservoirCellIndex == cellIndex)
            {
                if (face == faultFaces[fIdx].m_nativeFace )
                {
                    return rigFault;
                }
            }

            if (faultFaces[fIdx].m_oppositeReservoirCellIndex == cellIndex)
            {
                if (face == cvf::StructGridInterface::oppositeFace(faultFaces[fIdx].m_nativeFace))
                {
                    return rigFault;
                }
            }
        }
    }
#endif
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::findIntersectingCells(const cvf::BoundingBox& inputBB, std::vector<size_t>* cellIndices) const
{
    CVF_ASSERT(m_cellSearchTree.notNull());

    m_cellSearchTree->findIntersections(inputBB, cellIndices);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::buildCellSearchTree()
{
    if (m_cellSearchTree.isNull())
    {
        // build tree

        size_t cellCount = m_cells.size();

        std::vector<cvf::BoundingBox> cellBoundingBoxes;
        cellBoundingBoxes.resize(cellCount);

        for (size_t cIdx = 0; cIdx < cellCount; ++cIdx)
        {
            if (m_cells[cIdx].isInvalid()) continue;

            const std::array<size_t, 8>& cellIndices = m_cells[cIdx].cornerIndices();

            cvf::BoundingBox& cellBB = cellBoundingBoxes[cIdx];
            cellBB.add(m_nodes[cellIndices[0]]);
            cellBB.add(m_nodes[cellIndices[1]]);
            cellBB.add(m_nodes[cellIndices[2]]);
            cellBB.add(m_nodes[cellIndices[3]]);
            cellBB.add(m_nodes[cellIndices[4]]);
            cellBB.add(m_nodes[cellIndices[5]]);
            cellBB.add(m_nodes[cellIndices[6]]);
            cellBB.add(m_nodes[cellIndices[7]]);
        }

        m_cellSearchTree = new cvf::BoundingBoxTree;
        m_cellSearchTree->buildTreeFromBoundingBoxes(cellBoundingBoxes, nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigMainGrid::boundingBox() const
{
    if (m_boundingBox.isValid()) return m_boundingBox;

    for (const auto& node : m_nodes)
    {
        m_boundingBox.add(node);
    }

    return m_boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMainGrid::isTempGrid() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RigMainGrid::associatedWellPathName() const
{
    static const std::string EMPTY_STRING;
    return EMPTY_STRING;
}

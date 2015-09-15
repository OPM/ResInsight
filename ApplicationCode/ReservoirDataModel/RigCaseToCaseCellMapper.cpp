/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigCaseToCaseCellMapper.h"
#include "RigFemPart.h"
#include "RigMainGrid.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigFemPart* dependentFemPart)
    : m_masterGrid(masterEclGrid),
      m_dependentGrid(NULL),
      m_masterFemPart(dependentFemPart),
      m_dependentFemPart(NULL)
{
    m_masterCellOrIntervalIndex.resize(dependentFemPart->elementCount(), cvf::UNDEFINED_INT);
    
    #if 0
    // First search K=1 diagonally for a seed cell; A cell without collapsings, and without faults


    size_t minIJCount = masterEclGrid->cellCountI();
    if (minIJCount > masterEclGrid->cellCountJ())
        minIJCount =  masterEclGrid->cellCountJ();

    for (size_t ij = 0; ij < minIJCount; ++ij )
    {
        size_t localCellIdx = masterEclGrid->cellIndexFromIJK(ij, ij, 0);
        size_t reservoirCellIdx = masterEclGrid->reservoirCellIndex(localCellIdx);

        cvf::Vec3d vertices[8];
        masterEclGrid->cellCornerVertices(localCellIdx, vertices);
        if (!isCellNormal(vertices))
            continue;
        
        const RigFault* fault = masterEclGrid->findFaultFromCellIndexAndCellFace(reservoirCellIdx, cvf::StructGridInterface::POS_I);

    }
    #endif

    // Brute force:
    const std::vector<cvf::Vec3f>& nodeCoords =  dependentFemPart->nodes().coordinates;

    double cellSizeI, cellSizeJ, cellSizeK;
    masterEclGrid->characteristicCellSizes(&cellSizeI, &cellSizeJ, &cellSizeK);
    
    double xyTolerance = cellSizeI* 0;
    double zTolerance = cellSizeK* 0;

    int elementCount = dependentFemPart->elementCount();
    cvf::Vec3d elmCorners[8];
    for (int elmIdx = 0; elmIdx < elementCount; ++elmIdx)
    {
        if (dependentFemPart->elementType(elmIdx) != HEX8) continue;

        const int* cornerIndices = dependentFemPart->connectivities(elmIdx);

        elmCorners[0] = cvf::Vec3d(nodeCoords[cornerIndices[0]]);
        elmCorners[1] = cvf::Vec3d(nodeCoords[cornerIndices[1]]);
        elmCorners[2] = cvf::Vec3d(nodeCoords[cornerIndices[2]]);
        elmCorners[3] = cvf::Vec3d(nodeCoords[cornerIndices[3]]);
        elmCorners[4] = cvf::Vec3d(nodeCoords[cornerIndices[4]]);
        elmCorners[5] = cvf::Vec3d(nodeCoords[cornerIndices[5]]);
        elmCorners[6] = cvf::Vec3d(nodeCoords[cornerIndices[6]]);
        elmCorners[7] = cvf::Vec3d(nodeCoords[cornerIndices[7]]);

        cvf::BoundingBox elmBBox;
        for (int i = 0; i < 8 ; ++i) elmBBox.add(elmCorners[i]);

        std::vector<size_t> closeCells;
        masterEclGrid->findIntersectingCells(elmBBox, &closeCells);
        std::vector<int> matchingCells;

        for (size_t ccIdx = 0; ccIdx < closeCells.size(); ++ccIdx)
        {
            cvf::Vec3d cellCorners[8];
            size_t localCellIdx = masterEclGrid->cells()[closeCells[ccIdx]].gridLocalCellIndex();
            masterEclGrid->cellCornerVertices(localCellIdx, cellCorners);

            bool isMatching = false;

            #if 1 // Inside Bounding box test
            cvf::BoundingBox cellBBox;
            for (int i = 0; i < 8 ; ++i) cellBBox.add(elmCorners[i]);

            cvf::Vec3d cs = cellBBox.min();
            cvf::Vec3d cl = cellBBox.max();
            cvf::Vec3d es = elmBBox.min();
            cvf::Vec3d el = elmBBox.max();

            if (   ( (cs.x() + xyTolerance) >= es.x() && (cl.x() - xyTolerance) <= el.x())
                && ( (cs.y() + xyTolerance) >= es.y() && (cl.y() - xyTolerance) <= el.y())
                && ( (cs.z() + zTolerance ) >= es.z() && (cl.z() - zTolerance ) <= el.z()) )
            {
                // Cell bb equal or inside Elm bb   
                isMatching = true;
            }

            if (   ( (es.x() + xyTolerance)  >= cs.x() && (el.x() - xyTolerance) <= cl.x())
                && ( (es.y() + xyTolerance)  >= cs.y() && (el.y() - xyTolerance) <= cl.y())
                && ( (es.z() + zTolerance )  >= cs.z() && (el.z() - zTolerance ) <= cl.z()) )
            {
                // Elm bb equal or inside Cell bb   
                isMatching = true;
            }
            #endif

            if (isMatching)
            {
                matchingCells.push_back(static_cast<int>(closeCells[ccIdx]));
            }
            else
            {
                // Try fault corrections on the eclipse cell

                // Try zero volume correction
            }
        }

        if (matchingCells.size() == 1)
        {
            m_masterCellOrIntervalIndex[elmIdx] = matchingCells[0];
        }
        else if (matchingCells.size() > 1)
        {
            m_masterCellOrIntervalIndex[elmIdx] = -((int)(m_masterCellIndexSeries.size()));
            m_masterCellIndexSeries.push_back(matchingCells);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigMainGrid* dependentEclGrid)
    : m_masterGrid(masterEclGrid),
      m_dependentGrid(dependentEclGrid),
      m_masterFemPart(NULL),
      m_dependentFemPart(NULL)
{
    m_masterCellOrIntervalIndex.resize(dependentEclGrid->cells().size(), cvf::UNDEFINED_INT);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigFemPart* masterFemPart, RigMainGrid* dependentEclGrid)
    : m_masterGrid(NULL),
      m_dependentGrid(dependentEclGrid),
      m_masterFemPart(masterFemPart),
      m_dependentFemPart(NULL)
{
    m_masterCellOrIntervalIndex.resize(dependentEclGrid->cells().size(), cvf::UNDEFINED_INT);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigFemPart* masterFemPart, RigFemPart* dependentFemPart)
    : m_masterGrid(NULL),
      m_dependentGrid(NULL),
      m_masterFemPart(masterFemPart),
      m_dependentFemPart(dependentFemPart)
{
    m_masterCellOrIntervalIndex.resize(dependentFemPart->elementCount(), cvf::UNDEFINED_INT);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const int * RigCaseToCaseCellMapper::masterCaseCellIndices(int dependentCaseReservoirCellIndex, int* masterCaseCellIndexCount) const
{
    int seriesIndex = m_masterCellOrIntervalIndex[dependentCaseReservoirCellIndex];

    if (seriesIndex == cvf::UNDEFINED_INT)
    {
        (*masterCaseCellIndexCount) = 0;
        return NULL;
    }

    if (seriesIndex < 0)
    {
        (*masterCaseCellIndexCount) = static_cast<int>(m_masterCellIndexSeries[-seriesIndex].size());
        return &(m_masterCellIndexSeries[-seriesIndex][0]);    
    }
    else
    {
        (*masterCaseCellIndexCount) = 1;
        return &(m_masterCellOrIntervalIndex[dependentCaseReservoirCellIndex]);
    }
}

#if 0
enum RigHexIntersectResult
{
    MATCH, 
    UNRELATED
};

RigHexIntersectResult matchCells(const cvf::Vec3d hex1[8], const cvf::Vec3d hex2[8], double tolerance)
{
        

}
#endif
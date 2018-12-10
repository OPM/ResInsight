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
#include "RigCaseToCaseCellMapperTools.h"

#include "RigFemPart.h"
#include "RigMainGrid.h"
#include "RigFemPartGrid.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigMainGrid* dependentEclGrid)
    : m_masterGrid(masterEclGrid),
      m_dependentGrid(dependentEclGrid),
      m_masterFemPart(nullptr),
      m_dependentFemPart(nullptr)
{
    m_masterCellOrIntervalIndex.resize(dependentEclGrid->globalCellArray().size(), cvf::UNDEFINED_INT);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigFemPart* masterFemPart, RigMainGrid* dependentEclGrid)
    : m_masterGrid(nullptr),
      m_dependentGrid(dependentEclGrid),
      m_masterFemPart(masterFemPart),
      m_dependentFemPart(nullptr)
{
    m_masterCellOrIntervalIndex.resize(dependentEclGrid->globalCellArray().size(), cvf::UNDEFINED_INT);
    this->calculateEclToGeomCellMapping(dependentEclGrid, masterFemPart, false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigFemPart* masterFemPart, RigFemPart* dependentFemPart)
    : m_masterGrid(nullptr),
      m_dependentGrid(nullptr),
      m_masterFemPart(masterFemPart),
      m_dependentFemPart(dependentFemPart)
{
    m_masterCellOrIntervalIndex.resize(dependentFemPart->elementCount(), cvf::UNDEFINED_INT);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseToCaseCellMapper::RigCaseToCaseCellMapper(RigMainGrid* masterEclGrid, RigFemPart* dependentFemPart)
    : m_masterGrid(masterEclGrid),
      m_dependentGrid(nullptr),
      m_masterFemPart(dependentFemPart),
      m_dependentFemPart(nullptr)
{
    m_masterCellOrIntervalIndex.resize(dependentFemPart->elementCount(), cvf::UNDEFINED_INT);
    this->calculateEclToGeomCellMapping(masterEclGrid, dependentFemPart, true);
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
        return nullptr;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseCellMapper::addMapping(int depCaseCellIdx, int masterCaseMatchingCell)
{
    int mcOrSeriesIdx = m_masterCellOrIntervalIndex[depCaseCellIdx];
    if (mcOrSeriesIdx == cvf::UNDEFINED_INT)
    {
        m_masterCellOrIntervalIndex[depCaseCellIdx] = masterCaseMatchingCell;
    }
    else if (mcOrSeriesIdx >= 0)
    {
        int newSeriesIdx = static_cast<int>(m_masterCellIndexSeries.size());
        m_masterCellIndexSeries.push_back(std::vector<int>());
        m_masterCellIndexSeries.back().push_back(mcOrSeriesIdx);
        m_masterCellIndexSeries.back().push_back(masterCaseMatchingCell);
        m_masterCellOrIntervalIndex[depCaseCellIdx] = -newSeriesIdx;
    }
    else
    {
        m_masterCellIndexSeries[-mcOrSeriesIdx].push_back(masterCaseMatchingCell);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseToCaseCellMapper::calculateEclToGeomCellMapping(RigMainGrid* masterEclGrid, RigFemPart* dependentFemPart, bool eclipseIsMaster)
{
    // Find tolerance

    double cellSizeI, cellSizeJ, cellSizeK;
    masterEclGrid->characteristicCellSizes(&cellSizeI, &cellSizeJ, &cellSizeK);
    
    double xyTolerance  = cellSizeI* 0.4;
    double zTolerance   = cellSizeK* 0.4;

    bool isEclFaceNormalsOutwards = masterEclGrid->isFaceNormalsOutwards();

    cvf::Vec3d elmCorners[8];

    size_t cellCount =  masterEclGrid->cellCount();

    for (size_t cellIdx = 0; cellIdx < cellCount; ++cellIdx)
    {
        #ifdef _DEBUG
        { 
            // For debugging 
            size_t i, j, k;
            masterEclGrid->ijkFromCellIndex(cellIdx, &i, &j, &k); // Will not work when LGR present
        }
        #endif  

        cvf::Vec3d geoMechConvertedEclCell[8];
        RigCaseToCaseCellMapperTools::estimatedFemCellFromEclCell(masterEclGrid, cellIdx, geoMechConvertedEclCell);

        cvf::BoundingBox elmBBox;
        for (int i = 0; i < 8 ; ++i) elmBBox.add(geoMechConvertedEclCell[i]);

        std::vector<size_t> closeElements;
        dependentFemPart->findIntersectingCells(elmBBox, &closeElements);

        for (size_t ccIdx = 0; ccIdx < closeElements.size(); ++ccIdx)
        {
            int elmIdx = static_cast<int>(closeElements[ccIdx]);

            RigCaseToCaseCellMapperTools::elementCorners(dependentFemPart, elmIdx, elmCorners);

            RigCaseToCaseCellMapperTools::rotateCellTopologicallyToMatchBaseCell(geoMechConvertedEclCell, isEclFaceNormalsOutwards , elmCorners);

            bool isMatching = RigCaseToCaseCellMapperTools::isEclFemCellsMatching(geoMechConvertedEclCell, elmCorners, 
                                                    xyTolerance, zTolerance);

            if (isMatching)
            {
                if (eclipseIsMaster)
                    addMapping(elmIdx, static_cast<int>(cellIdx));
                else
                    addMapping(static_cast<int>(cellIdx), elmIdx);

                break;
            }
        }
    }
}

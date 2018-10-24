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

#include "RigActiveCellInfo.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo::RigActiveCellInfo()
    : m_reservoirActiveCellCount(0)
    , m_reservoirCellResultCount(0)
    , m_activeCellPositionMin(cvf::Vec3d::ZERO)
    , m_activeCellPositionMax(cvf::Vec3d::ZERO)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setReservoirCellCount(size_t reservoirCellCount)
{
    m_cellIndexToResultIndex.resize(reservoirCellCount, cvf::UNDEFINED_SIZE_T);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::reservoirCellCount() const
{
    return m_cellIndexToResultIndex.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::reservoirCellResultCount() const
{
    return m_reservoirCellResultCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigActiveCellInfo::isActive(size_t reservoirCellIndex) const
{
    if (m_cellIndexToResultIndex.size() == 0)
    {
        return true;
    }

    CVF_TIGHT_ASSERT(reservoirCellIndex < m_cellIndexToResultIndex.size());

    return m_cellIndexToResultIndex[reservoirCellIndex] != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::cellResultIndex(size_t reservoirCellIndex) const
{
    if (m_cellIndexToResultIndex.size() == 0)
    {
        return reservoirCellIndex;
    }

    CVF_TIGHT_ASSERT(reservoirCellIndex < m_cellIndexToResultIndex.size());

    return m_cellIndexToResultIndex[reservoirCellIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setCellResultIndex(size_t reservoirCellIndex, size_t reservoirCellResultIndex)
{
    CVF_TIGHT_ASSERT(reservoirCellResultIndex < m_cellIndexToResultIndex.size());

    m_cellIndexToResultIndex[reservoirCellIndex] = reservoirCellResultIndex;

    if (reservoirCellResultIndex >= m_reservoirCellResultCount)
    {
        m_reservoirCellResultCount = reservoirCellResultIndex + 1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGridCount(size_t gridCount)
{
    m_perGridActiveCellInfo.resize(gridCount);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGridActiveCellCounts(size_t gridIndex, size_t activeCellCount)
{
    CVF_ASSERT(gridIndex < m_perGridActiveCellInfo.size());

    m_perGridActiveCellInfo[gridIndex].setActiveCellCount(activeCellCount);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::computeDerivedData()
{
    m_reservoirActiveCellCount = 0;

    for (size_t i = 0; i < m_perGridActiveCellInfo.size(); i++)
    {
        m_reservoirActiveCellCount += m_perGridActiveCellInfo[i].activeCellCount();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::reservoirActiveCellCount() const
{
    return m_reservoirActiveCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setIJKBoundingBox(const cvf::Vec3st& min, const cvf::Vec3st& max)
{
    m_activeCellPositionMin = min;
    m_activeCellPositionMax = max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::IJKBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const
{
    min = m_activeCellPositionMin;
    max = m_activeCellPositionMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::gridActiveCellCounts(size_t gridIndex, size_t& activeCellCount) const
{
    activeCellCount = m_perGridActiveCellInfo[gridIndex].activeCellCount();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigActiveCellInfo::geometryBoundingBox() const
{
    return m_activeCellsBoundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGeometryBoundingBox(cvf::BoundingBox bb)
{
    m_activeCellsBoundingBox = bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::clear()
{
    m_perGridActiveCellInfo.clear();
    m_cellIndexToResultIndex.clear();
    m_reservoirActiveCellCount = 0;
    m_activeCellPositionMin    = cvf::Vec3st(0, 0, 0);
    m_activeCellPositionMax    = cvf::Vec3st(0, 0, 0);
    m_activeCellsBoundingBox.reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::addLgr(size_t cellCount)
{
    size_t currentGridCount          = m_perGridActiveCellInfo.size();
    size_t currentActiveCellCount    = reservoirActiveCellCount();
    size_t currentReservoirCellCount = reservoirCellCount();

    setGridCount(currentGridCount + 1);
    setGridActiveCellCounts(currentGridCount, cellCount);
    setReservoirCellCount(currentReservoirCellCount + cellCount);

    computeDerivedData();

    for (size_t i = 0; i < cellCount; i++)
    {
        setCellResultIndex(currentReservoirCellCount + i, currentActiveCellCount + i);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigActiveCellInfo::isCoarseningActive() const
{
    return m_reservoirCellResultCount != m_reservoirActiveCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo::GridActiveCellCounts::GridActiveCellCounts()
    : m_activeCellCount(0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::GridActiveCellCounts::activeCellCount() const
{
    return m_activeCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::GridActiveCellCounts::setActiveCellCount(size_t activeCellCount)
{
    m_activeCellCount = activeCellCount;
}

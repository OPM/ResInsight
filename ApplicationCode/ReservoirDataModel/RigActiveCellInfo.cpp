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

#include "RigActiveCellInfo.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo::RigActiveCellInfo()
    :   m_globalActiveCellCount(0),
        m_activeCellPositionMin(cvf::Vec3d::ZERO),
        m_activeCellPositionMax(cvf::Vec3d::ZERO)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGlobalCellCount(size_t globalCellCount)
{
    m_cellIndexToResultIndex.resize(globalCellCount, cvf::UNDEFINED_SIZE_T);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::globalCellCount() const
{
    m_cellIndexToResultIndex.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigActiveCellInfo::isActive(size_t globalCellIndex) const
{
    if (m_cellIndexToResultIndex.size() == 0)
    {
        return true;
    }

    CVF_TIGHT_ASSERT(globalCellIndex < m_cellIndexToResultIndex.size());

    return m_cellIndexToResultIndex[globalCellIndex] != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::cellResultIndex(size_t globalCellIndex) const
{
    if (m_cellIndexToResultIndex.size() == 0)
    {
        return globalCellIndex;
    }

    CVF_TIGHT_ASSERT(globalCellIndex < m_cellIndexToResultIndex.size());

    return m_cellIndexToResultIndex[globalCellIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setCellResultIndex(size_t globalCellIndex, size_t globalActiveCellIndex)
{
    CVF_TIGHT_ASSERT(globalActiveCellIndex < m_cellIndexToResultIndex.size());

    m_cellIndexToResultIndex[globalCellIndex] = globalActiveCellIndex;
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
    m_globalActiveCellCount = 0;

    for (size_t i = 0; i < m_perGridActiveCellInfo.size(); i++)
    {
        m_globalActiveCellCount += m_perGridActiveCellInfo[i].activeCellCount();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::globalActiveCellCount() const
{
    return m_globalActiveCellCount;
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
void RigActiveCellInfo::gridActiveCellCounts(size_t gridIndex, size_t& activeCellCount)
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
    m_globalActiveCellCount = 0;
    m_activeCellPositionMin = cvf::Vec3st(0,0,0);
    m_activeCellPositionMax = cvf::Vec3st(0,0,0);
    m_activeCellsBoundingBox.reset();
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

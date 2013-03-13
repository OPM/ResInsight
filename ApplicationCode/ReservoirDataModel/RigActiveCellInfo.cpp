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
    :   m_globalMatrixModelActiveCellCount(0),
       // m_globalFractureModelActiveCellCount(0),
        m_activeCellPositionMin(cvf::Vec3d::ZERO),
        m_activeCellPositionMax(cvf::Vec3d::ZERO)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGlobalCellCount(size_t globalCellCount)
{
    m_activeInMatrixModel.resize(globalCellCount, cvf::UNDEFINED_SIZE_T);
   // m_activeInFractureModel.resize(globalCellCount, cvf::UNDEFINED_SIZE_T);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigActiveCellInfo::isActiveInMatrixModel(size_t globalCellIndex) const
{
    if (m_activeInMatrixModel.size() == 0)
    {
        return false;
    }

    CVF_TIGHT_ASSERT(globalCellIndex < m_activeInMatrixModel.size());

    return m_activeInMatrixModel[globalCellIndex] != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::activeIndexInMatrixModel(size_t globalCellIndex) const
{
    if (m_activeInMatrixModel.size() == 0)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    CVF_TIGHT_ASSERT(globalCellIndex < m_activeInMatrixModel.size());

    return m_activeInMatrixModel[globalCellIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setActiveIndexInMatrixModel(size_t globalCellIndex, size_t globalActiveCellIndex)
{
    CVF_TIGHT_ASSERT(globalActiveCellIndex < m_activeInMatrixModel.size());

    m_activeInMatrixModel[globalCellIndex] = globalActiveCellIndex;
}
/*
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigActiveCellInfo::isActiveInFractureModel(size_t globalCellIndex) const
{
    if (m_activeInFractureModel.size() == 0)
    {
        return false;
    }

    CVF_TIGHT_ASSERT(globalCellIndex < m_activeInFractureModel.size());

    return m_activeInFractureModel[globalCellIndex] != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::activeIndexInFractureModel(size_t globalCellIndex) const
{
    if (m_activeInFractureModel.size() == 0)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    CVF_TIGHT_ASSERT(globalCellIndex < m_activeInFractureModel.size());
    
    return m_activeInFractureModel[globalCellIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setActiveIndexInFractureModel(size_t globalCellIndex, size_t globalActiveCellIndex)
{
    CVF_TIGHT_ASSERT(globalActiveCellIndex < m_activeInFractureModel.size());

    m_activeInFractureModel[globalCellIndex] = globalActiveCellIndex;
}
*/
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGridCount(size_t gridCount)
{
    m_perGridActiveCellInfo.resize(gridCount);
}
/*
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGridActiveCellCounts(size_t gridIndex, size_t matrixActiveCellCount, size_t fractureActiveCellCount)
{
    CVF_ASSERT(gridIndex < m_perGridActiveCellInfo.size());

    m_perGridActiveCellInfo[gridIndex].setMatrixModelActiveCellCount(matrixActiveCellCount);
    //m_perGridActiveCellInfo[gridIndex].setFractureModelActiveCellCount(fractureActiveCellCount);
}
*/

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGridActiveCellCounts(size_t gridIndex, size_t matrixActiveCellCount)
{
    CVF_ASSERT(gridIndex < m_perGridActiveCellInfo.size());

    m_perGridActiveCellInfo[gridIndex].setMatrixModelActiveCellCount(matrixActiveCellCount);
    //m_perGridActiveCellInfo[gridIndex].setFractureModelActiveCellCount(fractureActiveCellCount);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::computeDerivedData()
{
    m_globalMatrixModelActiveCellCount = 0;
    //m_globalFractureModelActiveCellCount = 0;

    for (size_t i = 0; i < m_perGridActiveCellInfo.size(); i++)
    {
        m_globalMatrixModelActiveCellCount += m_perGridActiveCellInfo[i].matrixModelActiveCellCount();
       // m_globalFractureModelActiveCellCount += m_perGridActiveCellInfo[i].fractureModelActiveCellCount();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::globalMatrixModelActiveCellCount() const
{
    return m_globalMatrixModelActiveCellCount;
}
/*
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::globalFractureModelActiveCellCount() const
{
    return m_globalFractureModelActiveCellCount;
}
*/
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setMatrixModelActiveCellsBoundingBox(const cvf::Vec3st& min, const cvf::Vec3st& max)
{
    m_activeCellPositionMin = min;
    m_activeCellPositionMax = max;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::matrixModelActiveCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const
{
    min = m_activeCellPositionMin;
    max = m_activeCellPositionMax;
}
/*
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setFractureModelActiveCellsBoundingBox(const cvf::Vec3st& min, const cvf::Vec3st& max)
{
    m_fractureModelActiveCellPositionMin = min;
    m_fractureModelActiveCellPositionMax = max;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::fractureModelActiveCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const
{
    min = m_fractureModelActiveCellPositionMin;
    max = m_fractureModelActiveCellPositionMax;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::gridActiveCellCounts(size_t gridIndex, size_t& matrixActiveCellCount, size_t& fractureActiveCellCount)
{
    matrixActiveCellCount = m_perGridActiveCellInfo[gridIndex].matrixModelActiveCellCount();
    fractureActiveCellCount = m_perGridActiveCellInfo[gridIndex].fractureModelActiveCellCount();
}
*/

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::gridActiveCellCounts(size_t gridIndex, size_t& matrixActiveCellCount)
{
    matrixActiveCellCount = m_perGridActiveCellInfo[gridIndex].matrixModelActiveCellCount();
    //fractureActiveCellCount = m_perGridActiveCellInfo[gridIndex].fractureModelActiveCellCount();
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigActiveCellInfo::matrixActiveCellsGeometryBoundingBox() const
{
    return m_matrixActiveCellsBoundingBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setMatrixActiveCellsGeometryBoundingBox(cvf::BoundingBox bb)
{
    m_matrixActiveCellsBoundingBox = bb;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::GridActiveCellCounts::matrixModelActiveCellCount() const
{
    return m_matrixModelActiveCellCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::GridActiveCellCounts::setMatrixModelActiveCellCount(size_t activeMatrixModelCellCount)
{
    m_matrixModelActiveCellCount = activeMatrixModelCellCount;
}
/*
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::GridActiveCellCounts::fractureModelActiveCellCount() const
{
    return m_fractureModelActiveCellCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::GridActiveCellCounts::setFractureModelActiveCellCount(size_t activeFractureModelCellCount)
{
    m_fractureModelActiveCellCount = activeFractureModelCellCount;
}
*/
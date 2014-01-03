//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################



#include "cvfCellRange.h"


namespace cvf {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CellRange::CellRange()
{
    m_min = cvf::Vec3st::UNDEFINED;
    m_max = cvf::Vec3st::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CellRange::CellRange(cvf::Vec3st min, cvf::Vec3st max)
{
    setRange(min, max);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CellRange::CellRange(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK)
{
    setRange(Vec3st(minI, minJ, minK), Vec3st(maxI, maxJ, maxK));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellRange::setRange(const cvf::Vec3st& min, const cvf::Vec3st& max)
{
    m_min = min;
    m_max = max;

    CVF_ASSERT(m_min.x() != cvf::UNDEFINED_SIZE_T);
    CVF_ASSERT(m_min.y() != cvf::UNDEFINED_SIZE_T);
    CVF_ASSERT(m_min.z() != cvf::UNDEFINED_SIZE_T);
    CVF_ASSERT(m_max.x() != cvf::UNDEFINED_SIZE_T);
    CVF_ASSERT(m_max.y() != cvf::UNDEFINED_SIZE_T);
    CVF_ASSERT(m_max.z() != cvf::UNDEFINED_SIZE_T);

    CVF_ASSERT(normalize());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CellRange::range(cvf::Vec3st& min, cvf::Vec3st& max) const
{
    min = m_min;
    max = m_max;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CellRange::normalize()
{
    if (m_min == cvf::Vec3st::UNDEFINED || m_max == cvf::Vec3st::UNDEFINED)
    {
        return false;
    }

    for (uint i = 0; i < 3; i++)
    {
        if (m_min[i] > m_max[i])
        {
            size_t tmp = m_max[i];
            m_max[i] = m_min[i];
            m_min[i] = tmp;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CellRange::isInRange(size_t i, size_t j, size_t k) const
{
    cvf::Vec3st test(i, j, k);

    for (uint idx = 0; idx < 3; idx++)
    {
        if (test[idx] < m_min[idx] || m_max[idx] <= test[idx])
        {
            return false;
        }
    }

    return true;
}



} // namespace cvf

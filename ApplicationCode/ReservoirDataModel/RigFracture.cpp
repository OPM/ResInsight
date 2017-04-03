/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RigFracture.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFractureData::RigFractureData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFracture::RigFracture()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFracture::setGeometry(const std::vector<cvf::uint>& triangleIndices, const std::vector<cvf::Vec3f>& nodeCoords)
{
    m_nodeCoords = nodeCoords;
    m_triangleIndices = triangleIndices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::uint>& RigFracture::triangleIndices() const
{
    return m_triangleIndices;
}

//--------------------------------------------------------------------------------------------------
/// Returns node coordinates in domain coordinate system
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3f>& RigFracture::nodeCoords() const
{
    return m_nodeCoords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFracture::setFractureData(const std::vector<RigFractureData>& data)
{
    m_fractureData = data;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RigFractureData>& RigFracture::fractureData() const
{
    return m_fractureData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFractureData* RigFracture::fractureData(size_t eclipseCellIndex)
{
    for (RigFractureData fracData : m_fractureData)
    {
        if (fracData.reservoirCellIndex == eclipseCellIndex)
        {
            return &fracData;
        }
    }

    RigFractureData newFractureData;
    return &newFractureData;
}

